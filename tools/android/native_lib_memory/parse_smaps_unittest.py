# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import parse_smaps


class ParseProcSmapTest(unittest.TestCase):
  def testSingleEntry(self):
    CONTENT = """7fff4fbfc000-7fff4fbfe000 r-xp 00000000 00:00 0      [vdso]
Size:                  8 kB
Rss:                   4 kB
Pss:                   0 kB
Shared_Clean:          4 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Referenced:            4 kB
Anonymous:             0 kB
AnonHugePages:         0 kB
ShmemPmdMapped:        0 kB
Shared_Hugetlb:        0 kB
Private_Hugetlb:       0 kB
Swap:                  0 kB
SwapPss:               0 kB
KernelPageSize:        4 kB
MMUPageSize:           4 kB
Locked:                0 kB
VmFlags: rd ex mr mw me de sd""".split('\n')
    mappings = parse_smaps._ParseProcSmapsLines(CONTENT)
    self.assertEquals(1, len(mappings))
    self.assertEquals(0x7fff4fbfc000, mappings[0].start)
    self.assertEquals(0x7fff4fbfe000, mappings[0].end)
    self.assertEquals('r-xp', mappings[0].permissions)
    self.assertEquals(0, mappings[0].offset)
    self.assertEquals('[vdso]', mappings[0].pathname)
    self.assertEquals(
        ['Size',
         'Rss',
         'Pss',
         'Shared_Clean',
         'Shared_Dirty',
         'Private_Clean',
         'Private_Dirty',
         'Referenced',
         'Anonymous',
         'AnonHugePages',
         'ShmemPmdMapped',
         'Shared_Hugetlb',
         'Private_Hugetlb',
         'Swap',
         'SwapPss',
         'KernelPageSize',
         'MMUPageSize',
         'Locked',
         'VmFlags'], mappings[0].fields.keys())
    self.assertEquals(8, mappings[0].fields['Size'])
    self.assertEquals('rd ex mr mw me de sd', mappings[0].fields['VmFlags'])

  def testSeveralEntries(self):
    CONTENT = """7fff4fbfc000-7fff4fbfe000 r-xp 00000000 00:00 0      [vdso]
Size:                  8 kB
VmFlags: rd ex mr mw me de sd
ffffffffff600000-ffffffffff601000 r-xp 00000010 00:00 0      /tmp/foo
Size:                  192 kB
VmFlags: rd ex""".split('\n')
    mappings = parse_smaps._ParseProcSmapsLines(CONTENT)
    self.assertEquals(2, len(mappings))
    self.assertEquals(0x7fff4fbfc000, mappings[0].start)
    self.assertEquals(0x7fff4fbfe000, mappings[0].end)
    self.assertEquals('r-xp', mappings[0].permissions)
    self.assertEquals(0, mappings[0].offset)
    self.assertEquals('[vdso]', mappings[0].pathname)
    self.assertEquals(['Size', 'VmFlags'], mappings[0].fields.keys())
    self.assertEquals(8, mappings[0].fields['Size'])
    self.assertEquals('rd ex mr mw me de sd', mappings[0].fields['VmFlags'])

    self.assertEquals(0xffffffffff600000, mappings[1].start)
    self.assertEquals(0xffffffffff601000, mappings[1].end)
    self.assertEquals('r-xp', mappings[1].permissions)
    self.assertEquals(0x10, mappings[1].offset)
    self.assertEquals('/tmp/foo', mappings[1].pathname)
    self.assertEquals(['Size', 'VmFlags'], mappings[1].fields.keys())
    self.assertEquals(192, mappings[1].fields['Size'])
    self.assertEquals('rd ex', mappings[1].fields['VmFlags'])

  def testToString(self):
    CONTENT = """7fff4fbfc000-7fff4fbfe000 r-xp 00000010 00:00 0      [vdso]
Size:                  8 kB
VmFlags: rd ex mr mw me de sd""".split('\n')
    mappings = parse_smaps._ParseProcSmapsLines(CONTENT)
    self.assertEquals(1, len(mappings))
    self.assertEquals("""7fff4fbfc000-7fff4fbfe000 r-xp 10 [vdso]
Size: 8 kB
VmFlags: rd ex mr mw me de sd""", mappings[0].ToString())


if __name__ == '__main__':
  unittest.main()
