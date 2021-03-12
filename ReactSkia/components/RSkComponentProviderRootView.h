#pragma once

#include "ReactSkia/components/RSkComponentProvider.h"

namespace facebook {
namespace react {

class RSkComponentProviderRootView : public RSkComponentProvider {
 public:
  RSkComponentProviderRootView();

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
