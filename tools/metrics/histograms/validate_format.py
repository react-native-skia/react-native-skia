#!/usr/bin/env python3
# Copyright 2013 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Verifies that the histograms XML file is well-formatted."""

import logging
import sys
import xml.dom.minidom

import extract_histograms
import histogram_paths
import merge_xml

# The allowlist of namespaces that are split across multiple files.
_NAMESPACES_IN_MULTIPLE_FILES = [
    'ash', 'chromeos', 'fcminvalidations', 'graphics', 'launch'
]


def CheckNamespaces():
  namespaces = {}
  has_errors = False
  # Iterate over HISTOGRAMS_XMLS rather than ALL_XMLS because it's fine for
  # histogram namespaces in obsolete_histograms.xml to also appear in
  # non-obsolete histograms.xml files.
  for path in histogram_paths.HISTOGRAMS_XMLS:
    tree = xml.dom.minidom.parse(path)

    def _GetNamespace(node):
      return node.getAttribute('name').lower().split('.')[0]

    namespaces_in_file = set(
        _GetNamespace(node) for node in extract_histograms.IterElementsWithTag(
            tree, 'histogram', depth=3))
    for namespace in namespaces_in_file:
      if (namespace in namespaces
          and namespace not in _NAMESPACES_IN_MULTIPLE_FILES):
        logging.error(
            'Namespace %s has already been used in %s. it\'s recommended to '
            'put histograms with the same namespace in the same file. If you '
            'intentionally want to split a namespace across multiple files, '
            'please add the namespace to the |_NAMESPACES_IN_MULTIPLE_FILES| '
            'in the validate_format.py.' % (namespace, namespaces[namespace]))
        has_errors = True
      namespaces[namespace] = path

  return has_errors


def main():
  doc = merge_xml.MergeFiles(histogram_paths.ALL_XMLS,
                             should_expand_owners=True)
  _, errors = extract_histograms.ExtractHistogramsFromDom(doc)
  errors = errors or CheckNamespaces()
  sys.exit(errors)

if __name__ == '__main__':
  main()
