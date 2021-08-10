/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "ReactSkia/utils/RnsUtils.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#if USE(GLX)
#include <GL/glx.h>
#endif

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"

#include "Window.h"
#include "PlatformDisplay.h"

#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/sdk/NotificationCenter.h"
typedef Window XWindow;

namespace RnsShell {

class WindowX11 : public Window {
public:
    WindowX11()
            : Window()
            , display_(nullptr)
            , window_(0)
#if USE(GLX)
            , fbConfig_(nullptr)
            , visualInfo_(nullptr)
#endif
            , MSAASampleCount_(1) {}
    ~WindowX11() override { this->closeWindow(); }

    bool initWindow(PlatformDisplay* display);

    uint64_t nativeWindowHandle() override {return (uint64_t) window_; }

    bool handleEvent(const XEvent& event);
    void setTitle(const char*) override;
    void show() override;

    static const XWindow& GetKey(const WindowX11& w) {
        return w.window_;
    }

    static uint32_t Hash(const XWindow& w) {
        return SkChecksum::Mix(w);
    }

    static SkTDynamicHash<WindowX11, XWindow> gWindowMap;
    void setRequestedDisplayParams(const DisplayParams&, bool allowReattach) override;

private:
    void closeWindow();
    void onKey(rnsKey eventKeyType, rnsKeyAction eventKeyAction);
    rnsKey  keyIdentifierForX11KeyCode(KeySym keycode);

    Display*     display_;
    XWindow      window_;
#if USE(GLX)
    GLXFBConfig* fbConfig_;
    XVisualInfo* visualInfo_;
#endif
    int          MSAASampleCount_;
    Atom         wmDeleteMessage_;
    typedef Window INHERITED;
};

}   // namespace RnsShell

