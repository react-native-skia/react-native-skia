/*
 * Copyright 2014 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Must be added before X11 headrs because Folly uses "Struct None" and X11 has "#define None 0L" which conflicts
#include "src/gpu/ganesh/gl/GrGLUtil.h"
// USE macro is defined here so need to include on before using USE macro
#include "ReactSkia/utils/RnsUtils.h"

// Define this to get a prototype for eglGetProcAddress on some systems
#define EGL_EGLEXT_PROTOTYPES 1
#define GL_GLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>

#if USE(OPENGL_ES)
#include <GLES2/gl2.h>
#if defined(GL_ES_VERSION_3_0) && GL_ES_VERSION_3_0
#include <GLES3/gl3.h>
#endif
#else
#include <GL/gl.h>
#endif

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

static GrGLFuncPtr egl_get(void* ctx, const char name[]) {

    SkASSERT(nullptr == ctx);
    SkASSERT(eglGetCurrentContext());

    // https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_get_all_proc_addresses.txt
    // eglGetProcAddress() is not guaranteed to support the querying of non-extension EGL functions.
#define M(X) if (0 == strcmp(#X, name)) { return (GrGLFuncPtr) X; }
    M(eglGetCurrentDisplay)
    M(eglQueryString)
    M(glActiveTexture)
    M(glAttachShader)
    M(glBindAttribLocation)
    M(glBindBuffer)
    M(glBindFramebuffer)
    M(glBindRenderbuffer)
    M(glBindTexture)
    M(glBlendColor)
    M(glBlendEquation)
    M(glBlendFunc)
    M(glBufferData)
    M(glBufferSubData)
    M(glCheckFramebufferStatus)
    M(glClear)
    M(glClearColor)
    M(glClearStencil)
    M(glColorMask)
    M(glCompileShader)
    M(glCompressedTexImage2D)
    M(glCompressedTexSubImage2D)
    M(glCopyTexSubImage2D)
    M(glCreateProgram)
    M(glCreateShader)
    M(glCullFace)
    M(glDeleteBuffers)
    M(glDeleteFramebuffers)
    M(glDeleteProgram)
    M(glDeleteRenderbuffers)
    M(glDeleteShader)
    M(glDeleteTextures)
    M(glDepthMask)
    M(glDisable)
    M(glDisableVertexAttribArray)
    M(glDrawArrays)
    M(glDrawElements)
    M(glEnable)
    M(glEnableVertexAttribArray)
    M(glFinish)
    M(glFlush)
    M(glFramebufferRenderbuffer)
    M(glFramebufferTexture2D)
    M(glFrontFace)
    M(glGenBuffers)
    M(glGenFramebuffers)
    M(glGenRenderbuffers)
    M(glGenTextures)
    M(glGenerateMipmap)
    M(glGetBufferParameteriv)
    M(glGetError)
    M(glGetFramebufferAttachmentParameteriv)
    M(glGetIntegerv)
    M(glGetProgramInfoLog)
    M(glGetProgramiv)
    M(glGetRenderbufferParameteriv)
    M(glGetShaderInfoLog)
    M(glGetShaderPrecisionFormat)
    M(glGetShaderiv)
    M(glGetString)
    M(glGetUniformLocation)
    M(glIsTexture)
    M(glLineWidth)
    M(glLinkProgram)
    M(glPixelStorei)
    M(glReadPixels)
    M(glRenderbufferStorage)
    M(glScissor)
    M(glShaderSource)
    M(glStencilFunc)
    M(glStencilFuncSeparate)
    M(glStencilMask)
    M(glStencilMaskSeparate)
    M(glStencilOp)
    M(glStencilOpSeparate)
    M(glTexImage2D)
    M(glTexParameterf)
    M(glTexParameterfv)
    M(glTexParameteri)
    M(glTexParameteriv)
    M(glTexSubImage2D)
    M(glUniform1f)
    M(glUniform1fv)
    M(glUniform1i)
    M(glUniform1iv)
    M(glUniform2f)
    M(glUniform2fv)
    M(glUniform2i)
    M(glUniform2iv)
    M(glUniform3f)
    M(glUniform3fv)
    M(glUniform3i)
    M(glUniform3iv)
    M(glUniform4f)
    M(glUniform4fv)
    M(glUniform4i)
    M(glUniform4iv)
    M(glUniformMatrix2fv)
    M(glUniformMatrix3fv)
    M(glUniformMatrix4fv)
    M(glUseProgram)
    M(glVertexAttrib1f)
    M(glVertexAttrib2fv)
    M(glVertexAttrib3fv)
    M(glVertexAttrib4fv)
    M(glVertexAttribPointer)
    M(glViewport)
#undef M

    return eglGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    if (nullptr == eglGetCurrentContext()) {
        return nullptr;
    }

    return GrGLMakeAssembledInterface(nullptr, egl_get);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
