/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "PlatformDisplay.h"

namespace RnsShell {

class PlatformDisplayMac final : public PlatformDisplay {
 public:
  static std::unique_ptr<PlatformDisplay> create();

  virtual ~PlatformDisplayMac();

 public:
  SkSize screenSize() override;
  float scaleFactor() override;

 private:
  PlatformDisplayMac();
  Type type() const override {
    return PlatformDisplay::Type::Mac;
  }
};

} // namespace RnsShell
