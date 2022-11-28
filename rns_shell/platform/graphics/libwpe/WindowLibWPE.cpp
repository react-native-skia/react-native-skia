/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/utils/SkUTF.h"
#include "WindowContextFactory.h"
#include "WindowLibWPE.h"
#include "platform/linux/TaskLoop.h"

using namespace std; 
namespace RnsShell {

SkTDynamicHash<WindowLibWPE, WPEWindowID> WindowLibWPE::gWindowMap;
Window* Window::mainWindow_;

GMainLoop       *WindowLibWPE::mainLoop_;
Application     *WindowLibWPE::mainApp_;

WindowLibWPE::~WindowLibWPE() {
    closeWindow();
    if(viewBackend_) {
        wpe_view_backend_destroy(viewBackend_);
        viewBackend_ = nullptr;
    }
    if(this == mainWindow_)
        mainWindow_ = nullptr;
}

bool WindowLibWPE::initViewBackend(wpe_view_backend* viewBackend) {

    if (nullptr == viewBackend) {
        RNS_LOG_ERROR("Invalid View Backend");
        return false;
    }

    // Register backend client
    static struct wpe_view_backend_client s_backendClient = {
        // set_size
        [](void* data, uint32_t width, uint32_t height) {
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            RNS_LOG_INFO("View Backend Size (" << width << " x " << height << ")");
            winwpe.setViewSize((int)width, (int)height);
        },
        // frame_displayed
        [](void* data) {
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            RNS_UNUSED(winwpe);
        },
        // padding
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
    wpe_view_backend_set_backend_client(viewBackend, &s_backendClient, this);

    // Register Input client
    static struct wpe_view_backend_input_client s_inputClient = {
        // handle_keyboard_event
        [](void* data, struct wpe_input_keyboard_event* event) {
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            rnsKey keycode = winwpe.keyIdentifierForWPEKeyCode(event->key_code);
            winwpe.onKey(keycode,event->pressed ?RNS_KEY_Press:RNS_KEY_Release);
            if (event->pressed
                && event->modifiers & wpe_input_keyboard_modifier_control
                && event->modifiers & wpe_input_keyboard_modifier_shift
                && event->key_code == WPE_KEY_Escape) {
                return;
            } else if(event->key_code == WPE_KEY_Escape) {
                g_main_loop_quit(mainLoop_);
                return;
            }
        },
        // handle_pointer_event
        [](void* data, struct wpe_input_pointer_event* event) {
            RNS_LOG_NOT_IMPL;
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
        },
        // handle_axis_event
        [](void* data, struct wpe_input_axis_event* event) {
            RNS_LOG_NOT_IMPL;
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
        },
        // handle_touch_event
        [](void* data, struct wpe_input_touch_event* event) {
            RNS_LOG_NOT_IMPL;
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
        },
        // padding
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    wpe_view_backend_set_input_client(viewBackend, &s_inputClient, this);
    wpe_view_backend_initialize(viewBackend);

    return true;
}

bool WindowLibWPE::initRenderTarget(wpe_view_backend* viewBackend, wpe_renderer_backend_egl* renderBackend,SkSize dimension) {

    if (nullptr == viewBackend || nullptr == renderBackend) {
        RNS_LOG_ERROR("Invalid View (" << viewBackend << ") or Render (" << renderBackend << ") Backend");
        return false;
    }

    rendererTarget_ = wpe_renderer_backend_egl_target_create(wpe_view_backend_get_renderer_host_fd(viewBackend));
    static struct wpe_renderer_backend_egl_target_client s_client = {
        // frame_complete
        [](void* data) {
        },
        // padding
        nullptr,
        nullptr,
        nullptr,
        nullptr,
#if defined(ENABLE_WINDOW_ID_PATCH)
        //window_id -> This is called from wpebackend-rdk
        [](void* data, unsigned int id) {
            RNS_LOG_NOT_IMPL;
        }
#endif
    };
    wpe_renderer_backend_egl_target_set_client(rendererTarget_, &s_client, this);
    if(!dimension.isEmpty()) {
       viewWidth_ =  dimension.width();
       viewHeight_ = dimension.height();
    } else if( viewWidth_ <= 0 || viewHeight_ <= 0) {
        RNS_LOG_ERROR("Invalid View Size.. using default width and height");
        viewWidth_ = 1280;
        viewHeight_ = 720;
    }
    wpe_renderer_backend_egl_target_initialize(rendererTarget_, renderBackend, std::max(0, viewWidth_), std::max(0, viewHeight_));

    return true;
}

// Blocking eventLoop for LIBWPE events, created with thread in main()
void Window::createEventLoop(Application* app) {
    WindowLibWPE::mainLoop_ = g_main_loop_new(g_main_context_get_thread_default(), FALSE);
    WindowLibWPE::mainApp_  = app;

    g_main_loop_run(WindowLibWPE::mainLoop_);

    TaskLoop::main().stop();
    return;
}

Window* Window::createNativeWindow(void* platformData,SkSize dimension,WindowType type) {
    PlatformDisplay *pDisplay = (PlatformDisplay*) platformData;

    RNS_LOG_ASSERT(pDisplay, "Invalid Platform Display");

    WindowLibWPE* window = new WindowLibWPE();
    if (!window->initWindow(pDisplay,dimension,type)) {
        delete window;
        return nullptr;
    }
    window->winType=type;
    if(!mainWindow_)
        mainWindow_ = window;
    return window;
}

bool WindowLibWPE::initWindow(PlatformDisplay *platformDisplay,SkSize dimension,WindowType winType) {

    Display* display = (dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay))->native();
    if( nullptr == display) {
        RNS_LOG_ERROR("Invalid display handler");
        return false;
    }

    viewBackend_=wpe_view_backend_create();//Creating ViewBackend per window
    struct wpe_renderer_backend_egl* renderBackend = (dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay))->renderBackend();

    if (requestedDisplayParams_.msaaSampleCount_ != MSAASampleCount_) {
        this->closeWindow();
    }

    // we already have a window
     if (display_) {
        return true;
    }

    display_ = display;
    platformDisplay_ = platformDisplay;
    MSAASampleCount_ = requestedDisplayParams_.msaaSampleCount_;

    if(false == initViewBackend(viewBackend_)) {
        return false;
    }
    if(false == initRenderTarget(viewBackend_, renderBackend,dimension)) {
        return false;
    }
    window_ = (reinterpret_cast<GLNativeWindowType>(wpe_renderer_backend_egl_target_get_native_window(rendererTarget_)));

    // add to hashtable of windows
    gWindowMap.add(this);

    if((this->winType == MainWindow ) && (WindowLibWPE::mainApp_))
        WindowLibWPE::mainApp_->sizeChanged(viewWidth_, viewHeight_);

    return true;
}

void WindowLibWPE::setViewSize(int width, int height) {
    Display* display = (dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay_))->native();
    display->setScreenSize(width, height);

