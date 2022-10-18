/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/core_modules/RSkAlertManager.h"
#include "ReactSkia/sdk/sdkStylesConfig.h"

namespace facebook {
namespace react {

//TODO:
//Need to use different FONT_SIZE values for 720p and 1080p
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
  std::scoped_lock lock(lockMsgDrawing_);
  if (alertPtr) {
    alertList_.push_back(alertPtr);
    if (ALERT_WINDOW_DESTRUCT == alertWindowState_) {
      alertWindowState_ = ALERT_WINDOW_CREATE;
      createAlertWindow();
    } else {
      if (ALERT_WINDOW_ACTIVE == alertWindowState_) {
        drawMsg();
        commitDrawCall();
      }
    }
  } else {
    if ((alertList_.size() == displayMsgIndex_)) {
      alertList_.pop_back();
      if (alertList_.empty()) {
        if (subWindowKeyEventId_ != -1) {
          NotificationCenter::subWindowCenter().removeListener(subWindowKeyEventId_);
          subWindowKeyEventId_ = -1;
        }
        closeWindow();
        alertWindowState_ = ALERT_WINDOW_DESTRUCT;

        return;
      }
      drawMsg();
      commitDrawCall();
    } else {
      std::list<std::shared_ptr<Alert>>::iterator it = alertList_.begin();
      std::advance(it, displayMsgIndex_);
      alertList_.erase(it);
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
  alertManagerInstance.processAlertMessages(
    std::make_shared<Alert>(
    alertProps["title"].getString(),
    alertProps["message"].getString()));

  return 0;
}

void RSkAlertManager::drawMsg() {
  if (alertList_.empty() || (ALERT_WINDOW_ACTIVE != alertWindowState_)) {
    RNS_LOG_INFO("Return From draw Alert Msg without drawing Alert Message!!!");
    return;
  }
  double titleWidth, messageWidth;
  double drawStartPointX, drawStartPointY;
  std::shared_ptr<Alert> alertRef = alertList_.back();
  windowDelegatorCanvas->clear(DARK_THEME_BACKGROUND_COLOR);
  titleWidth = font_.measureText(
    alertRef->getTitle().c_str(),
    strlen(alertRef->getTitle().c_str()),
    SkTextEncoding::kUTF8);

  if (!alertList_.empty()) {
    drawStartPointX = (mainWindowSize_.width() - titleWidth) / 2;
    drawStartPointY = (mainWindowSize_.height() - FONT_SIZE) / 2;
    if (alertRef->getTitle().length() != 0) {
      WindowDelegator::windowDelegatorCanvas->drawSimpleText(
        alertRef->getTitle().c_str(),
        alertRef->getTitle().length(),
        SkTextEncoding::kUTF8,
        drawStartPointX,
        drawStartPointY,
        font_,
        paint_);
    }

    drawStartPointY += FONT_SIZE + OFFSET_VERTICAL;
    if (alertRef->getMessage().length() != 0) {
      WindowDelegator::windowDelegatorCanvas->drawSimpleText(
        alertRef->getMessage().c_str(),
        alertRef->getMessage().length(),
        SkTextEncoding::kUTF8,
        drawStartPointX,
        drawStartPointY,
        font_,
        paint_);
    }
  }
  displayMsgIndex_ = alertList_.size();
}

void RSkAlertManager::windowReadyToDrawCB() {
  std::scoped_lock lock(lockMsgDrawing_);
  alertWindowState_ = ALERT_WINDOW_ACTIVE;
  drawMsg();
  commitDrawCall();
}

void RSkAlertManager::createAlertWindow() {
  mainWindowSize_ = RnsShell::Window::getMainWindowSize();
  std::function<void()> createWindowCB = std::bind(&RSkAlertManager::windowReadyToDrawCB, this);
  std::function<void()> forceFullScreenDrawCB = std::bind(&RSkAlertManager::drawMsg, this);
  createWindow(mainWindowSize_, createWindowCB, forceFullScreenDrawCB);
  if (subWindowKeyEventId_ == -1) {
    //TODO:
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

//TODO:
//We should avoid passing entire window and try for ID alone.
void RSkAlertManager::onHWKeyHandler(
  rnsKey keyValue,
  rnsKeyAction eventKeyAction,
  RnsShell::Window *window) {
  if ((eventKeyAction != RNS_KEY_Press) || (getWindow() != window)) {
    return;
  }
  RNS_LOG_INFO("KEY RECEIVED : " << RNSKeyMap[keyValue]);
  if (RNS_KEY_Select == keyValue) {
    processAlertMessages();
  }
}
} // namespace react
} // namespace facebook
