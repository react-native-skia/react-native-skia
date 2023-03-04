#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "react/renderer/componentregistry/ComponentDescriptorProvider.h"
#include "react/renderer/mounting/ShadowView.h"

namespace facebook {
namespace react {

class RSkComponentProvider {
 public:
  RSkComponentProvider() = default;
  virtual ~RSkComponentProvider() = default;

 public:
  virtual ComponentDescriptorProvider GetDescriptorProvider() = 0;
  virtual std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) = 0;

  virtual std::shared_ptr<RSkComponent> GetComponent(Tag tag) {
      auto it = registry_.find(tag);
      if (it != registry_.end()) {
           return it->second;
      }
      return nullptr;
  }

  std::shared_ptr<RSkComponent> CreateAndAddComponent(const ShadowView &shadowView) {
      auto component = this->CreateComponent(shadowView);
      registry_[shadowView.tag] = component;
      return component;
  }

  void DeleteComponent(Tag tag) {
      auto it = registry_.find(tag);
      if (it != registry_.end()) {
           auto component = it->second;
           registry_.erase(tag);
           component.reset();
      }
  }

 private:
  butter::map<Tag, std::shared_ptr<RSkComponent>> registry_;

};
using RSkComponentProviderProtocol = RSkComponentProvider *(*)();

} // namespace react
} // namespace facebook
