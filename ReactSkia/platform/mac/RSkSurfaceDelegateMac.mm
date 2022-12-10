/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/platform/mac/RSkSurfaceDelegateMac.h"

#include "ReactSkia/platform/mac/RCTSurfaceTouchHandler.h"
#include "rns_shell/compositor/RendererDelegate.h"
#include "rns_shell/platform/mac/WindowMac.h"

namespace ReactSkia {

// static
std::unique_ptr<RSkSurfaceDelegate> RSkSurfaceDelegate::createSurfaceDelegate()
{
  return std::make_unique<RSkSurfaceDelegateMac>();
}

void RSkSurfaceDelegateMac::SurfaceDidStart(
    facebook::react::RSkSurfaceWindow *surface,
    facebook::react::UIManager *uiManager,
    RnsShell::RendererDelegate *nativeRendererDelegate)
{
  RCTSurfaceTouchHandler *surfaceTouchHandler = [RCTSurfaceTouchHandler new];
  auto *window = reinterpret_cast<RnsShell::WindowMac *>(nativeRendererDelegate->nativeWindow());
  [surfaceTouchHandler attachToView:window->rootView() withSurface:surface uiManager:uiManager];
}

} // namespace ReactSkia
