#include "ReactSkia/ComponentViewRegistry.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

ComponentViewRegistry::ComponentViewRegistry() {
  descriptorProviderRegistry_ =
      std::make_unique<ComponentDescriptorProviderRegistry>();
}

ComponentDescriptorRegistry::Shared
ComponentViewRegistry::CreateComponentDescriptorRegistry(
    ComponentDescriptorParameters const &parameters) const {
  return descriptorProviderRegistry_->createComponentDescriptorRegistry(
      parameters);
}

void ComponentViewRegistry::Register(
    std::unique_ptr<RSkComponentProvider> provider) {
  auto descriptorProvider = provider->GetDescriptorProvider();
  descriptorProviderRegistry_->add(descriptorProvider);
  registry_[descriptorProvider.handle] = std::move(provider);
}

RSkComponentProvider *ComponentViewRegistry::GetProvider(
    ComponentName componentName) {
  for (const auto &kv : registry_) {
    if (kv.second->GetDescriptorProvider().name == componentName) {
      return kv.second.get();
    }
  }
  return nullptr;
}

RSkComponentProvider *ComponentViewRegistry::GetProvider(
    ComponentHandle componentHandle) {
  auto it = registry_.find(componentHandle);
  if (it != registry_.end()) {
    return it->second.get();
  }
  return nullptr;
}

} // namespace react
} // namespace facebook
