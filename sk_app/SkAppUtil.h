/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAppUtil_DEFINED
#define SkAppUtil_DEFINED

#define PLATFORM(SKA_FEATURE) (defined SKA_PLATFORM_##SKA_FEATURE && SKA_PLATFORM_##SKA_FEATURE)
#define USE(SKA_FEATURE) (defined USE_##SKA_FEATURE && USE_##SKA_FEATURE)
#define SKA_EXPORT __attribute__((visibility("default")))

#define SK_APP_NOT_IMPL  fprintf(stderr, "\033[22;35m[RNS] %-24s +%-4d : !!!!!!!!!! %s NOT IMPLEMENTED !!!!!!!!!!\n\033[22;0m", __FILE__, __LINE__, __func__);
#define SK_APP_TODO(msg, args...) fprintf(stderr, "\033[22;32m%s +%d : [TODO] \033[22;0m " msg, __FILE__, __LINE__, ## args);

#define SK_APP_LOG_ERROR(msg, args...)  fprintf(stderr, "\033[22;31m[ERROR] %s +%d : " msg "\033[22;0m" , __FILE__, __LINE__, ## args);
#define SK_APP_LOG_INFO(msg, args...)   fprintf(stdout, "\033[22;32m[INFO ] %s +%d : " msg "\033[22;0m" , __FILE__, __LINE__, ## args);
#define SK_APP_LOG_TRACE(msg, args...)  fprintf(stdout, "\033[22;35m[TRACE] %-24s +%-4d : %s\n\033[22;0m", __FILE__, __LINE__, __func__);

#define SK_APP_UNUSED(val) (void)val;

#endif // SkAppUtil_DEFINED
