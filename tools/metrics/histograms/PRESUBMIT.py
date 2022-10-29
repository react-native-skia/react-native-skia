# Copyright 2013 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details on the presubmit API built into depot_tools.
"""

USE_PYTHON3 = True
PRESUBMIT_VERSION = '2.0.0'


def GetPrettyPrintErrors(input_api, output_api, cwd, rel_path, results):
  """Runs pretty-print command for specified file."""
  args = [
      input_api.python3_executable, 'pretty_print.py', rel_path, '--presubmit',
      '--non-interactive'
  ]
  exit_code = input_api.subprocess.call(args, cwd=cwd)

  if exit_code != 0:
    error_msg = (
        '%s is not formatted correctly; run git cl format to fix.' % rel_path)
    results.append(output_api.PresubmitError(error_msg))


def GetPrefixErrors(input_api, output_api, cwd, rel_path, results):
  """Validates histogram prefixes in specified file."""
  exit_code = input_api.subprocess.call(
      [input_api.python3_executable, 'validate_prefix.py', rel_path], cwd=cwd)

  if exit_code != 0:
    error_msg = ('%s contains histogram(s) with disallowed prefix, please run '
                 'validate_prefix.py %s to fix.' % (rel_path, rel_path))
    results.append(output_api.PresubmitError(error_msg))


def GetObsoleteXmlErrors(input_api, output_api, cwd, results):
  """Validates all histograms in the file are obsolete."""
  exit_code = input_api.subprocess.call(
      [input_api.python3_executable, 'validate_obsolete_histograms.py'],
      cwd=cwd)

  if exit_code != 0:
    error_msg = (
        'metadata/obsolete_histograms.xml contains non-obsolete '
        'histograms, please run validate_obsolete_histograms.py to fix.')
    results.append(output_api.PresubmitError(error_msg))


def GetValidateHistogramsError(input_api, output_api, cwd, results):
  """Validates histograms format and index file."""
  exit_code = input_api.subprocess.call(
      [input_api.python3_executable, 'validate_format.py'], cwd=cwd)

  if exit_code != 0:
    error_msg = (
        'Histograms are not well-formatted; please run %s/validate_format.py '
        'and fix the reported errors.' % cwd)
    results.append(output_api.PresubmitError(error_msg))

  exit_code = input_api.subprocess.call(
      [input_api.python3_executable, 'validate_histograms_index.py'], cwd=cwd)

  if exit_code != 0:
    error_msg = (
        'Histograms index file is not up-to-date. Please run '
        '%s/histogram_paths.py to update it' % cwd)
    results.append(output_api.PresubmitError(error_msg))


def ValidateSingleFile(input_api, output_api, file_obj, cwd, results):
  """Does corresponding validations if histograms.xml or enums.xml is changed.

  Args:
    input_api: An input_api instance that contains information about changes.
    output_api: An output_api instance to create results of the PRESUBMIT check.
    file_obj: A file object of one of the changed files.
    cwd: Path to current working directory.
    results: The returned variable which is a list of output_api results.

  Returns:
    A boolean that True if a histograms.xml or enums.xml file is changed.
  """
  p = file_obj.AbsoluteLocalPath()
  # Only do PRESUBMIT checks when |p| is under |cwd|.
  if input_api.os_path.commonprefix([p, cwd]) != cwd:
    return False
  filepath = input_api.os_path.relpath(p, cwd)

  if 'test_data' in filepath:
    return False

  # If the changed file is obsolete_histograms.xml, validate all histograms
  # inside are obsolete.
  if 'obsolete_histograms.xml' in filepath:
    GetObsoleteXmlErrors(input_api, output_api, cwd, results)
    # Return false here because we don't need to validate format if users only
    # change obsolete_histograms.xml.
    return False

  # If the changed file is histograms.xml or histogram_suffixes_list.xml,
  # pretty-print and validate prefix it.
  elif ('histograms.xml' in filepath
        or 'histogram_suffixes_list.xml' in filepath):
    GetPrettyPrintErrors(input_api, output_api, cwd, filepath, results)
    # TODO(crbug/1120229): Re-enable validate prefix check once all histograms
    # are split.
    # GetPrefixErrors(input_api, output_api, cwd, filepath, results)
    return True

  # If the changed file is enums.xml, pretty-print it.
  elif 'enums.xml' in filepath:
    GetPrettyPrintErrors(input_api, output_api, cwd, filepath, results)
    return True

  return False


def CheckHistogramFormatting(input_api, output_api):
  """Checks that histograms.xml is pretty-printed and well-formatted."""
  results = []
  cwd = input_api.PresubmitLocalPath()
  xml_changed = False

  # Only for changed files, do corresponding checks if the file is
  # histograms.xml, enums.xml or obsolete_histograms.xml.
  for file_obj in input_api.AffectedTextFiles():
    is_changed = ValidateSingleFile(
        input_api, output_api, file_obj, cwd, results)
    xml_changed = xml_changed or is_changed

  # Run validate_format.py and validate_histograms_index.py, if changed files
  # contain histograms.xml or enums.xml.
  if xml_changed:
    GetValidateHistogramsError(input_api, output_api, cwd, results)

  return results
