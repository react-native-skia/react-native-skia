/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef WindowLibWPE_DEFINED
#define WindowLibWPE_DEFINED

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"
#include "Window.h"
#include "unix/platform/display/PlatformDisplay.h"
#include "unix/platform/display/libwpe/PlatformDisplayLibWPE.h"
#include <wpe/wpe.h>
#include <wpe/wpe-egl.h>

// We will need window id only when we have multipple windows and have to choose which window to update,
//for now we will refer our window with this ID always because we wont use multipple windows.
#define LIBWPE_DEFAULT_WINID 100

namespace sk_app {
typedef unsigned int WPEWindowID;
class WindowLibWPE : public Window {
public:
    WindowLibWPE()
            : Window()
            , fRendererTarget(nullptr)
            , fPlatformDisplay(nullptr)
            , fDisplay(nullptr)
            , fWindow(0)
            , fWindowId(LIBWPE_DEFAULT_WINID)
            , fViewWidth(-1)
            , fViewHeight(-1)
            , fMSAASampleCount(1) {}
    ~WindowLibWPE() override { this->closeWindow(); }

    bool initWindow(PlatformDisplay* display);

    void setTitle(const char*) override;
    void show() override;
    bool attach(BackendType) override;

    void onInval() override;
    void didRenderFrame() override;

    bool handleEvent();

    static const WPEWindowID& GetKey(const WindowLibWPE& w) {
        return w.fWindowId;
    }

    static uint32_t Hash(const WPEWindowID& w) {
        return SkChecksum::Mix(w);
    }

    static SkTDynamicHash<WindowLibWPE, WPEWindowID> gWindowMap;

    void markPendingPaint() { fPendingPaint = true; }
    void finishPaint() {
        if (fPendingPaint) {
            this->onPaint();
            fPendingPaint = false;
        }
    }

    void markPendingResize(int width, int height) {
        if (width != this->width() || height != this->height()){
            fPendingResize = true;
            fPendingWidth = width;
            fPendingHeight = height;
        }
    }
    void finishResize() {
        if (fPendingResize) {
            this->onResize(fPendingWidth, fPendingHeight);
            fPendingResize = false;
        }
    }

    void setRequestedDisplayParams(const DisplayParams&, bool allowReattach) override;

private:
    void setViewSize(int width, int height);
    bool initViewBackend(wpe_view_backend* viewBackend);
    bool initRenderTarget(wpe_view_backend* viewBackend, wpe_renderer_backend_egl* renderBackend);
    void closeWindow();

    struct wpe_renderer_backend_egl_target* fRendererTarget;
    PlatformDisplay *fPlatformDisplay;
    Display*   fDisplay;
    GLNativeWindowType fWindow;
    WPEWindowID     fWindowId;
    int     fViewWidth;
    int     fViewHeight;
    int     fMSAASampleCount;

    bool    fPendingPaint;
    int     fPendingWidth;
    int     fPendingHeight;
    bool    fPendingResize;

    BackendType fBackend;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif // WindowLibWPE_DEFINED
