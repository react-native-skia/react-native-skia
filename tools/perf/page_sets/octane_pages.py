# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from telemetry import story
from telemetry.util import statistics

from page_sets import press_story


_GB = 1024 * 1024 * 1024

DESCRIPTIONS = {
    'CodeLoad':
        'Measures how quickly a JavaScript engine can start executing code '
        'after loading a large JavaScript program, social widget being a '
        'common example. The source for test is derived from open source '
        'libraries (Closure, jQuery) (1,530 lines).',
    'Crypto':
        'Encryption and decryption benchmark based on code by Tom Wu '
        '(1698 lines).',
    'DeltaBlue':
        'One-way constraint solver, originally written in Smalltalk by John '
        'Maloney and Mario Wolczko (880 lines).',
    'EarleyBoyer':
        'Classic Scheme benchmarks, translated to JavaScript by Florian '
        'Loitsch\'s Scheme2Js compiler (4684 lines).',
    'Gameboy':
        'Emulate the portable console\'s architecture and runs a demanding 3D '
        'simulation, all in JavaScript (11,097 lines).',
    'Mandreel':
        'Runs the 3D Bullet Physics Engine ported from C++ to JavaScript via '
        'Mandreel (277,377 lines).',
    'NavierStokes':
        '2D NavierStokes equations solver, heavily manipulates double '
        'precision arrays. Based on Oliver Hunt\'s code (387 lines).',
    'PdfJS':
        'Mozilla\'s PDF Reader implemented in JavaScript. It measures decoding '
        'and interpretation time (33,056 lines).',
    'RayTrace':
        'Ray tracer benchmark based on code by Adam Burmister (904 lines).',
    'RegExp':
        'Regular expression benchmark generated by extracting regular '
        'expression operations from 50 of the most popular web pages '
        '(1761 lines).',
    'Richards':
        'OS kernel simulation benchmark, originally written in BCPL by Martin '
        'Richards (539 lines).',
    'Splay':
        'Data manipulation benchmark that deals with splay trees and exercises '
        'the automatic memory management subsystem (394 lines).',
}


class OctaneStory(press_story.PressStory):
  URL = 'http://chromium.github.io/octane/index.html?auto=1'
  NAME = 'Octane'

  def RunNavigateSteps(self, action_runner):
    total_memory = (
        action_runner.tab.browser.platform.GetSystemTotalPhysicalMemory())
    if total_memory is not None and total_memory < 1 * _GB:
      skipBenchmarks = '"zlib"'
    else:
      skipBenchmarks = ''
    self.script_to_evaluate_on_commit = """
        var __results = [];
        var __real_log = window.console.log;
        window.console.log = function(msg) {
          __results.push(msg);
          __real_log.apply(this, [msg]);
        }
        skipBenchmarks = [%s]
        """ % (skipBenchmarks)
    super(OctaneStory, self).RunNavigateSteps(action_runner)

  def ExecuteTest(self, action_runner):
    action_runner.WaitForJavaScriptCondition('window.completed', timeout=10)
    action_runner.WaitForJavaScriptCondition(
        '!document.getElementById("progress-bar-container")', timeout=1200)

  def ParseTestResults(self, action_runner):
    results_log = action_runner.EvaluateJavaScript('__results')
    all_scores = []
    for output in results_log:
      # Split the results into score and test name.
      # results log e.g., "Richards: 18343"
      score_and_name = output.split(': ', 2)
      assert len(score_and_name) == 2, \
        'Unexpected result format "%s"' % score_and_name
      if 'Skipped' not in score_and_name[1]:
        name = score_and_name[0]
        score = float(score_and_name[1])
        self.AddMeasurement(name, 'score', score,
                            description=DESCRIPTIONS.get(name))

        # Collect all test scores to compute geometric mean.
        all_scores.append(score)
    total = statistics.GeometricMean(all_scores)
    self.AddMeasurement('Total.Score', 'score', total,
                        description='Geometric mean of the scores of each '
                        'individual benchmark in the Octane collection.')


class OctaneStorySet(story.StorySet):
  def __init__(self):
    super(OctaneStorySet, self).__init__(
        archive_data_file='data/octane.json',
        cloud_storage_bucket=story.PUBLIC_BUCKET)

    self.AddStory(OctaneStory(self))