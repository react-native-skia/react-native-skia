#include "ReactSkia/components/RSkComponentProviderView.h"
#include "ReactSkia/components/RSkComponentView.h"
#include "ReactSkia/components/RSkViewComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderView::RSkComponentProviderView() {}

ComponentDescriptorProvider RSkComponentProviderView::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<RSkViewComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderView::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentView>(shadowView));
}

} // namespace react
} // namespace facebook
