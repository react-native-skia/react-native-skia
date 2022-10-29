# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Common methods and variables used by Cr-Fuchsia testing infrastructure."""

import json
import os
import platform
import re
import subprocess

from argparse import ArgumentParser
from typing import Iterable, List, Optional

from compatible_utils import get_ssh_prefix

DIR_SRC_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir))
REPO_ALIAS = 'fuchsia.com'
SDK_ROOT = os.path.join(DIR_SRC_ROOT, 'third_party', 'fuchsia-sdk', 'sdk')


def get_host_arch() -> str:
    """Retrieve CPU architecture of the host machine. """
    host_arch = platform.machine()
    # platform.machine() returns AMD64 on 64-bit Windows.
    if host_arch in ['x86_64', 'AMD64']:
        return 'x64'
    if host_arch == 'aarch64':
        return 'arm64'
    raise Exception('Unsupported host architecture: %s' % host_arch)


SDK_TOOLS_DIR = os.path.join(SDK_ROOT, 'tools', get_host_arch())
_FFX_TOOL = os.path.join(SDK_TOOLS_DIR, 'ffx')

# This global variable is used to set the environment variable
# |FFX_ISOLATE_DIR| when running ffx commands in E2E testing scripts.
_FFX_ISOLATE_DIR = None


def set_ffx_isolate_dir(isolate_dir: str) -> None:
    """Overwrites |_FFX_ISOLATE_DIR|."""

    global _FFX_ISOLATE_DIR  # pylint: disable=global-statement
    _FFX_ISOLATE_DIR = isolate_dir


def _run_repair_command(output):
    """Scans |output| for a self-repair command to run and, if found, runs it.

    Returns:
      True if a repair command was found and ran successfully. False otherwise.
    """
    # Check for a string along the lines of:
    # "Run `ffx doctor --restart-daemon` for further diagnostics."
    match = re.search('`ffx ([^`]+)`', output)
    if not match or len(match.groups()) != 1:
        return False  # No repair command found.
    args = match.groups()[0].split()

    try:
        run_ffx_command(args, suppress_repair=True)
    except subprocess.CalledProcessError:
        return False  # Repair failed.
    return True  # Repair succeeded.


def run_ffx_command(cmd: Iterable[str],
                    target_id: Optional[str] = None,
                    check: bool = True,
                    suppress_repair: bool = False,
                    **kwargs) -> subprocess.CompletedProcess:
    """Runs `ffx` with the given arguments, waiting for it to exit.

    If `ffx` exits with a non-zero exit code, the output is scanned for a
    recommended repair command (e.g., "Run `ffx doctor --restart-daemon` for
    further diagnostics."). If such a command is found, it is run and then the
    original command is retried. This behavior can be suppressed via the
    `suppress_repair` argument.

    Args:
        cmd: A sequence of arguments to ffx.
        target_id: Whether to execute the command for a specific target. The
            target_id could be in the form of a nodename or an address.
        check: If True, CalledProcessError is raised if ffx returns a non-zero
            exit code.
        suppress_repair: If True, do not attempt to find and run a repair
            command.
    Returns:
        A CompletedProcess instance
    Raises:
        CalledProcessError if |check| is true.
    """

    ffx_cmd = [_FFX_TOOL]
    if target_id:
        ffx_cmd.extend(('--target', target_id))
    ffx_cmd.extend(cmd)
    env = os.environ
    if _FFX_ISOLATE_DIR:
        env['FFX_ISOLATE_DIR'] = _FFX_ISOLATE_DIR
    try:
        return subprocess.run(ffx_cmd,
                              check=check,
                              encoding='utf-8',
                              env=env,
                              **kwargs)
    except subprocess.CalledProcessError as cpe:
        if suppress_repair or not _run_repair_command(cpe.output):
            raise

    # If the original command failed but a repair command was found and
    # succeeded, try one more time with the original command.
    return run_ffx_command(cmd, target_id, check, True, **kwargs)


def run_continuous_ffx_command(cmd: Iterable[str],
                               target_id: Optional[str] = None,
                               **kwargs) -> subprocess.Popen:
    """Runs an ffx command asynchronously."""
    ffx_cmd = [_FFX_TOOL]
    if target_id:
        ffx_cmd.extend(('--target', target_id))
    ffx_cmd.extend(cmd)
    return subprocess.Popen(ffx_cmd, encoding='utf-8', **kwargs)


def read_package_paths(out_dir: str, pkg_name: str) -> List[str]:
    """
    Returns:
        A list of the absolute path to all FAR files the package depends on.
    """
    with open(
            os.path.join(DIR_SRC_ROOT, out_dir, 'gen', 'package_metadata',
                         f'{pkg_name}.meta')) as meta_file:
        data = json.load(meta_file)
    packages = []
    for package in data['packages']:
        packages.append(os.path.join(DIR_SRC_ROOT, out_dir, package))
    return packages


def register_common_args(parser: ArgumentParser) -> None:
    """Register commonly used arguments."""
    common_args = parser.add_argument_group('common', 'common arguments')
    common_args.add_argument(
        '--out-dir',
        '-C',
        type=os.path.realpath,
        help='Path to the directory in which build files are located. ')


def register_device_args(parser: ArgumentParser) -> None:
    """Register device arguments."""
    device_args = parser.add_argument_group('device', 'device arguments')
    device_args.add_argument('--target-id',
                             default=os.environ.get('FUCHSIA_NODENAME'),
                             help=('Specify the target device. This could be '
                                   'a node-name (e.g. fuchsia-emulator) or an '
                                   'an ip address along with an optional port '
                                   '(e.g. [fe80::e1c4:fd22:5ee5:878e]:22222, '
                                   '1.2.3.4, 1.2.3.4:33333). If unspecified, '
                                   'the default target in ffx will be used.'))


def register_log_args(parser: ArgumentParser) -> None:
    """Register commonly used arguments."""

    log_args = parser.add_argument_group('logging', 'logging arguments')
    log_args.add_argument('--logs-dir',
                          type=os.path.realpath,
                          help=('Directory to write logs to.'))


def get_component_uri(package: str) -> str:
    """Retrieve the uri for a package."""
    return f'fuchsia-pkg://{REPO_ALIAS}/{package}#meta/{package}.cm'


def resolve_packages(packages: List[str], target_id: Optional[str]) -> None:
    """Ensure that all |packages| are installed on a device."""
    for package in packages:
        run_ffx_command(
            ['component', 'reload', f'/core/ffx-laboratory:{package}'],
            target_id,
            check=False)


# TODO(crbug.com/1342460): Remove when Telemetry tests are using CFv2 packages.
def resolve_v1_packages(packages: List[str], target_id: Optional[str]) -> None:
    """Ensure that all cfv1 packages are installed on a device."""

    ssh_address = run_ffx_command(('target', 'get-ssh-address'),
                                  target_id,
                                  capture_output=True).stdout.strip()
    for package in packages:
        resolve_cmd = [
            '--', 'pkgctl', 'resolve',
            'fuchsia-pkg://%s/%s' % (REPO_ALIAS, package)
        ]
        subprocess.run(get_ssh_prefix(ssh_address) + resolve_cmd, check=True)
