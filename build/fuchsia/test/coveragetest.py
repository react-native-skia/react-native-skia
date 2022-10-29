#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Ensure files in the directory are thoroughly tested."""

import importlib
import io
import sys
import unittest

import coverage  # pylint: disable=import-error

COVERED_FILES = [
    'flash_device.py', 'log_manager.py', 'publish_package.py', 'serve_repo.py',
    'test_server.py'
]


def main():
    """Gather coverage data, ensure included files are 100% covered."""

    cov = coverage.coverage(data_file=None,
                            include=COVERED_FILES,
                            config_file=True)
    cov.start()

    for file in COVERED_FILES:
        # pylint: disable=import-outside-toplevel
        # import tests after coverage start to also cover definition lines.
        module = importlib.import_module(file.replace('.py', '_unittests'))
        # pylint: enable=import-outside-toplevel

        tests = unittest.TestLoader().loadTestsFromModule(module)
        if not unittest.TextTestRunner().run(tests).wasSuccessful():
            return 1

    cov.stop()
    outf = io.StringIO()
    percentage = cov.report(file=outf, show_missing=True)
    if int(percentage) != 100:
        print(outf.getvalue())
        print('FATAL: Insufficient coverage (%.f%%)' % int(percentage))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
