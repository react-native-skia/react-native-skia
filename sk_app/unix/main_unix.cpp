/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "Application.h"
#include "PlatformDisplay.h"

#if PLATFORM(X11)
#include "x11/PlatformDisplayX11.h"
#include "x11/WindowX11.h"
#endif //PLATFORM(X11)

#include "tools/timer/Timer.h"

bool done = false;

#if PLATFORM(X11)
void xlib_runloop(sk_app::PlatformDisplay& pDisplay, sk_app::Application *app)
{
    Display* display = dynamic_cast<sk_app::PlatformDisplayX11&>(pDisplay).native();

    // Get the file descriptor for the X display
    const int x11_fd = ConnectionNumber(display);

    bool done = false;
    while (!done) {
        if (0 == XPending(display)) {
            // Only call select() when we have no events.

            // Create a file description set containing x11_fd
            fd_set in_fds;
            FD_ZERO(&in_fds);
            FD_SET(x11_fd, &in_fds);

            // Set a sleep timer
            struct timeval tv;
            tv.tv_usec = 10;
            tv.tv_sec = 0;

            // Wait for an event on the file descriptor or for timer expiration
            (void)select(1, &in_fds, nullptr, nullptr, &tv);
        }
        // Handle all pending XEvents (if any) and flush the input
        // Only handle a finite number of events before finishing resize and paint..
        if (int count = XPending(display)) {
            // collapse any Expose and Resize events.
            SkTHashSet<sk_app::WindowX11*> pendingWindows;
            while (count-- && !done) {
                XEvent event;
                XNextEvent(display, &event);

                sk_app::WindowX11* win = sk_app::WindowX11::gWindowMap.find(event.xany.window);
                if (!win) {
                    continue;
                }

                // paint and resize events get collapsed
                switch (event.type) {
                case Expose:
                    win->markPendingPaint();
                    pendingWindows.add(win);
                    break;
                case ConfigureNotify:
                    win->markPendingResize(event.xconfigurerequest.width,
                                           event.xconfigurerequest.height);
                    pendingWindows.add(win);
                    break;
                default:
                    if (win->handleEvent(event)) {
                        done = true;
                    }
                    break;
                }
            }
            pendingWindows.foreach([](sk_app::WindowX11* win) {
                win->finishResize();
                win->finishPaint();
            });
        } else {
            // We are only really "idle" when the timer went off with zero events.
            app->onIdle();
        }

        XFlush(display);
    }

}
#endif //PLATFORM(X11)

int main(int argc, char**argv) {

    sk_app::PlatformDisplay& pDisplay = sk_app::PlatformDisplay::sharedDisplayForCompositing();

    sk_app::Application* app = sk_app::Application::Create(argc, argv, reinterpret_cast<void*>(&pDisplay));

#if PLATFORM(X11)
    xlib_runloop(pDisplay, app);
#endif

    delete app;

    return 0;
}
