/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <iostream>

#include "include/core/SkSize.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

#if USE(EGL)
#include <EGL/egl.h>
#endif // USE(EGL)

namespace RnsShell {

#if USE(EGL) || USE(GLX)
class GLWindowContext;
#endif

class PlatformDisplay {
public:
    RNS_EXPORT static bool initialize();
    RNS_EXPORT static PlatformDisplay& sharedDisplay();
    RNS_EXPORT static PlatformDisplay& sharedDisplayForCompositing();
    virtual ~PlatformDisplay();

    enum class Type {
        X11,
        Wayland,
        DFB,
        Windows,
        WPE,
    };

    virtual Type type() const = 0;
    virtual SkSize screenSize() = 0;
    SkSize getCurrentScreenSize();
    void setCurrentScreenSize(int,int);

#if USE(EGL) || USE(GLX)
    RNS_EXPORT GLWindowContext* sharingGLContext();
#endif

#if USE(EGL)
    EGLDisplay eglDisplay() const;
    bool eglCheckVersion(int major, int minor) const;
#endif

protected:
    explicit PlatformDisplay(bool);
    bool nativeDisplayOwned_ { false };

#if USE(EGL)
    virtual void initializeEGLDisplay();

    EGLDisplay eglDisplay_;
#endif

#if USE(EGL) || USE(GLX)
    std::unique_ptr<GLWindowContext> sharingGLContext_;
#endif

private:
    static std::unique_ptr<PlatformDisplay> createPlatformDisplay();
    SkSize currentScreenSize_;

#if USE(EGL)
    void terminateEGLDisplay();

    bool eglDisplayInitialized_ { false };
    int eglMajorVersion_ { 0 };
    int eglMinorVersion_ { 0 };
#endif

};
} // namespace rns_shell

