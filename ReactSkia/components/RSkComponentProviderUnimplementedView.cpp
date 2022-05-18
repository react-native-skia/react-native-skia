/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "ReactSkia/ComponentViewRegistry.h"
#include "ReactSkia/components/RSkComponentProviderUnimplementedView.h"
#include "ReactSkia/components/RSkComponentUnimplementedView.h"

#include "react/renderer/components/unimplementedview/UnimplementedViewComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderUnimplementedView::RSkComponentProviderUnimplementedView() {}

ComponentDescriptorProvider
RSkComponentProviderUnimplementedView::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<UnimplementedViewComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderUnimplementedView::CreateComponent(
    const ShadowView &shadowView) {
  component_ = std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentUnimplementedView>(shadowView));
  return component_;
}

std::shared_ptr<RSkComponent> RSkComponentProviderUnimplementedView::GetComponent(Tag tag) {
  return component_;
}

} // namespace react
} // namespace facebook
