/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "ReactSkia/components/RSkComponentProvider.h"

#include "better/map.h"
#include "react/renderer/componentregistry/ComponentDescriptorRegistry.h"
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
      ComponentDescriptorParameters const &parameters) ;

  RSkComponentProvider *GetProvider(ComponentName componentName);
  RSkComponentProvider *GetProvider(ComponentHandle componentHandle);
  RSkComponentProvider *GetProvider(int tag);

  ComponentDescriptorProviderRegistry& providerRegistry() { return *descriptorProviderRegistry_; }
  const ComponentDescriptor* getComponentDescriptor(ComponentHandle componentHandle);

 private:
  std::unique_ptr<ComponentDescriptorProviderRegistry>
      descriptorProviderRegistry_;
  ComponentDescriptorRegistry::Shared componentDescriptorRegistry_;
  better::map<ComponentHandle, std::unique_ptr<RSkComponentProvider>> registry_;
};

} // namespace react
} // namespace facebook
