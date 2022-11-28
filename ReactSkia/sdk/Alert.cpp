/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/


#include "Alert.h"
#include "NotificationCenter.h"
#include "sdkStyleConfig.h"

namespace rns {
namespace sdk {

/* Alert Window composed of below Internal Components in order::
1. Background
 ______________________________
|                              |
|   *************              |
|   |2. Title & Message        |
|   *************              |
|                              |
|      Will Grow Further       |
|______________________________|
*/


Alert* Alert::alertHandler_{nullptr};

Alert* Alert::getAlertHandler() {
  if(alertHandler_ == nullptr ) {
    alertHandler_ = new Alert();
  }
  return alertHandler_;
}

void Alert::showAlert(alertInfo &alertData){

  Alert* alertHandle=Alert::getAlertHandler();

  std::lock_guard<std::mutex> lock(alertHandle->alertListAccessCtrlMutex_);

  alertHandle->alertInfoList_.push_back(alertData);
  alertHandle->triggerRenderRequest(ALERT_TITLE_AND_MESSAGE);

}

void Alert::drawRecentAlertTitleAndMsg(std::vector<SkIRect> &dirtyRect) {

  if((alertInfoList_.empty()) || (ALERT_WINDOW_ACTIVE != alertWindowState_) || (msgPendingToBeChanged_)) {
    return;
  }

  alertInfo alertRef=alertInfoList_.back();
  idOfMessageOnDisplay_= alertInfoList_.size();
  double drawStartPointX{0},drawStartPointY{0};

  SkIRect msgAndTitleDirtyRect;
  pictureCanvas_->clear(SKIA_SDK_DARK_THEME_BACKGROUND_COLOR);

  if (!alertRef.alertTitle.empty()) {

    double titleWidth=font_.measureText( alertRef.alertTitle.c_str(),
                                         alertRef.alertTitle.length(),
                                         SkTextEncoding::kUTF8);
    drawStartPointX=(alertWindowSize_.width() - titleWidth)/2;
    drawStartPointY=(alertWindowSize_.height() - textFontSize_) / 2;

    pictureCanvas_->drawSimpleText(
                              alertRef.alertTitle.c_str(),
                              alertRef.alertTitle.length(),
                              SkTextEncoding::kUTF8,
                              drawStartPointX,
                              drawStartPointY,
                              font_,
                              paint_);
    setWindowTittle(alertRef.alertTitle.c_str());

    msgAndTitleDirtyRect =SkIRect::MakeXYWH(drawStartPointX,
                                            drawStartPointY-textFontSize_,
                                            titleWidth,
                                            textFontSize_+lineSpacing_);

    RNS_LOG_DEBUG(" drawAlert Title :: "<<alertRef.alertTitle.c_str());
  }

  if (!alertRef.alertMessage.empty()) {

    double messageWidth=font_.measureText( alertRef.alertMessage.c_str(),
                                         alertRef.alertMessage.length(),
                                         SkTextEncoding::kUTF8);
    drawStartPointY += textFontSize_ + lineSpacing_;
    drawStartPointX=(alertWindowSize_.width() - messageWidth)/2;

    pictureCanvas_->drawSimpleText(
                            alertRef.alertMessage.c_str(),
                            alertRef.alertMessage.length(),
                            SkTextEncoding::kUTF8,
                            drawStartPointX,
                            drawStartPointY,
                            font_,
                            paint_);

    msgAndTitleDirtyRect.join(SkIRect::MakeXYWH(drawStartPointX,drawStartPointY-textFontSize_,
                               messageWidth,textFontSize_+lineSpacing_));
    RNS_LOG_DEBUG(" drawAlert Msg :: "<<alertRef.alertMessage.c_str());
  }

  dirtyRect.push_back(msgAndTitleDirtyRect);
}

inline void Alert::removeAlertFromAlertList(unsigned int msgIndex) {

  if(alertInfoList_.empty() || (msgIndex > alertInfoList_.size())) {
    return;
  }

  if(alertInfoList_.size() == idOfMessageOnDisplay_) {
    // Current Alert Message is the Latest Message
    alertInfoList_.pop_back();
  } else {
    //New Alert Message Pending to be displayed
    std::list<alertInfo>::iterator it = alertInfoList_.begin();
    std::advance(it, msgIndex);
    alertInfoList_.erase(it);
  }
}

void Alert::windowReadyToDrawCB() {

  setWindowTittle("Alert Window");

//Get Darw Parameters for screen Size
  textFontSize_=SKIA_SDK_TEXT_FONT_SIZE*(alertWindowSize_.width()/SKIA_SDK_DEFAULT_SCREEN_WIDTH);
  lineSpacing_=SKIA_SDK_LINE_SPACING*(alertWindowSize_.height()/SKIA_SDK_DEFAULT_SCREEN_HEIGHT);
  font_.setSize(textFontSize_);
  paint_.setColor(SKIA_SDK_DARK_THEME_FONT_COLOR);
  paint_.setAntiAlias(true);

  alertWindowState_ = ALERT_WINDOW_ACTIVE;

  triggerRenderRequest(ALERT_BACKGROUND,true);//Batch this render call along with Below one
  triggerRenderRequest(ALERT_TITLE_AND_MESSAGE);
}

void Alert::createAlertWindow() {
  alertWindowSize_ = RnsShell::Window::getMainWindowSize();
  
  std::function<void()> createWindowCB = std::bind(&Alert::windowReadyToDrawCB, this);
  createWindow(alertWindowSize_, createWindowCB);

  if (subWindowKeyEventId_ == -1) {
    std::function<void(rnsKey, rnsKeyAction,RnsShell::Window*)> handler = std::bind( &Alert::onHWKeyHandler,this,
                                std::placeholders::_1,  // KeyValue
                                std::placeholders::_2,  // eventKeyAction
                                std::placeholders::_3);  // window onject
    subWindowKeyEventId_ = NotificationCenter::subWindowCenter().addListener("onHWKeyEvent", handler);
  }
}

void Alert::onHWKeyHandler(rnsKey keyValue,rnsKeyAction eventKeyAction,RnsShell::Window *window) {
  RNS_LOG_DEBUG("KEY RECEIVED : " << RNSKeyMap[keyValue]);
  //TODO: Long KeyPress[OnRepeat Key Mode] will close all the Alert windows.
  //      This behaviour needs to be verified with reference and match it.
  if ((getWindow() != window) || (eventKeyAction != RNS_KEY_Press) || (RNS_KEY_Select != keyValue) ) {
    return;
  }
  if(idOfMessageOnDisplay_ == -1) {
    return;// No Valid Index, nothing to remove from display
  }

  msgPendingToBeChanged_=true;
  unsigned int msgIndex=idOfMessageOnDisplay_;

  std::lock_guard<std::mutex> lock(alertListAccessCtrlMutex_);

  removeAlertFromAlertList(msgIndex);
  msgPendingToBeChanged_=false;

  triggerRenderRequest(ALERT_TITLE_AND_MESSAGE);

}

void Alert::triggerRenderRequest(AlertComponents components,bool batchRenderRequest) {
  std::scoped_lock lock(alertActiontCtrlMutex_);
  SkPictureRecorder pictureRecorder_;
  std::string commandKey;
  std::vector<SkIRect>   dirtyRect;
  pictureCanvas_ = pictureRecorder_.beginRecording(SkRect::MakeXYWH(0, 0, alertWindowSize_.width(), alertWindowSize_.height()));
  bool invalidateFlag{true}; //to be set to indicate static or dynamic component.

  if(alertWindowState_ != ALERT_WINDOW_ACTIVE) {
      // Create Sub Window for Alert
      alertWindowState_ = ALERT_WINDOW_ON_CREATION;
      createAlertWindow();
      return; //continue processing on receiving window Ready CB
  } else if (alertInfoList_.empty() && (alertWindowState_ != ALERT_WINDOW_DESTRUCTED)) {
    // Close Window , When No Message Left
    closeWindow();
    alertWindowState_=ALERT_WINDOW_DESTRUCTED;
    return;// Nothing to process furter.
  }

  switch(components) {
    case ALERT_BACKGROUND:
      pictureCanvas_->clear(SKIA_SDK_DARK_THEME_BACKGROUND_COLOR);
      dirtyRect.push_back(SkIRect::MakeXYWH(0, 0, alertWindowSize_.width(), alertWindowSize_.height()));
      commandKey="AlertBackGround";
      invalidateFlag=false;
    break;
    case ALERT_TITLE_AND_MESSAGE:
      drawRecentAlertTitleAndMsg(dirtyRect);
      commandKey="AlertTitleAndMessage";
      break;
    default:
    break;
  }

  auto pic = pictureRecorder_.finishRecordingAsPicture();
  if(pic.get()) {
    RNS_LOG_DEBUG("SkPicture For " << commandKey << " :Command Count: " <<
    pic.get()->approximateOpCount() << " operations and size : " << pic.get()->approximateBytesUsed() <<
    " Dirty Rect Count : "<<dirtyRect.size());
  }
  if(ALERT_WINDOW_ACTIVE == alertWindowState_) {
    commitDrawCall(commandKey,{dirtyRect,pic,invalidateFlag},batchRenderRequest);
  }
}

}//sdk
}//rns
