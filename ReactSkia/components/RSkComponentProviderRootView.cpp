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
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentRootView>(shadowView));
}

} // namespace react
} // namespace facebook
