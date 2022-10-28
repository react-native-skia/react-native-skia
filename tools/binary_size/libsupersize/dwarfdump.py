#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Runs dwarfdump on passed-in .so."""

import bisect
import dataclasses
import subprocess
import typing

import path_util


@dataclasses.dataclass(order=True)
class _AddressRange:
  start: int
  stop: int


class _SourceMapper:
  def __init__(self, range_info_list):
    self._range_info_list = range_info_list
    self._largest_address = 0
    self._unmatched_queries_count = 0
    self._total_queries_count = 0

    if self._range_info_list:
      self._largest_address = self._range_info_list[-1][0].stop

  def FindSourceForTextAddress(self, address):
    """Returns source file path matching passed-in symbol address.

    Only symbols in the .text section of the elf file are supported.
    """
    self._total_queries_count += 1
    # Bisect against stop = self._largest_address + 1 to avoid bisecting against
    # the "source path" tuple component.
    bisect_index = bisect.bisect_right(
        self._range_info_list,
        (_AddressRange(address, self._largest_address + 1), '')) - 1
    if bisect_index >= 0:
      info = self._range_info_list[bisect_index]
      if info[0].start <= address < info[0].stop:
        return info[1]

    self._unmatched_queries_count += 1
    return None

  @property
  def unmatched_queries_ratio(self):
    return self._unmatched_queries_count / self._total_queries_count


def CreateAddressSourceMapper(elf_path, tool_prefix):
  """Runs dwarfdump. Returns object for querying source path given address."""
  return _SourceMapper(_Parse(elf_path, tool_prefix))


def CreateAddressSourceMapperForTest(lines):
  return _SourceMapper(_ParseDumpOutput(lines))


def ParseDumpOutputForTest(lines):
  return _ParseDumpOutput(lines)


def _Parse(elf_path, tool_prefix):
  cmd = [
      path_util.GetDwarfdumpPath(tool_prefix),
      elf_path,
      '--debug-info',
      '--summarize-types',
      '--recurse-depth=0',
  ]
  stdout = subprocess.check_output(cmd,
                                   stderr=subprocess.DEVNULL,
                                   encoding='utf-8')
  return _ParseDumpOutput(stdout.splitlines())


def _ParseDumpOutput(lines):
  """Parses passed-in dwarfdump stdout."""

  # List of (_AddressRange, source path) tuples.
  range_info_list = []

  line_it = iter(lines)
  line = next(line_it, None)
  while line is not None:
    if 'DW_TAG_compile_unit' not in line:
      line = next(line_it, None)
      continue

    line, address_ranges, source_path = _ParseCompileUnit(line_it)
    if source_path and address_ranges:
      for address_range in address_ranges:
        range_info_list.append((address_range, source_path))

  return sorted(range_info_list)


def _ParseCompileUnit(line_it):
  """Parses DW_AT_compile_unit block.

  Example:
  0x000026: DW_AT_compile_unit
              DW_AT_low_pc   (0x02f)
              DW_AT_high_pc  (0x03f)
              DW_AT_name     (foo.cc)
  """
  source_path = None
  single_range = _AddressRange(0, 0)
  range_addresses = []

  while True:
    line = next(line_it, None)

    dw_index = 0 if line is None else line.find('DW_')
    if dw_index < 0:
      continue

    if line is None or line.startswith('DW_TAG', dw_index):
      if range_addresses:
        # If compile unit specifies both DW_AT_ranges and DW_AT_low_pc,
        # DW_AT_low_pc is base offset. Base offset is currently unsupported.
        assert single_range.start == 0
      elif single_range.start > 0:
        range_addresses.append(single_range)
      return (line, range_addresses, source_path)

    if line.startswith('DW_AT_low_pc', dw_index):
      single_range.start = int(_ExtractDwValue(line), 16)
      if single_range.stop == 0:
        single_range.stop = single_range.start + 1
      continue
    if line.startswith('DW_AT_high_pc', dw_index):
      single_range.stop = int(_ExtractDwValue(line), 16)
      continue
    if line.startswith('DW_AT_name', dw_index):
      source_path = _ExtractDwValue(line)
      continue

    if line.startswith('DW_AT_ranges', dw_index):
      range_addresses = _ParseRanges(line_it)


def _ParseRanges(line_it):
  """Parses DW_AT_ranges from dwarfdump stdout.

  Example:
  [0x1, 0x2)
  [0x5, 0x10))
  """
  range_addresses = []

  line = next(line_it, None)
  while line is not None:
    num_opening_brackets = line.count('(') + line.count('[')
    num_closing_brackets = line.count(')') + line.count(']')

    tokens = line.strip('([]) \t').split(',')
    if len(tokens) == 2:
      start_address = int(tokens[0], 16)
      end_address = int(tokens[1], 16)
      # Dwarf spec does not assign special meaning to empty ranges.
      if start_address != end_address:
        range_addresses.append(_AddressRange(start_address, end_address))

    if num_closing_brackets > num_opening_brackets:
      break
    line = next(line_it, None)

  return range_addresses


def _ExtractDwValue(line):
  """Extract DW_AT_ value from dwarfdump stdout.

  Examples:
  DW_AT_name ("foo.cc")
  DW_AT_decl_line (177)
  DW_AT_low_pc (0x2)
  """
  lparen_index = line.rfind('(')
  if lparen_index < 0:
    return None
  rparen_index = line.find(')', lparen_index + 1)
  if rparen_index < 0:
    return None
  if (lparen_index < rparen_index - 2 and line[lparen_index + 1] == '"'
      and line[rparen_index - 1] == '"'):
    lparen_index += 1
    rparen_index -= 1
  return line[lparen_index + 1:rparen_index]
