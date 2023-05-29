#include "ReactSkia/components/RSkComponentProviderSafeAreaView.h"
#include "ReactSkia/components/RSkComponentSafeAreaView.h"

//#include "react/renderer/components/rncore/ComponentDescriptors.h"
#include <react/renderer/components/safeareaview/SafeAreaViewComponentDescriptor.h>
#include <react/renderer/components/safeareaview/SafeAreaViewShadowNode.h>

namespace facebook {
namespace react {

//using SafeAreaViewComponentDescriptor = ConcreteComponentDescriptor<SafeAreaViewShadowNode>;

RSkComponentProviderSafeAreaView::RSkComponentProviderSafeAreaView() {}

ComponentDescriptorProvider RSkComponentProviderSafeAreaView::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<SafeAreaViewComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderSafeAreaView::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentSafeAreaView>(shadowView));
}

} // namespace react
} // namespace facebook
