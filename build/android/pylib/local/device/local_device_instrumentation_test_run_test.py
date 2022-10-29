#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for local_device_instrumentation_test_run."""

# pylint: disable=protected-access


import unittest
import mock  # pylint: disable=import-error

from pylib.base import base_test_result
from pylib.base import mock_environment
from pylib.base import mock_test_instance
from pylib.local.device import local_device_instrumentation_test_run


class LocalDeviceInstrumentationTestRunTest(unittest.TestCase):

  def setUp(self):
    super().setUp()
    self._env = mock_environment.MockEnvironment()
    self._ti = mock_test_instance.MockTestInstance()
    self._obj = (
        local_device_instrumentation_test_run.LocalDeviceInstrumentationTestRun(
            self._env, self._ti))

  # TODO(crbug.com/797002): Decide whether the _ShouldRetry hook is worth
  # retaining and remove these tests if not.

  def testShouldRetry_failure(self):
    test = {
        'annotations': {},
        'class': 'SadTest',
        'method': 'testFailure',
        'is_junit4': True,
    }
    result = base_test_result.BaseTestResult(
        'SadTest.testFailure', base_test_result.ResultType.FAIL)
    self.assertTrue(self._obj._ShouldRetry(test, result))

  def testShouldRetry_retryOnFailure(self):
    test = {
        'annotations': {'RetryOnFailure': None},
        'class': 'SadTest',
        'method': 'testRetryOnFailure',
        'is_junit4': True,
    }
    result = base_test_result.BaseTestResult(
        'SadTest.testRetryOnFailure', base_test_result.ResultType.FAIL)
    self.assertTrue(self._obj._ShouldRetry(test, result))

  def testShouldRetry_notRun(self):
    test = {
        'annotations': {},
        'class': 'SadTest',
        'method': 'testNotRun',
        'is_junit4': True,
    }
    result = base_test_result.BaseTestResult(
        'SadTest.testNotRun', base_test_result.ResultType.NOTRUN)
    self.assertTrue(self._obj._ShouldRetry(test, result))

  def testIsWPRRecordReplayTest_matchedWithKey(self):
    test = {
        'annotations': {
            'Feature': {
                'value': ['WPRRecordReplayTest', 'dummy']
            }
        },
        'class': 'WPRDummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertTrue(
        local_device_instrumentation_test_run._IsWPRRecordReplayTest(test))

  def testIsWPRRecordReplayTest_noMatchedKey(self):
    test = {
        'annotations': {
            'Feature': {
                'value': ['abc', 'dummy']
            }
        },
        'class': 'WPRDummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertFalse(
        local_device_instrumentation_test_run._IsWPRRecordReplayTest(test))

  def testGetWPRArchivePath_matchedWithKey(self):
    test = {
        'annotations': {
            'WPRArchiveDirectory': {
                'value': 'abc'
            }
        },
        'class': 'WPRDummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertEqual(
        local_device_instrumentation_test_run._GetWPRArchivePath(test), 'abc')

  def testGetWPRArchivePath_noMatchedWithKey(self):
    test = {
        'annotations': {
            'Feature': {
                'value': 'abc'
            }
        },
        'class': 'WPRDummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertFalse(
        local_device_instrumentation_test_run._GetWPRArchivePath(test))

  def testIsRenderTest_matchedWithKey(self):
    test = {
        'annotations': {
            'Feature': {
                'value': ['RenderTest', 'dummy']
            }
        },
        'class': 'DummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertTrue(local_device_instrumentation_test_run._IsRenderTest(test))

  def testIsRenderTest_noMatchedKey(self):
    test = {
        'annotations': {
            'Feature': {
                'value': ['abc', 'dummy']
            }
        },
        'class': 'DummyTest',
        'method': 'testRun',
        'is_junit4': True,
    }
    self.assertFalse(local_device_instrumentation_test_run._IsRenderTest(test))

  def testReplaceUncommonChars(self):
    original = 'abc#edf'
    self.assertEqual(
        local_device_instrumentation_test_run._ReplaceUncommonChars(original),
        'abc__edf')
    original = 'abc#edf#hhf'
    self.assertEqual(
        local_device_instrumentation_test_run._ReplaceUncommonChars(original),
        'abc__edf__hhf')
    original = 'abcedfhhf'
    self.assertEqual(
        local_device_instrumentation_test_run._ReplaceUncommonChars(original),
        'abcedfhhf')
    original = None
    with self.assertRaises(ValueError):
      local_device_instrumentation_test_run._ReplaceUncommonChars(original)
    original = ''
    with self.assertRaises(ValueError):
      local_device_instrumentation_test_run._ReplaceUncommonChars(original)

  def testStoreDataInAppDir(self):
    env = mock.MagicMock()
    test_instance = mock.MagicMock()
    test_instance.store_data_in_app_directory = True
    device = mock.MagicMock()

    device.GetApplicationDataDirectory.return_value = 'app_dir'
    device.GetExternalStoragePath.return_value = 'external_dir'
    test_run = (
        local_device_instrumentation_test_run.LocalDeviceInstrumentationTestRun(
            env, test_instance))
    self.assertEqual(test_run._GetDataStorageRootDirectory(device), 'app_dir')

  def testStoreDataInExternalDir(self):
    env = mock.MagicMock()
    test_instance = mock.MagicMock()
    test_instance.store_data_in_app_directory = False
    device = mock.MagicMock()

    device.GetApplicationDataDirectory.return_value = 'app_dir'
    device.GetExternalStoragePath.return_value = 'external_dir'
    test_run = (
        local_device_instrumentation_test_run.LocalDeviceInstrumentationTestRun(
            env, test_instance))
    self.assertEqual(test_run._GetDataStorageRootDirectory(device),
                     'external_dir')


if __name__ == '__main__':
  unittest.main(verbosity=2)
