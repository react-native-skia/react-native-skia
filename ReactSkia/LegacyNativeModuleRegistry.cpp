#include "ReactSkia/LegacyNativeModuleRegistry.h"

#include <folly/dynamic.h>
#include <glog/logging.h>
#include <memory>

namespace facebook {
namespace react {

class LegacyUIManagerModule : public NativeModule {
 public:
  LegacyUIManagerModule() = default;

  std::string getName() override {
    return "UIManager";
  }
  std::string getSyncMethodName(unsigned int methodId) override {
    if (methodId == 0) {
      return "getConstantsForViewManager";
    }
    throw std::invalid_argument("Invalid methodId");
  }

  std::vector<MethodDescriptor> getMethods() override {
    return {{"getConstantsForViewManager", "sync"}};
  }

  folly::dynamic getConstants() override {
    return folly::dynamic::object();
  }

  void invoke(unsigned int reactMethodId, folly::dynamic &&params, int callId)
      override {}

  MethodCallResult callSerializableNativeHook(
      unsigned int reactMethodId,
      folly::dynamic &&args) override {
    if (reactMethodId == 0) {
      if (args[0] == "RCTView") {
        auto nativeProps = folly::dynamic::object("onLayout", true)("pointerEvents", true)("overflow", true)
#if TARGET_OS_TV
            ("focusable", true)("isTVSelectable", true)
            ("hasTVPreferredFocus", true)("tvParallaxProperties", true)("nextFocusUp", true)
            ("nextFocusDown", true)("nextFocusLeft", true)("nextFocusRight", true)
#endif
        ;
        auto directEventTypes = folly::dynamic::object(
            "topLayout",
            folly::dynamic::object("registrationName", "onLayout"));
        auto registry = folly::dynamic::object(
            "NativeProps", std::move(nativeProps))(
            "bubblingEventTypes", folly::dynamic::object())(
            "directEventTypes", std::move(directEventTypes));
        return {std::move(registry)};
      } else if (args[0] == "RCTImageView") {
        // NOTE(kudo): The ImageView config setup has two cases
        // 1. From JS typing (only happens in bridgeless mode)
        // 2. From native UIManager (the traditional way)
        // Since we don't use `global.RN$Bridgeless` right now, we should use
        // the second one to setup config. See ImageViewNativeComponent.js for
        // more details.
        auto nativeProps = folly::dynamic::object("blurRadius", true)(
            "defaultSrc", true)("fadeDuration", true)("headers", true)(
            "loadingIndicatorSrc", true)("onError", true)("onLoad", true)(
            "onLoadEnd", true)("onLoadStart", true)("onPartialLoad", true)(
            "onProgress", true)("progressiveRenderingEnabled", true)(
            "resizeMethod", true)("resizeMode", true)(
            "shouldNotifyLoadEvents", true)("source", true)("src", true)
            // NOTE(kudo): Trick to leverage ios type transform for UIEdgeInsets
            // to insetsDiffer
            ("capInsets", "UIEdgeInsets")
            // NOTE(kudo): Trick to leverage ios type transform for UIColor to
            // processColor
            ("overlayColor", "UIColor")("tintColor", "UIColor")
            // NOTE(kudo): Trick to leverage ios type transform for UIImage to
            // resolveAssetSource
            ("defaultSource", "UIImage");

        auto directEventTypes = folly::dynamic::object(
            "topLoadStart",
            folly::dynamic::object("registrationName", "onLoadStart"))(
            "topProgress",
            folly::dynamic::object("registrationName", "onProgress"))(
            "topError", folly::dynamic::object("registrationName", "onError"))(
            "topPartialLoad",
            folly::dynamic::object("registrationName", "onPartialLoad"))(
            "topLoad", folly::dynamic::object("registrationName", "onLoad"))(
            "topLoadEnd",
            folly::dynamic::object("registrationName", "onLoadEnd"));

        auto registry = folly::dynamic::object(
            "NativeProps", std::move(nativeProps))("baseModuleName", "RCTView")(
            "bubblingEventTypes", folly::dynamic::object())(
            "directEventTypes", std::move(directEventTypes));
        return {std::move(registry)};
      }else if (args[0] == "RCTSinglelineTextInputView") {
        auto nativeProps = folly::dynamic::object("allowFontScaling", true)(
        "autoCompleteTypeAndroid",true)(
        "autoCorrect",true)(
        "autoFocus",true)(
        "blurOnSubmit",true)(
        "caretHidden",true)(
        "clearButtonModeiOS",true)(
        "clearTextOnFocusiOS",true)(
        "contextMenuHidden",true)(
        "dataDetectorTypesiOS",true)(
        "defaultValue",true)(
        "disableFullscreenUIAndroid",true)(
        "editable",true)(
        "enablesReturnKeyAutomaticallyiOS",true)(
        "importantForAutofillAndroid",true)(
        "inlineImageLeftAndroid",true)(
        "inlineImagePaddingAndroid",true)(
        "inputAccessoryViewIDiOS",true)(
        "keyboardAppearanceiOS",true)(
        "keyboardType",true)(
        "maxFontSizeMultiplier",true)(
        "maxLength",true)(
        "multiline",true)(
        "numberOfLinesAndroid",true)(
        "placeholder",true)(
        "placeholderTextColor","UIColor")(  /* Using of UIColor, since we are using platform type as iOS for app bundle , iOS type is been used. */
        "text",true)(
        "returnKeyLabelAndroid",true)(
        "returnKeyType",true)(
        "rejectResponderTerminationiOS",true)(
        "scrollEnablediOS",true)(
        "secureTextEntry",true)(
        "selection",true)(
        "selectionColor",true)(
        "selectTextOnFocus",true)(
        "showSoftInputOnFocus",true)(
        "spellCheckiOS",true)(
        "textAlign",true)(
        "textContentTypeiOS",true)(
        "passwordRulesiOS",true)(
        "style",true)(
        "textBreakStrategyAndroid",true)(
        "underlineColorAndroidAndroid",true)(
        "value",true);
        auto directEventTypes = folly::dynamic::object(
            "topBlur",folly::dynamic::object("registrationName", "onBlur"))(
            "topChange", folly::dynamic::object("registrationName", "onChange"))(
            "topChangeText", folly::dynamic::object("registrationName", "onChangeText"))(
            "topContentSizeChange", folly::dynamic::object("registrationName", "onContentSizeChange"))(
            "topEndEditing", folly::dynamic::object("registrationName", "onEndEditing"))(
            "topPressIn", folly::dynamic::object("registrationName", "onPressIn"))(
            "topPressOut", folly::dynamic::object("registrationName", "onPressOut"))(
            "topFocus", folly::dynamic::object("registrationName", "onFocus"))(
            "topKeyPress", folly::dynamic::object("registrationName", "onKeyPress"))(
            "topLayout", folly::dynamic::object("registrationName", "onLayout"))(
            "topScroll", folly::dynamic::object("registrationName", "onScroll"))(
            "topSelectionChange", folly::dynamic::object("registrationName", "onSelectionChange"))(
            "topSubmitEditing", folly::dynamic::object("registrationName", "onSubmitEditing"));
        auto bubblingEventTypes = folly::dynamic::object(
            "topBlur",
            folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onBlur")("captured", "onBlurCapture")))(
            "topKeyPress",
            folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onKeyPress")("captured", "onKeyPressCapture")))(
            "toponChange",
            folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","onChange")("captured", "onChangeCapture")));
        auto registry = folly::dynamic::object(
            "NativeProps", std::move(nativeProps))("baseModuleName", "RCTView")(
            "bubblingEventTypes", std::move(bubblingEventTypes))(
            "directEventTypes", std::move(directEventTypes));

        return {std::move(registry)};

      } else if (args[0] == "RCTScrollContentView") {
        auto nativeProps = folly::dynamic::object("onLayout", true);
        auto registry =
            folly::dynamic::object("NativeProps", std::move(nativeProps))("baseModuleName", "RCTView");
        return {std::move(registry)};
      } else if(args[0] == "RCTScrollView"){
        auto nativeProps = folly::dynamic::object("alwaysBounceHorizontal", true)(
              "alwaysBounceVertical", true)(
              "automaticallyAdjustContentInsets", true)(
              "bounces", true)(
              "bouncesZoom", true)(
              "canCancelContentTouches", true)(
              "centerContent", true)(
              "contentInsetAdjustmentBehavior", true)(
              "decelerationRate", true)(
              "directionalLockEnabled", true)(
              "disableIntervalMomentum", true)(
              "fadingEdgeLength", true)(
              "indicatorStyle", true)(
              "keyboardDismissMode", true)(
              "maintainVisibleContentPosition", true)(
              "maximumZoomScale", true)(
              "minimumZoomScale", true)(
              "nestedScrollEnabled", true)(
              "onMomentumScrollBegin", true)(
              "onMomentumScrollEnd", true)(
              "onScroll", true)(
              "onScrollBeginDrag", true)(
              "onScrollEndDrag", true)(
              "onScrollToTop", true)(
              "overScrollMode", true)(
              "pagingEnabled", true)(
              "persistentScrollbar", true)(
              "scrollEnabled", true)(
              "scrollEventThrottle", true)(
              "scrollToOverflowEnabled", true)(
              "scrollsToTop", true)(
              "sendMomentumEvents", true)(
              "showsHorizontalScrollIndicator", true)(
              "showsVerticalScrollIndicator", true)(
              "snapToAlignment", true)(
              "snapToEnd", true)(
              "snapToInterval", true)(
              "snapToOffsets", true)(
              "snapToStart", true)(
              "zoomScale", true);

        auto directEventTypes = folly::dynamic::object(
              "topScrollToTop",
              folly::dynamic::object("registrationName", "onScrollToTop"));

        auto registry = folly::dynamic::object(
              "NativeProps", std::move(nativeProps))("baseModuleName", "RCTView")(
              "bubblingEventTypes", folly::dynamic::object())(
              "directEventTypes", std::move(directEventTypes));

        return {std::move(registry)};
      }
      throw std::invalid_argument(
          "Invalid getConstantsForViewManager viewManagerName");
    }
    throw std::invalid_argument("Invalid methodId");
  }
};

LegacyNativeModuleRegistry::LegacyNativeModuleRegistry()
    : ModuleRegistry({}, nullptr) {
  auto u = std::make_unique<LegacyUIManagerModule>();
  std::vector<std::unique_ptr<NativeModule>> modules;
  modules.emplace_back(std::move(u));
  registerModules(std::move(modules));
}

} // namespace react
} // namespace facebook
