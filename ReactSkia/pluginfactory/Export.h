/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if !defined(__RNS_PLUGIN_H_INSIDE__)
#error "Only <RnsRlugin.h> can be included directly."
#endif

#pragma once

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#define WIN32
#endif

/* For non-Clang compilers */
#ifndef __has_declspec_attribute
#define __has_declspec_attribute(x) 0
#endif

#if defined(WIN32) || (__has_declspec_attribute(dllexport) && __has_declspec_attribute(dllimport))
#define RNSP_EXPORT __declspec(dllexport)
#else
#define RNSP_EXPORT __attribute__((visibility("default")))
#endif

#define RNSP_USED __attribute__((used))
