/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef WindowX11_DEFINED
#define WindowX11_DEFINED

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"
#include "Window.h"
#include "PlatformDisplay.h"

#include <GL/glx.h>
#include <X11/Xlib.h>

typedef Window XWindow;

namespace sk_app {

class WindowX11 : public Window {
public:
    WindowX11()
            : Window()
            , fPlatformDisplay(nullptr)
            , fDisplay(nullptr)
            , fWindow(0)
            , fGC(nullptr)
            , fFBConfig(nullptr)
            , fVisualInfo(nullptr)
            , fMSAASampleCount(1) {}
    ~WindowX11() override { this->closeWindow(); }

    bool initWindow(PlatformDisplay* display);

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    bool handleEvent(const XEvent& event);

    static const XWindow& GetKey(const WindowX11& w) {
        return w.fWindow;
    }

    static uint32_t Hash(const XWindow& w) {
        return SkChecksum::Mix(w);
    }

    static SkTDynamicHash<WindowX11, XWindow> gWindowMap;

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
    void closeWindow();

    PlatformDisplay *fPlatformDisplay;
    Display*     fDisplay;
    XWindow      fWindow;
    GC           fGC;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    int          fMSAASampleCount;

    Atom     fWmDeleteMessage;

    bool     fPendingPaint;
    int      fPendingWidth;
    int      fPendingHeight;
    bool     fPendingResize;

    BackendType fBackend;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
