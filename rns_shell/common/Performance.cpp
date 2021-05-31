/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Performance.h"

static unsigned long long localFrameCount = 0;
double fpsTimeStampA = 0;
double fpsTimeStampB = 0;
static uint64_t localSwapBufferTimeMin = 0;
static uint64_t localSwapBufferTimeMax = 0;
static uint64_t localSwapBufferTimeSum = 0;

namespace RnsShell {

void Performance::displayFps() {
    if(localFrameCount != 0) {
        RNS_LOG_INFO_EVERY_N(60, " Total Frames : " << localFrameCount - 1 <<
                                 " Total Time : " << (fpsTimeStampB - fpsTimeStampA) * 1e-9 << " sec" <<
                                 " Fps : " << ((double) (localFrameCount -1)) * 1e+9 / (fpsTimeStampB - fpsTimeStampA) <<
                                 " swap(min, avg, max) us : (" << localSwapBufferTimeMin << "," <<
                                                           ((localFrameCount > 0) ? localSwapBufferTimeSum / localFrameCount : 0) << "," <<
                                                           localSwapBufferTimeMax << ") us");
    }
}

void Performance::takeSamples(uint64_t swapBufferTime) {

    if(localSwapBufferTimeSum == 0) {
        localSwapBufferTimeMin = swapBufferTime;
        localSwapBufferTimeMax = swapBufferTime;
        localSwapBufferTimeSum = swapBufferTime;
    } else {
        if(localSwapBufferTimeMin > swapBufferTime)
            localSwapBufferTimeMin = swapBufferTime;

        if(localSwapBufferTimeMax < swapBufferTime)
            localSwapBufferTimeMax = swapBufferTime;

        localSwapBufferTimeSum += swapBufferTime;
    }
    localFrameCount++;
    displayFps();

    if( localFrameCount == 1) {
        fpsTimeStampA = fpsTimeStampB = SkTime::GetNSecs();
    }
    if(localFrameCount > 1) {
        fpsTimeStampB = SkTime::GetNSecs();
    }
}

} // namespace RnsShell
