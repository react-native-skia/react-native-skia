# Copyright 2017 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from benchmarks import memory
from core import perf_benchmark
from core import platforms

from telemetry import benchmark
from telemetry import story
from telemetry.timeline import chrome_trace_category_filter
from telemetry.timeline import chrome_trace_config
from telemetry.web_perf import timeline_based_measurement
from contrib.vr_benchmarks import shared_vr_page_state as vr_state
from contrib.vr_benchmarks import webxr_sample_pages


class _BaseVRBenchmark(perf_benchmark.PerfBenchmark):

  # Trace categories that should be enabled for all VR benchmarks.
  COMMON_TRACE_CATEGORIES = [
      '-*', # Remove all default categories.
      'blink.console', # Necessary for memory measurements.
      'disabled-by-default-memory-infra', # Necessary for memory measurements.
      'gpu', # Necessary for various VR metrics.
      'toplevel', # Debug category.
      'viz', # Debug category.
    ]

  @classmethod
  def AddBenchmarkCommandLineArgs(cls, parser):
    parser.add_option(
        '--shared-prefs-file',
        help='The path relative to the Chromium source root '
        'to a file containing a JSON list of shared '
        'preference files to edit and how to do so. '
        'See examples in //chrome/android/'
        'shared_preference_files/test/')
    parser.add_option(
        '--disable-screen-reset',
        action='store_true',
        default=False,
        help='Disables turning screen off and on after each story. '
        'This is useful for local testing when turning off the '
        'screen leads to locking the phone, which makes Telemetry '
        'not produce valid results.')
    parser.add_option(
        '--disable-vrcore-install',
        action='store_true',
        default=False,
        help='Disables the automatic installation of VrCore during pre-test '
             'setup. This is useful for local testing on Pixel devices that '
             'haven\'t had VrCore removed as a system app.')
    parser.add_option(
        '--disable-keyboard-install',
        action='store_true',
        default=False,
        help='Disables the automatic installation of the VR keybaord during '
             'pre-test setup. This is useful for local testing if you want '
             'to use whatever version is already installed on the device '
             'instead of installing whatever is in the test APKs directory.')
    parser.add_option(
        '--remove-system-vrcore',
        action='store_true',
        default=False,
        help='Removes the system version of VrCore if it is installed. This '
             'is required if the system version is not already removed and '
             '--disable-vrcore-install is not passed.')
    parser.add_option(
        '--recording-wpr',
        action='store_true',
        default=False,
        help='Modifies benchmark behavior slightly while recording WPR files '
             'for it. This largely boils down to adding waits/sleeps in order '
             'to ensure that enough streaming data is recorded for the '
             'benchmark to run without issues.')
    parser.add_option(
        '--desktop-runtime',
        default='openxr',
        choices=list(vr_state.WindowsSharedVrPageState.DESKTOP_RUNTIMES.keys()),
        help='Which VR runtime to use on Windows. Defaults to %default')
    parser.add_option(
        '--use-real-runtime',
        action='store_true',
        default=False,
        help='Use the real runtime instead of a mock implementation. This '
             'requires the runtime to be installed on the system.')
    parser.add_option(
        '--mock-runtime-directory',
        help='The directory containing the mock runtime implementation to be '
             'used. Defaults to the "mock_vr_clients" subdirectory of the '
             'output directory.')


class _BaseWebVRWebXRBenchmark(_BaseVRBenchmark):

  # TODO(rmhasan): Remove the SUPPORTED_PLATFORMS lists.
  # SUPPORTED_PLATFORMS is deprecated, please put system specifier tags
  # from expectations.config in SUPPORTED_PLATFORM_TAGS.
  SUPPORTED_PLATFORM_TAGS = [platforms.ANDROID, platforms.WIN10]
  SUPPORTED_PLATFORMS = [
      story.expectations.ALL_ANDROID,
      story.expectations.WIN_10
  ]

  def CreateCoreTimelineBasedMeasurementOptions(self):
    category_filter = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    for category in self.COMMON_TRACE_CATEGORIES:
      category_filter.AddFilter(category)

    options = timeline_based_measurement.Options(category_filter)
    options.config.enable_android_graphics_memtrack = True
    options.config.enable_platform_display_trace = True

    options.SetTimelineBasedMetrics(
        ['memoryMetric', 'webvrMetric', 'webxrMetric'])
    options.config.chrome_trace_config.SetMemoryDumpConfig(
        chrome_trace_config.MemoryDumpConfig())
    return options



class _BaseWebXRBenchmark(_BaseWebVRWebXRBenchmark):

  def SetExtraBrowserOptions(self, options):
    memory.SetExtraBrowserOptionsForMemoryMeasurement(options)
    options.AppendExtraBrowserArgs([
        '--enable-features=WebXR',
    ])


@benchmark.Info(emails=['bsheedy@chromium.org', 'tiborg@chromium.org'])
# pylint: disable=too-many-ancestors
class XrWebXrStatic(_BaseWebXRBenchmark):
  """Measures WebXR performance with synthetic sample pages."""

  def CreateStorySet(self, options):
    del options
    return webxr_sample_pages.WebXrSamplePageSet()

  @classmethod
  def Name(cls):
    return 'xr.webxr.static'


class _BaseBrowsingBenchmark(_BaseVRBenchmark):

  # TODO(rmhasan): Remove the SUPPORTED_PLATFORMS lists.
  # SUPPORTED_PLATFORMS is deprecated, please put system specifier tags
  # from expectations.config in SUPPORTED_PLATFORM_TAGS.
  SUPPORTED_PLATFORM_TAGS = [platforms.ANDROID]
  SUPPORTED_PLATFORMS = [story.expectations.ALL_ANDROID]

  def CreateCoreTimelineBasedMeasurementOptions(self):
    category_filter = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    for category in self.COMMON_TRACE_CATEGORIES:
      category_filter.AddFilter(category)

    options = timeline_based_measurement.Options(category_filter)
    options.config.enable_android_graphics_memtrack = True
    options.config.enable_platform_display_trace = True
    options.SetTimelineBasedMetrics(['frameCycleDurationMetric',
      'memoryMetric'])
    options.config.chrome_trace_config.SetMemoryDumpConfig(
        chrome_trace_config.MemoryDumpConfig())
    return options

  def SetExtraBrowserOptions(self, options):
    options.flush_os_page_caches_on_start = True
    options.AppendExtraBrowserArgs([
        '--enable-gpu-benchmarking',
        '--touch-events=enabled',
        '--enable-vr-shell',
    ])
