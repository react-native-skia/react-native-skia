/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "cxxreact/ModuleRegistry.h"
#include "ReactSkia/ComponentViewRegistry.h"

namespace facebook {
namespace react {

class Instance;

class LegacyNativeModuleRegistry : public ModuleRegistry {
 public:
  LegacyNativeModuleRegistry(ComponentViewRegistry *componentViewRegistry);
};

} // namespace react
} // namespace facebook
