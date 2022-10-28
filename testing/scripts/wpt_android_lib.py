# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import contextlib
import json
import logging
import os
import sys

import common
import wpt_common

logger = logging.getLogger(__name__)

SRC_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))

BUILD_ANDROID = os.path.join(SRC_DIR, 'build', 'android')
BLINK_TOOLS_DIR = os.path.join(
    SRC_DIR, 'third_party', 'blink', 'tools')
CATAPULT_DIR = os.path.join(SRC_DIR, 'third_party', 'catapult')
DEFAULT_WPT = os.path.join(
    SRC_DIR, 'third_party', 'wpt_tools', 'wpt', 'wpt')
PYUTILS = os.path.join(CATAPULT_DIR, 'common', 'py_utils')
TOMBSTONE_PARSER = os.path.join(SRC_DIR, 'build', 'android', 'tombstones.py')

if PYUTILS not in sys.path:
  sys.path.append(PYUTILS)

if BLINK_TOOLS_DIR not in sys.path:
  sys.path.append(BLINK_TOOLS_DIR)

if BUILD_ANDROID not in sys.path:
  sys.path.append(BUILD_ANDROID)

import devil_chromium

from blinkpy.web_tests.port.android import (
    PRODUCTS, PRODUCTS_TO_EXPECTATION_FILE_PATHS, ANDROID_WEBLAYER,
    ANDROID_WEBVIEW, CHROME_ANDROID, ANDROID_DISABLED_TESTS)

from devil.android import apk_helper
from devil.android import device_utils
from devil.android.tools import system_app
from devil.android.tools import webview_app

from py_utils.tempfile_ext import NamedTemporaryDirectory
from pylib.local.emulator import avd

class PassThroughArgs(argparse.Action):
  pass_through_args = []
  def __call__(self, parser, namespace, values, option_string=None):
    if option_string:
      if self.nargs == 0:
        self.add_unique_pass_through_arg(option_string)
      elif self.nargs is None:
        self.add_unique_pass_through_arg('{}={}'.format(option_string, values))
      else:
        raise ValueError("nargs {} not supported: {} {}".format(
            self.nargs, option_string, values))

  @classmethod
  def add_unique_pass_through_arg(cls, arg):
    if arg not in cls.pass_through_args:
      cls.pass_through_args.append(arg)


