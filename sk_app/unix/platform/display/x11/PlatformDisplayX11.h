/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PlatformDisplayX11_DEFINED
#define PlatformDisplayX11_DEFINED

#include "PlatformDisplay.h"

#if PLATFORM(X11)
typedef struct _XDisplay Display;

namespace sk_app {

class PlatformDisplayX11 final : public PlatformDisplay {
public:
    static std::unique_ptr<PlatformDisplay> create();
    static std::unique_ptr<PlatformDisplay> create(Display*);

    virtual ~PlatformDisplayX11();

    Display* native() const { return fDisplay; }

private:
    PlatformDisplayX11(Display*, bool);

    Type type() const override { return PlatformDisplay::Type::X11; }

#if USE(EGL)
    void initializeEGLDisplay() override;
#endif

    Display* fDisplay { nullptr };
};

} // namespace sk_app

#endif // PLATFORM(X11)
#endif // PlatformDisplayX11_DEFINED
