#!/usr/bin/env python3
# Copyright 2020 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Builds and runs a test by filename.

This script finds the appropriate test suites for the specified test file or
directory, builds it, then runs it with the (optionally) specified filter,
passing any extra args on to the test runner.

Examples:
# Run the test target for bit_cast_unittest.cc. Use a custom test filter instead
# of the automatically generated one.
autotest.py -C out/Desktop bit_cast_unittest.cc --gtest_filter=BitCastTest*

# Find and run UrlUtilitiesUnitTest.java's tests, pass remaining parameters to
# the test binary.
autotest.py -C out/Android UrlUtilitiesUnitTest --fast-local-dev -v

# Run all tests under base/strings
autotest.py -C out/foo --run-all base/strings

# Run only the test on line 11. Useful when running autotest.py from your text
# editor.
autotest.py -C out/foo --line 11 base/strings/strcat_unittest.cc
"""

import argparse
import locale
import os
import json
import re
import subprocess
import sys

from enum import Enum
from pathlib import Path

USE_PYTHON_3 = f'This script will only run under python3.'

SRC_DIR = Path(__file__).parent.parent.resolve()
sys.path.append(str(SRC_DIR / 'build' / 'android'))
from pylib import constants

DEPOT_TOOLS_DIR = SRC_DIR / 'third_party' / 'depot_tools'
DEBUG = False

# Some test suites use suffixes that would also match non-test-suite targets.
# Those test suites should be manually added here.
_OTHER_TEST_TARGETS = [
    '//chrome/test:browser_tests',
    '//chrome/test:unit_tests',
]

_TEST_TARGET_REGEX = re.compile(
    r'(_browsertests|_junit_tests|_perftests|_test_.*apk|_unittests|' +
    r'_wpr_tests)$')

TEST_FILE_NAME_REGEX = re.compile(r'(.*Test\.java)|(.*_[a-z]*test\.cc)')

# Some tests don't directly include gtest.h and instead include it via gmock.h
# or a test_utils.h file, so make sure these cases are captured. Also include
# files that use <...> for #includes instead of quotes.
GTEST_INCLUDE_REGEX = re.compile(
    r'#include.*(gtest|gmock|_test_utils|browser_test)\.h("|>)')


def ExitWithMessage(*args):
  print(*args, file=sys.stderr)
  sys.exit(1)


class TestValidity(Enum):
  NOT_A_TEST = 0  # Does not match test file regex.
  MAYBE_A_TEST = 1  # Matches test file regex, but doesn't include gtest files.
  VALID_TEST = 2  # Matches test file regex and includes gtest files.


def IsTestFile(file_path):
  if not TEST_FILE_NAME_REGEX.match(file_path):
    return TestValidity.NOT_A_TEST
  if file_path.endswith('.cc'):
    # Try a bit harder to remove non-test files for c++. Without this,
    # 'autotest.py base/' finds non-test files.
    try:
      with open(file_path, 'r', encoding='utf-8') as f:
        if GTEST_INCLUDE_REGEX.search(f.read()) is not None:
          return TestValidity.VALID_TEST
    except IOError:
      pass
    # It may still be a test file, even if it doesn't include a gtest file.
    return TestValidity.MAYBE_A_TEST
  return TestValidity.VALID_TEST


class CommandError(Exception):
  """Exception thrown when a subcommand fails."""

  def __init__(self, command, return_code, output=None):
    Exception.__init__(self)
    self.command = command
    self.return_code = return_code
    self.output = output

  def __str__(self):
    message = (f'\n***\nERROR: Error while running command {self.command}'
               f'.\nExit status: {self.return_code}\n')
    if self.output:
      message += f'Output:\n{self.output}\n'
    message += '***'
    return message


def StreamCommandOrExit(cmd, **kwargs):
  try:
    subprocess.check_call(cmd, **kwargs)
  except subprocess.CalledProcessError as e:
    sys.exit(1)


def RunCommand(cmd, **kwargs):
  try:
    # Set an encoding to convert the binary output to a string.
    return subprocess.check_output(
        cmd, **kwargs, encoding=locale.getpreferredencoding())
  except subprocess.CalledProcessError as e:
    raise CommandError(e.cmd, e.returncode, e.output) from None


def BuildTestTargetsWithNinja(out_dir, targets, dry_run):
  """Builds the specified targets with ninja"""
  # Use autoninja from PATH to match version used for manual builds.
  ninja_path = 'autoninja'
  if sys.platform.startswith('win32'):
    ninja_path += '.bat'
  cmd = [ninja_path, '-C', out_dir] + targets
  print('Building: ' + ' '.join(cmd))
  if (dry_run):
    return True
  try:
    subprocess.check_call(cmd)
  except subprocess.CalledProcessError as e:
    return False
  return True


def RecursiveMatchFilename(folder, filename):
  current_dir = os.path.split(folder)[-1]
  if current_dir.startswith('out') or current_dir.startswith('.'):
    return [[], []]
  exact = []
  close = []
  with os.scandir(folder) as it:
    for entry in it:
      if (entry.is_symlink()):
        continue
      if (entry.is_file() and filename in entry.path and
          not os.path.basename(entry.path).startswith('.')):
        file_validity = IsTestFile(entry.path)
        if file_validity is TestValidity.VALID_TEST:
          exact.append(entry.path)
        elif file_validity is TestValidity.MAYBE_A_TEST:
          close.append(entry.path)
      if entry.is_dir():
        # On Windows, junctions are like a symlink that python interprets as a
        # directory, leading to exceptions being thrown. We can just catch and
        # ignore these exceptions like we would ignore symlinks.
        try:
          matches = RecursiveMatchFilename(entry.path, filename)
          exact += matches[0]
          close += matches[1]
        except FileNotFoundError as e:
          if DEBUG:
            print(f'Failed to scan directory "{entry}" - junction?')
          pass
  return [exact, close]


def FindTestFilesInDirectory(directory):
  test_files = []
  if DEBUG:
    print('Test files:')
  for root, _, files in os.walk(directory):
    for f in files:
      path = os.path.join(root, f)
      file_validity = IsTestFile(path)
      if file_validity is TestValidity.VALID_TEST:
        if DEBUG:
          print(path)
        test_files.append(path)
      elif DEBUG and file_validity is TestValidity.MAYBE_A_TEST:
        print(path + ' matched but doesn\'t include gtest files, skipping.')
  return test_files


def FindMatchingTestFiles(target):
  # Return early if there's an exact file match.
  if os.path.isfile(target):
    # If the target is a C++ implementation file, try to guess the test file.
    if target.endswith('.cc') or target.endswith('.h'):
      target_validity = IsTestFile(target)
      if target_validity is TestValidity.VALID_TEST:
        return [target]
      alternate = f"{target.rsplit('.', 1)[0]}_unittest.cc"
      alt_validity = TestValidity.NOT_A_TEST if not os.path.isfile(
          alternate) else IsTestFile(alternate)
      if alt_validity is TestValidity.VALID_TEST:
        return [alternate]

      # If neither the target nor its alternative were valid, check if they just
      # didn't include the gtest files before deciding to exit.
      if target_validity is TestValidity.MAYBE_A_TEST:
        return [target]
      if alt_validity is TestValidity.MAYBE_A_TEST:
        return [alternate]
      ExitWithMessage(f"{target} doesn't look like a test file")
    return [target]
  # If this is a directory, return all the test files it contains.
  if os.path.isdir(target):
    files = FindTestFilesInDirectory(target)
    if not files:
      ExitWithMessage('No tests found in directory')
    return files

  if sys.platform.startswith('win32') and os.path.altsep in target:
    # Use backslash as the path separator on Windows to match os.scandir().
    if DEBUG:
      print('Replacing ' + os.path.altsep + ' with ' + os.path.sep + ' in: '
            + target)
    target = target.replace(os.path.altsep, os.path.sep)
  if DEBUG:
    print('Finding files with full path containing: ' + target)

  [exact, close] = RecursiveMatchFilename(SRC_DIR, target)
  if DEBUG:
    if exact:
      print('Found exact matching file(s):')
      print('\n'.join(exact))
    if close:
      print('Found possible matching file(s):')
      print('\n'.join(close))

  test_files = exact if len(exact) > 0 else close
  if len(test_files) > 1:
    # Arbitrarily capping at 10 results so we don't print the name of every file
    # in the repo if the target is poorly specified.
    test_files = test_files[:10]
    ExitWithMessage(f'Target "{target}" is ambiguous. Matching files: '
                    f'{test_files}')
  if not test_files:
    ExitWithMessage(f'Target "{target}" did not match any files.')
  return test_files


def IsTestTarget(target):
  if _TEST_TARGET_REGEX.search(target):
    return True
  return target in _OTHER_TEST_TARGETS


def HaveUserPickTarget(paths, targets):
  # Cap to 10 targets for convenience [0-9].
  targets = targets[:10]
  target_list = '\n'.join(f'{i}. {t}' for i, t in enumerate(targets))

  user_input = input(f'Target "{paths}" is used by multiple test targets.\n' +
                     target_list + '\nPlease pick a target: ')
  try:
    value = int(user_input)
    return targets[value]
  except (ValueError, IndexError):
    print('Try again')
    return HaveUserPickTarget(paths, targets)


# A persistent cache to avoid running gn on repeated runs of autotest.
class TargetCache:
  def __init__(self, out_dir):
    self.out_dir = out_dir
    self.path = os.path.join(out_dir, 'autotest_cache')
    self.gold_mtime = self.GetBuildNinjaMtime()
    self.cache = {}
    try:
      mtime, cache = json.load(open(self.path, 'r'))
      if mtime == self.gold_mtime:
        self.cache = cache
    except Exception:
      pass

  def Save(self):
    with open(self.path, 'w') as f:
      json.dump([self.gold_mtime, self.cache], f)

  def Find(self, test_paths):
    key = ' '.join(test_paths)
    return self.cache.get(key, None)

  def Store(self, test_paths, test_targets):
    key = ' '.join(test_paths)
    self.cache[key] = test_targets

  def GetBuildNinjaMtime(self):
    return os.path.getmtime(os.path.join(self.out_dir, 'build.ninja'))

  def IsStillValid(self):
    return self.GetBuildNinjaMtime() == self.gold_mtime


def FindTestTargets(target_cache, out_dir, paths, run_all):
  # Normalize paths, so they can be cached.
  paths = [os.path.realpath(p) for p in paths]
  test_targets = target_cache.Find(paths)
  used_cache = True
  if not test_targets:
    used_cache = False

    # Use gn refs to recursively find all targets that depend on |path|, filter
    # internal gn targets, and match against well-known test suffixes, falling
    # back to a list of known test targets if that fails.
    gn_path = os.path.join(DEPOT_TOOLS_DIR, 'gn')
    if sys.platform.startswith('win32'):
      gn_path += '.bat'

    cmd = [gn_path, 'refs', out_dir, '--all'] + paths
    targets = RunCommand(cmd).splitlines()
    targets = [t for t in targets if '__' not in t]
    test_targets = [t for t in targets if IsTestTarget(t)]

  if not test_targets:
    ExitWithMessage(
        f'Target(s) "{paths}" did not match any test targets. Consider adding'
        f' one of the following targets to the top of {__file__}: {targets}')

  target_cache.Store(paths, test_targets)
  target_cache.Save()

  if len(test_targets) > 1:
    if run_all:
      print(f'Warning, found {len(test_targets)} test targets.',
            file=sys.stderr)
      if len(test_targets) > 10:
        ExitWithMessage('Your query likely involves non-test sources.')
      print('Trying to run all of them!', file=sys.stderr)
    else:
      test_targets = [HaveUserPickTarget(paths, test_targets)]

  test_targets = list(set([t.split(':')[-1] for t in test_targets]))

  return (test_targets, used_cache)


def RunTestTargets(out_dir, targets, gtest_filter, extra_args, dry_run,
                   no_try_android_wrappers, no_fast_local_dev):

  for target in targets:

    # Look for the Android wrapper script first.
    path = os.path.join(out_dir, 'bin', f'run_{target}')
    if no_try_android_wrappers or not os.path.isfile(path):
      # If the wrapper is not found or disabled use the Desktop target
      # which is an executable.
      path = os.path.join(out_dir, target)
    elif not no_fast_local_dev:
      # Usually want this flag when developing locally.
      extra_args = extra_args + ['--fast-local-dev']

    cmd = [path, f'--gtest_filter={gtest_filter}'] + extra_args
    print('Running test: ' + ' '.join(cmd))
    if not dry_run:
      StreamCommandOrExit(cmd)


def BuildCppTestFilter(filenames, line):
  make_filter_command = [
      sys.executable, SRC_DIR / 'tools' / 'make_gtest_filter.py'
  ]
  if line:
    make_filter_command += ['--line', str(line)]
  else:
    make_filter_command += ['--class-only']
  make_filter_command += filenames
  return RunCommand(make_filter_command).strip()


def BuildJavaTestFilter(filenames):
  return ':'.join('*.{}*'.format(os.path.splitext(os.path.basename(f))[0])
                  for f in filenames)


def BuildTestFilter(filenames, line):
  java_files = [f for f in filenames if f.endswith('.java')]
  cc_files = [f for f in filenames if f.endswith('.cc')]
  filters = []
  if java_files:
    filters.append(BuildJavaTestFilter(java_files))
  if cc_files:
    filters.append(BuildCppTestFilter(cc_files, line))

  return ':'.join(filters)


def main():
  parser = argparse.ArgumentParser(
      description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
  parser.add_argument('--out-dir',
                      '--output-directory',
                      '-C',
                      metavar='OUT_DIR',
                      help='output directory of the build')
  parser.add_argument(
      '--run-all',
      action='store_true',
      help='Run all tests for the file or directory, instead of just one')
  parser.add_argument('--line',
                      type=int,
                      help='run only the test on this line number. c++ only.')
  parser.add_argument(
      '--gtest_filter', '-f', metavar='FILTER', help='test filter')
  parser.add_argument(
      '--dry-run',
      '-n',
      action='store_true',
      help='Print ninja and test run commands without executing them.')
  parser.add_argument(
      '--no-try-android-wrappers',
      action='store_true',
      help='Do not try to use Android test wrappers to run tests.')
  parser.add_argument('--no-fast-local-dev',
                      action='store_true',
                      help='Do not add --fast-local-dev for Android tests.')
  parser.add_argument('file',
                      metavar='FILE_NAME',
                      help='test suite file (eg. FooTest.java)')

  args, _extras = parser.parse_known_args()

  if args.out_dir:
    constants.SetOutputDirectory(args.out_dir)
  constants.CheckOutputDirectory()
  out_dir: str = constants.GetOutDirectory()

  if not os.path.isdir(out_dir):
    parser.error(f'OUT_DIR "{out_dir}" does not exist.')
  target_cache = TargetCache(out_dir)
  filenames = FindMatchingTestFiles(args.file)

  targets, used_cache = FindTestTargets(target_cache, out_dir, filenames,
                                        args.run_all)

  gtest_filter = args.gtest_filter
  if not gtest_filter:
    gtest_filter = BuildTestFilter(filenames, args.line)

  if not gtest_filter:
    ExitWithMessage('Failed to derive a gtest filter')

  assert targets
  build_ok = BuildTestTargetsWithNinja(out_dir, targets, args.dry_run)

  # If we used the target cache, it's possible we chose the wrong target because
  # a gn file was changed. The build step above will check for gn modifications
  # and update build.ninja. Use this opportunity the verify the cache is still
  # valid.
  if used_cache and not target_cache.IsStillValid():
    target_cache = TargetCache(out_dir)
    new_targets, _ = FindTestTargets(target_cache, out_dir, filenames,
                                     args.run_all)
    if targets != new_targets:
      # Note that this can happen, for example, if you rename a test target.
      print('gn config was changed, trying to build again', file=sys.stderr)
      targets = new_targets
      build_ok = BuildTestTargetsWithNinja(out_dir, targets, args.dry_run)

  if not build_ok: sys.exit(1)

  RunTestTargets(out_dir, targets, gtest_filter, _extras, args.dry_run,
                 args.no_try_android_wrappers, args.no_fast_local_dev)


if __name__ == '__main__':
  sys.exit(main())
