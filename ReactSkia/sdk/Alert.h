/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <atomic>
#include <mutex>

#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontMgr.h"

#include "RNSKeyCodeMapping.h"
#include "WindowDelegator.h"

namespace rns {
namespace sdk {

class Alert : public WindowDelegator {

  public :
    struct AlertInfo {
      std::string alertTitle;
      std::string alertMessage;
    };
    typedef struct AlertInfo alertInfo;

    static Alert* getAlertHandler(); // Interface to Instantiate & get Alert singleton class object
    static void showAlert(alertInfo &alertData);

  private:
    enum AlertWindowState {
      ALERT_WINDOW_ON_CREATION, // Window getting Created
      ALERT_WINDOW_ACTIVE, // Window created & ready To draw
      ALERT_WINDOW_DESTRUCTED // Window Destroyed
    };
    enum AlertComponents {
      ALERT_BACKGROUND,
      ALERT_TITLE_AND_MESSAGE,
    };

    std::list<alertInfo> alertInfoList_;
    static Alert* alertHandler_;
    std::mutex alertListAccessCtrlMutex_;
    std::mutex alertActiontCtrlMutex_;
    SkSize alertWindowSize_;
    int idOfMessageOnDisplay_{-1};
    std::atomic<bool> msgPendingToBeChanged_{false};
    int subWindowKeyEventId_{-1};
    AlertWindowState alertWindowState_{ALERT_WINDOW_DESTRUCTED};
    double textFontSize_;
    double lineSpacing_;
    SkFont font_;
    SkPaint paint_;
    SkCanvas*     pictureCanvas_{nullptr};

    Alert()=default;
    ~Alert()=default;

    void windowReadyToDrawCB();
    void createAlertWindow();
    void triggerRenderRequest(AlertComponents components,bool batchRenderRequest=false);
    void drawRecentAlertTitleAndMsg(std::vector<SkIRect> &dirtyRect);
    void removeAlertFromAlertList(unsigned int msgIndex);
    void onHWKeyHandler(rnsKey key, rnsKeyAction eventKeyAction,RnsShell::Window *window);
};

}//sdk
}//rns
