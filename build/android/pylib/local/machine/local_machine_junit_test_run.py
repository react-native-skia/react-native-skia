# Copyright 2016 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import logging
import multiprocessing
import os
import queue
import re
import subprocess
import sys
import tempfile
import threading
import time
import zipfile

from six.moves import range  # pylint: disable=redefined-builtin
from devil.utils import cmd_helper
from py_utils import tempfile_ext
from pylib import constants
from pylib.base import base_test_result
from pylib.base import test_run
from pylib.constants import host_paths
from pylib.results import json_results


# These Test classes are used for running tests and are excluded in the test
# runner. See:
# https://android.googlesource.com/platform/frameworks/testing/+/android-support-test/runner/src/main/java/android/support/test/internal/runner/TestRequestBuilder.java
# base/test/android/javatests/src/org/chromium/base/test/BaseChromiumAndroidJUnitRunner.java # pylint: disable=line-too-long
_EXCLUDED_CLASSES_PREFIXES = ('android', 'junit', 'org/bouncycastle/util',
                              'org/hamcrest', 'org/junit', 'org/mockito')

# Suites we shouldn't shard, usually because they don't contain enough test
# cases.
_EXCLUDED_SUITES = {
    'password_check_junit_tests',
    'touch_to_fill_junit_tests',
}


# It can actually take longer to run if you shard too much, especially on
# smaller suites. Locally media_base_junit_tests takes 4.3 sec with 1 shard,
# and 6 sec with 2 or more shards.
_MIN_CLASSES_PER_SHARD = 8

# Running the largest test suite with a single shard takes about 22 minutes.
_SHARD_TIMEOUT = 30 * 60

# RegExp to detect logcat lines, e.g., 'I/AssetManager: not found'.
_LOGCAT_RE = re.compile(r'[A-Z]/[\w\d_-]+:')


