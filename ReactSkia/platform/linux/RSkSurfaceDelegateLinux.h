/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ReactSkia/RSkSurfaceDelegate.h"

namespace ReactSkia {

class RSkSurfaceDelegateLinux : public RSkSurfaceDelegate {
 public:
  RSkSurfaceDelegateLinux() = default;
  RSkSurfaceDelegateLinux(const RSkSurfaceDelegateLinux &) = delete;
  RSkSurfaceDelegateLinux &operator=(const RSkSurfaceDelegateLinux &) = delete;
  RSkSurfaceDelegateLinux(RSkSurfaceDelegateLinux &&other) = default;

 public:
  void SurfaceDidStart(
      facebook::react::RSkSurfaceWindow *surface,
      facebook::react::UIManager *uiManager,
      RnsShell::RendererDelegate *nativeRendererDelegate) override;
};

} // namespace ReactSkia
