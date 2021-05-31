/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/utils/RnsLog.h"

namespace RnsShell {

class Performance {
public:
    static void takeSamples(uint64_t swapBufferTime);
    static void displayFps();
};
} //namespace RnsShell
