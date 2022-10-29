#!/usr/bin/env vpython3
# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""File for testing serve_repo.py."""

import argparse
import unittest
import unittest.mock as mock

import serve_repo

from common import REPO_ALIAS

_REPO_DIR = 'test_repo_dir'
_REPO_NAME = 'test_repo_name'
_TARGET = 'test_target'


class ServeRepoTest(unittest.TestCase):
    """Unittests for serve_repo.py."""

    def setUp(self) -> None:
        self._namespace = argparse.Namespace(repo=_REPO_DIR,
                                             repo_name=_REPO_NAME,
                                             target_id=_TARGET)

    @mock.patch('serve_repo.run_ffx_command')
    @mock.patch('serve_repo.get_config')
    def test_ffx_config(self, mock_get_config, mock_ffx) -> None:
        """Test |_ensure_ffx_config| returns True if config is changed,
        False otherwise."""

        mock_get_config.side_effect = ['true', '\"ffx\"']

        serve_repo.run_serve_cmd('start', self._namespace)
        self.assertEqual(mock_ffx.call_count, 3)

        mock_ffx.reset_mock()
        mock_get_config.reset_mock()
        mock_get_config.side_effect = ['false', '\"not_ffx\"']

        serve_repo.run_serve_cmd('start', self._namespace)
        self.assertEqual(mock_ffx.call_count, 6)

    @mock.patch('serve_repo.run_ffx_command')
    @mock.patch('serve_repo._ensure_ffx_config', return_value=True)
    def test_run_serve_cmd_start(self, mock_config, mock_ffx) -> None:
        """Test |run_serve_cmd| function for start."""

        serve_repo.run_serve_cmd('start', self._namespace)
        self.assertEqual(mock_config.call_count, 2)
        self.assertEqual(mock_ffx.call_count, 4)
        first_call = mock_ffx.call_args_list[0]
        self.assertEqual(['doctor', '--restart-daemon'], first_call[0][0])
        second_call = mock_ffx.call_args_list[1]
        self.assertEqual(['repository', 'server', 'start'], second_call[0][0])
        third_call = mock_ffx.call_args_list[2]
        self.assertEqual(
            ['repository', 'add-from-pm', _REPO_DIR, '-r', _REPO_NAME],
            third_call[0][0])
        fourth_call = mock_ffx.call_args_list[3]
        self.assertEqual([
            'target', 'repository', 'register', '-r', _REPO_NAME, '--alias',
            REPO_ALIAS
        ], fourth_call[0][0])
        self.assertEqual(_TARGET, fourth_call[0][1])

    @mock.patch('serve_repo.run_ffx_command')
    def test_run_serve_cmd_stop(self, mock_ffx) -> None:
        """Test |run_serve_cmd| function for stop."""

        serve_repo.run_serve_cmd('stop', self._namespace)
        self.assertEqual(mock_ffx.call_count, 3)
        first_call = mock_ffx.call_args_list[0]
        self.assertEqual(
            ['target', 'repository', 'deregister', '-r', _REPO_NAME],
            first_call[0][0])
        self.assertEqual(_TARGET, first_call[0][1])
        second_call = mock_ffx.call_args_list[1]
        self.assertEqual(['repository', 'remove', _REPO_NAME],
                         second_call[0][0])
        third_call = mock_ffx.call_args_list[2]
        self.assertEqual(['repository', 'server', 'stop'], third_call[0][0])

    @mock.patch('serve_repo.run_serve_cmd')
    def test_serve_repository(self, mock_serve) -> None:
        """Tests |serve_repository| context manager."""

        with serve_repo.serve_repository(self._namespace):
            self.assertEqual(mock_serve.call_count, 1)
        self.assertEqual(mock_serve.call_count, 2)

    def test_main_start_no_serve_repo_flag(self) -> None:
        """Tests not specifying directory for start raises a ValueError."""

        with mock.patch('sys.argv', ['serve_repo.py', 'start']):
            with self.assertRaises(ValueError):
                serve_repo.main()

    @mock.patch('serve_repo.run_serve_cmd')
    def test_main_stop(self, mock_serve) -> None:
        """Tests |main| function."""

        with mock.patch('sys.argv', ['serve_repo.py', 'stop']):
            serve_repo.main()
            self.assertEqual(mock_serve.call_count, 1)


if __name__ == '__main__':
    unittest.main()
