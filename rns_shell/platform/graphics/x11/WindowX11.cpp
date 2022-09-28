/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

// Must be added before X11 headrs because Folly uses "Struct None" and X11 has "#define None 0L" which conflicts
#include "platform/linux/TaskLoop.h"

#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include "src/utils/SkUTF.h"

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "GLWindowContext.h"
#endif
#include "WindowX11.h"
#include "WindowContextFactory.h"

#include "x11/PlatformDisplayX11.h"

namespace RnsShell {

SkTDynamicHash<WindowX11, XWindow> WindowX11::gWindowMap;
Window* Window::mainWindow_;

const long kEventMask = ExposureMask | StructureNotifyMask |
                        KeyPressMask | KeyReleaseMask |
                        PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

Window* Window::createNativeWindow(void* platformData,SkSize dimension,WindowType type) {
    PlatformDisplay *pDisplay = (PlatformDisplay*) platformData;

    RNS_LOG_ASSERT(pDisplay, "Invalid Platform Display");

    WindowX11* window = new WindowX11();
    if (!window->initWindow(pDisplay,dimension,type)) {
        delete window;
        return nullptr;
    }
    if(!mainWindow_) {
        Display* display = dynamic_cast<PlatformDisplayX11*>(pDisplay)->native();
        XSelectInput (display, DefaultRootWindow(display), ExposureMask | StructureNotifyMask); // Root window is used to monitor screen resolution change in X11
        mainWindow_ = window;
    }
    window->winType=type;
    return window;
}

// Blocking eventLoop for X11 events, created with thread in main()
void Window::createEventLoop(Application* app) {
    PlatformDisplay& pDisplay = PlatformDisplay::sharedDisplayForCompositing();
    Display* display = dynamic_cast<PlatformDisplayX11&>(pDisplay).native();
    const int x11_fd = ConnectionNumber(display);

    bool done = false;

    while (!done) {
        if (int count = XPending(display)) {
            while (count-- && !done) {
                XEvent event;
                XNextEvent(display, &event);
                switch (event.type) {
                    case ConfigureNotify:{

                        if(event.xconfigurerequest.window == DefaultRootWindow(display)) {
                            RNS_LOG_INFO(" ROOT Window(Screen) Size :" << event.xconfigurerequest.width << "x" << event.xconfigurerequest.height);
                            auto screenSize = pDisplay.getCurrentScreenSize();

                            if(((screenSize.width()!=event.xconfigurerequest.width) ||
                                (screenSize.height()!=event.xconfigurerequest.height))) {

                                pDisplay.setCurrentScreenSize(event.xconfigurerequest.width,event.xconfigurerequest.height);
                                NotificationCenter::defaultCenter().emit("dimensionEventNotification");

                            }

                        } else {
                                RNS_LOG_INFO("Resize Request with (Width x Height) : (" << event.xconfigurerequest.width <<
                                    " x " << event.xconfigurerequest.height << ")");
                                WindowX11* win = WindowX11::gWindowMap.find(event.xconfigurerequest.window);
                                if(win && (win->winType == MainWindow )) { // Only for main window
                                    auto windowDimension = win->getWindowDimension();

                                    if((windowDimension.width()!=event.xconfigurerequest.width) ||
                                    (windowDimension.height()!=event.xconfigurerequest.height)) {

                                        win->setWindowDimension(event.xconfigurerequest.width,event.xconfigurerequest.height);
                                        app->sizeChanged(event.xconfigurerequest.width, event.xconfigurerequest.height);
                                        NotificationCenter::defaultCenter().emit("dimensionEventNotification");
                                    }
                                }
                        }
                        break;
                    }
                    case UnmapNotify:
                        RNS_LOG_DEBUG("Nothing to be done for UnmapNotify: Happens on window closure");
                    break;
                    default:
                        WindowX11* win = WindowX11::gWindowMap.find(event.xany.window);
                        if (win && (win->handleEvent(event))) {
                            done = true;
                        }
                    break;
                } // switch
            }// while
        }// if(XPending)
        XFlush(display);
    } // while(!done)

    TaskLoop::main().stop();
}

bool WindowX11::initWindow(PlatformDisplay *platformDisplay,SkSize dimension,WindowType type) {

    Display* display = (dynamic_cast<PlatformDisplayX11*>(platformDisplay))->native();

    if (requestedDisplayParams_.msaaSampleCount_ != MSAASampleCount_) {
        this->closeWindow();
    }
    // we already have a window
    if (display_) {
        return true;
    }
    display_ = display;

    // Default HD
    int initialWidth =  1280;
    int initialHeight = 720;
    Screen *screen = nullptr;

    /* Read first screens display resolution*/
    if(!dimension.isEmpty()) {
       initialWidth =  dimension.width();
       initialHeight = dimension.height();
    } else if(ScreenCount(display) > 0 && (screen = ScreenOfDisplay(display, 0))) {
        initialWidth = screen->width;
        initialHeight = screen->height;
    }

#if USE(GLX)
    // Attempt to create a window that supports GL

    // We prefer the more recent glXChooseFBConfig but fall back to glXChooseVisual. They have
    // slight differences in how attributes are specified.
    static int constexpr kChooseFBConfigAtt[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_STENCIL_SIZE, 8,
        None
    };
    // For some reason glXChooseVisual takes a non-const pointer to the attributes.
    int chooseVisualAtt[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_STENCIL_SIZE, 8,
        None
    };
    RNS_LOG_ASSERT((nullptr == visualInfo_), "Invalid Platform Visual Info");
    if (requestedDisplayParams_.msaaSampleCount_ > 1) {
        static const GLint kChooseFBConifgAttCnt = SK_ARRAY_COUNT(kChooseFBConfigAtt);
        GLint msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 4];
        memcpy(msaaChooseFBConfigAtt, kChooseFBConfigAtt, sizeof(kChooseFBConfigAtt));
        RNS_LOG_ASSERT((None == msaaChooseFBConfigAtt[kChooseFBConifgAttCnt - 1]), "No FB config available");
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt - 1] = GLX_SAMPLE_BUFFERS_ARB;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 0] = 1;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 1] = GLX_SAMPLES_ARB;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 2] = requestedDisplayParams_.msaaSampleCount_;
        msaaChooseFBConfigAtt[kChooseFBConifgAttCnt + 3] = None;
        int n;
        fbConfig_ = glXChooseFBConfig(display_, DefaultScreen(display_), msaaChooseFBConfigAtt, &n);
        if (n > 0) {
            visualInfo_ = glXGetVisualFromFBConfig(display_, *fbConfig_);
        } else {
            static const GLint kChooseVisualAttCnt = SK_ARRAY_COUNT(chooseVisualAtt);
            GLint msaaChooseVisualAtt[kChooseVisualAttCnt + 4];
            memcpy(msaaChooseVisualAtt, chooseVisualAtt, sizeof(chooseVisualAtt));
            RNS_LOG_ASSERT((None == msaaChooseVisualAtt[kChooseVisualAttCnt - 1]), "No Visual config available");
            msaaChooseFBConfigAtt[kChooseVisualAttCnt - 1] = GLX_SAMPLE_BUFFERS_ARB;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 0] = 1;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 1] = GLX_SAMPLES_ARB;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 2] =
                    requestedDisplayParams_.msaaSampleCount_;
            msaaChooseFBConfigAtt[kChooseVisualAttCnt + 3] = None;
            visualInfo_ = glXChooseVisual(display, DefaultScreen(display), msaaChooseVisualAtt);
            fbConfig_ = nullptr;
        }
    }
    if (nullptr == visualInfo_) {
        int n;
        fbConfig_ = glXChooseFBConfig(display_, DefaultScreen(display_), kChooseFBConfigAtt, &n);
        if (n > 0) {
            visualInfo_ = glXGetVisualFromFBConfig(display_, *fbConfig_);
        } else {
            visualInfo_ = glXChooseVisual(display, DefaultScreen(display), chooseVisualAtt);
            fbConfig_ = nullptr;
        }
    }

    if (visualInfo_) {
        Colormap colorMap = XCreateColormap(display,
                                            RootWindow(display, visualInfo_->screen),
                                            visualInfo_->visual,
                                            AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = colorMap;
        swa.event_mask = kEventMask;
        window_ = XCreateWindow(display,
                                RootWindow(display, visualInfo_->screen),
                                0, 0, // x, y
                                initialWidth, initialHeight,
                                0, // border width
                                visualInfo_->depth,
                                InputOutput,
                                visualInfo_->visual,
                                CWEventMask | CWColormap,
                                &swa);
    } else
#endif // USE(GLX)
    {
        // Create a simple window instead.  We will not be able to show GL
        window_ = XCreateSimpleWindow(display,
                                      DefaultRootWindow(display),
                                      0, 0,  // x, y
                                      initialWidth, initialHeight,
                                      0,     // border width
                                      0,     // border value
                                      0);    // background value
        XSelectInput(display, window_, kEventMask);
    }

    if (!window_) {
        return false;
    }

    MSAASampleCount_ = requestedDisplayParams_.msaaSampleCount_;

    // set up to catch window delete message
    wmDeleteMessage_ = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window_, &wmDeleteMessage_, 1);

    // add to hashtable of windows
    gWindowMap.add(this);

    SkString title("React Native Skia : ");
    title.append("OpenGL");
    setTitle(title.c_str());
    show();
    return true;
}

