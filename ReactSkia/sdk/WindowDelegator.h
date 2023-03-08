/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <semaphore.h>
#include <thread>

#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"

#include "build/build_config.h"
#include "rns_shell/common/Window.h"
#include "rns_shell/compositor/Compositor.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/graphics/WindowContextFactory.h"

#if BUILDFLAG(IS_MAC)
#include "rns_shell/platform/mac/TaskLoop.h"
#elif BUILDFLAG(IS_LINUX)
#include "rns_shell/platform/linux/TaskLoop.h"
#endif

namespace rns {
namespace sdk {

struct pictureCommand {
  std::vector<SkIRect> dirtyRect;
  sk_sp<SkPicture> pictureCommand;
  bool invalidate;
};
typedef struct pictureCommand PictureObject;

typedef std::pair<std::string,PictureObject> PictureCommandPair;
typedef std::vector<PictureCommandPair>  PictureCommandPairs;

class WindowDelegator {
  public:

    WindowDelegator(){};
   ~WindowDelegator(){};

    void createWindow(SkSize windowSize,std::function<void ()> windowReadyTodrawCB,bool runOnTaskRunner=true);
    void closeWindow();
    void closeNativeWindow();
    void setWindowTittle(const char* titleString);
    void commitDrawCall(std::string pictureCommandKey,PictureObject pictureObj,bool batchCommit=false);
    RnsShell::Window* getWindow(){ return window_;}
  
  private:
    void onExposeHandler(RnsShell::Window* window);
    void windowWorkerThread();
    void createNativeWindow();
    void renderToDisplay(std::string pictureCommandKey,PictureObject pictureObj,bool batchCommit);
    void updateRecentCommand(std::string pictureCommandKey,PictureObject & pictureObj,int bufferAge=0,bool isUpdateDirtyRect=false);
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    void generateDirtyRect(std::vector<SkIRect> &componentDirtRects);
    bool supportsPartialUpdate_{false};
    std::vector<SkIRect> fullScreenDirtyRects_;
#endif/*RNS_SHELL_PARTIAL_UPDATES*/
    std::unique_ptr<RnsShell::WindowContext> windowContext_{nullptr};
    RnsShell::Window* window_{nullptr};
    sk_sp<SkSurface>  backBuffer_;
    SkCanvas *windowDelegatorCanvas_{nullptr};
    std::vector<SkIRect> dirtyRects_;

/*To fulfill OpenGl requirement of create & rendering to be handled from same thread context*/
    std::unique_ptr<RnsShell::TaskLoop> windowTaskRunner_{nullptr};
    bool ownsTaskrunner_{false};
/* members to fullfill X11 suggestion of "draw on receiving expose event to avoid data loss" */
    sem_t *semReadyToDraw_;
    std::mutex renderCtrlMutex;
    std::mutex renderCtrlMutex_;
    std::thread workerThread_;

    std::function<void ()> windowReadyTodrawCB_{nullptr};

    RnsShell::PlatformDisplay::Type displayPlatForm_;
    int exposeEventID_{-1};
    SkSize windowSize_;
    bool windowActive{false};

    PictureCommandPairs recentComponentCommands_;
};

} // namespace sdk
} // namespace rns


