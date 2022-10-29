#!/usr/bin/env python3
# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Prints android-binary-size result for a given commit or commit range."""

import argparse
import collections
import concurrent.futures
import csv
import json
import os
import posixpath
import re
import subprocess
import sys

# Max number of commits to show when given a range and no -n parameter.
_COMMIT_LIMIT = 200

# Commit ranges where size bot was giving invalid results.
_BAD_COMMIT_RANGES = [
    range(1045024, 1045552),  # https://crbug.com/1361952
]

_COMMIT_RE = re.compile(r'^commit (?:(?!^commit).)*', re.DOTALL | re.MULTILINE)

_MAIN_FIELDS_RE = re.compile(
    r'^commit (\S+).*?'
    r'^Date:\s+(.*?)$.*?'
    r'^    (\S.*?)$', re.DOTALL | re.MULTILINE)

_REVIEW_RE = re.compile(r'^    Reviewed-on: (\S+)', re.MULTILINE)
_CRREV_RE = re.compile(r'^    Cr-Commit-Position:.*?(\d+)', re.MULTILINE)
_GERRIT_RE = re.compile(r'https://([^/]+)/c/(.*?)/\+/(\d+)')

_CommitInfo = collections.namedtuple(
    '_CommitInfo', 'git_hash date subject review_url cr_position')


def _parse_commit(text):
  git_hash, date, subject = _MAIN_FIELDS_RE.match(text).groups()
  review_url = ([''] + _REVIEW_RE.findall(text))[-1]
  cr_position = int((['0'] + _CRREV_RE.findall(text))[-1])
  return _CommitInfo(git_hash, date, subject, review_url, cr_position)


def _git_log(git_log_args):
  cmd = ['git', 'log']

  if len(git_log_args) == 1 and '..' not in git_log_args[0]:
    # Single commit rather than commit range.
    cmd += ['-n1']
  elif not any(x.startswith('-n') for x in git_log_args):
    # Ensure there's a limit on number of commits.
    cmd += [f'-n{_COMMIT_LIMIT}']

  cmd += git_log_args

  log_output = subprocess.check_output(cmd, encoding='utf8')
  ret = [_parse_commit(x) for x in _COMMIT_RE.findall(log_output)]

  if len(ret) == _COMMIT_LIMIT:
    sys.stderr.write(
        f'Limiting to {_COMMIT_LIMIT} commits. Use -n## to override\n')
  return ret


def _query_size(review_url, internal):
  if not review_url:
    return '<missing>'
  m = _GERRIT_RE.match(review_url)
  if not m:
    return '<bad URL>'
  host, project, change_num = m.groups()
  if internal:
    project = 'chrome'
    builder = 'android-internal-binary-size'
  else:
    project = 'chromium'
    builder = 'android-binary-size'

  cmd = ['bb', 'ls', '-json', '-p']
  # Request results for all patchsets, assuming fewer than 30.
  for patchset in range(1, 30):
    cmd += [
        '-predicate',
        """{
"builder":{"project":"%s","bucket":"try","builder":"%s"},
"gerrit_changes":[{
    "host":"%s","project":"%s",
    "change":"%s","patchset":"%d"}
]}""" % (project, builder, host, project, change_num, patchset)
    ]
  result = subprocess.run(cmd,
                          check=False,
                          stdout=subprocess.PIPE,
                          encoding='utf8')
  if result.returncode:
    return '<missing>'

  # Take the last one that has a size set (output is in reverse order already).
  for json_str in result.stdout.splitlines():
    try:
      obj = json.loads(json_str)
    except json.JSONDecodeError:
      sys.stderr.write(f'Problem JSON:\n{json_str}\n')
      sys.exit(1)

    properties = obj.get('output', {}).get('properties', {})
    listings = properties.get('binary_size_plugin', {}).get('listings', [])
    for listing in listings:
      if listing['name'] == 'Android Binary Size':
        return listing['delta']
  return '<unknown>'


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--csv', action='store_true', help='Print as CSV')
  parser.add_argument('--internal',
                      action='store_true',
                      help='Query android-internal-binary-size (Googlers only)')
  args, git_log_args = parser.parse_known_args()

  # Ensure user has authenticated.
  result = subprocess.run(['bb', 'auth-info'],
                          check=False,
                          stdout=subprocess.DEVNULL)
  if result.returncode:
    sys.stderr.write('First run: bb auth-login\n')
    sys.exit(1)

  commit_infos = _git_log(git_log_args)
  if not commit_infos:
    sys.stderr.write('Did not find any commits.\n')
    sys.exit(1)

  print(f'Fetching bot results for {len(commit_infos)} commits...')

  if args.csv:
    print_func = csv.writer(sys.stdout).writerow
  else:
    print_func = lambda v: print('{:<12}{:14}{:12}{:32}{}'.format(*v))

  print_func(('Commit #', 'Git Hash', 'Size', 'Date', 'Subject'))
  num_bad_commits = 0
  with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
    sizes = [
        pool.submit(_query_size, info.review_url, args.internal)
        for info in commit_infos
    ]
    for info, size in zip(commit_infos, sizes):
      if any(info.cr_position in r for r in _BAD_COMMIT_RANGES):
        num_bad_commits += 1
      size_str = size.result().replace(' bytes', '').lstrip('+')
      crrev_str = info.cr_position or ''
      print_func(
          (crrev_str, info.git_hash[:12], size_str, info.date, info.subject))
  if num_bad_commits:
    print(f'Includes {num_bad_commits} commits from known bad revision range.')


if __name__ == '__main__':
  main()
