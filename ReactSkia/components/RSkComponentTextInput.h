/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <atomic>

#include "modules/skparagraph/include/TextStyle.h"
#include "react/renderer/components/textinput/TextInputShadowNode.h"
#include "react/renderer/components/textinput/TextInputEventEmitter.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/sdk/OnScreenKeyBoard.h"
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
  ~RSkComponentTextInput();
  void onHandleKey(rnsKey eventKeyType, bool keyRepeat, bool *stopPropagation)override;
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpdate) override;
  /*
    TODO  Need to Add command function to Handle Command.
  */
  void handleCommand(std::string commandName,folly::dynamic args)override;
  void onHandleBlur()override;
  void onHandleFocus()override;
 protected:
  void OnPaint(SkCanvas *canvas) override;
 private:
  std::atomic<bool> isInEditingMode_;
  bool editable_ = true;
  bool caretHidden_ = false;
  bool isTextInputInFocus_=false;
  bool secureTextEntry_=false;
  int eventCount_;
  int maxLength_;
  std::string displayString_{}; // Text to be displayed on screen
  std::string placeholderString_{}; // Placeholder Text
  SharedColor placeholderColor_;  // Placeholder Text Color
  SharedColor selectionColor_;
  struct cursor cursor_;
  SkPaint cursorPaint_;
  std::shared_ptr<skia::textlayout::Paragraph> paragraph_;

#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  std::atomic<bool> showSoftInputOnFocus_=true;//To decide OnScreen KeyBoard to be used or not
  bool isOSKActive_=false; // maintaining KB launch state, to decide need to Exit OSK
  rns::sdk::OSKConfig oskLaunchConfig_;
#endif/*FEATURE_ONSCREEN_KEYBOARD*/
  void drawAndSubmit(bool isFlushDisplay=true);
  void drawTextInput(
      SkCanvas *canvas,
      LayoutMetrics layout,
      const TextInputProps& props,
      struct RSkSkTextLayout &textLayout);
  void processEventKey(rnsKey eventKeyType,bool* stopPropagation,bool *waitForupdateProps, bool updateString);
  void keyEventProcessingThread();
  void requestForEditingMode(bool isFlushDisplay = true);
  void resignFromEditingMode(bool isFlushDisplay = true);
  void drawCursor(SkCanvas *canvas, LayoutMetrics layout);
};

} // namespace react
} // namespace facebook
