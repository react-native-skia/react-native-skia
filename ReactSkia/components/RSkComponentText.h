#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/textlayoutmanager/RSkTextLayoutManager.h"

namespace facebook {
namespace react {

class RSkComponentText final : public RSkComponent {
 public:
  RSkComponentText(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentRawText final : public RSkComponent {
 public:
  RSkComponentRawText(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentParagraph final : public RSkComponent {
 public:
  RSkComponentParagraph(const ShadowView &shadowView);
  std::shared_ptr<skia::textlayout::ParagraphBuilder> paraBuilder;
  uint32_t expectedAttachmentCount;
  uint32_t currentAttachmentCount;

 protected:
  void OnPaint(SkCanvas *canvas) override;

 private:
  /* Method to check if parent is paragraph component */
  bool isParentParagraph() {
     RSkComponent *parent = getParent();
     if((nullptr != parent)
        && ( nullptr != parent->getComponentData().componentName)
        && (!strcmp(parent->getComponentData().componentName,"Paragraph"))){
           return true;
     }
     return false;
  }

  /* Method to get parent paragraph component,
   * until it reaches the top paragraph parent which has paragraph builder*/
  RSkComponentParagraph* getParentParagraph() {
     if(isParentParagraph()) {
        RSkComponentParagraph *parent = (RSkComponentParagraph*)getParent();
        if(parent->paraBuilder) {
           return parent;
        } else {
           return parent->getParentParagraph();
        }
     }
     return nullptr;
  }

  RSkTextLayoutManager textLayoutManager_;
};

} // namespace react
} // namespace facebook
