/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "ReactSkia/sdk/WindowDelegator.h"

#include <fcntl.h>
#include <vector>

#include "NotificationCenter.h"

namespace rns {
namespace sdk {
/*
 *> Window & Canvas created for Client and maintained with in window delegator
 *> Supports Partial Update with dirty Rect
 *> Maintains last updated picture command + dirty Rect for each components

    Window Dlegator Fuctional Logic :
    ================================
 *> Recorded canvas commands  to be sent by clients and window delegator renders it on real canvas.
 *> Works on expectation,client knows its screen's component and screen layout is fixed.
 *> Supports Partial Update, So Expects client would do component by component rendering all the time.
 *> As part of recorded commands, expects dirty regions associted with this draw and the updating component name
 *> Recent recorded command maintained for all the components.This will be used to redraw the screen, when current draw buffer misses old frames.

   Rendering Logic followed by Window Delegator:
   =============================================
 *> When a rendered component updated, Dirty Rect would be "Dirty Rect of last update for that Component" &
   "Dirty Rect of the current update received from client".
 (If buffer Age supported in Backend).
 *> If Buffer Age is "1", Write buffer is up to date and just needs to render received commands from the client.
 *>When Buffer Age is "0".Write buffer to be consider as empty and needs to redraw all the components in received order
    to fill the missed frames
 *> When "Buffer Age is anything other than "0" & "1", it means it is not empty but it misses few frames.
    In this case, update the components which are set as inavlidate by client.
    This is to reduce/ avoid rending of static component in the screen. So client is expected to set
    "invalidate flag as false" for static/root components such as BACKGROUND and true for dynamic or components whose content will change.
*/
void WindowDelegator::createWindow(SkSize windowSize,std::function<void ()> windowReadyCB,bool runOnTaskRunner) {

  windowSize_=windowSize;
  windowReadyTodrawCB_=windowReadyCB;

  if(runOnTaskRunner) {
    ownsTaskrunner_ = runOnTaskRunner;
    windowTaskRunner_ = std::make_unique<RnsShell::TaskLoop>();
    workerThread_=std::thread (&WindowDelegator::windowWorkerThread,this);
    windowTaskRunner_->waitUntilRunning();
    windowTaskRunner_->dispatch([&](){createNativeWindow();});
  } else {
    createNativeWindow();
  }
}

void  WindowDelegator::createNativeWindow() {

  displayPlatForm_=RnsShell::PlatformDisplay::sharedDisplayForCompositing().type();

  if(displayPlatForm_ == RnsShell::PlatformDisplay::Type::X11) {
    /*For X11 draw should be done after expose event received*/
    semReadyToDraw_ = sem_open(__func__, O_CREAT | O_EXCL, S_IRWXU, 0);
    sem_unlink(__func__);
    // Registering expose event
    std::function<void(RnsShell::Window*)> handler = std::bind(&WindowDelegator::onExposeHandler,this,
                                                                         std::placeholders::_1);
    exposeEventID_ = NotificationCenter::defaultCenter().addListener("windowExposed",handler);
  }
  window_ = RnsShell::Window::createNativeWindow(&RnsShell::PlatformDisplay::sharedDisplayForCompositing(),
                                                 SkSize::Make(windowSize_.width(),windowSize_.height()),
                                                 RnsShell::SubWindow);
  if(window_) {
    windowContext_ = RnsShell::WCF::createContextForWindow(window_->nativeWindowHandle(),
               &RnsShell::PlatformDisplay::sharedDisplayForCompositing(), RnsShell::DisplayParams());
    if(windowContext_) {
      windowContext_->makeContextCurrent();
      backBuffer_ = windowContext_->getBackbufferSurface();
      windowDelegatorCanvas_ = backBuffer_->getCanvas();
#if USE(RNS_SHELL_PARTIAL_UPDATES)
      supportsPartialUpdate_=windowContext_->supportsPartialUpdate();
      fullScreenDirtyRectVec_.push_back(SkIRect::MakeXYWH (0,0,windowContext_->width(),windowContext_->height()));
#endif/*RNS_SHELL_PARTIAL_UPDATES*/
      windowActive = true;
      if(displayPlatForm_ == RnsShell::PlatformDisplay::Type::X11) {
        sem_post(semReadyToDraw_);
      } else if(windowReadyTodrawCB_) windowReadyTodrawCB_();
    } else {
      RNS_LOG_ERROR("Invalid windowContext for nativeWindowHandle : " << window_->nativeWindowHandle());
    }
  }
}

void WindowDelegator::closeWindow() {
  RNS_LOG_TODO("Sync between rendering & Exit to be handled ");
  windowActive = false;
  std::scoped_lock lock(renderCtrlMutex_);
  if(ownsTaskrunner_){
   windowTaskRunner_->stop();
  }
  if (workerThread_.joinable() ) {
    workerThread_.join();
  }
  if(exposeEventID_ != -1) {
    NotificationCenter::defaultCenter().removeListener(exposeEventID_);
    exposeEventID_=-1;
  }
  if(window_) {
    window_->closeWindow();
    delete window_;
    window_=nullptr;
    windowContext_ = nullptr;
    backBuffer_ = nullptr;
  }
  sem_close(semReadyToDraw_);
  semReadyToDraw_ = nullptr;
  windowDelegatorCanvas_=nullptr;
  windowReadyTodrawCB_=nullptr;
  std::map<std::string,PictureObject> emptyMap;
  recentComponentCommandMap_.swap(emptyMap);
}

void WindowDelegator::commitDrawCall(std::string pictureCommandKey,PictureObject pictureObj,bool batchCommit) {
  if(!windowActive) { return; }
  if( ownsTaskrunner_ ) {
    if( windowTaskRunner_->running() )
      windowTaskRunner_->dispatch([=](){ renderToDisplay(pictureCommandKey,pictureObj,batchCommit); });
  } else {
    renderToDisplay(pictureCommandKey,pictureObj,batchCommit);
  }
}

inline void WindowDelegator::renderToDisplay(std::string pictureCommandKey,PictureObject pictureObj,bool batchCommit) {
  if(!windowActive) { return;}

#ifdef SHOW_RENDER_COMMAND_INFO
RNS_LOG_INFO("Rendering component  : " << pictureCommandKey);
RNS_LOG_INFO("Count of Dirt Rect   : " <<  pictureObj.dirtyRect.size());
RNS_LOG_INFO("Draw Command Count   : "<< pictureObj.pictureCommand.get()->approximateOpCount());
RNS_LOG_INFO(" operations and size : " << pictureObj.pictureCommand.get()->approximateBytesUsed());
RNS_LOG_INFO(" Batching Request    : "<<batchCommit);
#endif

  std::scoped_lock lock(renderCtrlMutex_);

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  int bufferAge=windowContext_->bufferAge();

  if((bufferAge !=1) && batchCommit) {
    //To avoid reduntant painting   & dirt Rect calculation. Just store the command.
    //Rest will be handled , when Render Requested by client.
    recentComponentCommandMap_[pictureCommandKey]=pictureObj;
    return;
  }

  // Add current component's old dirty Rect
  auto iter=recentComponentCommandMap_.find(pictureCommandKey);
  if((iter != recentComponentCommandMap_.end()) && supportsPartialUpdate_ && (bufferAge!=0)) {
    generateDirtyRect(iter->second.dirtyRect);
  }

  recentComponentCommandMap_[pictureCommandKey]=pictureObj;

  if(bufferAge != 1) {
// use Stored commands to fill missed frames in the write buffer
    std::map<std::string,PictureObject>::reverse_iterator it = recentComponentCommandMap_.rbegin();
    bool fullScreenAddedAsDirtyRect{false};
    for( ;it != recentComponentCommandMap_.rend() ;it++ ) {
      if(it->second.pictureCommand.get() ) {

        it->second.pictureCommand->playback(windowDelegatorCanvas_);

          if(supportsPartialUpdate_ && !fullScreenAddedAsDirtyRect) {
            if(bufferAge ==0 ) {
                //Update complete Screen if Buffer Age is "0"
                RNS_LOG_DEBUG("Buffer Age is 0, Doing Full Screen Update : ");
                generateDirtyRect(fullScreenDirtyRectVec_);
                fullScreenAddedAsDirtyRect=true;
            } else if(it->second.invalidate) {
              RNS_LOG_DEBUG("Updating dirty Rect for component : "<<it->first);
              generateDirtyRect((it->second).dirtyRect);
            }
          }
      }
    }
  } else
#endif/*RNS_SHELL_HAS_GPU_SUPPORT*/
  {
    if(pictureObj.pictureCommand.get()) {
      pictureObj.pictureCommand->playback(windowDelegatorCanvas_);
#if USE(RNS_SHELL_PARTIAL_UPDATES)
      if(supportsPartialUpdate_) {
        generateDirtyRect(pictureObj.dirtyRect);
      }
#endif/*RNS_SHELL_PARTIAL_UPDATES*/
    }
  }

#ifdef SHOW_DIRTY_RECT
  SkPaint paint;
  paint.setColor(SK_ColorGREEN);
  paint.setStrokeWidth(2);
  paint.setStyle(SkPaint::kStroke_Style);
  RNS_LOG_INFO(" Count of Dirty Rect :: "<<dirtyRectVec_.size());
  for(SkIRect rectIt:dirtyRectVec_) {
    windowDelegatorCanvas_->drawIRect(rectIt,paint);
  }
#endif/*SHOW_DIRTY_RECT*/
  if(!batchCommit) {
// Render paint only if not  to be batched.
    if(backBuffer_) {
      backBuffer_->flushAndSubmit();
    }
    if(windowContext_) {
      windowContext_->swapBuffers(dirtyRectVec_);
      std::vector<SkIRect> emptyVect;
      dirtyRectVec_.swap(emptyVect);
    }
  }
}

#if USE(RNS_SHELL_PARTIAL_UPDATES)
inline void WindowDelegator:: generateDirtyRect(std::vector<SkIRect> &componentDirtRectVec){
  for(SkIRect& comDirtyRect:componentDirtRectVec) {
    bool addToDirtyRect{true};
    if(!dirtyRectVec_.empty()) {
      std::vector<SkIRect>::iterator it = dirtyRectVec_.begin();
      while( it != dirtyRectVec_.end()) {
        SkIRect &dirtyRect = *it;
        if((dirtyRect == comDirtyRect) || dirtyRect.contains(comDirtyRect) ) {
          addToDirtyRect=false;// If same or part of existing ignore
          break;
        }
        if(comDirtyRect.contains(dirtyRect)) {
          it=dirtyRectVec_.erase(it);//Erase existing dirtRect , if it is part of new one
        } else {
          ++it;
        }
      }
    }
    if(addToDirtyRect){
      dirtyRectVec_.push_back(comDirtyRect);
    }
  }
}
#endif /*RNS_SHELL_PARTIAL_UPDATES*/
void WindowDelegator::setWindowTittle(const char* titleString) {
  if(window_) window_->setTitle(titleString);
}

void WindowDelegator::onExposeHandler(RnsShell::Window* window) {

  if(window  == window_) {
    sem_wait(semReadyToDraw_);
    window_->show();
    if(exposeEventID_ != -1) {
      NotificationCenter::defaultCenter().removeListener(exposeEventID_);
      exposeEventID_=-1;
    }
    if(windowReadyTodrawCB_) windowReadyTodrawCB_();
  }
}

 void WindowDelegator::windowWorkerThread() {
  windowTaskRunner_->run();
}

} // namespace sdk
} // namespace rns