class LocalMachineJunitTestRun(test_run.TestRun):
  # override
  def TestPackage(self):
    return self._test_instance.suite

  # override
  def SetUp(self):
    pass

  def _GetFilterArgs(self, test_filter_override=None):
    ret = []
    if test_filter_override:
      ret += ['-gtest-filter', ':'.join(test_filter_override)]
    elif self._test_instance.test_filter:
      ret += ['-gtest-filter', self._test_instance.test_filter]

    if self._test_instance.package_filter:
      ret += ['-package-filter', self._test_instance.package_filter]
    if self._test_instance.runner_filter:
      ret += ['-runner-filter', self._test_instance.runner_filter]

    return ret

  def _CreateJarArgsList(self, json_result_file_paths, group_test_list, shards):
    # Creates a list of jar_args. The important thing is each jar_args list
    # has a different json_results file for writing test results to and that
    # each list of jar_args has its own test to run as specified in the
    # -gtest-filter.
    jar_args_list = [['-json-results-file', result_file]
                     for result_file in json_result_file_paths]
    for index, jar_arg in enumerate(jar_args_list):
      test_filter_override = group_test_list[index] if shards > 1 else None
      jar_arg += self._GetFilterArgs(test_filter_override)

    return jar_args_list

  def _CreateJvmArgsList(self):
    # Creates a list of jvm_args (robolectric, code coverage, etc...)
    jvm_args = [
        '-Drobolectric.dependency.dir=%s' %
        self._test_instance.robolectric_runtime_deps_dir,
        '-Ddir.source.root=%s' % constants.DIR_SOURCE_ROOT,
        # Use locally available sdk jars from 'robolectric.dependency.dir'
        '-Drobolectric.offline=true',
        '-Drobolectric.resourcesMode=binary',
        '-Drobolectric.logging=stdout',
    ]
    if self._test_instance.debug_socket:
      jvm_args += [
          '-agentlib:jdwp=transport=dt_socket'
          ',server=y,suspend=y,address=%s' % self._test_instance.debug_socket
      ]

    if self._test_instance.coverage_dir:
      if not os.path.exists(self._test_instance.coverage_dir):
        os.makedirs(self._test_instance.coverage_dir)
      elif not os.path.isdir(self._test_instance.coverage_dir):
        raise Exception('--coverage-dir takes a directory, not file path.')
      if self._test_instance.coverage_on_the_fly:
        jacoco_coverage_file = os.path.join(
            self._test_instance.coverage_dir,
            '%s.exec' % self._test_instance.suite)
        jacoco_agent_path = os.path.join(host_paths.DIR_SOURCE_ROOT,
                                         'third_party', 'jacoco', 'lib',
                                         'jacocoagent.jar')

        # inclnolocationclasses is false to prevent no class def found error.
        jacoco_args = '-javaagent:{}=destfile={},inclnolocationclasses=false'
        jvm_args.append(
            jacoco_args.format(jacoco_agent_path, jacoco_coverage_file))
      else:
        jvm_args.append('-Djacoco-agent.destfile=%s' %
                        os.path.join(self._test_instance.coverage_dir,
                                     '%s.exec' % self._test_instance.suite))

    return jvm_args

  @property
  def _wrapper_path(self):
    return os.path.join(constants.GetOutDirectory(), 'bin', 'helper',
                        self._test_instance.suite)

  #override
  def GetTestsForListing(self):
    cmd = [self._wrapper_path, '--list-tests'] + self._GetFilterArgs()
    lines = subprocess.check_output(cmd, encoding='utf8').splitlines()

    PREFIX = '#TEST# '
    prefix_len = len(PREFIX)
    # Filter log messages other than test names (Robolectric logs to stdout).
    return sorted(l[prefix_len:] for l in lines if l.startswith(PREFIX))

  # override
  def RunTests(self, results, raw_logs_fh=None):
    # This avoids searching through the classparth jars for tests classes,
    # which takes about 1-2 seconds.
    # Do not shard when a test filter is present since we do not know at this
    # point which tests will be filtered out.
    if (self._test_instance.shards == 1 or self._test_instance.test_filter
        or self._test_instance.suite in _EXCLUDED_SUITES):
      test_classes = []
      shards = 1
    else:
      test_classes = _GetTestClasses(self._wrapper_path)
      shards = ChooseNumOfShards(test_classes, self._test_instance.shards)

    logging.info('Running tests on %d shard(s).', shards)
    group_test_list = GroupTestsForShard(shards, test_classes)

    with tempfile_ext.NamedTemporaryDirectory() as temp_dir:
      cmd_list = [[self._wrapper_path] for _ in range(shards)]
      json_result_file_paths = [
          os.path.join(temp_dir, 'results%d.json' % i) for i in range(shards)
      ]
      jar_args_list = self._CreateJarArgsList(json_result_file_paths,
                                              group_test_list, shards)
      if jar_args_list:
        for i in range(shards):
          cmd_list[i].extend(
              ['--jar-args', '"%s"' % ' '.join(jar_args_list[i])])

      jvm_args = self._CreateJvmArgsList()
      if jvm_args:
        for cmd in cmd_list:
          cmd.extend(['--jvm-args', '"%s"' % ' '.join(jvm_args)])

      AddPropertiesJar(cmd_list, temp_dir, self._test_instance.resource_apk)

      show_logcat = logging.getLogger().isEnabledFor(logging.INFO)
      num_omitted_lines = 0
      for line in _RunCommandsAndSerializeOutput(cmd_list):
        if raw_logs_fh:
          raw_logs_fh.write(line)
        if show_logcat or not _LOGCAT_RE.match(line):
          sys.stdout.write(line)
        else:
          num_omitted_lines += 1

      if num_omitted_lines > 0:
        logging.critical('%d log lines omitted.', num_omitted_lines)
      sys.stdout.flush()
      if raw_logs_fh:
        raw_logs_fh.flush()

      results_list = []
      try:
        for json_file_path in json_result_file_paths:
          with open(json_file_path, 'r') as f:
            results_list += json_results.ParseResultsFromJson(
                json.loads(f.read()))
      except IOError:
        # In the case of a failure in the JUnit or Robolectric test runner
        # the output json file may never be written.
        results_list = [
            base_test_result.BaseTestResult('Test Runner Failure',
                                            base_test_result.ResultType.UNKNOWN)
        ]

      test_run_results = base_test_result.TestRunResults()
      test_run_results.AddResults(results_list)
      results.append(test_run_results)

  # override
  def TearDown(self):
    pass


def AddPropertiesJar(cmd_list, temp_dir, resource_apk):
  # Create properties file for Robolectric test runners so they can find the
  # binary resources.
  properties_jar_path = os.path.join(temp_dir, 'properties.jar')
  with zipfile.ZipFile(properties_jar_path, 'w') as z:
    z.writestr('com/android/tools/test_config.properties',
               'android_resource_apk=%s' % resource_apk)
    z.writestr('robolectric.properties',
               'application = android.app.Application')

  for cmd in cmd_list:
    cmd.extend(['--classpath', properties_jar_path])


