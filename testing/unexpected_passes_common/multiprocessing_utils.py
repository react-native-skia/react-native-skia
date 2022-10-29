# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

from typing import Optional

from pathos import pools


def GetProcessPool(nodes: Optional[int] = None) -> pools.ProcessPool:
  """Returns a pathos.pools.ProcessPool instance.

  Split out for ease of unittesting since pathos can still run into pickling
  issues with MagicMocks used in tests.

  Args:
    nodes: How many processes processes to spawn in the process pool.

  Returns:
    A pathos.pools.ProcessPool instance.
  """
  return pools.ProcessPool(nodes=nodes)
