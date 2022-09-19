/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <semaphore.h>
#include <thread>
#include <map>

#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"

#include "rns_shell/compositor/Compositor.h"
#include "rns_shell/common/Window.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/graphics/WindowContextFactory.h"
#include "rns_shell/platform/linux/TaskLoop.h"

namespace rns {
namespace sdk {

struct pictureCommand {
  SkIRect dirtyRect;
  sk_sp<SkPicture> pictureCommand;
};

typedef struct pictureCommand PictureObject;
class WindowDelegator {
  public:

    WindowDelegator(){};
   ~WindowDelegator(){};

    void createWindow(SkSize windowSize,std::function<void ()> windowReadyTodrawCB,bool runOnTaskRunner=true);
    void closeWindow();
    void setWindowTittle(const char* titleString);
    void commitDrawCall(std::string keyRef,PictureObject pictureObj);
    void setBasePicCommand(std::string keyName) {keyNameBasePicCommand_=keyName;}

  private:
    void onExposeHandler(RnsShell::Window* window);
    void windowWorkerThread();
    void createNativeWindow();
    void renderToDisplay(std::string keyRef,PictureObject pictureObj);

    std::unique_ptr<RnsShell::WindowContext> windowContext_{nullptr};
    RnsShell::Window* window_{nullptr};
    sk_sp<SkSurface>  backBuffer_;
    SkCanvas *windowDelegatorCanvas_{nullptr};

/*To fulfill OpenGl requirement of create & rendering to be handled from same thread context*/
    std::unique_ptr<RnsShell::TaskLoop> windowTaskRunner_{nullptr};
    bool ownsTaskrunner_{false};
/* members to fullfill X11 suggestion of "draw on receiving expose event to avoid data loss" */
    sem_t semReadyToDraw_;
    std::mutex renderCtrlMutex;
    std::thread workerThread_;

    std::function<void ()> windowReadyTodrawCB_{nullptr};

    RnsShell::PlatformDisplay::Type displayPlatForm_;
    int exposeEventID_{-1};
    SkSize windowSize_;
    bool windowActive{false};

    std::map<std::string,PictureObject> drawHistorybin_;
    std::string keyNameBasePicCommand_;
};

} // namespace sdk
} // namespace rns


