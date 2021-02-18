/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/


#include "GLWindowContext.h"

#if PLATFORM(X11)
#include "x11/WindowX11.h"
#elif PLATFORM(LIBWPE)
#include "libwpe/WindowLibWPE.h"
#endif

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    PlatformDisplay *pDisplay = (PlatformDisplay*) platformData;

    SkASSERT(pDisplay);

#if PLATFORM(X11)
    WindowX11* window = new WindowX11();
#elif PLATFORM(LIBWPE)
    WindowLibWPE* window = new WindowLibWPE();
#endif

    if (!window->initWindow(pDisplay)) {
        delete window;
        return nullptr;
    }

    return window;
}

}   // namespace sk_app
