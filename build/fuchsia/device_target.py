# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Implements commands for running and interacting with Fuchsia on devices."""

import boot_data
import logging
import os
import pkg_repo
import re
import subprocess
import target
import time

from common import EnsurePathExists, GetHostToolPathFromPlatform, \
                   RunGnSdkFunction, SubprocessCallWithTimeout

# The maximum times to attempt mDNS resolution when connecting to a freshly
# booted Fuchsia instance before aborting.
BOOT_DISCOVERY_ATTEMPTS = 30

# Number of failed connection attempts before redirecting system logs to stdout.
CONNECT_RETRY_COUNT_BEFORE_LOGGING = 10

# Number of seconds between each device discovery.
BOOT_DISCOVERY_DELAY_SECS = 4

# Time between a reboot command is issued and when connection attempts from the
# host begin.
_REBOOT_SLEEP_PERIOD = 20

# File indicating version of an image downloaded to the host
_BUILD_ARGS = "buildargs.gn"

# File on device that indicates Fuchsia version.
_ON_DEVICE_VERSION_FILE = '/config/build-info/version'

# File on device that indicates Fuchsia product.
_ON_DEVICE_PRODUCT_FILE = '/config/build-info/product'


def GetTargetType():
  return DeviceTarget


class DeviceTarget(target.Target):
  """Prepares a device to be used as a deployment target. Depending on the
  command line parameters, it automatically handling a number of preparatory
  steps relating to address resolution.

  If |_node_name| is unset:
    If there is one running device, use it for deployment and execution.

    If there are more than one running devices, then abort and instruct the
    user to re-run the command with |_node_name|

  If |_node_name| is set:
    If there is a running device with a matching nodename, then it is used
    for deployment and execution.

  If |_host| is set:
    Deploy to a device at the host IP address as-is."""

  def __init__(self, out_dir, target_cpu, host, node_name, port, ssh_config,
               fuchsia_out_dir, os_check, logs_dir, system_image_dir):
    """out_dir: The directory which will contain the files that are
                   generated to support the deployment.
    target_cpu: The CPU architecture of the deployment target. Can be
                "x64" or "arm64".
    host: The address of the deployment target device.
    node_name: The node name of the deployment target device.
    port: The port of the SSH service on the deployment target device.
    ssh_config: The path to SSH configuration data.
    fuchsia_out_dir: The path to a Fuchsia build output directory, for
                     deployments to devices paved with local Fuchsia builds.
    os_check: If 'check', the target's SDK version must match.
              If 'update', the target will be repaved if the SDK versions
                  mismatch.
              If 'ignore', the target's SDK version is ignored.
    system_image_dir: The directory which contains the files used to pave the
                      device."""

    super(DeviceTarget, self).__init__(out_dir, target_cpu, logs_dir)

    self._host = host
    self._port = port
    self._fuchsia_out_dir = None
    self._node_name = node_name or os.environ.get('FUCHSIA_NODENAME')
    self._system_image_dir = system_image_dir
    self._os_check = os_check
    self._pkg_repo = None
    if not self._system_image_dir and self._os_check != 'ignore':
      raise Exception("Image directory must be provided if a repave is needed.")

    if self._host and self._node_name:
      raise Exception('Only one of "--host" or "--name" can be specified.')

    if fuchsia_out_dir:
      if ssh_config:
        raise Exception('Only one of "--fuchsia-out-dir" or "--ssh_config" can '
                        'be specified.')

      self._fuchsia_out_dir = os.path.expanduser(fuchsia_out_dir)
      # Use SSH keys from the Fuchsia output directory.
      self._ssh_config_path = os.path.join(self._fuchsia_out_dir, 'ssh-keys',
                                           'ssh_config')
      self._os_check = 'ignore'

    elif ssh_config:
      # Use the SSH config provided via the commandline.
      self._ssh_config_path = os.path.expanduser(ssh_config)

    else:
      return_code, ssh_config_raw, _ = RunGnSdkFunction(
          'fuchsia-common.sh', 'get-fuchsia-sshconfig-file')
      if return_code != 0:
        raise Exception('Could not get Fuchsia ssh config file.')
      self._ssh_config_path = os.path.expanduser(ssh_config_raw.strip())

  @staticmethod
  def CreateFromArgs(args):
    return DeviceTarget(args.out_dir, args.target_cpu, args.host,
                        args.node_name, args.port, args.ssh_config,
                        args.fuchsia_out_dir, args.os_check, args.logs_dir,
                        args.system_image_dir)

  @staticmethod
  def RegisterArgs(arg_parser):
    device_args = arg_parser.add_argument_group(
        'device', 'External device deployment arguments')
    device_args.add_argument('--host',
                             help='The IP of the target device. Optional.')
    device_args.add_argument('--node-name',
                             help='The node-name of the device to boot or '
                             'deploy to. Optional, will use the first '
                             'discovered device if omitted.')
    device_args.add_argument('--port',
                             '-p',
                             type=int,
                             default=None,
                             help='The port of the SSH service running on the '
                             'device. Optional.')
    device_args.add_argument('--ssh-config',
                             '-F',
                             help='The path to the SSH configuration used for '
                             'connecting to the target device.')
    device_args.add_argument(
        '--os-check',
        choices=['check', 'update', 'ignore'],
        default='ignore',
        help="Sets the OS version enforcement policy. If 'check', then the "
        "deployment process will halt if the target\'s version doesn\'t "
        "match. If 'update', then the target device will automatically "
        "be repaved. If 'ignore', then the OS version won\'t be checked.")
    device_args.add_argument('--system-image-dir',
                             help="Specify the directory that contains the "
                             "Fuchsia image used to pave the device. Only "
                             "needs to be specified if 'os_check' is not "
                             "'ignore'.")

  def _Discover(self):
    """Queries mDNS for the IP address of a booted Fuchsia instance whose name
    matches |_node_name| on the local area network. If |_node_name| isn't
    specified, and there is only one device on the network, then returns the
    IP address of that advice.

    Sets |_host_name| and returns True if the device was found,
    or waits up to |timeout| seconds and returns False if the device couldn't
    be found."""

    dev_finder_path = GetHostToolPathFromPlatform('device-finder')

    with open(os.devnull, 'w') as devnull:
      if self._node_name:
        command = [
            dev_finder_path,
            'resolve',
            '-device-limit',
            '1',  # Exit early as soon as a host is found.
            self._node_name
        ]
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=devnull)
      else:
        proc = self.RunFFXCommand(['target', 'list', '-f', 'simple'],
                                  stdout=subprocess.PIPE,
                                  stderr=devnull)

    # TODO(crbug.com/1198733): Switch to encoding='utf-8' once we drop Python 2
    # support.
    output = set(proc.communicate()[0].decode('utf-8').strip().split('\n'))
    if proc.returncode != 0:
      return False

    if self._node_name:
      # Handle the result of "device-finder resolve".
      self._host = output.pop().strip()
    else:
      name_host_pairs = [x.strip().split(' ') for x in output]

      if len(name_host_pairs) > 1:
        raise Exception('More than one device was discovered on the network. '
                        'Use --node-name <name> to specify the device to use.'
                        'List of devices: {}'.format(output))
      assert len(name_host_pairs) == 1
      # Check if device has both address and name.
      if len(name_host_pairs[0]) < 2:
        return False
      self._host, self._node_name = name_host_pairs[0]

    logging.info('Found device "%s" at address %s.' % (self._node_name,
                                                       self._host))

    return True

  def Start(self):
    if self._host:
      self._WaitUntilReady()
    else:
      device_found = self._Discover()

      if device_found:
        self._WaitUntilReady()
        if self._os_check == 'ignore':
          return

        # If accessible, check version.
        new_version = self._GetSdkHash()
        installed_version = self._GetInstalledSdkVersion()
        if new_version == installed_version:
          logging.info('Fuchsia version installed on device matches Chromium '
                       'SDK version. Skipping pave.')
        else:
          if self._os_check == 'check':
            raise Exception('Image and Fuchsia version installed on device '
                            'does not match. Abort.')
          logging.info('Putting device in recovery mode')
          self.RunCommandPiped(['dm', 'reboot-recovery'],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT)
          self._ProvisionDevice()
      else:
        if self._node_name:
          logging.info('Could not detect device %s.' % self._node_name)
          if self._os_check == 'update':
            logging.info('Assuming it is in zedboot. Continuing with paving...')
            self._ProvisionDevice()
            return
        raise Exception('Could not find device. If the device is connected '
                        'to the host remotely, make sure that --host flag '
                        'is set and that remote serving is set up.')

  def _GetInstalledSdkVersion(self):
    """Retrieves installed OS version from device.

    Returns:
      Tuple of strings, containing (product, version number)
    """
    return (self.GetFileAsString(_ON_DEVICE_PRODUCT_FILE).strip(),
            self.GetFileAsString(_ON_DEVICE_VERSION_FILE).strip())

  def _GetSdkHash():
    """Read version of hash in pre-installed package directory.
    Returns:
      Tuple of (product, version) of image to be installed.
    Raises:
      VersionNotFoundError: if contents of buildargs.gn cannot be found or the
      version number cannot be extracted.
    """

    # TODO(crbug.com/1261961): Stop processing buildargs.gn directly.
    with open(os.path.join(self._system_image_dir, _BUILD_ARGS)) as f:
      contents = f.readlines()
    if not contents:
      raise VersionNotFoundError('Could not retrieve %s' % _BUILD_ARGS)
    version_key = 'build_info_version'
    product_key = 'build_info_product'
    info_keys = [product_key, version_key]
    version_info = {}
    for line in contents:
      for k in info_keys:
        match = re.match(r'%s = "(.*)"' % k, line)
        if match:
          version_info[k] = match.group(1)
    if not (version_key in version_info and product_key in version_info):
      raise VersionNotFoundError(
          'Could not extract version info from %s. Contents: %s' %
          (_BUILD_ARGS, contents))

    return (version_info[product_key], version_info[version_key])

  def GetPkgRepo(self):
    if not self._pkg_repo:
      if self._fuchsia_out_dir:
        # Deploy to an already-booted device running a local Fuchsia build.
        self._pkg_repo = pkg_repo.ExternalPkgRepo(
            os.path.join(self._fuchsia_out_dir, 'amber-files'),
            os.path.join(self._fuchsia_out_dir, '.build-id'))
      else:
        # Create an ephemeral package repository, then start both "pm serve" as
        # well as the bootserver.
        self._pkg_repo = pkg_repo.ManagedPkgRepo(self)

    return self._pkg_repo

  def _ParseNodename(self, output):
    # Parse the nodename from bootserver stdout.
    m = re.search(r'.*Proceeding with nodename (?P<nodename>.*)$', output,
                  re.MULTILINE)
    if not m:
      raise Exception('Couldn\'t parse nodename from bootserver output.')
    self._node_name = m.groupdict()['nodename']
    logging.info('Booted device "%s".' % self._node_name)

    # Repeatedly search for a device for |BOOT_DISCOVERY_ATTEMPT|
    # number of attempts. If a device isn't found, wait
    # |BOOT_DISCOVERY_DELAY_SECS| before searching again.
    logging.info('Waiting for device to join network.')
    for _ in xrange(BOOT_DISCOVERY_ATTEMPTS):
      if self._Discover():
        break
      time.sleep(BOOT_DISCOVERY_DELAY_SECS)

    if not self._host:
      raise Exception('Device %s couldn\'t be discovered via mDNS.' %
                      self._node_name)

    self._WaitUntilReady();

  def _GetEndpoint(self):
    return (self._host, self._port)

  def _GetSshConfigPath(self):
    return self._ssh_config_path

  def _ProvisionDevice(self):
    _, auth_keys, _ = RunGnSdkFunction('fuchsia-common.sh',
                                       'get-fuchsia-auth-keys-file')
    pave_command = [
        os.path.join(self._system_image_dir, 'pave.sh'), '--authorized-keys',
        auth_keys.strip()
    ]
    if self._node_name:
      pave_command.extend(['-n', self._node_name, '-1'])
    logging.warning(' '.join(pave_command))
    return_code, stdout, stderr = SubprocessCallWithTimeout(pave_command,
                                                            timeout_secs=300)
    if return_code != 0:
      raise Exception('Could not pave device.')
    self._ParseNodename(stderr)

  def Restart(self):
    """Restart the device."""

    self.RunCommandPiped('dm reboot')
    time.sleep(_REBOOT_SLEEP_PERIOD)
    self.Start()

  def Stop(self):
    try:
      super(DeviceTarget, self).Stop()
    finally:
      # End multiplexed ssh connection, ensure that ssh logging stops before
      # tests/scripts return.
      if self.IsStarted():
        self.RunCommand(['-O', 'exit'])
