#!/usr/bin/env python
# Copyright 2017 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

import model

sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'common'))
import presubmit_util

def main(argv):
  dirname = os.path.dirname(os.path.realpath(__file__))
  xml = dirname + '/structured.xml'
  old_xml = dirname + '/structured.old.xml'
  presubmit_util.DoPresubmitMain(argv, xml,
                                 old_xml, lambda x: repr(model.Model(x)))


if '__main__' == __name__:
  sys.exit(main(sys.argv))
