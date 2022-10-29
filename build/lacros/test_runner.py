#!/usr/bin/env python3
#
# Copyright 2020 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script facilitates running tests for lacros on Linux.

  In order to run lacros tests on Linux, please first follow bit.ly/3juQVNJ
  to setup build directory with the lacros-chrome-on-linux build configuration,
  and corresponding test targets are built successfully.

  * Example usages:

  ./build/lacros/test_runner.py test out/lacros/url_unittests
  ./build/lacros/test_runner.py test out/lacros/browser_tests

  The commands above run url_unittests and browser_tests respecitively, and more
  specifically, url_unitests is executed directly while browser_tests is
  executed with the latest version of prebuilt ash-chrome, and the behavior is
  controlled by |_TARGETS_REQUIRE_ASH_CHROME|, and it's worth noting that the
  list is maintained manually, so if you see something is wrong, please upload a
  CL to fix it.

  ./build/lacros/test_runner.py test out/lacros/browser_tests \\
      --gtest_filter=BrowserTest.Title

  The above command only runs 'BrowserTest.Title', and any argument accepted by
  the underlying test binary can be specified in the command.

  ./build/lacros/test_runner.py test out/lacros/browser_tests \\
    --ash-chrome-version=793554

  The above command runs tests with a given version of ash-chrome, which is
  useful to reproduce test failures, the version corresponds to the commit
  position of commits on the master branch, and a list of prebuilt versions can
  be found at: gs://ash-chromium-on-linux-prebuilts/x86_64.

  ./testing/xvfb.py ./build/lacros/test_runner.py test out/lacros/browser_tests

  The above command starts ash-chrome with xvfb instead of an X11 window, and
  it's useful when running tests without a display attached, such as sshing.

  For version skew testing when passing --ash-chrome-path-override, the runner
  will try to find the ash major version and Lacros major version. If ash is
  newer(major version larger), the runner will not run any tests and just
  returns success.
