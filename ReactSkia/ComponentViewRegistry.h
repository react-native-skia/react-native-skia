#pragma once

#include "ReactSkia/components/RSkComponentProvider.h"

#include "butter/map.h"
#include "react/renderer/componentregistry/ComponentDescriptorProviderRegistry.h"
#include "react/renderer/core/ReactPrimitives.h"

namespace facebook {
namespace react {

class ComponentViewRegistry {
 public:
  ComponentViewRegistry();
  ComponentViewRegistry(ComponentViewRegistry &&) = default;
  ComponentViewRegistry &operator=(ComponentViewRegistry &&) = default;

  void Register(std::unique_ptr<RSkComponentProvider> provider);
  void Register(std::unique_ptr<RSkComponentProvider> provider, ComponentHandle handle);

  ComponentDescriptorRegistry::Shared CreateComponentDescriptorRegistry(
      ComponentDescriptorParameters const &parameters) const;

  RSkComponentProvider *GetProvider(ComponentName componentName);
  RSkComponentProvider *GetProvider(ComponentHandle componentHandle);

  ComponentDescriptorProviderRegistry& providerRegistry() { return *descriptorProviderRegistry_; }

 private:
  std::unique_ptr<ComponentDescriptorProviderRegistry>
      descriptorProviderRegistry_;
  butter::map<ComponentHandle, std::unique_ptr<RSkComponentProvider>> registry_;
};

} // namespace react
} // namespace facebook
