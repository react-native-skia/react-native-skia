/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/components/RSkComponentProviderTextInput.h"
#include "ReactSkia/components/RSkComponentTextInput.h"

#include "react/renderer/components/textinput/TextInputComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderTextInput::RSkComponentProviderTextInput() {}

ComponentDescriptorProvider RSkComponentProviderTextInput::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<TextInputComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderTextInput::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentTextInput>(shadowView));
}

} // namespace react
} // namespace facebook
