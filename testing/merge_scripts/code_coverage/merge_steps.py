#!/usr/bin/python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script merges code coverage profiles from multiple steps."""

import argparse
import os
import sys

import merge_lib as merger


def _merge_steps_argument_parser(*args, **kwargs):
  parser = argparse.ArgumentParser(*args, **kwargs)
  parser.add_argument('--input-dir', required=True, help=argparse.SUPPRESS)
  parser.add_argument(
      '--output-file', required=True, help='where to store the merged data')
  parser.add_argument(
      '--llvm-profdata', required=True, help='path to llvm-profdata executable')
  parser.add_argument(
      '--profdata-filename-pattern',
      default='.*',
      help='regex pattern of profdata filename to merge for current test type. '
          'If not present, all profdata files will be merged.')
  # TODO(crbug.com/1077304) - migrate this to sparse=False as default, and have
  # --sparse to set sparse
  parser.add_argument(
      '--no-sparse',
      action='store_false',
      dest='sparse',
      help='run llvm-profdata without the sparse flag.')
  # TODO(crbug.com/1077304) - The intended behaviour is to default sparse to
  # false. --no-sparse above was added as a workaround, and will be removed.
  # This is being introduced now in support of the migration to intended
  # behavior. Ordering of args matters here, as the default is set by the former
  # (sparse defaults to False because of ordering. See merge_results unit tests
  # for details)
  parser.add_argument(
      '--sparse',
      action='store_true',
      dest='sparse',
      help='run llvm-profdata with the sparse flag.')
  return parser


def main():
  desc = "Merge profdata files in <--input-dir> into a single profdata."
  parser = _merge_steps_argument_parser(description=desc)
  params = parser.parse_args()
  merger.merge_profiles(params.input_dir, params.output_file, '.profdata',
                        params.llvm_profdata, params.profdata_filename_pattern,
                        sparse=params.sparse)


if __name__ == '__main__':
  sys.exit(main())
