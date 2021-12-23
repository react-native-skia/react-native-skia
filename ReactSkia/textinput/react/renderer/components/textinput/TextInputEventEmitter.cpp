/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextInputEventEmitter.h"

namespace facebook {
namespace react {

static jsi::Value textInputMetricsPayload(
    jsi::Runtime &runtime,
    TextInputMetrics const &textInputMetrics) {
  auto payload = jsi::Object(runtime);

  payload.setProperty(
      runtime,
      "text",
      jsi::String::createFromUtf8(runtime, textInputMetrics.text));

  payload.setProperty(runtime, "eventCount", textInputMetrics.eventCount);
  {
    auto selection = jsi::Object(runtime);
    selection.setProperty(
        runtime, "start", textInputMetrics.selectionRange.location);
    selection.setProperty(
        runtime,
        "end",
        textInputMetrics.selectionRange.location +
            textInputMetrics.selectionRange.length);
    payload.setProperty(runtime, "selection", selection);
  }

  return payload;
};
static jsi::Value textInputMetricsSelectionPayload(
    jsi::Runtime &runtime,
    TextInputMetrics const &textInputMetrics) {
  auto payload = jsi::Object(runtime);
  {
    auto selection = jsi::Object(runtime);
    selection.setProperty(
        runtime, "start", textInputMetrics.selectionRange.location);
    selection.setProperty(
        runtime,
        "end",
        textInputMetrics.selectionRange.location +
            textInputMetrics.selectionRange.length);
    payload.setProperty(runtime, "selection", selection);
  }

  return payload;
};
static jsi::Value textInputMetricsLayoutEventPayload(
    jsi::Runtime &runtime,
    TextInputMetrics const &textInputMetrics){
  auto payload = jsi::Object(runtime);
  {
     auto layoutEvent = jsi::Object(runtime);
     layoutEvent.setProperty(
        runtime, "width", textInputMetrics.contentSize.width);
    layoutEvent.setProperty(
        runtime, "height", textInputMetrics.contentSize.height);
    layoutEvent.setProperty(
        runtime, "x", textInputMetrics.contentOffset.x);
    layoutEvent.setProperty(
        runtime, "y", textInputMetrics.contentOffset.y);
    payload.setProperty(runtime, "LayoutEvent", layoutEvent);
  }
  return payload;
};

static jsi::Value textInputMetricsEditTextPayload(
    jsi::Runtime &runtime,
    TextInputMetrics const &textInputMetrics) {
  auto payload = jsi::Object(runtime);

  payload.setProperty(
      runtime,
      "text",
      jsi::String::createFromUtf8(runtime, textInputMetrics.text));

  payload.setProperty(runtime, "eventCount", textInputMetrics.eventCount);
  return payload;
};

static jsi::Value textInputMetricsContentSizePayload(
    jsi::Runtime &runtime,
    TextInputMetrics const &textInputMetrics) {
  auto payload = jsi::Object(runtime);
  {
    auto contentSize = jsi::Object(runtime);
    contentSize.setProperty(
        runtime, "width", textInputMetrics.contentSize.width);
    contentSize.setProperty(
        runtime, "height", textInputMetrics.contentSize.height);
    payload.setProperty(runtime, "contentSize", contentSize);
  }
  return payload;
};

static jsi::Value keyPressMetricsPayload(
    jsi::Runtime &runtime,
    KeyPressMetrics const &keyPressMetrics) {
  auto payload = jsi::Object(runtime);
  payload.setProperty(runtime, "eventCount", keyPressMetrics.eventCount);

  std::string key;
  if (keyPressMetrics.text == "back") {
    key = "Backspace";
  } else {
    if (keyPressMetrics.text == "select") {
      key = "Enter";
    } else if (keyPressMetrics.text == "tab") {
      key = "Tab";
    } else {
      key = keyPressMetrics.text.front();
    }
  }
  payload.setProperty(
      runtime, "key", jsi::String::createFromUtf8(runtime, key));
  return payload;
};

void TextInputEventEmitter::onFocus(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "focus",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsLayoutEventPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onBlur(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "blur",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsEditTextPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onChange(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "change",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsEditTextPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onChangeText(
    TextInputMetrics const &textInputMetrics) const {
  dispatchTextInputEvent("changeText", textInputMetrics);
}

void TextInputEventEmitter::onContentSizeChange(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "contentSizeChange",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsContentSizePayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onSelectionChange(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "selectionChange",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsSelectionPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onEndEditing(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "endEditing",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsEditTextPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onSubmitEditing(
    TextInputMetrics const &textInputMetrics) const {
  dispatchEvent(
      "submitEditing",
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsEditTextPayload(runtime, textInputMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::onKeyPress(
    KeyPressMetrics const &keyPressMetrics) const {
  dispatchEvent(
      "keyPress",
      [keyPressMetrics](jsi::Runtime &runtime) {
        return keyPressMetricsPayload(runtime, keyPressMetrics);
      },
      EventPriority::AsynchronousBatched);
}

void TextInputEventEmitter::dispatchTextInputEvent(
    std::string const &name,
    TextInputMetrics const &textInputMetrics,
    EventPriority priority) const {
  dispatchEvent(
      name,
      [textInputMetrics](jsi::Runtime &runtime) {
        return textInputMetricsPayload(runtime, textInputMetrics);
      },
      priority);
}

} // namespace react
} // namespace facebook
