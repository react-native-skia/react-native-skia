/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "react/renderer/core/ConcreteComponentDescriptor.h"
#include "react/renderer/components/unimplementedview/UnimplementedViewShadowNode.h"

#include "ReactSkia/components/RSkComponentProvider.h"

namespace facebook {
namespace react {

class RSkComponentProviderUnimplementedView : public RSkComponentProvider {
 public:
  RSkComponentProviderUnimplementedView();

 public:
  ComponentDescriptorProvider GetDescriptorProvider() override;
  std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) override;

  std::shared_ptr<RSkComponent> GetComponent(Tag tag) override;

 private:
  std::shared_ptr<RSkComponent> component_;
};

} // namespace react
} // namespace facebook
