/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cxxreact/JsArgumentHelpers.h>

#include "ReactSkia/utils/RnsLog.h"

#include "UiManagerModule.h"

// Reference : react-native/ReactCommon/cxxreact/SampleCxxModule.cpp

using namespace folly;

namespace facebook {
namespace xplat {
namespace uimanager {

// To be Generated using codeGen ??
dynamic Uimanager::getConstantsForThirdpartyViewManager(std::string viewManagerName) {
  throw std::invalid_argument("Invalid getConstantsForViewManager viewManagerName");
  return folly::dynamic::object();
}

dynamic Uimanager::getConstantsForViewManager(std::string viewManagerName) {
  if(viewManagerName == "RCTView") {
    auto nativeProps = folly::dynamic::object("onLayout", true)("pointerEvents", true)
#if defined(TARGET_OS_TV) && TARGET_OS_TV
      ("focusable", true)("isTVSelectable", true)
      ("hasTVPreferredFocus", true)("tvParallaxProperties", true)("nextFocusUp", true)
      ("nextFocusDown", true)("nextFocusLeft", true)("nextFocusRight", true)
#endif //TARGET_OS_TV
      ("overflow", true);

    auto bubblingEventTypes = folly::dynamic::object();
    auto directEventTypes = folly::dynamic::object("topLayout", folly::dynamic::object("registrationName", "onLayout"));

    auto registry = folly::dynamic::object(
        NATIVE_PROPS_KEY, std::move(nativeProps))(
        BUBBLING_EVENTS_KEY, std::move(bubblingEventTypes))(
        DIRECT_EVENTS_KEY, std::move(directEventTypes));
    return {std::move(registry)};
  } else if (viewManagerName == "RCTImageView") {
     auto nativeProps = folly::dynamic::object("blurRadius", true)(
      "defaultSrc", true)("fadeDuration", true)("headers", true)(
      "loadingIndicatorSrc", true)("onError", true)("onLoad", true)(
      "onLoadEnd", true)("onLoadStart", true)("onPartialLoad", true)(
      "onProgress", true)("progressiveRenderingEnabled", true)(
      "resizeMethod", true)("resizeMode", true)(
      "shouldNotifyLoadEvents", true)("source", true)("src", true)
      ("capInsets", "UIEdgeInsets") // NOTE(kudo): Trick to leverage ios type transform for UIEdgeInsets to insetsDiffer
      ("overlayColor", "UIColor")("tintColor", "UIColor") // NOTE(kudo): Trick to leverage ios type transform for UIColor to processColor
      ("defaultSource", "UIImage"); // NOTE(kudo): Trick to leverage ios type transform for UIImage to resolveAssetSource

    auto bubblingEventTypes = folly::dynamic::object();
    auto directEventTypes = folly::dynamic::object(
      "topLoadStart", folly::dynamic::object("registrationName", "onLoadStart"))(
      "topProgress", folly::dynamic::object("registrationName", "onProgress"))(
      "topError", folly::dynamic::object("registrationName", "onError"))(
      "topPartialLoad", folly::dynamic::object("registrationName", "onPartialLoad"))(
      "topLoad", folly::dynamic::object("registrationName", "onLoad"))(
      "topLoadEnd", folly::dynamic::object("registrationName", "onLoadEnd"));

    auto registry = folly::dynamic::object(
        NATIVE_PROPS_KEY, std::move(nativeProps))(
        BASE_MODULE_NAME_KEY, "RCTView")(
        BUBBLING_EVENTS_KEY, std::move(bubblingEventTypes))(
        DIRECT_EVENTS_KEY, std::move(directEventTypes));
    return {std::move(registry)};
  } else if (viewManagerName == "RCTSinglelineTextInputView") {
    auto nativeProps = folly::dynamic::object("allowFontScaling", true)("autoComplete",true)("autoCorrect",true)("autoFocus",true)(
      "blurOnSubmit",true)(
      "caretHidden",true)("clearButtonMode",true)("clearTextOnFocus",true)("contextMenuHidden",true)(
      "dataDetectorTypes",true)("defaultValue",true)("disableFullscreenUI",true)(
      "editable",true)("enablesReturnKeyAutomatically",true)(
      "importantForAutofil",true)("inlineImageLeft",true)("inlineImagePadding",true)("inputAccessoryViewID",true)(
      "keyboardAppearance",true)("keyboardType",true)(
      "maxFontSizeMultiplier",true)("maxLength",true)("multiline",true)(
      "numberOfLines",true)(
      "placeholder",true)("placeholderTextColor","UIColor")(  // Using of UIColor, since we are using platform type as iOS for app bundle , iOS type is been used.
      "text",true)(
      "returnKeyLabel",true)("returnKeyType",true)("rejectResponderTermination",true)(
      "scrollEnabled",true)("secureTextEntry",true)("selection",true)("selectionColor","UIColor")("selectTextOnFocus",true)("showSoftInputOnFocus",true)("spellCheck",true)(
      "textAlign",true)("textContentType",true)(
      "passwordRules",true)(
      "style",true)(
      "textBreakStrategy",true)(
      "underlineColorAndroid","UIColor")(
      "value",true);

    auto directEventTypes = folly::dynamic::object();

    auto bubblingEventTypes = folly::dynamic::object(
      "topBlur",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onBlur")("captured", "onBlurCapture")))(
      "topFocus",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onFocus")("captured", "onFocusCapture")))(
      "topKeyPress",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onKeyPress")("captured", "onKeyPressCapture")))(
      "topSubmitEditing",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onSubmitEditing")("captured", "onSubmitEditing")))(
      "topEndEditing",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onEndEditing")("captured", "onEndEditingCapture")))(
      "topSelectionChange",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onSelectionChange")("captured", "onSelectionChangeCapture")))(
      "topContentSizeChange",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onContentSizeChange")("captured", "onContentSizeChangeCapture")))(
      "topScroll",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onScroll")("captured", "onScrollCapture")))(
      "toChangeText",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onChangeText")("captured", "onChangeTextCapture")))(
      "topChange",
      folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onChange")("captured", "onChangeCapture")));

    auto registry = folly::dynamic::object(
        NATIVE_PROPS_KEY, std::move(nativeProps))(
        BASE_MODULE_NAME_KEY, "RCTView")(
        BUBBLING_EVENTS_KEY, std::move(bubblingEventTypes))(
        DIRECT_EVENTS_KEY, std::move(directEventTypes));
    return {std::move(registry)};
  } else if (viewManagerName == "RCTScrollContentView") { // Same as RCTView ?? May be add with (RCTView || RCTScrollContentView)
      auto nativeProps = folly::dynamic::object("onLayout", true);
      auto registry = folly::dynamic::object(
        NATIVE_PROPS_KEY, std::move(nativeProps))(
        BASE_MODULE_NAME_KEY, "RCTView");
      return {std::move(registry)};
  } else if (viewManagerName == "RCTScrollView") {
    auto nativeProps = folly::dynamic::object("alwaysBounceHorizontal", true)(
      "alwaysBounceVertical", true)("automaticallyAdjustContentInsets", true)(
      "bounces", true)("bouncesZoom", true)(
      "canCancelContentTouches", true)("centerContent", true)("contentInsetAdjustmentBehavior", true)(
      "decelerationRate", true)("directionalLockEnabled", true)(
      "disableIntervalMomentum", true)(
      "fadingEdgeLength", true)(
      "indicatorStyle", true)(
      "scrollIndicatorInsets", true)(
      "keyboardDismissMode", true)(
      "maintainVisibleContentPosition", true)("maximumZoomScale", true)("minimumZoomScale", true)(
      "nestedScrollEnabled", true)(
      "onMomentumScrollBegin", true)("onMomentumScrollEnd", true)("onScroll", true)("onScrollBeginDrag", true)("onScrollEndDrag", true)("onScrollToTop", true)("overScrollMode", true)(
      "pagingEnabled", true)("persistentScrollbar", true)(
      "scrollEnabled", true)("scrollEventThrottle", true)("scrollToOverflowEnabled", true)("scrollsToTop", true)("sendMomentumEvents", true)("showsHorizontalScrollIndicator", true)(
      "showsVerticalScrollIndicator", true)("snapToAlignment", true)("snapToEnd", true)("snapToInterval", true)("snapToOffsets", true)("snapToStart", true)(
      "zoomScale", true);
    auto bubblingEventTypes = folly::dynamic::object();
    auto directEventTypes = folly::dynamic::object("topScrollToTop", folly::dynamic::object("registrationName", "onScrollToTop"));

    auto registry = folly::dynamic::object(
        NATIVE_PROPS_KEY, std::move(nativeProps))(
        BASE_MODULE_NAME_KEY, "RCTView")(
        BUBBLING_EVENTS_KEY, std::move(bubblingEventTypes))(
        DIRECT_EVENTS_KEY, std::move(directEventTypes));
    return {std::move(registry)};
  } else {
    RNS_LOG_WARN("viewManager : " << viewManagerName << " not available in default list, check in thirdparty list");
  }

  // Reached here only if it doesnt find any view manager in default inbuilt-list
  return Uimanager::getConstantsForThirdpartyViewManager(viewManagerName);
}

void Uimanager::updateView(int Tag, std::string viewManagerName, dynamic props) {
  (void)componentViewRegistry_;
  RNS_LOG_NOT_IMPL;
}

UimanagerModule::UimanagerModule(std::unique_ptr<Uimanager> uimanager)
    : uimanager_(std::move(uimanager)) {}

std::string UimanagerModule::getName() {
  return "UIManager";
}

auto UimanagerModule::getConstants() -> std::map<std::string, folly::dynamic> {
  return {};
}

auto UimanagerModule::getMethods() -> std::vector<Method> {
  return {
      Method(
          "getConstantsForViewManager",
          [this] (dynamic args) -> dynamic {
            return(uimanager_->getConstantsForViewManager(jsArgAsString(args, 0)));
          },
          SyncTag),
      Method(
          "updateView",
          [this] (dynamic args) {
            uimanager_->updateView(jsArgAsInt(args, 0), jsArgAsString(args, 1), jsArgAsDynamic(args, 2));
          }),
  };
}

std::unique_ptr<xplat::module::CxxModule> UimanagerModule::createModule(ComponentViewRegistry *componentViewRegistry) {
  return std::make_unique<facebook::xplat::uimanager::UimanagerModule>(
      std::make_unique<facebook::xplat::uimanager::Uimanager>(componentViewRegistry));
}

} // namespace uimanager
} // namespace xplat
} // namespace facebook
