/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cxxreact/CxxModule.h>
#include <cxxreact/NativeModule.h>

namespace facebook {
namespace react {

class LegacyNativeModule : public NativeModule {
 public:
  LegacyNativeModule(
      std::string name,
      xplat::module::CxxModule::Provider provider)
      : name_(std::move(name)),
        provider_(provider) {}

  std::string getName() override;
  std::string getSyncMethodName(unsigned int methodId) override;
  std::vector<MethodDescriptor> getMethods() override;
  folly::dynamic getConstants() override;
  void invoke(unsigned int reactMethodId, folly::dynamic &&params, int callId) override;
  MethodCallResult callSerializableNativeHook(
      unsigned int hookId,
      folly::dynamic &&args) override;

 private:
  void lazyInit();

  std::string name_;
  xplat::module::CxxModule::Provider provider_;
  std::unique_ptr<xplat::module::CxxModule> module_;
  std::vector<xplat::module::CxxModule::Method> methods_;
};


} // namespace react
} // namespace facebook
