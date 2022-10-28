#!/usr/bin/env python
#
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Deploys Fuchsia packages to a package repository in a Fuchsia
build output directory."""

import pkg_repo
import argparse
import os
import sys



def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--package',
                      action='append',
                      required=True,
                      help='Paths to packages to install.')
  parser.add_argument('--fuchsia-out-dir',
                      required=True,
                      help='Path to a Fuchsia build output directory. '
                      'Setting the GN arg '
                      '"default_fuchsia_build_dir_for_installation" '
                      'will cause it to be passed here.')
  args, _ = parser.parse_known_args()
  assert args.package

  fuchsia_out_dir = os.path.expanduser(args.fuchsia_out_dir)
  repo = pkg_repo.ExternalPkgRepo(os.path.join(fuchsia_out_dir, 'amber-files'),
                                  os.path.join(fuchsia_out_dir, '.build-id'))
  print('Installing packages and symbols in package repo %s...' %
        repo.GetPath())

  for package in args.package:
    repo.PublishPackage(package)

  print('Installation success.')

  return 0


if __name__ == '__main__':
  sys.exit(main())
