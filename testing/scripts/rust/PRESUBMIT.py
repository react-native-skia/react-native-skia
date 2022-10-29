# Copyright 2018 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Runs Python unit tests in this directory.
"""

PRESUBMIT_VERSION = '2.0.0'
USE_PYTHON3 = True

def CheckPythonUnittestsPass(input_api, output_api):
    # The rust scripts and tests don't work on Windows.
    if input_api.is_windows:
        return []
    results = []
    this_dir = input_api.PresubmitLocalPath()

    results += input_api.RunTests(
        input_api.canned_checks.GetUnitTestsInDirectory(
            input_api,
            output_api,
            this_dir,
            files_to_check=['.*unittest.*\.py$'],
            env=None,
            run_on_python2=False,
            run_on_python3=True,
            skip_shebang_check=True))

    return results
