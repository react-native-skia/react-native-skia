# Copyright 2012 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for checkdeps tool.
"""

USE_PYTHON3 = True


def CheckChange(input_api, output_api):
  return input_api.canned_checks.RunUnitTests(
      input_api, output_api,
      [input_api.os_path.join(input_api.PresubmitLocalPath(),
                              'checkdeps_test.py')],
                              # `run_on_python3` defaults to `True`.
                              run_on_python2=not USE_PYTHON3)


# Mandatory entrypoint.
def CheckChangeOnUpload(input_api, output_api):
  return CheckChange(input_api, output_api)


# Mandatory entrypoint.
def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api)
