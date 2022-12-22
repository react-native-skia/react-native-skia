/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/textlayoutmanager/RSkTextLayoutManager.h"

namespace facebook {
namespace react {

class RSkComponentText final : public RSkComponent {
 public:
  RSkComponentText(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpadte) override;
 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentRawText final : public RSkComponent {
 public:
  RSkComponentRawText(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpadte) override;
 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentParagraph final : public RSkComponent {
 public:
  RSkComponentParagraph(const ShadowView &shadowView);
  uint32_t expectedAttachmentCount;
  uint32_t currentAttachmentCount;
  struct RSkSkTextLayout textLayout;
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpadte) override;
 protected:
  void OnPaint(SkCanvas *canvas) override;

 private:
  /* Method to check if parent is paragraph component */
  ParagraphAttributes paragraphAttributes_;
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
        if(parent->textLayout.builder) {
           return parent;
        } else {
           return parent->getParentParagraph();
        }
     }
     return nullptr;
  }

};

} // namespace react
} // namespace facebook
