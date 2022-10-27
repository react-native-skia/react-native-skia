/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cxxreact/CxxNativeModule.h>
#include <cxxreact/CxxModule.h>
#include <cxxreact/MessageQueueThread.h>
#include <cxxreact/NativeModule.h>

namespace facebook {
namespace react {

using RSkLegacyNativeModuleProviderProtocol = xplat::module::CxxModule*(*)();

class LegacyNativeModule : public CxxNativeModule {
 public:
  LegacyNativeModule(
      std::string name,
      std::weak_ptr<Instance> instance,
      xplat::module::CxxModule::Provider provider,
      std::shared_ptr<MessageQueueThread> messageQueueThread)
      : CxxNativeModule(instance,name,provider,messageQueueThread) {}
};

} // namespace react
} // namespace facebook
