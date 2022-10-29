# Copyright 2020 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from benchmarks import system_health
from telemetry import benchmark

from contrib.system_health_scroll_jank import janky_story_set

_BENCHMARK_UMA = [
    'Browser.MainThreadsCongestion',
    'Compositing.Display.DrawToSwapUs',
    'CompositorLatency.TotalLatency',
    'CompositorLatency.Type',
    'Event.Latency.ScrollJank',
    'Event.Latency.ScrollUpdate.JankyDuration',
    'Event.Latency.ScrollUpdate.JankyEvents',
    'Event.Latency.ScrollUpdate.TotalDuration',
    'Event.Latency.ScrollUpdate.TotalEvents',
    'Event.Jank.ScrollUpdate.TotalJankyAndNonJankyDuration2',
    'EventLatency.FirstGestureScrollUpdate.Touchscreen.TotalLatency',
    'EventLatency.GestureScrollUpdate.Touchscreen.TotalLatency',
    'EventLatency.FirstGestureScrollUpdate.Wheel.TotalLatency',
    'EventLatency.GestureScrollUpdate.Wheel.TotalLatency',
    'Graphics.Smoothness.Checkerboarding.TouchScroll',
    'Graphics.Smoothness.Checkerboarding.WheelScroll',
    'Graphics.Smoothness.Jank.Compositor.TouchScroll',
    'Graphics.Smoothness.Jank.Main.TouchScroll',
    'Graphics.Smoothness.PercentDroppedFrames3.AllAnimations',
    'Graphics.Smoothness.PercentDroppedFrames3.AllInteractions',
    'Graphics.Smoothness.PercentDroppedFrames3.AllSequences',
    'Memory.GPU.PeakMemoryUsage2.Scroll',
    'Memory.GPU.PeakMemoryUsage2.PageLoad',
]


@benchmark.Info(emails=['khokhlov@google.com'])
class SystemHealthScrollJankMobile(system_health.MobileCommonSystemHealth):
  """A subset of system_health.common_mobile benchmark.

  Contains only stories related to monitoring jank during scrolling.
  This benchmark is used for running experimental scroll jank metrics.

  """

  @classmethod
  def Name(cls):
    return 'system_health.scroll_jank_mobile'

  def CreateCoreTimelineBasedMeasurementOptions(self):
    options = super(SystemHealthScrollJankMobile,
                    self).CreateCoreTimelineBasedMeasurementOptions()
    options.ExtendTraceCategoryFilter([
        'benchmark', 'cc', 'input', 'disabled-by-default-histogram_samples',
        'latencyInfo'
    ])
    options.config.chrome_trace_config.EnableUMAHistograms(*_BENCHMARK_UMA)
    options.SetTimelineBasedMetrics([
        'renderingMetric',
        'umaMetric',
        # Unless --experimentatil-tbmv3-metric flag is used, the following tbmv3
        # metrics do nothing.
        'tbmv3:uma_metrics',
        'tbmv3:scroll_jank',
    ])
    return options

  def CreateStorySet(self, options):
    return janky_story_set.JankyStorySet()
