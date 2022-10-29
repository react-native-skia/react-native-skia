#!/usr/bin/env vpython3
# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sys
from pathlib import Path

_HERE_DIR = Path(__file__).parent
_SOURCE_MAP_CREATOR = (_HERE_DIR / 'create_js_source_maps.js').resolve()

_NODE_PATH = (_HERE_DIR.parent.parent.parent / 'third_party' / 'node').resolve()
sys.path.append(str(_NODE_PATH))
import node


def main(argv):
  parser = argparse.ArgumentParser()
  parser.add_argument('--originals', required=True, nargs="*")
  parser.add_argument('--sources', required=True, nargs="*")
  parser.add_argument('--outputs', required=True, nargs="*")
  parser.add_argument('--inline-sourcemaps', action='store_true')
  args = parser.parse_args(argv)

  for original_file, source_file, output_file in zip(args.originals,
                                                     args.sources,
                                                     args.outputs):
    # Invokes "node create_js_source_maps.js (args)""
    # We can't use third_party/node/node.py directly from the gni template
    # because we don't have a good way to specify the path to
    # create_js_source_maps.js in a gni template.
    # TODO(crbug.com/1337530): Launching node is expensive. Instead, update the
    # JS script to accept multiple input/output files and do only one node run.
    node.RunNode(
        [str(_SOURCE_MAP_CREATOR), original_file, source_file, output_file] +
        (['--inline-sourcemaps'] if args.inline_sourcemaps else []))


if __name__ == '__main__':
  main(sys.argv[1:])
