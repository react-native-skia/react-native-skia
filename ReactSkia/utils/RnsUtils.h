/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "include/core/SkTime.h"

#define PLATFORM(RNS_FEATURE) (defined RNS_PLATFORM_##RNS_FEATURE && RNS_PLATFORM_##RNS_FEATURE)
#define USE(RNS_FEATURE) (defined USE_##RNS_FEATURE && USE_##RNS_FEATURE)
#define ENABLE(RNS_FEATURE) (defined ENABLE_##RNS_FEATURE && ENABLE_##RNS_FEATURE)

#define RNS_EXPORT __attribute__((visibility("default")))
#define RNS_UNUSED(val) (void)val;
#define RNS_USED __attribute__((used))
#define RNS_MAKE_NONCOPYABLE(ClassName) \
    private: \
        ClassName(const ClassName&) = delete; \
        ClassName& operator=(const ClassName&) = delete; \

#define RNS_EXPORT_MODULE(ModuleName) \
RNS_USED xplat::module::CxxModule* ModuleName##Cls(void) { \
  return new ModuleName();\
}\

#define RNS_EXPORT_COMPONENT_PROVIDER(ComponentName) \
RNS_USED RSkComponentProvider* RSkComponentProvider##ComponentName##Cls(void) { \
  return new RSkComponentProvider##ComponentName();\
}\

#define RNS_GET_TIME_STAMP_MS(marker) \
    double marker = SkTime::GetMSecs();

#define RNS_GET_TIME_STAMP_US(marker) \
    double marker = SkTime::GetNSecs() * 1e-3;

#define RNS_GET_TIME_STAMP_NS(marker) \
    double marker = SkTime::GetNSecs();

// Profiling
#define RNS_PROFILE_API_OFF(msg, instruction) instruction
#define RNS_PROFILE_API_AVG_OFF(msg, instruction) instruction
#ifdef RNS_ENABLE_API_PERF
    #define RNS_PROFILE_API_ON(msg, instruction) \
        {\
            double startMarker= SkTime::GetMSecs(); \
            instruction; \
            RNS_LOG_INFO(msg << " took " <<  (SkTime::GetMSecs() - startMarker) << " ms"); \
        }
    #define RNS_PROFILE_API_AVG_ON(msg, instruction) \
        {\
            static unsigned long long localCount = 0;\
            static double start = 0, total = 0; \
            start = SkTime::GetMSecs(); \
            instruction; \
            total += (SkTime::GetMSecs() - start); \
            localCount++;\
            RNS_LOG_INFO(msg << " Average(" <<  ( total / localCount) << ") ms"); \
        }
    #define RNS_PROFILE_START(marker) \
            double rnsProfileVar_##marker= SkTime::GetMSecs();
    #define RNS_PROFILE_END(msg, marker) \
            RNS_LOG_INFO(msg << #marker << " took " <<  (SkTime::GetMSecs() - rnsProfileVar_##marker) << " ms");
#else
    #define RNS_PROFILE_API_ON(msg, instruction) instruction
    #define RNS_PROFILE_API_AVG_ON(msg, instruction) instruction
    #define RNS_PROFILE_START(marker)
    #define RNS_PROFILE_END(msg, marker)
#endif
#define RNS_SECONDS_TO_MILLISECONDS(time) ((time)*1000)