"""

import argparse
import json
import os
import logging
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import zipfile

_SRC_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.path.pardir, os.path.pardir))
sys.path.append(os.path.join(_SRC_ROOT, 'third_party', 'depot_tools'))

# Base GS URL to store prebuilt ash-chrome.
_GS_URL_BASE = 'gs://ash-chromium-on-linux-prebuilts/x86_64'

# Latest file version.
_GS_URL_LATEST_FILE = _GS_URL_BASE + '/latest/ash-chromium.txt'

# GS path to the zipped ash-chrome build with any given version.
_GS_ASH_CHROME_PATH = 'ash-chromium.zip'

# Directory to cache downloaded ash-chrome versions to avoid re-downloading.
_PREBUILT_ASH_CHROME_DIR = os.path.join(os.path.dirname(__file__),
                                        'prebuilt_ash_chrome')

# Number of seconds to wait for ash-chrome to start.
ASH_CHROME_TIMEOUT_SECONDS = (
    300 if os.environ.get('ASH_WRAPPER', None) else 10)

# List of targets that require ash-chrome as a Wayland server in order to run.
_TARGETS_REQUIRE_ASH_CHROME = [
    'app_shell_unittests',
    'aura_unittests',
    'browser_tests',
    'components_unittests',
    'compositor_unittests',
    'content_unittests',
    'dbus_unittests',
    'extensions_unittests',
    'media_unittests',
    'message_center_unittests',
    'snapshot_unittests',
    'sync_integration_tests',
    'unit_tests',
    'views_unittests',
    'wm_unittests',

    # regex patterns.
    '.*_browsertests',
    '.*interactive_ui_tests'
]

# List of targets that require ash-chrome to support crosapi mojo APIs.
_TARGETS_REQUIRE_MOJO_CROSAPI = [
    # TODO(jamescook): Add 'browser_tests' after multiple crosapi connections
    # are allowed. For now we only enable crosapi in targets that run tests
    # serially.
    'interactive_ui_tests',
    'lacros_chrome_browsertests',
    'lacros_chrome_browsertests_run_in_series'
]

# Default test filter file for each target. These filter files will be
# used by default if no other filter file get specified.
_DEFAULT_FILTER_FILES_MAPPING = {
    'browser_tests': 'linux-lacros.browser_tests.filter',
    'components_unittests': 'linux-lacros.components_unittests.filter',
    'content_browsertests': 'linux-lacros.content_browsertests.filter',
    'interactive_ui_tests': 'linux-lacros.interactive_ui_tests.filter',
    'lacros_chrome_browsertests':
    'linux-lacros.lacros_chrome_browsertests.filter',
    'sync_integration_tests': 'linux-lacros.sync_integration_tests.filter',
    'unit_tests': 'linux-lacros.unit_tests.filter',
}


def _GetAshChromeDirPath(version):
  """Returns a path to the dir storing the downloaded version of ash-chrome."""
  return os.path.join(_PREBUILT_ASH_CHROME_DIR, version)


def _remove_unused_ash_chrome_versions(version_to_skip):
  """Removes unused ash-chrome versions to save disk space.

  Currently, when an ash-chrome zip is downloaded and unpacked, the atime/mtime
  of the dir and the files are NOW instead of the time when they were built, but
  there is no garanteen it will always be the behavior in the future, so avoid
  removing the current version just in case.

  Args:
    version_to_skip (str): the version to skip removing regardless of its age.
  """
  days = 7
  expiration_duration = 60 * 60 * 24 * days

  for f in os.listdir(_PREBUILT_ASH_CHROME_DIR):
    if f == version_to_skip:
      continue

    p = os.path.join(_PREBUILT_ASH_CHROME_DIR, f)
    if os.path.isfile(p):
      # The prebuilt ash-chrome dir is NOT supposed to contain any files, remove
      # them to keep the directory clean.
      os.remove(p)
      continue
    chrome_path = os.path.join(p, 'test_ash_chrome')
    if not os.path.exists(chrome_path):
      chrome_path = p
    age = time.time() - os.path.getatime(chrome_path)
    if age > expiration_duration:
      logging.info(
          'Removing ash-chrome: "%s" as it hasn\'t been used in the '
          'past %d days', p, days)
      shutil.rmtree(p)

def _GsutilCopyWithRetry(gs_path, local_name, retry_times=3):
  """Gsutil copy with retry.

  Args:
    gs_path: The gs path for remote location.
    local_name: The local file name.
    retry_times: The total try times if the gsutil call fails.

  Raises:
    RuntimeError: If failed to download the specified version, for example,
        if the version is not present on gcs.
  """
  import download_from_google_storage
  gsutil = download_from_google_storage.Gsutil(
      download_from_google_storage.GSUTIL_DEFAULT_PATH)
  exit_code = 1
  retry = 0
  while exit_code and retry < retry_times:
    retry += 1
    exit_code = gsutil.call('cp', gs_path, local_name)
  if exit_code:
    raise RuntimeError('Failed to download: "%s"' % gs_path)


def _DownloadAshChromeIfNecessary(version):
  """Download a given version of ash-chrome if not already exists.

  Args:
    version: A string representing the version, such as "793554".

  Raises:
      RuntimeError: If failed to download the specified version, for example,
          if the version is not present on gcs.
  """

  def IsAshChromeDirValid(ash_chrome_dir):
    # This function assumes that once 'chrome' is present, other dependencies
    # will be present as well, it's not always true, for example, if the test
    # runner process gets killed in the middle of unzipping (~2 seconds), but
    # it's unlikely for the assumption to break in practice.
    return os.path.isdir(ash_chrome_dir) and os.path.isfile(
        os.path.join(ash_chrome_dir, 'test_ash_chrome'))

  ash_chrome_dir = _GetAshChromeDirPath(version)
  if IsAshChromeDirValid(ash_chrome_dir):
    return

  shutil.rmtree(ash_chrome_dir, ignore_errors=True)
  os.makedirs(ash_chrome_dir)
  with tempfile.NamedTemporaryFile() as tmp:
    logging.info('Ash-chrome version: %s', version)
    gs_path = _GS_URL_BASE + '/' + version + '/' + _GS_ASH_CHROME_PATH
    _GsutilCopyWithRetry(gs_path, tmp.name)

    # https://bugs.python.org/issue15795. ZipFile doesn't preserve permissions.
    # And in order to workaround the issue, this function is created and used
    # instead of ZipFile.extractall().
    # The solution is copied from:
    # https://stackoverflow.com/questions/42326428/zipfile-in-python-file-permission
    def ExtractFile(zf, info, extract_dir):
      zf.extract(info.filename, path=extract_dir)
      perm = info.external_attr >> 16
      os.chmod(os.path.join(extract_dir, info.filename), perm)

    with zipfile.ZipFile(tmp.name, 'r') as zf:
      # Extra all files instead of just 'chrome' binary because 'chrome' needs
      # other resources and libraries to run.
      for info in zf.infolist():
        ExtractFile(zf, info, ash_chrome_dir)

  _remove_unused_ash_chrome_versions(version)


def _GetLatestVersionOfAshChrome():
  """Returns the latest version of uploaded ash-chrome."""
  with tempfile.NamedTemporaryFile() as tmp:
    _GsutilCopyWithRetry(_GS_URL_LATEST_FILE, tmp.name)
    with open(tmp.name, 'r') as f:
      return f.read().strip()


def _WaitForAshChromeToStart(tmp_xdg_dir, lacros_mojo_socket_file,
                             enable_mojo_crosapi, ash_ready_file):
  """Waits for Ash-Chrome to be up and running and returns a boolean indicator.

  Determine whether ash-chrome is up and running by checking whether two files
  (lock file + socket) have been created in the |XDG_RUNTIME_DIR| and the lacros
  mojo socket file has been created if enabling the mojo "crosapi" interface.
  TODO(crbug.com/1107966): Figure out a more reliable hook to determine the
  status of ash-chrome, likely through mojo connection.

  Args:
    tmp_xdg_dir (str): Path to the XDG_RUNTIME_DIR.
    lacros_mojo_socket_file (str): Path to the lacros mojo socket file.
    enable_mojo_crosapi (bool): Whether to bootstrap the crosapi mojo interface
        between ash and the lacros test binary.
    ash_ready_file (str): Path to a non-existing file. After ash is ready for
        testing, the file will be created.

  Returns:
    A boolean indicating whether Ash-chrome is up and running.
  """

  def IsAshChromeReady(tmp_xdg_dir, lacros_mojo_socket_file,
                       enable_mojo_crosapi, ash_ready_file):
    # There should be 2 wayland files.
    if len(os.listdir(tmp_xdg_dir)) < 2:
      return False
    if enable_mojo_crosapi and not os.path.exists(lacros_mojo_socket_file):
      return False
    return os.path.exists(ash_ready_file)

  time_counter = 0
  while not IsAshChromeReady(tmp_xdg_dir, lacros_mojo_socket_file,
                             enable_mojo_crosapi, ash_ready_file):
    time.sleep(0.5)
    time_counter += 0.5
    if time_counter > ASH_CHROME_TIMEOUT_SECONDS:
      break

  return IsAshChromeReady(tmp_xdg_dir, lacros_mojo_socket_file,
                          enable_mojo_crosapi, ash_ready_file)


def _ExtractAshMajorVersion(file_path):
  """Extract major version from file_path.

  File path like this:
  ../../lacros_version_skew_tests_v94.0.4588.0/test_ash_chrome

  Returns:
    int representing the major version. Or 0 if it can't extract
        major version.
  """
  m = re.search(
      'lacros_version_skew_tests_v(?P<version>[0-9]+).[0-9]+.[0-9]+.[0-9]+/',
      file_path)
  if (m and 'version' in m.groupdict().keys()):
    return int(m.group('version'))
  logging.warning('Can not find the ash version in %s.' % file_path)
  # Returns ash major version as 0, so we can still run tests.
  # This is likely happen because user is running in local environments.
  return 0


def _FindLacrosMajorVersionFromMetadata():
  # This handles the logic on bots. When running on bots,
  # we don't copy source files to test machines. So we build a
  # metadata.json file which contains version information.
  if not os.path.exists('metadata.json'):
    logging.error('Can not determine current version.')
    # Returns 0 so it can't run any tests.
    return 0
  version = ''
  with open('metadata.json', 'r') as file:
    content = json.load(file)
    version = content['content']['version']
  return int(version[:version.find('.')])


def _FindLacrosMajorVersion():
  """Returns the major version in the current checkout.

  It would try to read src/chrome/VERSION. If it's not available,
  then try to read metadata.json.

  Returns:
    int representing the major version. Or 0 if it fails to
    determine the version.
  """
  version_file = os.path.abspath(
      os.path.join(os.path.abspath(os.path.dirname(__file__)),
                   '../../chrome/VERSION'))
  # This is mostly happens for local development where
  # src/chrome/VERSION exists.
  if os.path.exists(version_file):
    lines = open(version_file, 'r').readlines()
    return int(lines[0][lines[0].find('=') + 1:-1])
  return _FindLacrosMajorVersionFromMetadata()


def _ParseSummaryOutput(forward_args):
  """Find the summary output file path.

  Args:
    forward_args (list): Args to be forwarded to the test command.

  Returns:
    None if not found, or str representing the output file path.
  """
  logging.warning(forward_args)
  for arg in forward_args:
    if arg.startswith('--test-launcher-summary-output='):
      return arg[len('--test-launcher-summary-output='):]
  return None


def _IsRunningOnBots(forward_args):
  """Detects if the script is running on bots or not.

  Args:
    forward_args (list): Args to be forwarded to the test command.

  Returns:
    True if the script is running on bots. Otherwise returns False.
  """
  return '--test-launcher-bot-mode' in forward_args


def _RunTestWithAshChrome(args, forward_args):
  """Runs tests with ash-chrome.

  Args:
    args (dict): Args for this script.
    forward_args (list): Args to be forwarded to the test command.
  """
  if args.ash_chrome_path_override:
    ash_chrome_file = args.ash_chrome_path_override
    ash_major_version = _ExtractAshMajorVersion(ash_chrome_file)
    lacros_major_version = _FindLacrosMajorVersion()
    if ash_major_version > lacros_major_version:
      logging.warning('''Not running any tests, because we do not \
support version skew testing for Lacros M%s against ash M%s''' %
                      (lacros_major_version, ash_major_version))
      # Create an empty output.json file so result adapter can read
      # the file. Or else result adapter will report no file found
      # and result infra failure.
      output_json = _ParseSummaryOutput(forward_args)
      if output_json:
        with open(output_json, 'w') as f:
          f.write("""{"all_tests":[],"disabled_tests":[],"global_tags":[],
"per_iteration_data":[],"test_locations":{}}""")
      # Although we don't run any tests, this is considered as success.
      return 0
    if not os.path.exists(ash_chrome_file):
      logging.error("""Can not find ash chrome at %s. Did you download \
