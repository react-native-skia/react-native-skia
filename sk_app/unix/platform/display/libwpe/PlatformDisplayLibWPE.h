/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PlatformDisplayLibWPE_DEFINED
#define PlatformDisplayLibWPE_DEFINED

#include "PlatformDisplay.h"

#if USE(WPE_RENDERER)

struct wpe_view_backend;
struct wpe_renderer_backend_egl;

namespace sk_app {

class Display {
public:
    struct wpe_view_backend* viewBackend() const { return fViewBackend; }

    Display();

private:
    ~Display();
    struct wpe_view_backend* fViewBackend = { nullptr };
};

class PlatformDisplayLibWPE final : public PlatformDisplay {
public:
    static std::unique_ptr<PlatformDisplayLibWPE> create();

    virtual ~PlatformDisplayLibWPE();

    bool initialize(int);

    Display* native() const { return fDisplay; }
    struct wpe_renderer_backend_egl* renderBackend() const { return fRendererBackend; }

private:
    PlatformDisplayLibWPE(Display *display);

    Type type() const override { return PlatformDisplay::Type::WPE; }

    Display* fDisplay;
    struct wpe_renderer_backend_egl* fRendererBackend = { nullptr };
};

} // namespace sk_app

#endif // USE(WPE_RENDERER)
#endif // PlatformDisplayLibWPE_DEFINED