class WPTAndroidAdapter(wpt_common.BaseWptScriptAdapter):

  def __init__(self, device):
    self.pass_through_wpt_args = []
    self.pass_through_binary_args = []
    self._metadata_dir = None
    self._device = device
    super(WPTAndroidAdapter, self).__init__()
    # Arguments from add_extra_argumentsparse were added so
    # its safe to parse the arguments and set self._options
    self.parse_args()
    self.output_directory = os.path.join(SRC_DIR, 'out', self.options.target)
    self.mojo_js_directory = os.path.join(self.output_directory, 'gen')

  @property
  def rest_args(self):
    rest_args = super(WPTAndroidAdapter, self).rest_args

    # Update the output directory to the default if it's not set.
    self.maybe_set_default_isolated_script_test_output()

    # Here we add all of the arguments required to run WPT tests on Android.
    rest_args.extend([self.options.wpt_path])

    # By default, WPT will treat unexpected passes as errors, so we disable
    # that to be consistent with Chromium CI.
    rest_args.extend(['--no-fail-on-unexpected-pass'])
    if self.options.default_exclude:
      rest_args.extend(['--default-exclude'])

    # vpython has packages needed by wpt, so force it to skip the setup
    rest_args.extend(['--venv=' + SRC_DIR, '--skip-venv-setup'])

    rest_args.extend(['run',
      '--tests=' + wpt_common.EXTERNAL_WPT_TESTS_DIR,
      '--test-type=' + self.options.test_type,
      '--device-serial', self._device.serial,
      '--webdriver-binary',
      self.options.webdriver_binary,
      '--symbols-path',
      self.output_directory,
      '--stackwalk-binary',
      TOMBSTONE_PARSER,
      '--headless',
      '--no-pause-after-test',
      '--no-capture-stdio',
      '--no-manifest-download',
      '--binary-arg=--enable-blink-features=MojoJS,MojoJSTest',
      '--binary-arg=--enable-blink-test-features',
      '--binary-arg=--disable-field-trial-config',
      '--enable-mojojs',
      '--mojojs-path=' + self.mojo_js_directory,
      '--binary-arg=--enable-features=DownloadService<DownloadServiceStudy',
      '--binary-arg=--force-fieldtrials=DownloadServiceStudy/Enabled',
      '--binary-arg=--force-fieldtrial-params=DownloadServiceStudy.Enabled:'
      'start_up_delay_ms/0',
    ])
    # if metadata was created then add the metadata directory
    # to the list of wpt arguments
    if self._metadata_dir:
      rest_args.extend(['--metadata', self._metadata_dir])

    if self.options.verbose >= 3:
      rest_args.extend(['--log-mach=-', '--log-mach-level=debug',
                        '--log-mach-verbose'])

    if self.options.verbose >= 4:
      rest_args.extend(['--webdriver-arg=--verbose',
                        '--webdriver-arg="--log-path=-"'])

    if self.options.log_wptreport:
      wpt_output = self.options.isolated_script_test_output
      self.wptreport = os.path.join(os.path.dirname(wpt_output),
                                       'reports.json')
      rest_args.extend(['--log-wptreport',
                        self.wptreport])

    rest_args.extend(self.pass_through_wpt_args)

    return rest_args

  @property
  def browser_specific_expectations_path(self):
    raise NotImplementedError

  def _extra_metadata_builder_args(self):
    args = ['--additional-expectations=%s' % path
            for path in self.options.additional_expectations]
    if not self.options.ignore_browser_specific_expectations:
      args.extend(['--additional-expectations',
                   self.browser_specific_expectations_path])

    return args

  def _maybe_build_metadata(self):
    metadata_builder_cmd = [
      sys.executable,
      os.path.join(wpt_common.BLINK_TOOLS_DIR, 'build_wpt_metadata.py'),
      '--android-product',
      self.options.product,
      '--metadata-output-dir',
      self._metadata_dir,
      '--additional-expectations',
      ANDROID_DISABLED_TESTS,
      '--use-subtest-results',
    ]
    if self.options.ignore_default_expectations:
      metadata_builder_cmd += [ '--ignore-default-expectations' ]
    metadata_builder_cmd.extend(self._extra_metadata_builder_args())
    return common.run_command(metadata_builder_cmd)

  def run_test(self):
    with NamedTemporaryDirectory() as tmp_dir, self._install_apks():
      self._metadata_dir = os.path.join(tmp_dir, 'metadata_dir')
      metadata_command_ret = self._maybe_build_metadata()
      if metadata_command_ret != 0:
        return metadata_command_ret

      # If there is no metadata then we need to create an
      # empty directory to pass to wptrunner
      if not os.path.exists(self._metadata_dir):
        os.makedirs(self._metadata_dir)
      return super(WPTAndroidAdapter, self).run_test()

  def _install_apks(self):
    raise NotImplementedError

  def clean_up_after_test_run(self):
    # Avoid having a dangling reference to the temp directory
    # which was deleted
    self._metadata_dir = None

  def add_extra_arguments(self, parser):
    # TODO: |pass_through_args| are broke and need to be supplied by way of
    # --binary-arg".
    class BinaryPassThroughArgs(PassThroughArgs):
      pass_through_args = self.pass_through_binary_args
    class WPTPassThroughArgs(PassThroughArgs):
      pass_through_args = self.pass_through_wpt_args

    # Add this so that product argument does not go in self._rest_args
    # when self.parse_args() is called
    parser.add_argument('--target', '-t', default='Release',
                        help='Specify the target build subdirectory under'
                        ' src/out/.')
    parser.add_argument('--product', help=argparse.SUPPRESS)
    parser.add_argument('--webdriver-binary', required=True,
                        help='Path of the webdriver binary.  It needs to have'
                        ' the same major version as the apk.')
    parser.add_argument('--wpt-path', default=DEFAULT_WPT,
                        help='Controls the path of the WPT runner to use'
                        ' (therefore tests).  Defaults the revision rolled into'
                        ' Chromium.')
    parser.add_argument('--additional-expectations',
                        action='append', default=[],
                        help='Paths to additional test expectations files.')
    parser.add_argument('--ignore-default-expectations', action='store_true',
                        help='Do not use the default set of'
                        ' TestExpectations files.')
    parser.add_argument('--ignore-browser-specific-expectations',
                        action='store_true', default=False,
                        help='Ignore browser specific expectation files.')
    parser.add_argument('--test-type', default='testharness',
                        help='Specify to experiment with other test types.'
                        ' Currently only the default is expected to work.')
    parser.add_argument('--verbose', '-v', action='count', default=0,
                        help='Verbosity level.')
    parser.add_argument('--repeat',
                        action=WPTPassThroughArgs, type=int,
                        help='Number of times to run the tests.')
    parser.add_argument('--include', metavar='TEST_OR_DIR',
                        action=WPTPassThroughArgs,
                        help='Test(s) to run, defaults to run all tests.')
    parser.add_argument('--include-file',
                        action=WPTPassThroughArgs,
                        help='A file listing test(s) to run')
    parser.add_argument('--default-exclude', action='store_true', default=False,
                        help="Only run the tests explicitly given in arguments."
                             "No tests will run if the list is empty, and the "
                             "program will exit with status code 0.")
    parser.add_argument('--list-tests', action=WPTPassThroughArgs, nargs=0,
                        help="Don't run any tests, just print out a list of"
                        ' tests that would be run.')
    parser.add_argument('--webdriver-arg', action=WPTPassThroughArgs,
                        help='WebDriver args.')
    parser.add_argument('--log-wptreport',
                        action='store_true', default=False,
                        help="Generates a test report in JSON format.")
    parser.add_argument('--log-raw', metavar='RAW_REPORT_FILE',
                        action=WPTPassThroughArgs,
                        help="Log raw report.")
    parser.add_argument('--log-html', metavar='HTML_REPORT_FILE',
                        action=WPTPassThroughArgs,
                        help="Log html report.")
    parser.add_argument('--log-xunit', metavar='XUNIT_REPORT_FILE',
                        action=WPTPassThroughArgs,
                        help="Log xunit report.")
    parser.add_argument('--enable-features', action=BinaryPassThroughArgs,
                        help='Chromium features to enable during testing.')
    parser.add_argument('--disable-features', action=BinaryPassThroughArgs,
                        help='Chromium features to disable during testing.')
    parser.add_argument('--disable-field-trial-config',
                        action=BinaryPassThroughArgs,
                        help='Disable test trials for Chromium features.')
    parser.add_argument('--force-fieldtrials', action=BinaryPassThroughArgs,
                        help='Force trials for Chromium features.')
    parser.add_argument('--force-fieldtrial-params',
                        action=BinaryPassThroughArgs,
                        help='Force trial params for Chromium features.')
    add_emulator_args(parser)


