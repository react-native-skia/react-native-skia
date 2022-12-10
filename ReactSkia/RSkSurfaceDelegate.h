/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

namespace RnsShell {
class RendererDelegate;
} // namespace RnsShell

namespace facebook {
namespace react {
class RSkSurfaceWindow;
class UIManager;
} // namespace react
} // namespace facebook

namespace ReactSkia {

/**
 * A RSkSurfaceWindow delegate to do platform customization
 */
class RSkSurfaceDelegate {
 public:
  virtual ~RSkSurfaceDelegate() = default;

  virtual void SurfaceDidStart(
      facebook::react::RSkSurfaceWindow *surface,
      facebook::react::UIManager *uiManager,
      RnsShell::RendererDelegate *nativeRendererDelegate) = 0;

 public:
  static std::unique_ptr<RSkSurfaceDelegate> createSurfaceDelegate();
};

} // namespace ReactSkia