void WindowX11::closeWindow() {
    if (display_) {
        gWindowMap.remove(window_);
        XDestroyWindow(display_, window_);
        window_ = 0;
#if USE(GLX)
        if (fbConfig_) {
            XFree(fbConfig_);
            fbConfig_ = nullptr;
        }
        if (visualInfo_) {
            XFree(visualInfo_);
            visualInfo_ = nullptr;
        }
#endif
        display_ = nullptr;
    }
}

bool WindowX11::handleEvent(const XEvent& event) {
    int shiftLevel= (event.xkey.state & ShiftMask) ? 1 : 0;
    int capsLock = (event.xkey.state & LockMask) ? 1 : 0;
    /* Shift & CapsLock Combination Logic
     * There two possibilties
     * 1. CapsLock ON
     *    a. shift ON  --> LowerCase
     *    b. shift OFF --> UpperCase
     *
     * 2. CapsLock OFF 
     *    a. shift ON  --> UpperCase
     *    b. shift OFF --> LowerCase
     */
    if (event.type == KeyRelease && XEventsQueued(display_, QueuedAfterReading)) {
        XEvent nev;
        XPeekEvent(display_, &nev);
        if (nev.type == KeyPress && nev.xkey.time == event.xkey.time &&
            nev.xkey.keycode == event.xkey.keycode) {
            //Unlike wpe backend, X11 sends both key press and release when key-repeat is on. 
            //this code is for ignoring key release when keyrepeat is on
            RNS_LOG_DEBUG("[handleEvent] KeyRelease is ignored in Key Repeate mode");
            return false;
        }
    }
    KeySym keysym = XkbKeycodeToKeysym(display_, event.xkey.keycode,0,(shiftLevel^capsLock));
    switch (event.type) {
        case MapNotify:
            break;

        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == wmDeleteMessage_ &&
                gWindowMap.count() == 1) {
                return true;
            }
            break;

        case KeyRelease:
            RNS_LOG_DEBUG("[handleEvent] Actual KeyRelease");
        case KeyPress:
            onKey( keyIdentifierForX11KeyCode(keysym), (event.type == KeyRelease ) ? RNS_KEY_Release : RNS_KEY_Press);
            break; 
        case ButtonPress:
            RNS_LOG_NOT_IMPL;
            break;
        case Expose:
            onExpose();
        break;
        default:
            // these events should be handled in the main event loop
            RNS_LOG_ASSERT(event.type != ConfigureNotify, "Should handle this is main loop ??");
            break;
    }
    return false;
}

void WindowX11::setTitle(const char* title) {
    XTextProperty textproperty;
    XStringListToTextProperty(const_cast<char**>(&title), 1, &textproperty);
    XSetWMName(display_, window_, &textproperty);
}

void WindowX11::show() {
    XMapWindow(display_, window_);
}

void WindowX11::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
    RNS_LOG_NOT_IMPL;
    //INHERITED::setRequestedDisplayParams(params, allowReattach);
}

void WindowX11::onExpose() {
    NotificationCenter::defaultCenter().emit("windowExposed",(Window*)this);
}

void WindowX11::onKey(rnsKey eventKeyType, rnsKeyAction eventKeyAction) {
// #if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
    if(winType == SubWindow)
        NotificationCenter::subWindowCenter().emit("onHWKeyEvent", eventKeyType, eventKeyAction, (Window*)this);
    else
// #endif/*FEATURE_ONSCREEN_KEYBOARD*/
        NotificationCenter::defaultCenter().emit("onHWKeyEvent", eventKeyType, eventKeyAction);
    return;
}

}   // namespace RnsShell
