/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "include/core/SkSurface.h"
#include "WindowContextFactory.h"
#include "RasterWindowContext.h"

typedef Window XWindow;

namespace RnsShell {

class RasterWindowContextX11 : public RasterWindowContext {
public:
    static std::unique_ptr<WindowContext> createContext(GLNativeWindowType window, PlatformDisplay* platformDisplay, const DisplayParams& params);
    RasterWindowContextX11(GLNativeWindowType , PlatformDisplay*, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
<<<<<<< HEAD
    void swapBuffers(std::vector<SkIRect> &damage) override;
    bool makeContextCurrent() override { return true; }
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool hasSwapBuffersWithDamage() override { return false; }
    bool hasBufferCopy() override;
#endif
=======
    void swapBuffers() override;
    bool makeContextCurrent() override { return true; }
>>>>>>> RNS Shell Implementation  (#8)
    bool isValid() override { return SkToBool(window_); }
    void initializeContext();
    void setDisplayParams(const DisplayParams& params) override;

protected:
    sk_sp<SkSurface> backbufferSurface_;
    Display* display_;
    XWindow  window_;
    GC       gc_;
    typedef RasterWindowContext INHERITED;
};

}   // namespace RnsShell

