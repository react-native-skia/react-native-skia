/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <string>

#include "rns_shell/compositor/RendererDelegate.h"

namespace RnsShell {

class Application : public RendererDelegate {
public:
    static Application* Create(int argc, char** argv);
    Application();
    virtual ~Application() {}

    PlatformDisplayID displayID() const { return displayID_; }
    uint32_t identifier();
    void sizeChanged(int width, int height);
    virtual void onResize(SkSize newSize) = 0;

private:
    PlatformDisplayID displayID_ { 0 };
    void windowScreenChanged(PlatformDisplayID displayId);

    typedef RendererDelegate INHERITED;
};

}   // namespace RnsShell
