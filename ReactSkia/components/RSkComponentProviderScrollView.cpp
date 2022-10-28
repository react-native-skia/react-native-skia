/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "react/renderer/components/scrollview/ScrollViewComponentDescriptor.h"

#include "ReactSkia/components/RSkComponentProviderScrollView.h"
#include "ReactSkia/components/RSkComponentScrollView.h"

namespace facebook {
namespace react {

RSkComponentProviderScrollView::RSkComponentProviderScrollView() {}

ComponentDescriptorProvider RSkComponentProviderScrollView::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<ScrollViewComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderScrollView::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentScrollView>(shadowView));
}

} // namespace react
} // namespace facebook