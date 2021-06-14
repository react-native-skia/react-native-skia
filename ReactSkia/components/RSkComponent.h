#pragma once

#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/utils/RnsLog.h"

#include "include/core/SkCanvas.h"
#include "react/renderer/mounting/ShadowView.h"

#include "rns_shell/compositor/layers/Layer.h"

namespace facebook {
namespace react {

enum ComponentUpdateMask {
  ComponentUpdateMaskNone = 0,
  ComponentUpdateMaskProps = 1 << 0,
  ComponentUpdateMaskEventEmitter = 1 << 1,
  ComponentUpdateMaskState = 1 << 2,
  ComponentUpdateMaskLayoutMetrics = 1 << 3,

  ComponentUpdateMaskAll = ComponentUpdateMaskProps | ComponentUpdateMaskEventEmitter |
      ComponentUpdateMaskState | ComponentUpdateMaskLayoutMetrics
};

struct Component {
  Component( const ShadowView &shadowView)
    : props(shadowView.props)
    , eventEmitter(shadowView.eventEmitter)
    , layoutMetrics(shadowView.layoutMetrics)
    , state(shadowView.state)
  {}

  Props::Shared props{};
  EventEmitter::Shared eventEmitter{};
  LayoutMetrics layoutMetrics{EmptyLayoutMetrics};
  State::Shared state{};
};

class RSkComponent;

class RSkComponent : public RnsShell::Layer {
 public:
  RSkComponent(const ShadowView &shadowView);
  RSkComponent(RSkComponent &&) = default;
  RSkComponent &operator=(RSkComponent &&) = default;

  virtual ~RSkComponent();

  virtual void mountChildComponent(
    std::shared_ptr<RSkComponent> newChildComponent,
    const int index);

  virtual void unmountChildComponent(
    std::shared_ptr<RSkComponent> oldChildComponent,
    const int index);

  virtual void updateComponentData(const ShadowView &newShadowView , const uint32_t updateMask);
  Component getComponentData() { return component_;};
  Rect getAbsoluteFrame(){return Rect{absOrigin_,component_.layoutMetrics.frame.size} ;};
  std::shared_ptr<RnsShell::Layer> layer() { return layer_; }

 protected:
  virtual void OnPaint(SkCanvas *canvas) = 0;

 private:
  void requiresLayer(const ShadowView &shadowView);
  sk_sp<SkPicture> getPicture();
  // RnsShell::Layer implementations
  void onPaint(SkSurface *surface) override;

 private:
  RSkComponent *parent_;
  std::shared_ptr<RnsShell::Layer> layer_;
  Point absOrigin_;
  Component component_;
#ifdef RNS_ENABLE_API_PERF
  ComponentName componentName_;
#endif
};

} // namespace react
} // namespace facebook
