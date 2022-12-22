/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <glog/logging.h>

// Logging type. SYSLOG type or use default LOG type
#undef RNS_LOG_TYPE_SYSLOG

#ifdef RNS_LOG_TYPE_SYSLOG
    #define RNSLOG SYSLOG
    #define RNSLOG_IF SYSLOG_IF
    #define RNSLOG_EVERY_N SYSLOG_EVERY_N
    #define VRNSLOG(verboselevel) SYSLOG_IF(INFO, VLOG_IS_ON(verboselevel))
    #define VRNSLOG_IF(verboselevel, cond) SYSLOG_IF(INFO, (VLOG_IS_ON(verboselevel) && (cond)))
#else
    #define RNSLOG LOG
    #define RNSLOG_IF LOG_IF
    #define RNSLOG_EVERY_N LOG_EVERY_N
    #define VRNSLOG VLOG
    #define VRNSLOG_IF VLOG_IF
#endif
// SYSLOG type doesnt have IF_EVERY_N support so we will use VLOG_EVERY_N for both log types
#define VRNSLOG_EVERY_N VLOG_EVERY_N

// Helper Logging Macros
#define RNS_LOG_NOT_IMPL_MSG(msg)  RNSLOG(WARNING) << "!!!!!!!!!! " << msg << " NOT IMPLEMENTED !!!!!!!!!!"
#define RNS_LOG_NOT_IMPL  RNS_LOG_NOT_IMPL_MSG(__func__)
#define RNS_LOG_TODO(msg) RNSLOG(WARNING) << "[TODO] : " <<  msg
#define RNS_LOG_ASSERT(cond, msg) CHECK(cond) << " => " << msg
#define RNS_PLOG(msg) PLOG(ERROR) << msg;
#define RNS_PLOG_IF(cond, msg) PLOG_IF(ERROR, (cond)) << msg

// Basic Logging
#define RNS_LOG_TRACE(msg)    VRNSLOG(2) << msg
#define RNS_LOG_DEBUG(msg)    VRNSLOG(1) << msg
#define RNS_LOG_INFO(msg)     RNSLOG(INFO) << msg
#define RNS_LOG_WARN(msg)     RNSLOG(WARNING) << msg
#define RNS_LOG_ERROR(msg)    RNSLOG(ERROR) << msg
#define RNS_LOG_FATAL(msg)    RNSLOG(FATAL) << msg

// Conditional Logging
#define RNS_LOG_TRACE_IF(cond, msg) VRNSLOG_IF(2, (cond)) << msg
#define RNS_LOG_DEBUG_IF(cond, msg) VRNSLOG_IF(1, (cond)) << msg
#define RNS_LOG_INFO_IF(cond, msg) RNSLOG_IF(INFO, (cond)) << msg
#define RNS_LOG_WARN_IF(cond, msg) RNSLOG_IF(WARNING, (cond)) << msg
#define RNS_LOG_ERROR_IF(cond, msg) RNSLOG_IF(ERROR, (cond)) << msg

// Occasional Logging
#define RNS_LOG_TRACE_EVERY_N(occurrences, msg)  VRNSLOG_EVERY_N(2, occurrences) << msg
#define RNS_LOG_DEBUG_EVERY_N(occurrences, msg)  VRNSLOG_EVERY_N(1, occurrences) << msg
#define RNS_LOG_INFO_EVERY_N(occurrences, msg)  RNSLOG_EVERY_N(INFO, occurrences) << msg
#define RNS_LOG_WARN_EVERY_N(occurrences, msg)  RNSLOG_EVERY_N(WARNING, occurrences) << msg
#define RNS_LOG_ERROR_EVERY_N(occurrences, msg)  RNSLOG_EVERY_N(ERROR, occurrences) << msg

// SYSLOG doesnt have IF_EVERY_N and FIRST_N log support, so we will use default loging for these types

// Conditional Occasional Logging
#define RNS_LOG_TRACE_IF_EVERY_N(cond, occurrences, msg)  VLOG_IF_EVERY_N(2, (cond), occurrences) << msg
#define RNS_LOG_DEBUG_IF_EVERY_N(cond, occurrences, msg)  VLOG_IF_EVERY_N(1, (cond), occurrences) << msg
#define RNS_LOG_INFO_IF_EVERY_N(cond, occurrences, msg)  LOG_IF_EVERY_N(INFO, (cond), occurrences) << msg
#define RNS_LOG_WARN_IF_EVERY_N(cond, occurrences, msg)  LOG_IF_EVERY_N(WARNING, (cond), occurrences) << msg
#define RNS_LOG_ERROR_IF_EVERY_N(cond, occurrences, msg)  LOG_IF_EVERY_N(ERROR, (cond), occurrences) << msg

// First "N" Occurrences Logging
#define RNS_LOG_TRACE_FIRST_N(occurrences, msg)  VLOG_FIRST_N(2, occurrences) << msg
#define RNS_LOG_DEBUG_FIRST_N(occurrences, msg)  VLOG_FIRST_N(1, occurrences) << msg
#define RNS_LOG_INFO_FIRST_N(occurrences, msg)  LOG_FIRST_N(INFO, occurrences) << msg
#define RNS_LOG_WARN_FIRST_N(occurrences, msg)  LOG_FIRST_N(WARNING, occurrences) << msg
#define RNS_LOG_ERROR_FIRST_N(occurrences, msg)  LOG_FIRST_N(ERROR, occurrences) << msg