    viewWidth_ = width;
    viewHeight_ = height;
    if((this->winType == MainWindow ) && (WindowLibWPE::mainApp_) )
        WindowLibWPE::mainApp_->sizeChanged(viewWidth_, viewHeight_);
}

void WindowLibWPE::closeWindow() {
    if (display_) {
        display_ = nullptr;
    }
    if(rendererTarget_) {
        wpe_renderer_backend_egl_target_destroy(rendererTarget_);
        rendererTarget_ = nullptr;
    }
}

bool WindowLibWPE::handleEvent() {
    RNS_LOG_NOT_IMPL;
    return false;
}

void WindowLibWPE::setTitle(const char* title) {
    RNS_LOG_NOT_IMPL;
}

void WindowLibWPE::show() {
    RNS_LOG_NOT_IMPL;
}

void WindowLibWPE::didRenderFrame() {
    if(rendererTarget_)
        wpe_renderer_backend_egl_target_frame_rendered(rendererTarget_);
}

void WindowLibWPE::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
#if defined(SK_VULKAN)
    // Vulkan on unix crashes if we try to reinitialize the vulkan context without remaking the
    // window.
    if (backend_ == kVulkan_BackendType && allowReattach) {
        // Need to change these early, so attach() creates the window context correctly
        requestedDisplayParams_ = params;

        this->detach();
        this->attach(fBackend);
        return;
    }
#endif

    INHERITED::setRequestedDisplayParams(params, allowReattach);
}

void WindowLibWPE::onKey(rnsKey eventKeyType, rnsKeyAction eventKeyAction){
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD) || ENABLE(FEATURE_ALERT)
    if(winType == SubWindow) {
        NotificationCenter::subWindowCenter().emit("onHWKeyEvent", eventKeyType, eventKeyAction,(Window*)this);
    } else
#endif/*FEATURE_ONSCREEN_KEYBOARD*/
    {
        NotificationCenter::defaultCenter().emit("onHWKeyEvent", eventKeyType, eventKeyAction);
    }
    return;
}

}   // namespace RnsShell
