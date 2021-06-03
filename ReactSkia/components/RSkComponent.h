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
    : tag(shadowView.tag)
    , componentName(shadowView.componentName ? shadowView.componentName : "Rootview")
    , props(shadowView.props)
    , eventEmitter(shadowView.eventEmitter)
    , layoutMetrics(shadowView.layoutMetrics)
    , state(shadowView.state)
  {}

  Tag tag;
  ComponentName componentName;
  Props::Shared props{};
  EventEmitter::Shared eventEmitter{};
  LayoutMetrics layoutMetrics{EmptyLayoutMetrics};
  State::Shared state{};
};

class RSkComponent;

class RSkComponent : public RnsShell::Layer, public std::enable_shared_from_this<RSkComponent>  {
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

  void requiresLayer(const ShadowView &shadowView);

  RSkComponent *getParent() {return parent_; };

 protected:
  virtual void OnPaint(SkCanvas *canvas) = 0;

 private:
  sk_sp<SkPicture> getPicture();
  // RnsShell::Layer implementations
  void onPaint(SkCanvas*) override;

 private:
  RSkComponent *parent_;
  std::shared_ptr<RnsShell::Layer> layer_;
  Point absOrigin_;
  Component component_;

  typedef Layer INHERITED;
};

} // namespace react
} // namespace facebook
