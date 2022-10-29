#!/usr/bin/env python3
# Copyright 2012 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Dump functions called by static intializers in a Linux Release binary.

Usage example:
  tools/linux/dump-static-intializers.py out/Release/chrome

A brief overview of static initialization:
1) the compiler writes out, per object file, a function that contains
   the static intializers for that file.
2) the compiler also writes out a pointer to that function in a special
   section.
3) at link time, the linker concatenates the function pointer sections
   into a single list of all initializers.
4) at run time, on startup the binary runs all function pointers.

The functions in (1) all have mangled names of the form
  _GLOBAL__I_foobar.cc or __cxx_global_var_initN
using objdump, we can disassemble those functions and dump all symbols that
they reference.
"""

# Needed so pylint does not complain about print('', end='').
from __future__ import print_function

import optparse
import os
import re
import subprocess
import sys

# A map of symbol => informative text about it.
NOTES = {
  '__cxa_atexit@plt': 'registers a dtor to run at exit',
  'std::__ioinit': '#includes <iostream>, use <ostream> instead',
}

# Determine whether this is a git checkout (as opposed to e.g. svn).
IS_GIT_WORKSPACE = (subprocess.Popen(
    ['git', 'rev-parse'], stderr=subprocess.PIPE).wait() == 0)


class Demangler:
  """A wrapper around c++filt to provide a function to demangle symbols."""

  def __init__(self, toolchain):
    # llvm toolchain uses cxx rather than c++.
    path = toolchain + 'cxxfilt'
    if not os.path.exists(path):
      path = toolchain + 'c++filt'
    if not os.path.exists(path):
      # Android currently has an issue where the llvm toolchain in the ndk does
      # not contain c++filt. Hopefully fixed in next NDK update...
      path = 'c++filt'
    self.cppfilt = subprocess.Popen([path],
                                    stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE,
                                    universal_newlines=True)

  def Demangle(self, sym):
    """Given mangled symbol |sym|, return its demangled form."""
    self.cppfilt.stdin.write(sym + '\n')
    self.cppfilt.stdin.flush()
    return self.cppfilt.stdout.readline().strip()


# Matches for example: "cert_logger.pb.cc", capturing "cert_logger".
protobuf_filename_re = re.compile(r'(.*)\.pb\.cc$')
def QualifyFilenameAsProto(filename):
  """Attempt to qualify a bare |filename| with a src-relative path, assuming it
  is a protoc-generated file.  If a single match is found, it is returned.
  Otherwise the original filename is returned."""
  if not IS_GIT_WORKSPACE:
    return filename
  match = protobuf_filename_re.match(filename)
  if not match:
    return filename
  basename = match.groups(0)
  cmd = ['git', 'ls-files', '--', '*/%s.proto' % basename]
  gitlsfiles = subprocess.Popen(cmd,
                                stdout=subprocess.PIPE,
                                universal_newlines=True)
  candidate = filename
  for line in gitlsfiles.stdout:
    if candidate != filename:
      return filename # Multiple hits, can't help.
    candidate = line.strip()
  return candidate


# Regex matching the substring of a symbol's demangled text representation most
# likely to appear in a source file.
# Example: "v8::internal::Builtins::InitBuiltinFunctionTable()" becomes
# "InitBuiltinFunctionTable", since the first (optional & non-capturing) group
# picks up any ::-qualification and the last fragment picks up a suffix that
# starts with an opener.
symbol_code_name_re = re.compile(r'^(?:[^(<[]*::)?([^:(<[]*).*?$')
def QualifyFilename(filename, symbol):
  """Given a bare filename and a symbol that occurs in it, attempt to qualify
  it with a src-relative path.  If more than one file matches, return the
  original filename."""
  if not IS_GIT_WORKSPACE:
    return filename
  match = symbol_code_name_re.match(symbol)
  if not match:
    return filename
  symbol = match.group(1)
  cmd = ['git', 'grep', '-l', symbol, '--', '*/' + filename]
  gitgrep = subprocess.Popen(cmd,
                             stdout=subprocess.PIPE,
                             universal_newlines=True)
  candidate = filename
  for line in gitgrep.stdout:
    if candidate != filename:  # More than one candidate; return bare filename.
      return filename
    candidate = line.strip()
  return candidate


# Regex matching nm output for the symbols we're interested in. The two formats
# we are interested in are _GLOBAL__sub_I_<filename> and _cxx_global_var_initN.
# See test_ParseNmLine for examples.
nm_re = re.compile(
    r'''(\S+)\s(\S+)\st\s                # Symbol start address and size
        (
          (?:_ZN12)?_GLOBAL__(?:sub_)?I_ # Pattern with filename
        |
          __cxx_global_var_init\d*       # Pattern without filename
        )(.*)                            # capture the filename''',
    re.X)
def ParseNmLine(line):
  """Parse static initializers from a line of nm output.

  Given a line of nm output, parse static initializers as a
  (file, start, size, symbol) tuple."""
  match = nm_re.match(line)
  if match:
    addr, size, prefix, filename = match.groups()
    return (filename, int(addr, 16), int(size, 16), prefix+filename)
  return None


def test_ParseNmLine():
  """Verify the nm_re regex matches some sample lines."""
  parse = ParseNmLine(
    '0000000001919920 0000000000000008 t '
    '_ZN12_GLOBAL__I_safe_browsing_service.cc')
  assert parse == ('safe_browsing_service.cc', 26319136, 8,
                   '_ZN12_GLOBAL__I_safe_browsing_service.cc'), parse

  parse = ParseNmLine(
    '00000000026b9eb0 0000000000000024 t '
    '_GLOBAL__sub_I_extension_specifics.pb.cc')
  assert parse == ('extension_specifics.pb.cc', 40607408, 36,
                   '_GLOBAL__sub_I_extension_specifics.pb.cc'), parse

  parse = ParseNmLine(
    '0000000002e75a60 0000000000000016 t __cxx_global_var_init')
  assert parse == ('', 48716384, 22, '__cxx_global_var_init'), parse

  parse = ParseNmLine(
    '0000000002e75a60 0000000000000016 t __cxx_global_var_init89')
  assert parse == ('', 48716384, 22, '__cxx_global_var_init89'), parse


# Just always run the test; it is fast enough.
test_ParseNmLine()


def ParseNm(toolchain, binary):
  """Yield static initializers for the given binary.

  Given a binary, yield static initializers as (file, start, size, symbol)
  tuples."""
  nm = subprocess.Popen([toolchain + 'nm', '-S', binary],
                        stdout=subprocess.PIPE,
                        universal_newlines=True)
  for line in nm.stdout:
    parse = ParseNmLine(line)
    if parse:
      yield parse


# Regex matching objdump output for the symbols we're interested in.
# Example line:
#     12354ab:  (disassembly, including <FunctionReference>)
disassembly_re = re.compile(r'^\s+[0-9a-f]+:.*<(\S+)>')
def ExtractSymbolReferences(toolchain, binary, start, end, symbol):
  """Given a span of addresses, returns symbol references from disassembly."""
  cmd = [toolchain + 'objdump', binary, '--disassemble',
         '--start-address=0x%x' % start, '--stop-address=0x%x' % end]
  objdump = subprocess.Popen(cmd,
                             stdout=subprocess.PIPE,
                             universal_newlines=True)

  refs = set()
  for line in objdump.stdout:
    if '__static_initialization_and_destruction' in line:
      raise RuntimeError('code mentions '
                         '__static_initialization_and_destruction; '
                         'did you accidentally run this on a Debug binary?')
    match = disassembly_re.search(line)
    if match:
      (ref,) = match.groups()
      if ref.startswith('.LC') or ref.startswith('_DYNAMIC'):
        # Ignore these, they are uninformative.
        continue
      if re.match(symbol, ref):
        # Probably a relative jump within this function.
        continue
      refs.add(ref)

  return sorted(refs)


def main():
  parser = optparse.OptionParser(usage='%prog [option] filename')
  parser.add_option('-d', '--diffable', dest='diffable',
                    action='store_true', default=False,
                    help='Prints the filename on each line, for more easily '
                         'diff-able output. (Used by sizes.py)')
  parser.add_option('-t', '--toolchain-prefix', dest='toolchain',
                    action='store', default='',
                    help='Toolchain prefix to append to all tool invocations '
                         '(nm, objdump).')
  opts, args = parser.parse_args()
  if len(args) != 1:
    parser.error('missing filename argument')
    return 1
  binary = args[0]

  demangler = Demangler(opts.toolchain)
  file_count = 0
  initializer_count = 0

  files = ParseNm(opts.toolchain, binary)
  if opts.diffable:
    files = sorted(files)
  for filename, addr, size, symbol in files:
    file_count += 1
    ref_output = []

    qualified_filename = QualifyFilenameAsProto(filename)

    if size == 2:
      # gcc generates a two-byte 'repz retq' initializer when there is a
      # ctor even when the ctor is empty.  This is fixed in gcc 4.6, but
      # Android uses gcc 4.4.
      ref_output.append('[empty ctor, but it still has cost on gcc <4.6]')
    else:
      for ref in ExtractSymbolReferences(opts.toolchain, binary, addr,
                                         addr+size, symbol):
        initializer_count += 1

        ref = demangler.Demangle(ref)
        if qualified_filename == filename:
          qualified_filename = QualifyFilename(filename, ref)

        note = ''
        if ref in NOTES:
          note = NOTES[ref]
        elif ref.endswith('_2eproto()'):
          note = 'protocol compiler bug: crbug.com/105626'

        if note:
          ref_output.append('%s [%s]' % (ref, note))
        else:
          ref_output.append(ref)

    if opts.diffable:
      if ref_output:
        print('\n'.join(
            '# ' + qualified_filename + ' ' + r for r in ref_output))
      else:
        print('# %s: (empty initializer list)' % qualified_filename)
    else:
      print('%s (initializer offset 0x%x size 0x%x)' % (qualified_filename,
                                                        addr, size))
      print(''.join('  %s\n' % r for r in ref_output))

  if opts.diffable:
    print('#', end=' ')
  print('Found %d static initializers in %d files.' % (initializer_count,
                                                       file_count))

  return 0


if '__main__' == __name__:
  sys.exit(main())
