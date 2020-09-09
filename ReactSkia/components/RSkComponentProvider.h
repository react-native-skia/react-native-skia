#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "react/renderer/componentregistry/ComponentDescriptorProvider.h"
#include "react/renderer/mounting/ShadowView.h"

namespace facebook {
namespace react {

class RSkComponentProvider {
 public:
  RSkComponentProvider() = default;
  virtual ~RSkComponentProvider() = default;

 public:
  virtual ComponentDescriptorProvider GetDescriptorProvider() = 0;
  virtual std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) = 0;
};

} // namespace react
} // namespace facebook
