/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ReactSkia/components/RSkViewProps.h"
#include "react/renderer/components/view/ConcreteViewShadowNode.h"

namespace facebook {
namespace react {

extern const char RSkViewComponentName[];

/*
 * `ShadowNode` for <View> component.
 */
class RSkViewShadowNode final : public ConcreteViewShadowNode<
                                    RSkViewComponentName,
                                    RSkViewProps,
                                    ViewEventEmitter> {
 public:
  static ShadowNodeTraits BaseTraits() {
    auto traits = BaseShadowNode::BaseTraits();
    traits.set(ShadowNodeTraits::Trait::View);
    return traits;
  }

  RSkViewShadowNode(
      ShadowNodeFragment const &fragment,
      ShadowNodeFamily::Shared const &family,
      ShadowNodeTraits traits);

  RSkViewShadowNode(
      ShadowNode const &sourceShadowNode,
      ShadowNodeFragment const &fragment);

 private:
  void initialize() noexcept;
};

} // namespace react
} // namespace facebook
