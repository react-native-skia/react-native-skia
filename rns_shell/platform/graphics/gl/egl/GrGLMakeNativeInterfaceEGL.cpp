/*
 * Copyright 2014 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Must be added before X11 headrs because Folly uses "Struct None" and X11 has "#define None 0L" which conflicts
#include "src/gpu/gl/GrGLUtil.h"

// Define this to get a prototype for eglGetProcAddress on some systems
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include "ReactSkia/utils/RnsUtils.h"

static GrGLFuncPtr egl_get(void* ctx, const char name[]) {

    SkASSERT(nullptr == ctx);
    SkASSERT(eglGetCurrentContext());
    return eglGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    if (nullptr == eglGetCurrentContext()) {
        return nullptr;
    }

    return GrGLMakeAssembledInterface(nullptr, egl_get);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