the ash from CIPD? If you don't plan to build your own ash, you need \
to download first. Example commandlines:
 $ cipd auth-login
 $ echo "chromium/testing/linux-ash-chromium/x86_64/ash.zip \
version:92.0.4515.130" > /tmp/ensure-file.txt
 $ cipd ensure -ensure-file /tmp/ensure-file.txt \
-root lacros_version_skew_tests_v92.0.4515.130
 Then you can use --ash-chrome-path-override=\
lacros_version_skew_tests_v92.0.4515.130/test_ash_chrome
""" % ash_chrome_file)
      return 1
  elif args.ash_chrome_path:
    ash_chrome_file = args.ash_chrome_path
  else:
    ash_chrome_version = (args.ash_chrome_version
                          or _GetLatestVersionOfAshChrome())
    _DownloadAshChromeIfNecessary(ash_chrome_version)
    logging.info('Ash-chrome version: %s', ash_chrome_version)

    ash_chrome_file = os.path.join(_GetAshChromeDirPath(ash_chrome_version),
                                   'test_ash_chrome')
  try:
    # Starts Ash-Chrome.
    tmp_xdg_dir_name = tempfile.mkdtemp()
    tmp_ash_data_dir_name = tempfile.mkdtemp()

    # Please refer to below file for how mojo connection is set up in testing.
    # //chrome/browser/ash/crosapi/test_mojo_connection_manager.h
    lacros_mojo_socket_file = '%s/lacros.sock' % tmp_ash_data_dir_name
    lacros_mojo_socket_arg = ('--lacros-mojo-socket-for-testing=%s' %
                              lacros_mojo_socket_file)
    ash_ready_file = '%s/ash_ready.txt' % tmp_ash_data_dir_name
    enable_mojo_crosapi = any(t == os.path.basename(args.command)
                              for t in _TARGETS_REQUIRE_MOJO_CROSAPI)
    ash_wayland_socket_name = 'wayland-exo'

    ash_process = None
    ash_env = os.environ.copy()
    ash_env['XDG_RUNTIME_DIR'] = tmp_xdg_dir_name
    ash_cmd = [
        ash_chrome_file,
        '--user-data-dir=%s' % tmp_ash_data_dir_name,
        '--enable-wayland-server',
        '--no-startup-window',
        '--disable-lacros-keep-alive',
        '--disable-login-lacros-opening',
        '--enable-features=LacrosSupport,LacrosPrimary,LacrosOnly',
        '--ash-ready-file-path=%s' % ash_ready_file,
        '--wayland-server-socket=%s' % ash_wayland_socket_name,
    ]
    if enable_mojo_crosapi:
      ash_cmd.append(lacros_mojo_socket_arg)

    # Users can specify a wrapper for the ash binary to do things like
    # attaching debuggers. For example, this will open a new terminal window
    # and run GDB.
    #   $ export ASH_WRAPPER="gnome-terminal -- gdb --ex=r --args"
    ash_wrapper = os.environ.get('ASH_WRAPPER', None)
    if ash_wrapper:
      logging.info('Running ash with "ASH_WRAPPER": %s', ash_wrapper)
      ash_cmd = list(ash_wrapper.split()) + ash_cmd

    ash_process_has_started = False
    total_tries = 3
    num_tries = 0

    # Create a log file if the user wanted to have one.
    log = None
    if args.ash_logging_path:
      log = open(args.ash_logging_path, 'a')
    # Ash logs can be useful. Enable ash log by default on bots.
    elif _IsRunningOnBots(forward_args):
      summary_file = _ParseSummaryOutput(forward_args)
      if summary_file:
        ash_log_path = os.path.join(os.path.dirname(summary_file),
                                    'ash_chrome.log')
        log = open(ash_log_path, 'a')

    while not ash_process_has_started and num_tries < total_tries:
      num_tries += 1
      if log is None:
        ash_process = subprocess.Popen(ash_cmd, env=ash_env)
      else:
        ash_process = subprocess.Popen(ash_cmd,
                                       env=ash_env,
                                       stdout=log,
                                       stderr=log)

      ash_process_has_started = _WaitForAshChromeToStart(
          tmp_xdg_dir_name, lacros_mojo_socket_file, enable_mojo_crosapi,
          ash_ready_file)
      if ash_process_has_started:
        break

      logging.warning('Starting ash-chrome timed out after %ds',
                      ASH_CHROME_TIMEOUT_SECONDS)
      logging.warning('Are you using test_ash_chrome?')
      logging.warning('Printing the output of "ps aux" for debugging:')
      subprocess.call(['ps', 'aux'])
      if ash_process and ash_process.poll() is None:
        ash_process.kill()

    if not ash_process_has_started:
      raise RuntimeError('Timed out waiting for ash-chrome to start')

    # Starts tests.
    if enable_mojo_crosapi:
      forward_args.append(lacros_mojo_socket_arg)

    test_env = os.environ.copy()
    test_env['WAYLAND_DISPLAY'] = ash_wayland_socket_name
    test_env['EGL_PLATFORM'] = 'surfaceless'
    test_env['XDG_RUNTIME_DIR'] = tmp_xdg_dir_name
    test_process = subprocess.Popen([args.command] + forward_args, env=test_env)
    return test_process.wait()

  finally:
    if ash_process and ash_process.poll() is None:
      ash_process.terminate()
      # Allow process to do cleanup and exit gracefully before killing.
      time.sleep(0.5)
      ash_process.kill()

    shutil.rmtree(tmp_xdg_dir_name, ignore_errors=True)
    shutil.rmtree(tmp_ash_data_dir_name, ignore_errors=True)


def _RunTestDirectly(args, forward_args):
  """Runs tests by invoking the test command directly.

  args (dict): Args for this script.
  forward_args (list): Args to be forwarded to the test command.
  """
  try:
    p = None
    p = subprocess.Popen([args.command] + forward_args)
    return p.wait()
  finally:
    if p and p.poll() is None:
      p.terminate()
      time.sleep(0.5)
      p.kill()


def _HandleSignal(sig, _):
  """Handles received signals to make sure spawned test process are killed.

  sig (int): An integer representing the received signal, for example SIGTERM.
  """
  logging.warning('Received signal: %d, killing spawned processes', sig)

  # Don't do any cleanup here, instead, leave it to the finally blocks.
  # Assumption is based on https://docs.python.org/3/library/sys.html#sys.exit:
  # cleanup actions specified by finally clauses of try statements are honored.

  # https://tldp.org/LDP/abs/html/exitcodes.html:
  # Exit code 128+n -> Fatal error signal "n".
  sys.exit(128 + sig)


def _ExpandFilterFileIfNeeded(test_target, forward_args):
  if (test_target in _DEFAULT_FILTER_FILES_MAPPING.keys() and not any(
      [arg.startswith('--test-launcher-filter-file') for arg in forward_args])):
    file_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..', '..', 'testing',
                     'buildbot', 'filters',
                     _DEFAULT_FILTER_FILES_MAPPING[test_target]))
    forward_args.append(f'--test-launcher-filter-file={file_path}')


def _RunTest(args, forward_args):
  """Runs tests with given args.

  args (dict): Args for this script.
  forward_args (list): Args to be forwarded to the test command.

  Raises:
      RuntimeError: If the given test binary doesn't exist or the test runner
          doesn't know how to run it.
  """

  if not os.path.isfile(args.command):
    raise RuntimeError('Specified test command: "%s" doesn\'t exist' %
                       args.command)

  test_target = os.path.basename(args.command)
  _ExpandFilterFileIfNeeded(test_target, forward_args)

  # |_TARGETS_REQUIRE_ASH_CHROME| may not always be accurate as it is updated
  # with a best effort only, therefore, allow the invoker to override the
  # behavior with a specified ash-chrome version, which makes sure that
  # automated CI/CQ builders would always work correctly.
  requires_ash_chrome = any(
      re.match(t, test_target) for t in _TARGETS_REQUIRE_ASH_CHROME)
  if not requires_ash_chrome and not args.ash_chrome_version:
    return _RunTestDirectly(args, forward_args)

  return _RunTestWithAshChrome(args, forward_args)


def Main():
  for sig in (signal.SIGTERM, signal.SIGINT):
    signal.signal(sig, _HandleSignal)

  logging.basicConfig(level=logging.INFO)
  arg_parser = argparse.ArgumentParser()
  arg_parser.usage = __doc__

  subparsers = arg_parser.add_subparsers()

  test_parser = subparsers.add_parser('test', help='Run tests')
  test_parser.set_defaults(func=_RunTest)

  test_parser.add_argument(
      'command',
      help='A single command to invoke the tests, for example: '
      '"./url_unittests". Any argument unknown to this test runner script will '
      'be forwarded to the command, for example: "--gtest_filter=Suite.Test"')

  version_group = test_parser.add_mutually_exclusive_group()
  version_group.add_argument(
      '--ash-chrome-version',
      type=str,
      help='Version of an prebuilt ash-chrome to use for testing, for example: '
      '"793554", and the version corresponds to the commit position of commits '
      'on the main branch. If not specified, will use the latest version '
      'available')
  version_group.add_argument(
      '--ash-chrome-path',
      type=str,
      help='Path to an locally built ash-chrome to use for testing. '
      'In general you should build //chrome/test:test_ash_chrome.')

  # This is for version skew testing. The current CI/CQ builder builds
  # an ash chrome and pass it using --ash-chrome-path. In order to use the same
  # builder for version skew testing, we use a new argument to override
  # the ash chrome.
  test_parser.add_argument(
      '--ash-chrome-path-override',
      type=str,
      help='The same as --ash-chrome-path. But this will override '
      '--ash-chrome-path or --ash-chrome-version if any of these '
      'arguments exist.')
  test_parser.add_argument(
      '--ash-logging-path',
      type=str,
      help='File & path to ash-chrome logging output while running Lacros '
      'browser tests. If not provided, no output will be generated.')

  args = arg_parser.parse_known_args()
  return args[0].func(args[0], args[1])


if __name__ == '__main__':
  sys.exit(Main())
