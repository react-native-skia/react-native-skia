/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "modules/skparagraph/include/TextStyle.h"
#include "react/renderer/components/textinput/TextInputShadowNode.h"
#include "react/renderer/components/textinput/TextInputEventEmitter.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/textlayoutmanager/RSkTextLayoutManager.h"


namespace facebook {
namespace react {
struct cursor{
  int locationFromEnd;//Relative position from end
  int end;// lentgh of the string
};
class RSkComponentTextInput final : public RSkComponent {
 public:
  RSkComponentTextInput(const ShadowView &shadowView);
  void onHandleKey(rnsKey  eventKeyType,bool *stopPropagate)override;
  RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
  /*
    TODO  Need to Add command function to Handle Command.
  */
 protected:
  void OnPaint(SkCanvas *canvas) override;
 private:
  bool isInEditingMode_;
  std::string displayString_{}; // Text to be displayed on screen
  std::string placeholderString_{}; // Placeholder Text
  SharedColor placeholderColor_;  // Placeholder Text Color
  bool mutableFlag_ = false;
  int eventCount_;
  struct cursor cursor_;
  //Moved drawtextinput as private member of the class.
  std::shared_ptr<skia::textlayout::Paragraph> paragraph_;
  void drawTextInput(
      SkCanvas *canvas,
      LayoutMetrics layout,
      std::shared_ptr<skia::textlayout::ParagraphBuilder> &builder,
      const TextInputProps& props);
};

} // namespace react
} // namespace facebook
