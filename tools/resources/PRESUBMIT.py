# Copyright 2019 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

USE_PYTHON3 = True


def CheckChangeOnUpload(*args):
  return _CommonChecks(*args)


def CheckChangeOnCommit(*args):
  return _CommonChecks(*args)


def _CommonChecks(input_api, output_api):
  cwd = input_api.PresubmitLocalPath()
  path = input_api.os_path
  files = [path.basename(f.LocalPath()) for f in input_api.AffectedFiles()]

  if any(f for f in files if f.startswith('svgo_presubmit')):
    tests = [path.join(cwd, 'svgo_presubmit_test.py')]
    return input_api.canned_checks.RunUnitTests(input_api,
                                                output_api,
                                                tests,
                                                run_on_python2=False)

  return []
