#include "ReactSkia/components/RSkComponentProviderImage.h"
#include "ReactSkia/components/RSkComponentImage.h"

#include "react/renderer/components/image/ImageComponentDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderImage::RSkComponentProviderImage() {}

ComponentDescriptorProvider RSkComponentProviderImage::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<ImageComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderImage::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentImage>(shadowView));
}

} // namespace react
} // namespace facebook