class WPTWeblayerAdapter(WPTAndroidAdapter):

  WEBLAYER_SHELL_PKG = 'org.chromium.weblayer.shell'
  WEBLAYER_SUPPORT_PKG = 'org.chromium.weblayer.support'

  @contextlib.contextmanager
  def _install_apks(self):
    install_weblayer_shell_as_needed = _maybe_install_user_apk(
        self._device, self.options.weblayer_shell, self.WEBLAYER_SHELL_PKG)
    install_weblayer_support_as_needed = _maybe_install_user_apk(
        self._device, self.options.weblayer_support, self.WEBLAYER_SUPPORT_PKG)
    install_webview_provider_as_needed = _maybe_install_webview_provider(
        self._device, self.options.webview_provider)

    with install_weblayer_shell_as_needed,   \
         install_weblayer_support_as_needed, \
         install_webview_provider_as_needed:
      yield

  @property
  def browser_specific_expectations_path(self):
    return PRODUCTS_TO_EXPECTATION_FILE_PATHS[ANDROID_WEBLAYER]

  def add_extra_arguments(self, parser):
    super(WPTWeblayerAdapter, self).add_extra_arguments(parser)
    parser.add_argument('--weblayer-shell',
                        help='WebLayer Shell apk to install.')
    parser.add_argument('--weblayer-support',
                        help='WebLayer Support apk to install.')
    parser.add_argument('--webview-provider',
                        help='Webview provider apk to install.')

  @property
  def rest_args(self):
    args = super(WPTWeblayerAdapter, self).rest_args
    args.append(ANDROID_WEBLAYER)
    return args


class WPTWebviewAdapter(WPTAndroidAdapter):

  def __init__(self, device):
    super(WPTWebviewAdapter, self).__init__(device)
    if self.options.system_webview_shell is not None:
      self.system_webview_shell_pkg = apk_helper.GetPackageName(
          self.options.system_webview_shell)
    else:
      self.system_webview_shell_pkg = 'org.chromium.webview_shell'

  @contextlib.contextmanager
  def _install_apks(self):
    install_shell_as_needed = _maybe_install_user_apk(
        self._device, self.options.system_webview_shell,
        self.system_webview_shell_pkg)
    install_webview_provider_as_needed = _maybe_install_webview_provider(
        self._device, self.options.webview_provider)
    with install_shell_as_needed, install_webview_provider_as_needed:
      yield

  @property
  def browser_specific_expectations_path(self):
    return PRODUCTS_TO_EXPECTATION_FILE_PATHS[ANDROID_WEBVIEW]

  def add_extra_arguments(self, parser):
    super(WPTWebviewAdapter, self).add_extra_arguments(parser)
    parser.add_argument('--system-webview-shell',
                        help=('System WebView Shell apk to install. If not '
                              'specified then the on-device WebView apk '
                              'will be used.'))
    parser.add_argument('--webview-provider',
                        help='Webview provider APK to install.')

  @property
  def rest_args(self):
    args = super(WPTWebviewAdapter, self).rest_args
    args.extend(['--package-name', self.system_webview_shell_pkg])
    args.append(ANDROID_WEBVIEW)
    return args


