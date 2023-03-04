/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ReactSkia/textinput/react/renderer/components/textinput/TextInputShadowNode.h"

#include <react/renderer/core/ConcreteComponentDescriptor.h>

namespace facebook {
namespace react {

/*
 * Descriptor for <TextInput> component.
 */
class TextInputComponentDescriptor final
    : public ConcreteComponentDescriptor<TextInputShadowNode> {
 public:
  TextInputComponentDescriptor(ComponentDescriptorParameters const &parameters)
      : ConcreteComponentDescriptor<TextInputShadowNode>(parameters) {
    textLayoutManager_ =
        std::make_shared<TextLayoutManager const>(contextContainer_);
  }

 protected:
  void adopt(ShadowNode::Unshared const &shadowNode) const override {
    ConcreteComponentDescriptor::adopt(shadowNode);

    auto concreteShadowNode =
        std::static_pointer_cast<TextInputShadowNode>(shadowNode);

    concreteShadowNode->setTextLayoutManager(textLayoutManager_);
  }

 private:
  std::shared_ptr<TextLayoutManager const> textLayoutManager_;
};

} // namespace react
} // namespace facebook
