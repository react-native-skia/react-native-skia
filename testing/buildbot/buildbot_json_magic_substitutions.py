# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A set of functions to programmatically substitute test arguments.

Arguments for a test that start with $$MAGIC_SUBSTITUTION_ will be replaced with
the output of the corresponding function in this file. For example,
$$MAGIC_SUBSTITUTION_Foo would be replaced with the return value of the Foo()
function.

This is meant as an alternative to many entries in test_suite_exceptions.pyl if
the differentiation can be done programmatically.
"""

MAGIC_SUBSTITUTION_PREFIX = '$$MAGIC_SUBSTITUTION_'


def ChromeOSTelemetryRemote(test_config):
  """Substitutes the correct CrOS remote Telemetry arguments.

  VMs use a hard-coded remote address and port, while physical hardware use
  a magic hostname.

  Args:
    test_config: A dict containing a configuration for a specific test on a
        specific builder.
  """
  def StringContainsSubstring(s, sub_strs):
    for sub_str in sub_strs:
      if sub_str in s:
        return True
    return False
  VM_POOLS = [
    'chromium.tests.cros.vm',
    'chrome.tests.cros-vm',
  ]
  HW_POOLS = [
    'chrome-cros-dut',
    'chrome.cros-dut',
  ]
  dimensions = test_config.get('swarming', {}).get('dimension_sets', [])
  assert len(dimensions)
  pool = dimensions[0].get('pool')
  if not pool:
    raise RuntimeError(
        'No pool set for CrOS test, unable to determine whether running on '
        'a VM or physical hardware.')

  if StringContainsSubstring(pool, VM_POOLS):
    return [
      '--remote=127.0.0.1',
      # By default, CrOS VMs' ssh servers listen on local port 9222.
      '--remote-ssh-port=9222',
    ]
  if StringContainsSubstring(pool, HW_POOLS):
    return [
      # Magic hostname that resolves to a CrOS device in the test lab.
      '--remote=variable_chromeos_device_hostname',
    ]
  raise RuntimeError('Unknown CrOS pool %s' % pool)


def TestOnlySubstitution(_):
  """Magic substitution used for unittests."""
  return ['--magic-substitution-success']
