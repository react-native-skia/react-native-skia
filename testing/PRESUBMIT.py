# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Top-level presubmit script for testing.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details on the presubmit API built into depot_tools.
"""

USE_PYTHON3 = True


def CommonChecks(input_api, output_api):
  testing_env = dict(input_api.environ)
  testing_env.update({
      'PYTHONPATH': input_api.PresubmitLocalPath(),
      'PYTHONDONTWRITEBYTECODE': '1',
  })

  output = []
  output.extend(input_api.canned_checks.RunUnitTestsInDirectory(
      input_api, output_api, '.', [r'^.+_unittest\.py$']))
  output.extend(input_api.canned_checks.RunUnitTestsInDirectory(
      input_api,
      output_api,
      input_api.os_path.join(input_api.PresubmitLocalPath(),
                             'unexpected_passes_common'),
      [r'^.+_unittest\.py$'],
      env=testing_env))
  output.extend(input_api.canned_checks.RunPylint(
      input_api, output_api, files_to_skip=[r'gmock.*', r'gtest.*']))
  return output


def CheckChangeOnUpload(input_api, output_api):
  return CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CommonChecks(input_api, output_api)
