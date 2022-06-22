/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <glib.h>

#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"

#include "Window.h"
#include "PlatformDisplay.h"
#include "platform/graphics/libwpe/PlatformDisplayLibWPE.h"
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "GLWindowContext.h"
#else
#include "WindowContextFactory.h"
#endif

// FIXME We will need window id only when we have multipple windows and have to choose which window to update,
//for now we will refer our window with this ID always because we wont use multipple windows.
#define LIBWPE_DEFAULT_WINID 10

namespace RnsShell {
typedef unsigned int WPEWindowID;
class WindowLibWPE : public Window {
public:
    WindowLibWPE()
            : Window()
            , rendererTarget_(nullptr)
            , platformDisplay_(nullptr)
            , display_(nullptr)
            , window_(0)
            , windowId_(LIBWPE_DEFAULT_WINID)
            , viewWidth_(-1)
            , viewHeight_(-1)
            , MSAASampleCount_(1) {}

    ~WindowLibWPE() override;
    bool initWindow(PlatformDisplay* display,SkSize dimension,WindowType winType);
    void closeWindow() override;
    uint64_t nativeWindowHandle() override {return (uint64_t) window_; }
    SkSize getWindowSize() override { return {viewWidth_, viewHeight_}; }

    bool handleEvent();
    void setTitle(const char*) override;
    void show() override;

    void didRenderFrame() override;

    static const WPEWindowID& GetKey(const WindowLibWPE& w) {
        return w.windowId_;
    }

    static uint32_t Hash(const WPEWindowID& w) {
        return SkChecksum::Mix(w);
    }

    void setRequestedDisplayParams(const DisplayParams&, bool allowReattach) override;

    static SkTDynamicHash<WindowLibWPE, WPEWindowID> gWindowMap;
    static GMainLoop       *mainLoop_;
    static Application     *mainApp_; // Addded for triggering resize from window event.
private:
    void setViewSize(int width, int height);
    bool initViewBackend(wpe_view_backend* viewBackend);
    bool initRenderTarget(wpe_view_backend* viewBackend, wpe_renderer_backend_egl* renderBackend,SkSize dimension);
    void onKey(rnsKey keyType,rnsKeyAction eventKeyAction);
    rnsKey keyIdentifierForWPEKeyCode(int keyCode);

    struct wpe_renderer_backend_egl_target* rendererTarget_;
    struct wpe_view_backend* viewBackend_;
    PlatformDisplay *platformDisplay_;
    Display*   display_;
    GLNativeWindowType window_;
    WPEWindowID     windowId_;
    int     viewWidth_;
    int     viewHeight_;
    int     MSAASampleCount_;
    typedef Window INHERITED;
};

}   // namespace RnsShell
