/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontMgr.h"

#include "jsi/JSIDynamic.h"
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/core_modules/Alert.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/sdk/WindowDelegator.h"

#include <mutex>

namespace facebook {
namespace react {

enum AlertWindowState {
    ALERT_WINDOW_CREATE = 1, // Window is Created
    ALERT_WINDOW_ACTIVE = 2, // Window is Active
    ALERT_WINDOW_INACTIVE = 3, // Window is In-Active
    ALERT_WINDOW_DESTRUCT = 4 // Window is Destructed
};

class RSkAlertManager : public rns::sdk::WindowDelegator, public TurboModule {
public:

    RSkAlertManager(
        const std::string &name,
        const std::shared_ptr<facebook::react::CallInvoker> jsInvoker,
        facebook::react::Instance *bridgeInstance );

    ~RSkAlertManager() = default;

    static facebook::jsi::Value alertWithArgsWrapper(
        facebook::jsi::Runtime &rt,
        TurboModule &turboModule,
        const facebook::jsi::Value *args,
        size_t count);

private:
    int displayMsgIndex_{0};
    int subWindowKeyEventId_{-1};
    std::list<std::shared_ptr<Alert>> alertList_;
    SkSize mainWindowSize_;
    AlertWindowState alertWindowState_{ALERT_WINDOW_DESTRUCT};
    SkFont font_;
    SkPaint paint_;

    void processAlertMessages(std::shared_ptr<Alert> alertPtr=NULL);
    void drawMsg();
    void windowReadyToDrawCB();
    void createAlertWindow();
    void onHWKeyHandler(rnsKey key, rnsKeyAction eventKeyAction, RnsShell::Window* window);
};
} // react
} // facebook
