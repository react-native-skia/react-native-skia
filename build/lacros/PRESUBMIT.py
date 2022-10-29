# Copyright 2020 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Presubmit script for changes affecting //build/lacros"""

USE_PYTHON3 = True


def _CommonChecks(input_api, output_api):
  # Don't run lacros tests on Windows.
  if input_api.is_windows:
    return []
  tests = input_api.canned_checks.GetUnitTestsInDirectory(
      input_api,
      output_api,
      '.', [r'^.+_test\.py$'],
      run_on_python2=False,
      run_on_python3=True,
      skip_shebang_check=True)
  return input_api.RunTests(tests)


def CheckChangeOnUpload(input_api, output_api):
  return _CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return _CommonChecks(input_api, output_api)
