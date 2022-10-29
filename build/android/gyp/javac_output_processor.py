#!/usr/bin/env python3
#
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Contains helper class for processing javac output."""

import os
import pathlib
import re
import sys
import traceback

from util import build_utils

sys.path.insert(
    0,
    os.path.join(build_utils.DIR_SOURCE_ROOT, 'third_party', 'colorama', 'src'))
import colorama
sys.path.insert(
    0,
    os.path.join(build_utils.DIR_SOURCE_ROOT, 'tools', 'android',
                 'modularization', 'convenience'))
import lookup_dep


class JavacOutputProcessor:
  def __init__(self, target_name):
    self._target_name = self._RemoveSuffixesIfPresent(
        ["__compile_java", "__errorprone", "__header"], target_name)
    self._suggested_deps = set()

    # Example: ../../ui/android/java/src/org/chromium/ui/base/Clipboard.java:45:
    fileline_prefix = (
        r'(?P<fileline>(?P<file>[-.\w/\\]+.java):(?P<line>[0-9]+):)')

    self._warning_re = re.compile(
        fileline_prefix + r'(?P<full_message> warning: (?P<message>.*))$')
    self._error_re = re.compile(fileline_prefix +
                                r'(?P<full_message> (?P<message>.*))$')
    self._marker_re = re.compile(r'\s*(?P<marker>\^)\s*$')

    self._symbol_not_found_re_list = [
        # Example:
        # error: package org.chromium.components.url_formatter does not exist
        re.compile(fileline_prefix +
                   r'( error: package [\w.]+ does not exist)$'),
        # Example: error: cannot find symbol
        re.compile(fileline_prefix + r'( error: cannot find symbol)$'),
        # Example: error: symbol not found org.chromium.url.GURL
        re.compile(fileline_prefix + r'( error: symbol not found [\w.]+)$'),
    ]

    # Example: import org.chromium.url.GURL;
    self._import_re = re.compile(r'\s*import (?P<imported_class>[\w\.]+);$')

    self._warning_color = [
        'full_message', colorama.Fore.YELLOW + colorama.Style.DIM
    ]
    self._error_color = [
        'full_message', colorama.Fore.MAGENTA + colorama.Style.BRIGHT
    ]
    self._marker_color = ['marker', colorama.Fore.BLUE + colorama.Style.BRIGHT]

    self._class_lookup_index = None

    colorama.init()

  def Process(self, lines):
    """ Processes javac output.

      - Applies colors to output.
      - Suggests GN dep to add for 'unresolved symbol in Java import' errors.
      """
    lines = self._ElaborateLinesForUnknownSymbol(iter(lines))
    for line in lines:
      yield self._ApplyColors(line)
    if self._suggested_deps:

      def yellow(text):
        return colorama.Fore.YELLOW + text + colorama.Fore.RESET

      # Show them in quotes so they can be copy/pasted into BUILD.gn files.
      yield yellow('Hint:') + ' One or more errors due to missing GN deps.'
      yield (yellow('Hint:') + ' Try adding the following to ' +
             yellow(self._target_name))
      for dep in sorted(self._suggested_deps):
        yield '    "{}",'.format(dep)

  def _ElaborateLinesForUnknownSymbol(self, lines):
    """ Elaborates passed-in javac output for unresolved symbols.

    Looks for unresolved symbols in imports.
    Adds:
    - Line with GN target which cannot compile.
    - Mention of unresolved class if not present in error message.
    - Line with suggestion of GN dep to add.

    Args:
      lines: Generator with javac input.
    Returns:
      Generator with processed output.
    """
    previous_line = next(lines, None)
    line = next(lines, None)
    while previous_line != None:
      try:
        self._LookForUnknownSymbol(previous_line, line)
      except Exception:
        elaborated_lines = ['Error in _LookForUnknownSymbol ---']
        elaborated_lines += traceback.format_exc().splitlines()
        elaborated_lines += ['--- end _LookForUnknownSymbol error']
        for elaborated_line in elaborated_lines:
          yield elaborated_line

      yield previous_line
      previous_line = line
      line = next(lines, None)

  def _ApplyColors(self, line):
    """Adds colors to passed-in line and returns processed line."""
    if self._warning_re.match(line):
      line = self._Colorize(line, self._warning_re, self._warning_color)
    elif self._error_re.match(line):
      line = self._Colorize(line, self._error_re, self._error_color)
    elif self._marker_re.match(line):
      line = self._Colorize(line, self._marker_re, self._marker_color)
    return line

  def _LookForUnknownSymbol(self, line, next_line):
    if not next_line:
      return

    import_re_match = self._import_re.match(next_line)
    if not import_re_match:
      return

    for regex in self._symbol_not_found_re_list:
      if regex.match(line):
        break
    else:
      return

    if self._class_lookup_index is None:
      self._class_lookup_index = lookup_dep.ClassLookupIndex(
          pathlib.Path(os.getcwd()),
          should_build=False,
      )

    class_to_lookup = import_re_match.group('imported_class')
    suggested_deps = self._class_lookup_index.match(class_to_lookup)

    if not suggested_deps:
      return

    suggested_deps = self._DisambiguateDeps(suggested_deps)
    suggested_deps_str = ', '.join(s.target for s in suggested_deps)

    if len(suggested_deps) > 1:
      suggested_deps_str = 'one of: ' + suggested_deps_str

    self._suggested_deps.add(suggested_deps_str)

  @staticmethod
  def _DisambiguateDeps(class_entries):
    if len(class_entries) == 1:
      return class_entries

    # android_library_factory() targets set low_classpath_priority=true, and any
    # target that is the "impl" side of a target that uses jar_excluded_patterns
    # should use this as well.
    # We should generally always suggest depending on the non-impl library
    # target.
    # TODO(crbug.com/1296711): Also use "visibility" a hint here.
    low_entries = [x for x in class_entries if x.low_classpath_priority]
    class_entries = low_entries or class_entries

    # E.g. javax_annotation_jsr250_api_java.
    jsr_entries = [x for x in class_entries if 'jsr' in x.target]
    class_entries = jsr_entries or class_entries
    return class_entries

  @staticmethod
  def _RemoveSuffixesIfPresent(suffixes, text):
    for suffix in suffixes:
      if text.endswith(suffix):
        return text[:-len(suffix)]
    return text

  @staticmethod
  def _Colorize(line, regex, color):
    match = regex.match(line)
    start = match.start(color[0])
    end = match.end(color[0])
    return (line[:start] + color[1] + line[start:end] + colorama.Fore.RESET +
            colorama.Style.RESET_ALL + line[end:])
