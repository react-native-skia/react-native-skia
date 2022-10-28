#!/usr/bin/env python
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs //tools/metrics/metrics_python_tests.py as a 'script' test."""

import json
import os
import sys


import common


def main_run(args):
  with common.temporary_file() as tempfile_path:
    rc = common.run_command(['vpython',
        os.path.join(common.SRC_DIR, 'testing', 'test_env.py'),
        os.path.join(common.SRC_DIR, 'tools', 'metrics',
                     'metrics_python_tests.py'),
        '--isolated-script-test-output', tempfile_path
    ], cwd=os.path.join(common.SRC_DIR, 'out', args.build_config_fs))

    with open(tempfile_path) as f:
      isolated_results = json.load(f)

  results = common.parse_common_test_results(isolated_results,
                                             test_separator='.')

  failures = [
      '%s: %s' % (k, v) for k, v in results['unexpected_failures'].items()
  ]
  common.record_local_script_results(
      'metrics_python_tests', args.output, failures, True)

  return rc


def main_compile_targets(args):
  json.dump([], args.output)


if __name__ == '__main__':
  funcs = {
    'run': main_run,
    'compile_targets': main_compile_targets,
  }
  sys.exit(common.run_script(sys.argv[1:], funcs))
