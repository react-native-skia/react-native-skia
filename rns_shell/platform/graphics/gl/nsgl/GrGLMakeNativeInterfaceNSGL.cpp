/*
 * Copyright 2014 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/private/SkTemplates.h"

#include <dlfcn.h>
#include <memory>

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
  static const char kPath[] =
      "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib";
  std::unique_ptr<void, SkFunctionWrapper<int(void *), dlclose>> lib(
      dlopen(kPath, RTLD_LAZY));
  return GrGLMakeAssembledGLInterface(
      lib.get(), [](void *ctx, const char *name) {
        return (GrGLFuncPtr)dlsym(ctx ? ctx : RTLD_DEFAULT, name);
      });
}
