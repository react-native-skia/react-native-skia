/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "ReactSkia/utils/RnsUtils.h"

#if PLATFORM(LIBWPE) || USE(WPE_RENDERER)
#include <wpe/wpe.h>
#include <wpe/wpe-egl.h>

#include "PlatformDisplay.h"

namespace RnsShell {

class Display {
public:
    struct wpe_view_backend* viewBackend() const { return viewBackend_; }

    Display();
    struct Screen {
        int     width;
        int     height;
    };
    Screen screen() { return screen_;}
    void setScreenSize(int w, int h) { screen_.width = w; screen_.height = h;}
private:
    ~Display();
    struct wpe_view_backend* viewBackend_ = { nullptr };
    Screen screen_;
};

class PlatformDisplayLibWPE final : public PlatformDisplay {
public:
    static std::unique_ptr<PlatformDisplayLibWPE> create();

    virtual ~PlatformDisplayLibWPE();

    bool initialize(int);

    Display* native() const { return display_; }
    struct wpe_renderer_backend_egl* renderBackend() const { return rendererBackend_; }

private:
    PlatformDisplayLibWPE(Display *display);

    Type type() const override { return PlatformDisplay::Type::WPE; }

    Display* display_;
    struct wpe_renderer_backend_egl* rendererBackend_ = { nullptr };
};

} // namespace RnsShell

#endif // USE(WPE_RENDERER)
