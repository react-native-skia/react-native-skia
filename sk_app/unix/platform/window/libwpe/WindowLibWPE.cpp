/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/utils/SkUTF.h"
#include "GLWindowContext.h"
#include "WindowLibWPE.h"
#include "WindowContextFactory.h"

namespace sk_app {

SkTDynamicHash<WindowLibWPE, WPEWindowID> WindowLibWPE::gWindowMap;

bool WindowLibWPE::initViewBackend(wpe_view_backend* viewBackend) {

    if (nullptr == viewBackend) {
        SK_APP_LOG_ERROR("Invalid View Backend\n");
        return false;
    }

    // Register backend client
    static struct wpe_view_backend_client s_backendClient = {
        // set_size
        [](void* data, uint32_t width, uint32_t height)
        {
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            SK_APP_LOG_INFO("View Backend Size (%u x %u)\n", width, height);
            winwpe.setViewSize((int)width, (int)height);
        },
        // frame_displayed
        [](void* data)
        {
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            SK_APP_UNUSED(winwpe);
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
        [](void* data, struct wpe_input_keyboard_event* event)
        {
            SK_APP_NOT_IMPL
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
            if (event->pressed
                && event->modifiers & wpe_input_keyboard_modifier_control
                && event->modifiers & wpe_input_keyboard_modifier_shift
                && event->key_code == WPE_KEY_G) {
                return;
            }
        },
        // handle_pointer_event
        [](void* data, struct wpe_input_pointer_event* event)
        {
            SK_APP_NOT_IMPL
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
        },
        // handle_axis_event
        [](void* data, struct wpe_input_axis_event* event)
        {
            SK_APP_NOT_IMPL
            auto& winwpe = *reinterpret_cast<WindowLibWPE*>(data);
        },
        // handle_touch_event
        [](void* data, struct wpe_input_touch_event* event)
        {
            SK_APP_NOT_IMPL
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

bool WindowLibWPE::initRenderTarget(wpe_view_backend* viewBackend, wpe_renderer_backend_egl* renderBackend) {

    if (nullptr == viewBackend || nullptr == renderBackend) {
        SK_APP_LOG_ERROR("Invalid View (%p) or Render (%p) Backend\n", viewBackend, renderBackend);
        return false;
    }

    fRendererTarget = wpe_renderer_backend_egl_target_create(wpe_view_backend_get_renderer_host_fd(viewBackend));
    static struct wpe_renderer_backend_egl_target_client s_client = {
        // frame_complete
        [](void* data)
        {
        },
        // padding
        nullptr,
        nullptr,
        nullptr,
        nullptr,
#if defined(ENABLE_WINDOW_ID_PATCH)
        //window_id -> This is called from wpebackend-rdk
        [](void* data, unsigned int id)
        {
            SK_APP_NOT_IMPL
        }
#endif
    };
    wpe_renderer_backend_egl_target_set_client(fRendererTarget, &s_client, this);
    if( fViewWidth <=0 || fViewHeight <= 0) {
        SK_APP_LOG_ERROR("Invalid View Size.. using default width and height\n");
        fViewWidth = 1280;
        fViewHeight = 720;
    }
    wpe_renderer_backend_egl_target_initialize(fRendererTarget, renderBackend, std::max(0, fViewWidth), std::max(0, fViewHeight));

    return true;
}

bool WindowLibWPE::initWindow(PlatformDisplay *platformDisplay) {

    Display* display = (dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay))->native();
    if( nullptr == display) {
        SK_APP_LOG_ERROR("Invalid display handler\n");
        return false;
    }

    struct wpe_view_backend* viewBackend = display->viewBackend();
    struct wpe_renderer_backend_egl* renderBackend = (dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay))->renderBackend();

    if (fRequestedDisplayParams.fMSAASampleCount != fMSAASampleCount) {
        this->closeWindow();
    }

    // we already have a window
     if (fDisplay) {
        return true;
    }

    if(false == initViewBackend(viewBackend)) {
        return false;
    }
    if(false == initRenderTarget(viewBackend, renderBackend)) {
        return false;
    }

    fDisplay = display;
    fPlatformDisplay = platformDisplay;
    fWindow = (reinterpret_cast<GLNativeWindowType>(wpe_renderer_backend_egl_target_get_native_window(fRendererTarget)));
    fMSAASampleCount = fRequestedDisplayParams.fMSAASampleCount;

    // add to hashtable of windows
    gWindowMap.add(this);

    // init event variables
    fPendingPaint = false;
    fPendingResize = false;

    return true;
}

void WindowLibWPE::setViewSize(int width, int height) {
    fViewWidth = width;
    fViewHeight = height;
}

void WindowLibWPE::closeWindow() {
    if (fDisplay) {
        fDisplay = nullptr;
    }
    if(fRendererTarget) {
        wpe_renderer_backend_egl_target_destroy(fRendererTarget);
        fRendererTarget = nullptr;
    }
}

bool WindowLibWPE::handleEvent() {
    SK_APP_NOT_IMPL
    return false;
}

void WindowLibWPE::setTitle(const char* title) {
    SK_APP_NOT_IMPL
}

void WindowLibWPE::show() {
    SK_APP_NOT_IMPL
}

bool WindowLibWPE::attach(BackendType attachType) {

    fBackend = attachType;

    this->initWindow(fPlatformDisplay);

    window_context_factory::UnixWindowInfo winInfo;
    winInfo.fWindow = fWindow;
    if( fViewWidth > 0 && fViewHeight > 0) {
        winInfo.fWidth = fViewWidth;
        winInfo.fHeight = fViewHeight;
    } else {
        SK_APP_LOG_ERROR("Invalid View Size.. using default width and height\n");
        winInfo.fWidth = winInfo.fHeight = 0;
    }

    switch (attachType) {
        case kNativeGL_BackendType:
            fWindowContext =
                    window_context_factory::MakeGLForUnix(winInfo, fRequestedDisplayParams);
            break;
        case kRaster_BackendType:
            fWindowContext =
                    window_context_factory::MakeRasterForUnix(winInfo, fRequestedDisplayParams);
            break;
    }

    this->onBackendCreated();
    return (SkToBool(fWindowContext));
}

void WindowLibWPE::didRenderFrame() {
    if(fRendererTarget)
        wpe_renderer_backend_egl_target_frame_rendered(fRendererTarget);
}

void WindowLibWPE::onInval() {
    SK_APP_NOT_IMPL
}

void WindowLibWPE::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
#if defined(SK_VULKAN)
    // Vulkan on unix crashes if we try to reinitialize the vulkan context without remaking the
    // window.
    if (fBackend == kVulkan_BackendType && allowReattach) {
        // Need to change these early, so attach() creates the window context correctly
        fRequestedDisplayParams = params;

        this->detach();
        this->attach(fBackend);
        return;
    }
#endif

    INHERITED::setRequestedDisplayParams(params, allowReattach);
}

}   // namespace sk_app
