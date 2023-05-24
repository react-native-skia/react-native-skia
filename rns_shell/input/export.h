// Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
// Copyright (C) Kudo Chien.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef RNS_SHELL_INPUT_EXPORT_H_
#define RNS_SHELL_INPUT_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(INPUT_IMPLEMENTATION)
#define INPUT_EXPORT __declspec(dllexport)
#else
#define INPUT_EXPORT __declspec(dllimport)
#endif  // defined(INPUT_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(INPUT_IMPLEMENTATION)
#define INPUT_EXPORT __attribute__((visibility("default")))
#else
#define INPUT_EXPORT
#endif
#endif

#else  // defined(COMPONENT_BUILD)
#define INPUT_EXPORT
#endif

#endif  // RNS_SHELL_INPUT_EXPORT_H_
