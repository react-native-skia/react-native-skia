#include "ReactSkia/components/RSkComponentProviderRootView.h"
#include "ReactSkia/components/RSkComponentRootView.h"

#include "react/renderer/components/root/RootComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderRootView::RSkComponentProviderRootView() {}

ComponentDescriptorProvider
RSkComponentProviderRootView::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<RootComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderRootView::CreateComponent(
    const ShadowView &shadowView) {
  component_ = std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentRootView>(shadowView));
  return component_;
}

std::shared_ptr<RSkComponent> RSkComponentProviderRootView::GetComponent(Tag tag) {
  return component_;
}

} // namespace react
} // namespace facebook
