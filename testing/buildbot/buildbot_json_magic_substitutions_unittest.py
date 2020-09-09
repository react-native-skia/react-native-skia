#!/usr/bin/python
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import buildbot_json_magic_substitutions as magic_substitutions


def CreateConfigWithPool(pool):
  return {
    'swarming': {
      'dimension_sets': [
        {
          'pool': pool,
        },
      ],
    },
  }


class ChromeOSTelemetryRemoteTest(unittest.TestCase):

  def testVirtualMachineSubstitutions(self):
    test_config = CreateConfigWithPool('chromium.tests.cros.vm')
    self.assertEqual(magic_substitutions.ChromeOSTelemetryRemote(test_config),
                     [
                       '--remote=127.0.0.1',
                       '--remote-ssh-port=9222',
                     ])

  def testPhysicalHardwareSubstitutions(self):
    test_config = CreateConfigWithPool('chrome-cros-dut')
    self.assertEqual(magic_substitutions.ChromeOSTelemetryRemote(test_config),
                     ['--remote=variable_chromeos_device_hostname'])

  def testNoPool(self):
    test_config = CreateConfigWithPool(None)
    with self.assertRaisesRegexp(RuntimeError, 'No pool *'):
      magic_substitutions.ChromeOSTelemetryRemote(test_config)

  def testUnknownPool(self):
    test_config = CreateConfigWithPool('totally-legit-pool')
    with self.assertRaisesRegexp(RuntimeError, 'Unknown CrOS pool *'):
      magic_substitutions.ChromeOSTelemetryRemote(test_config)


if __name__ == '__main__':
  unittest.main()
