/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RSkAlertManager.h"

#include "sdk/sdkStylesConfig.h"

namespace facebook {
namespace react {

std::mutex lockMsgDrawing;

#define FONT_SIZE 24
#define LINE_SPACE 14
#define OFFSET_VERTICAL 30

RSkAlertManager::RSkAlertManager(
    const std::string &name,
    std::shared_ptr<facebook::react::CallInvoker> jsInvoker,
    facebook::react::Instance *bridgeInstance)
    : TurboModule(name, jsInvoker) {
  methodMap_["alertWithArgs"] = MethodMetadata{1, alertWithArgsWrapper};
}

void RSkAlertManager::processAlertMessages(std::shared_ptr<Alert> alertPtr) {
  alertPropsList_.push_back(alertPtr);
  if (ALERT_WINDOW_DESTRUCT == alertWindowState_) {
    alertWindowState_ = ALERT_WINDOW_CREATE;
    createAlertWindow();
  } else {
    if (ALERT_WINDOW_ACTIVE == alertWindowState_) {
      drawAlertMsg();
      commitDrawCall();
    }
  }
}

facebook::jsi::Value RSkAlertManager::alertWithArgsWrapper(
    facebook::jsi::Runtime &rt,
    TurboModule &turboModule,
    const facebook::jsi::Value *args,
    size_t count) {
  auto &alertManagerInstance = static_cast<RSkAlertManager &>(turboModule);
  auto alertProps = facebook::jsi::dynamicFromValue(rt, args[0]);
  alertManagerInstance.processAlertMessages(std::make_shared<Alert>(
      alertProps["title"].getString(), alertProps["message"].getString()));

  return 0;
}

void RSkAlertManager::drawAlertMsg() {
  std::scoped_lock lock(lockMsgDrawing);
  if (alertPropsList_.empty() || (ALERT_WINDOW_ACTIVE != alertWindowState_)) {
    RNS_LOG_INFO("Return From draw Alert Msg without drawing Alert Message!!!");
    return;
  }
  double titleWidth, messageWidth;
  double drawStartPointX, drawStartPointY;
  std::shared_ptr<Alert> alertRef = alertPropsList_.back();
  windowDelegatorCanvas->clear(DARK_THEME_BACKGROUND_COLOR);
  titleWidth = font_.measureText(
      alertRef->getTitle().c_str(),
      strlen(alertRef->getTitle().c_str()),
      SkTextEncoding::kUTF8);

  if (!alertPropsList_.empty()) {
    drawStartPointX = (mainWindowSize_.width() - titleWidth) / 2;
    drawStartPointY = (mainWindowSize_.height() - FONT_SIZE) / 2;
    WindowDelegator::windowDelegatorCanvas->drawSimpleText(
        alertRef->getTitle().c_str(),
        strlen(alertRef->getTitle().c_str()),
        SkTextEncoding::kUTF8,
        drawStartPointX,
        drawStartPointY,
        font_,
        paint_);

    drawStartPointY = ((mainWindowSize_.height() - FONT_SIZE) / 2) + (FONT_SIZE) + OFFSET_VERTICAL;
    WindowDelegator::windowDelegatorCanvas->drawSimpleText(
        alertRef->getMessage().c_str(),
        strlen(alertRef->getMessage().c_str()),
        SkTextEncoding::kUTF8,
        drawStartPointX,
        drawStartPointY,
        font_,
        paint_);
  }
  displayMsgIndex_ = alertPropsList_.size();
}

void RSkAlertManager::windowReadyToDrawCB() {
  alertWindowState_ = ALERT_WINDOW_ACTIVE;
  drawAlertMsg();
  commitDrawCall();
}

void RSkAlertManager::createAlertWindow() {
  mainWindowSize_ = RnsShell::Window::getMainWindowSize();
  std::function<void()> createWindowCB =
      std::bind(&RSkAlertManager::windowReadyToDrawCB, this);
  std::function<void()> forceFullScreenDrawCB =
      std::bind(&RSkAlertManager::drawAlertMsg, this);
  setWindowTittle("Alert Window!!!");
  createWindow(mainWindowSize_, createWindowCB, forceFullScreenDrawCB);
  if (subWindowKeyEventId_ == -1) {
    std::function<void(rnsKey, rnsKeyAction, RnsShell::Window*)> handler =
        std::bind(
            &RSkAlertManager::onHWKeyHandler,
            this,
            std::placeholders::_1,  // KeyValue
            std::placeholders::_2,  // eventKeyAction
            std::placeholders::_3); // Window
    subWindowKeyEventId_ = NotificationCenter::subWindowCenter().addListener("onHWKeyEvent", handler);
  }
  font_.setSize(FONT_SIZE);
  paint_.setColor(DARK_THEME_FONT_COLOR);
}

void RSkAlertManager::onHWKeyHandler(
    rnsKey keyValue,
    rnsKeyAction eventKeyAction,
    RnsShell::Window *window) {
  if ((eventKeyAction != RNS_KEY_Press) || (getWindow() != window)) {
    return;
  }

  RNS_LOG_INFO("KEY RECEIVED : " << RNSKeyMap[keyValue]);

  if (RNS_KEY_Select == keyValue) {
    if ((alertPropsList_.size() == displayMsgIndex_)) {
      alertPropsList_.pop_back();
      if (alertPropsList_.empty()) {
        if (subWindowKeyEventId_ != -1) {
          NotificationCenter::subWindowCenter().removeListener(
              subWindowKeyEventId_);
          subWindowKeyEventId_ = -1;
        }
        closeWindow();
        alertWindowState_ = ALERT_WINDOW_DESTRUCT;

        return;
      }

      drawAlertMsg();
      commitDrawCall();
    } else {
      std::list<std::shared_ptr<Alert>>::iterator it = alertPropsList_.begin();
      std::advance(it, displayMsgIndex_);
      alertPropsList_.erase(it);
    }
  }
}
} // namespace react
} // namespace facebook
