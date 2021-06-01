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
#elif PLATFORM(LIBWPE)
#include <wpe/wpe.h>
#include <glib.h>
#include "libwpe/PlatformDisplayLibWPE.h"
#include "libwpe/WindowLibWPE.h"
#endif //PLATFORM(X11)

#include "tools/timer/Timer.h"

#include <glog/logging.h>

bool done = false;

// static
folly::EventBase sk_app::Application::s_event_base;

#if PLATFORM(X11)
void xlib_runloop(sk_app::PlatformDisplay& pDisplay, sk_app::Application *app)
{
    Display* display = dynamic_cast<sk_app::PlatformDisplayX11&>(pDisplay).native();

    // Get the file descriptor for the X display
    const int x11_fd = ConnectionNumber(display);

    LOG(INFO) << "xlib main loop";

    bool done = false;
    while (!done) {
        sk_app::Application::GetMainTaskRunner().loop();

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
#elif PLATFORM(LIBWPE)
static void* libwpe_loop_thread_function(gpointer loop)
{
    GMainLoop *pMainLoop = (GMainLoop*)loop;
    g_main_loop_run(pMainLoop);
    return (void *)0;
}

void libwpe_runloop(sk_app::PlatformDisplay& pDisplay, sk_app::Application *app)
{
    auto* loop = g_main_loop_new(g_main_context_get_thread_default(), FALSE);
    void* display = dynamic_cast<sk_app::PlatformDisplayLibWPE&>(pDisplay).native();
    SK_APP_UNUSED(display);

    if(g_thread_new("LibWpeLoop", libwpe_loop_thread_function, loop) == NULL)
    {
        SK_APP_LOG_ERROR("xxxxxxxxxx g_thread_new Failed xxxxxxxxxx g_main_loop_run will not work\n");
        return;
    }

    while (!done) {

        SkTHashSet<sk_app::WindowLibWPE*> pendingWindows;
        sk_app::WindowLibWPE* win = sk_app::WindowLibWPE::gWindowMap.find(LIBWPE_DEFAULT_WINID);
        if (!win) {
            SK_APP_LOG_ERROR("Invalide Pending Window\n");
            continue;
        }
        win->markPendingPaint();
        pendingWindows.add(win);
        if(true) {  // FIXME decide when we need to paint
            pendingWindows.foreach([](sk_app::WindowLibWPE* win) {
                win->finishResize();
                win->finishPaint();
            });
        } else {
            // We are only really "idle" when the timer went off with zero events.
            app->onIdle();
        }
        usleep(100000);
    }
}
#endif

int main(int argc, char**argv) {

    sk_app::PlatformDisplay& pDisplay = sk_app::PlatformDisplay::sharedDisplayForCompositing();

#if PLATFORM(LIBWPE) || USE(WEP_RENDERER)
    int hostClientFileDescriptor = wpe_renderer_host_create_client();
    const char *implementationLibraryName = wpe_loader_get_loaded_implementation_library_name();

    wpe_loader_init(implementationLibraryName);
    dynamic_cast<sk_app::PlatformDisplayLibWPE&>(pDisplay).initialize(hostClientFileDescriptor);
#endif

    sk_app::Application* app = sk_app::Application::Create(argc, argv, reinterpret_cast<void*>(&pDisplay));

#if PLATFORM(X11)
    xlib_runloop(pDisplay, app);
#elif PLATFORM(LIBWPE)
    libwpe_runloop(pDisplay, app);
#endif

    delete app;

    return 0;
}
