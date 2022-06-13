#pragma once
#include "include/core/SkCanvas.h"

#include "react/renderer/mounting/ShadowView.h"
#include "react/renderer/components/view/ViewProps.h"

#include "ReactSkia/core_modules/RSkSpatialNavigatorContainer.h"

#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

#include "rns_shell/compositor/layers/Layer.h"

namespace facebook {
namespace react {

using namespace RnsShell;

enum PictureType {
   PictureTypeShadow = 1,
   PictureTypeBorder,
   PictureTypeAll
};

enum ComponentUpdateMask {
  ComponentUpdateMaskNone = 0,
  ComponentUpdateMaskProps = 1 << 0,
  ComponentUpdateMaskEventEmitter = 1 << 1,
  ComponentUpdateMaskState = 1 << 2,
  ComponentUpdateMaskLayoutMetrics = 1 << 3,

  ComponentUpdateMaskAll = ComponentUpdateMaskProps | ComponentUpdateMaskEventEmitter |
      ComponentUpdateMaskState | ComponentUpdateMaskLayoutMetrics
};

struct CommonProps{
    SkColor backgroundColor;
    SkColor foregroundColor;
    int pointerEvents;
    EdgeInsets hitSlop;
    int zIndex{};
    /* TODO Add TVOS properties */
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
  struct CommonProps commonProps;
};

class RSkComponent;

class RSkComponent : public RnsShell::Layer , public SpatialNavigator::Container, public std::enable_shared_from_this<RSkComponent>  {
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

  virtual void updateComponentData(const ShadowView &newShadowView , const uint32_t updateMask , bool forceUpdate);

  virtual RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpadate) = 0;

  virtual RnsShell::LayerInvalidateMask updateComponentState(const ShadowView &newShadowView,bool forceUpadate) {
     /* TODO Return default None here when state update is handled with proper mask */
     return RnsShell::LayerInvalidateAll;
  };
  virtual void handleCommand(std::string commandName,folly::dynamic args){RNS_LOG_NOT_IMPL;};
  virtual void onHandleKey(rnsKey  eventKeyType, bool keyRepeat, bool* stopPropagate){*stopPropagate=false;};
  virtual bool isContainer() const { return false; }
  virtual void onHandleBlur() {RNS_LOG_DEBUG("[onHandleBlur] componentName "<<component_.componentName);};
  virtual void onHandleFocus() {RNS_LOG_DEBUG("[onHandleFocus]componentName "<<component_.componentName);};
  Component getComponentData() { return component_;}
  std::shared_ptr<RnsShell::Layer> layer() { return layer_; }
  const SkIRect& getLayerAbsoluteFrame(){ return(layer_->absoluteFrame());}
  const SkIRect getScreenFrame();
  RSkComponent *getParent() {return parent_; };

  SpatialNavigator::Container *nearestAncestorContainer();
  bool hasAncestor(const SpatialNavigator::Container* ancestor);
  bool isFocusable();

  void requiresLayer(const ShadowView &shadowView, Layer::Client& layerClient);
  RnsShell::LayerInvalidateMask updateProps(const ShadowView &newShadowView , bool forceUpdate);

 protected:
  virtual void OnPaint(SkCanvas *canvas) = 0;
  virtual void OnPaintShadow(SkCanvas *canvas);
  virtual void OnPaintBorder(SkCanvas *canvas);
  sk_sp<SkPicture> getPicture(PictureType type=PictureTypeAll);
 private:
  // RnsShell::Layer implementations
  void onPaint(SkCanvas*) override;

 private:
  RSkComponent *parent_;
  std::shared_ptr<RnsShell::Layer> layer_;
  Component component_;

  typedef Layer INHERITED;
};

} // namespace react
} // namespace facebook
