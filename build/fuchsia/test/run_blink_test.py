# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Implements commands for running blink web tests."""

import os
import subprocess

from argparse import Namespace
from typing import Optional

from common import DIR_SRC_ROOT, resolve_packages
from test_runner import TestRunner

_BLINK_TEST_SCRIPT = os.path.join(DIR_SRC_ROOT, 'third_party', 'blink',
                                  'tools', 'run_web_tests.py')


class BlinkTestRunner(TestRunner):
    """Test runner for running blink web tests."""

    def __init__(self, out_dir: str, test_args: Namespace,
                 target_id: Optional[str]) -> None:
        super().__init__(out_dir, test_args, ['content_shell'], target_id)

    # TODO(crbug.com/1278939): Remove when blink tests use CFv2 content_shell.
    @staticmethod
    def is_cfv2() -> bool:
        return False

    def run_test(self):
        resolve_packages(self.packages, self._target_id)
        test_cmd = [_BLINK_TEST_SCRIPT]
        test_cmd.append('--platform=fuchsia')
        if self._test_args:
            test_cmd.extend(self._test_args)
        return subprocess.run(test_cmd, check=True)
