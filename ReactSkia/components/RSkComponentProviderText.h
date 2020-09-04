#pragma once

#include "ReactSkia/components/RSkComponentProvider.h"

namespace facebook {
namespace react {

class RSkComponentProviderText : public RSkComponentProvider {
 public:
  RSkComponentProviderText();

 public:
  ComponentDescriptorProvider GetDescriptorProvider() override;
  std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) override;
};

class RSkComponentProviderRawText : public RSkComponentProvider {
 public:
  RSkComponentProviderRawText();

 public:
  ComponentDescriptorProvider GetDescriptorProvider() override;
  std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) override;
};

class RSkComponentProviderParagraph : public RSkComponentProvider {
 public:
  RSkComponentProviderParagraph();

 public:
  ComponentDescriptorProvider GetDescriptorProvider() override;
  std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) override;
};

} // namespace react
} // namespace facebook