class WPTClankAdapter(WPTAndroidAdapter):

  @contextlib.contextmanager
  def _install_apks(self):
    install_clank_as_needed = _maybe_install_user_apk(
        self._device, self.options.chrome_apk)
    with install_clank_as_needed:
      yield

  @property
  def browser_specific_expectations_path(self):
    return PRODUCTS_TO_EXPECTATION_FILE_PATHS[CHROME_ANDROID]

  def add_extra_arguments(self, parser):
    super(WPTClankAdapter, self).add_extra_arguments(parser)
    parser.add_argument(
        '--chrome-apk', help='Chrome apk to install.')
    parser.add_argument(
        '--chrome-package-name',
        help=('The package name of Chrome to test,'
              ' defaults to that of the compiled Chrome apk.'))

  @property
  def rest_args(self):
    args = super(WPTClankAdapter, self).rest_args
    if not self.options.chrome_package_name and not self.options.chrome_apk:
      raise Exception('Either the --chrome-package-name or --chrome-apk '
                      'command line arguments must be used.')
    if not self.options.chrome_package_name:
      self.options.chrome_package_name = apk_helper.GetPackageName(
          self.options.chrome_apk)
      logger.info("Using Chrome apk's default package %s." %
                  self.options.chrome_package_name)
    args.extend(['--package-name', self.options.chrome_package_name])
    # add the product postional argument
    args.append(CHROME_ANDROID)
    return args


def add_emulator_args(parser):
  parser.add_argument(
      '--avd-config',
      type=os.path.realpath,
      help='Path to the avd config textpb. '
      '(See //tools/android/avd/proto/ for message definition'
      ' and existing textpb files.)')
  parser.add_argument(
      '--emulator-window',
      action='store_true',
      default=False,
      help='Enable graphical window display on the emulator.')


@contextlib.contextmanager
def get_device(args):
  instance = None
  try:
    if args.avd_config:
      avd_config = avd.AvdConfig(args.avd_config)
      logger.warning('Install emulator from ' + args.avd_config)
      avd_config.Install()
      instance = avd_config.CreateInstance()
      instance.Start(writable_system=True, window=args.emulator_window)
      device_utils.DeviceUtils(instance.serial).WaitUntilFullyBooted()

    #TODO(weizhong): when choose device, make sure abi matches with target
    devices = device_utils.DeviceUtils.HealthyDevices()
    if devices:
      yield devices[0]
    else:
      yield
  finally:
    if instance:
      instance.Stop()


def _maybe_install_webview_provider(device, apk):
  if apk:
    logger.info('Will install WebView apk at ' + apk)
    return webview_app.UseWebViewProvider(device, apk)
  else:
    return _no_op()


def _maybe_install_user_apk(device, apk, expected_pkg=None):
  """contextmanager to install apk on device.

  Args:
    device: DeviceUtils instance on which to install the apk.
    apk: Apk file path on host.
    expected_pkg:  Optional, check that apk's package name matches.
  Returns:
    If apk evaluates to false, returns a do-nothing contextmanager.
    Otherwise, returns a contextmanager to install apk on device.
  """
  if apk:
    pkg = apk_helper.GetPackageName(apk)
    if expected_pkg and pkg != expected_pkg:
      raise ValueError('{} has incorrect package name: {}, expected {}.'.format(
          apk, pkg, expected_pkg))
    install_as_needed = _app_installed(device, apk, pkg)
    logger.info('Will install ' + pkg + ' at ' + apk)
  else:
    install_as_needed = _no_op()
  return install_as_needed


@contextlib.contextmanager
def _app_installed(device, apk, pkg):
  device.Install(apk)
  try:
    yield
  finally:
    device.Uninstall(pkg)


# Dummy contextmanager to simplify multiple optional managers.
@contextlib.contextmanager
def _no_op():
  yield

