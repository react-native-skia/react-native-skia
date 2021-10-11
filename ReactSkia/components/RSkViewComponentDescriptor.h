#pragma once

#include "ReactSkia/components/RSkViewShadowNode.h"
#include "react/renderer/core/ConcreteComponentDescriptor.h"

namespace facebook {
namespace react {

class RSkViewComponentDescriptor
    : public ConcreteComponentDescriptor<RSkViewShadowNode> {
 public:
  RSkViewComponentDescriptor(ComponentDescriptorParameters const &parameters)
      : ConcreteComponentDescriptor<RSkViewShadowNode>(parameters) {}
};

} // namespace react
} // namespace facebook
