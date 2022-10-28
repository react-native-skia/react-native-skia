#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script is used by chrome_tests.gypi's js2webui action to maintain the
argument lists and to generate inlinable tests.
"""

from __future__ import print_function

import json
import argparse
import os
import subprocess
import sys
import shutil


def HasSameContent(filename, content):
  '''Returns true if the given file is readable and has the given content.'''
  try:
    with open(filename, 'rb') as file:
      return file.read() == content
  except:
    # Ignore all errors and fall back on a safe bet.
    return False


def main ():
  parser = argparse.ArgumentParser()
  parser.add_argument('v8_shell')
  parser.add_argument('test_api_js')
  parser.add_argument('js2webui_js')
  parser.add_argument('test_type')
  parser.add_argument('inputfile')
  parser.add_argument('srcrootdir')
  parser.add_argument('cxxoutfile')
  parser.add_argument('jsoutfile')
  parser.add_argument('-v', '--verbose', action='store_true')
  parser.add_argument('-n',
                      '--impotent',
                      action='store_true',
                      help="don't execute; just print (as if verbose)")
  parser.add_argument('--deps_js',
                      action="store",
                      help=("Path to deps.js for dependency resolution, " +
                            "optional."))
  parser.add_argument('--external',
                      action='store',
                      help="Load V8's initial snapshot from " +
                      "external files (y/n)")
  args = parser.parse_args()

  cmd = [args.v8_shell]
  arguments = [
      args.js2webui_js, args.inputfile, args.srcrootdir, args.deps_js,
      args.cxxoutfile, args.test_type
  ]
  cmd.extend([
      '-e', "arguments=" + json.dumps(arguments), args.test_api_js,
      args.js2webui_js
  ])
  if args.verbose or args.impotent:
    print(cmd)
  if not args.impotent:
    try:
      p = subprocess.Popen(
          cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
      out, err = p.communicate()
      if p.returncode != 0:
        sys.stderr.write((out + err).decode('utf-8'))
        return 1
      if not HasSameContent(args.cxxoutfile, out):
        with open(args.cxxoutfile, 'wb') as f:
          f.write(out)
      shutil.copyfile(args.inputfile, args.jsoutfile)
    except Exception as ex:
      if os.path.exists(args.cxxoutfile):
        os.remove(args.cxxoutfile)
      if os.path.exists(args.jsoutfile):
        os.remove(args.jsoutfile)
      raise


if __name__ == '__main__':
  sys.exit(main())
