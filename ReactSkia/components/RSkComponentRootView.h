#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentRootView final : public RSkComponent {
 public:
  RSkComponentRootView(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;

  virtual bool isContainer() const override { return true; }
  virtual bool isVisible(RSkComponent* candidate) override;

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
