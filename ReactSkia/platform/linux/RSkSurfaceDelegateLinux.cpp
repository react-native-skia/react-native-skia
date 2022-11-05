/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/platform/linux/RSkSurfaceDelegateLinux.h"

namespace ReactSkia {

// static
std::unique_ptr<RSkSurfaceDelegate> createSurfaceDelegate() {
  return std::make_unique<RSkSurfaceDelegateLinux>();
}

void RSkSurfaceDelegateLinux::SurfaceDidStart(
    facebook::react::RSkSurfaceWindow *surface,
    facebook::react::UIManager *uiManager,
    RnsShell::RendererDelegate *nativeRendererDelegate) {}

} // namespace ReactSkia
