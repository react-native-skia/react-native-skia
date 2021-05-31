/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"

#include "Application.h"
#include "DisplayParams.h"

class GrDirectContext;
class SkCanvas;
class SkSurface;
class SkSurfaceProps;
class SkString;

namespace RnsShell {

class WindowContext;

class Window {
public:
    static Window* createNativeWindow(void* platformData);
    static void createEventLoop(Application* app);

    virtual ~Window();

    virtual void setTitle(const char*) = 0;
    virtual void show() = 0;
    virtual uint64_t nativeWindowHandle() = 0;

    enum BackendType {
        kNativeGL_BackendType,
        kRaster_BackendType,

        kLast_BackendType = kRaster_BackendType
    };
    enum {
        kBackendTypeCount = kLast_BackendType + 1
    };

    int width() const;
    int height() const;

    virtual const DisplayParams& getRequestedDisplayParams() { return requestedDisplayParams_; }
    virtual void setRequestedDisplayParams(const DisplayParams&, bool allowReattach = true);

    // Actual parameters in effect, obtained from the native window.
    int sampleCount() const;
    int stencilBits() const;
    void onResize(int w, int h);

    // Returns null if there is not a GPU backend or if the backend is not yet created.
    GrDirectContext* directContext() const;
    virtual void didRenderFrame() {};

protected:
    Window();

    DisplayParams          requestedDisplayParams_;
};

}   // namespace RnsShell
