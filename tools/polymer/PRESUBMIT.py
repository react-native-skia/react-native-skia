# Copyright 2019 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for files in tools/polymer/

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

USE_PYTHON3 = True


def RunPolymerTests(input_api, output_api):
  presubmit_path = input_api.PresubmitLocalPath()
  sources = [
      'polymer_test.py', 'html_to_wrapper_test.py', 'css_to_wrapper_test.py'
  ]
  tests = [input_api.os_path.join(presubmit_path, s) for s in sources]
  return input_api.canned_checks.RunUnitTests(input_api,
                                              output_api,
                                              tests,
                                              run_on_python2=False)


def _CheckChangeOnUploadOrCommit(input_api, output_api):
  results = []
  affected = input_api.AffectedFiles()

  webui_sources = set(['polymer.py', 'html_to_wrapper.py', 'css_to_wrapper.py'])
  affected_files = [input_api.os_path.basename(f.LocalPath()) for f in affected]
  if webui_sources.intersection(set(affected_files)):
    results += RunPolymerTests(input_api, output_api)

  return results


def CheckChangeOnUpload(input_api, output_api):
  return _CheckChangeOnUploadOrCommit(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return _CheckChangeOnUploadOrCommit(input_api, output_api)
