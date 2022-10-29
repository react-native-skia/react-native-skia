# Copyright 2012 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Top-level presubmit script for checkperms.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details on the presubmit API built into depot_tools.
"""


USE_PYTHON3 = True


def CommonChecks(input_api, output_api):
  output = []
  output.extend(
      input_api.canned_checks.RunPylint(input_api, output_api, version='2.7'))
  # Run it like if it were a unit test.
  output.extend(
      input_api.canned_checks.RunUnitTests(input_api,
                                           output_api, ['./checkperms.py'],
                                           run_on_python2=False))
  return output


def CheckChangeOnUpload(input_api, output_api):
  return CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CommonChecks(input_api, output_api)