def ChooseNumOfShards(test_classes, shards):
  # Don't override requests to not shard.
  if shards == 1:
    return 1

  # Sharding doesn't reduce runtime on just a few tests.
  if shards > (len(test_classes) // _MIN_CLASSES_PER_SHARD) or shards < 1:
    shards = max(1, (len(test_classes) // _MIN_CLASSES_PER_SHARD))

  # Local tests of explicit --shard values show that max speed is achieved
  # at cpu_count() / 2.
  # Using -XX:TieredStopAtLevel=1 is required for this result. The flag reduces
  # CPU time by two-thirds, making sharding more effective.
  shards = max(1, min(shards, multiprocessing.cpu_count() // 2))
  # Can have at minimum one test_class per shard.
  shards = min(len(test_classes), shards)

  return shards


def GroupTestsForShard(num_of_shards, test_classes):
  """Groups tests that will be ran on each shard.

  Args:
    num_of_shards: number of shards to split tests between.
    test_classes: A list of test_class files in the jar.

  Return:
    Returns a dictionary containing a list of test classes.
  """
  test_dict = {i: [] for i in range(num_of_shards)}

  # Round robin test distribiution to reduce chance that a sequential group of
  # classes all have an unusually high number of tests.
  for count, test_cls in enumerate(test_classes):
    test_cls = test_cls.replace('.class', '*')
    test_cls = test_cls.replace('/', '.')
    test_dict[count % num_of_shards].append(test_cls)

  return test_dict


def _RunCommandsAndSerializeOutput(cmd_list):
  """Runs multiple commands in parallel and yields serialized output lines.

  Args:
    cmd_list: List of commands.

  Returns: N/A

  Raises:
    TimeoutError: If timeout is exceeded.
  """
  num_shards = len(cmd_list)
  assert num_shards > 0
  procs = []
  temp_files = []
  for i, cmd in enumerate(cmd_list):
    # Shard 0 yields results immediately, the rest write to files.
    if i == 0:
      temp_files.append(None)  # Placeholder.
      procs.append(
          cmd_helper.Popen(
              cmd,
              stdout=subprocess.PIPE,
              stderr=subprocess.STDOUT,
          ))
    else:
      temp_file = tempfile.TemporaryFile(mode='w+t', encoding='utf-8')
      temp_files.append(temp_file)
      procs.append(cmd_helper.Popen(
          cmd,
          stdout=temp_file,
          stderr=temp_file,
      ))

  timeout_time = time.time() + _SHARD_TIMEOUT
  timed_out = False

  yield '\n'
  yield 'Shard 0 output:\n'

  # The following will be run from a thread to pump Shard 0 results, allowing
  # live output while allowing timeout.
  def pump_stream_to_queue(f, q):
    try:
      for line in iter(f.readline, ''):
        q.put(line)
    except ValueError:  # Triggered if |f.close()| gets called.
      pass

  shard_0_q = queue.Queue()
  shard_0_pump = threading.Thread(target=pump_stream_to_queue,
                                  args=(procs[0].stdout, shard_0_q))
  shard_0_pump.start()

  # Wait for processes to finish, while forwarding Shard 0 results.
  shard_to_check = 0
  while shard_to_check < num_shards:
    if shard_0_pump.is_alive():
      while not shard_0_q.empty():
        yield shard_0_q.get_nowait()
    if procs[shard_to_check].poll() is not None:
      shard_to_check += 1
    else:
      time.sleep(.1)
    if time.time() > timeout_time:
      timed_out = True
      break

  # Handle Shard 0 timeout.
  if shard_0_pump.is_alive():
    procs[0].stdout.close()
  shard_0_pump.join()

  # Emit all output (possibly incomplete due to |time_out|) in shard order.
  while not shard_0_q.empty():
    yield shard_0_q.get_nowait()
  for i in range(1, num_shards):
    f = temp_files[i]
    yield '\n'
    yield 'Shard %d output:\n' % i
    f.seek(0)
    for line in f.readlines():
      yield line
    f.close()

  # Handle Shard 1+ timeout.
  if timed_out:
    for i, p in enumerate(procs):
      if p.poll() is None:
        p.kill()
        yield 'Index of timed out shard: %d\n' % i

    yield 'Output in shards may be cutoff due to timeout.\n'
    yield '\n'
    raise cmd_helper.TimeoutError('Junit shards timed out.')


def _GetTestClasses(file_path):
  test_jar_paths = subprocess.check_output([file_path,
                                            '--print-classpath']).decode()
  test_jar_paths = test_jar_paths.split(':')

  test_classes = []
  for test_jar_path in test_jar_paths:
    # Avoid searching through jars that are for the test runner.
    # TODO(crbug.com/1144077): Use robolectric buildconfig file arg.
    if 'third_party/robolectric/' in test_jar_path:
      continue

    test_classes += _GetTestClassesFromJar(test_jar_path)

  logging.info('Found %d test classes in class_path jars.', len(test_classes))
  return test_classes


def _GetTestClassesFromJar(test_jar_path):
  """Returns a list of test classes from a jar.

  Test files end in Test, this is enforced:
  //tools/android/errorprone_plugin/src/org/chromium/tools/errorprone
  /plugin/TestClassNameCheck.java

  Args:
    test_jar_path: Path to the jar.

  Return:
    Returns a list of test classes that were in the jar.
  """
  class_list = []
  with zipfile.ZipFile(test_jar_path, 'r') as zip_f:
    for test_class in zip_f.namelist():
      if test_class.startswith(_EXCLUDED_CLASSES_PREFIXES):
        continue
      if test_class.endswith('Test.class') and '$' not in test_class:
        class_list.append(test_class)

  return class_list
