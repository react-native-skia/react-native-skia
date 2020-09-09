#include "ReactSkia/components/RSkComponentProviderText.h"
#include "ReactSkia/components/RSkComponentText.h"

#include "react/renderer/components/text/ParagraphComponentDescriptor.h"
#include "react/renderer/components/text/RawTextComponentDescriptor.h"
#include "react/renderer/components/text/TextComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderText::RSkComponentProviderText() {}

ComponentDescriptorProvider RSkComponentProviderText::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<TextComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderText::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentText>(shadowView));
}

RSkComponentProviderRawText::RSkComponentProviderRawText() {}

ComponentDescriptorProvider
RSkComponentProviderRawText::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<RawTextComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderRawText::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentRawText>(shadowView));
}

RSkComponentProviderParagraph::RSkComponentProviderParagraph() {}

ComponentDescriptorProvider
RSkComponentProviderParagraph::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<ParagraphComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderParagraph::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentParagraph>(shadowView));
}

} // namespace react
} // namespace facebook
