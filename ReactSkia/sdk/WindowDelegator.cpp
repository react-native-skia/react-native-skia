/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <vector>

#include "NotificationCenter.h"
#include "WindowDelegator.h"

namespace rns {
namespace sdk {

#define SHOW_DIRTY_RECT
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
    sem_init(&semReadyToDraw_,0,0);
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
      windowActive = true;
      if(displayPlatForm_ == RnsShell::PlatformDisplay::Type::X11) {
        sem_post(&semReadyToDraw_);
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
  if(ownsTaskrunner_) windowTaskRunner_->stop();

  if(ownsTaskrunner_) {
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
  sem_destroy(&semReadyToDraw_);
  windowDelegatorCanvas_=nullptr;
  windowReadyTodrawCB_=nullptr;
  std::map<std::string,PictureObject> emptyMap;
  drawHistorybin_.swap(emptyMap);
}

void WindowDelegator::commitDrawCall(std::string pictureCommandKey,PictureObject pictureObj) {
  if(!windowActive) return;
  if( ownsTaskrunner_ )  {
    if( windowTaskRunner_->running() )
      windowTaskRunner_->dispatch([=](){ renderToDisplay(pictureCommandKey,pictureObj); });
  } else {
    renderToDisplay(pictureCommandKey,pictureObj);
  }
}

inline void WindowDelegator::renderToDisplay(std::string pictureCommandKey,PictureObject pictureObj) {
  if(!windowActive) return;
  std::vector<SkIRect> dirtyRect;
/* Works on the Expection, the client would seperated its window in to sepearte component
 * and update/changes happens on portion of the screen i.e. w.r.t component
 * pictureCommandKey : component/Updating Area's key name
 * PictureObject     : Hold the recorded canvas command to the Area/component
*/
#ifdef SHOW_DIRTY_RECT
  SkPaint paint;
  paint.setColor(SK_ColorGREEN);
  paint.setStrokeWidth(2);
  paint.setStyle(SkPaint::kStroke_Style);
#endif /*SHOW_DIRTY_RECT*/
  std::scoped_lock lock(renderCtrlMutex_);
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  int bufferAge=windowContext_->bufferAge();
  if(!pictureCommandKey.empty() && (bufferAge == 1)) {
// Add last updated area of current component to dirty Rect
    auto iter=drawHistorybin_.find(pictureCommandKey);
    if(iter != drawHistorybin_.end()) {
        for(auto oldDirtyRectIt:iter->second.dirtyRect) {
          dirtyRect.push_back(oldDirtyRectIt);
          #ifdef SHOW_DIRTY_RECT
          windowDelegatorCanvas_->drawIRect(oldDirtyRectIt,paint);
          #endif/*SHOW_DIRTY_RECT*/
        }
    }
  }
  // update with latest Picture Obj for current component
  drawHistorybin_[pictureCommandKey]=pictureObj;

  if(bufferAge != 1) {
// when draw buffer don't have all the frame, redraw missed frames from command history bin
    std::map<std::string,PictureObject>::reverse_iterator it = drawHistorybin_.rbegin();
    for( ;it != drawHistorybin_.rend() ;it++ ) {
      if(it->second.pictureCommand.get() ) {
          RNS_LOG_ERROR("Parsing dirt Rect :: "<<(it->first));
        it->second.pictureCommand->playback(windowDelegatorCanvas_);
        RNS_LOG_DEBUG("SkPicture ( "  << it->second.pictureCommand << " )For " <<
                it->second.pictureCommand.get()->approximateOpCount() << " operations and size : " << it->second.pictureCommand.get()->approximateBytesUsed());
        if((bufferAge ==0) ||((it->first).compare(basePictureCommandKey_))) {
// Base Picture command needs to be drawn  when draw buffer is empty
            for(auto dirtyRectIt:(it->second).dirtyRect) {
              dirtyRect.push_back(dirtyRectIt);
              RNS_LOG_ERROR("Added dirt Rect :: "<<(it->first));
              #ifdef SHOW_DIRTY_RECT
              windowDelegatorCanvas_->drawIRect(dirtyRectIt,paint);
              #endif/*SHOW_DIRTY_RECT*/
            }
        }
      }
    }
  } else
#endif/*RNS_SHELL_HAS_GPU_SUPPORT*/
  {
    if(pictureObj.pictureCommand.get()) {
      RNS_LOG_INFO("SkPicture ( "  << pictureObj.pictureCommand << " )For " <<
                pictureObj.pictureCommand.get()->approximateOpCount() << " operations and size : " << pictureObj.pictureCommand.get()->approximateBytesUsed());
      pictureObj.pictureCommand->playback(windowDelegatorCanvas_);
      for(auto dirtyRectIt:pictureObj.dirtyRect) {
        dirtyRect.push_back(dirtyRectIt);
        RNS_LOG_ERROR("Added dirt Rect :: "<<pictureCommandKey);
        #ifdef SHOW_DIRTY_RECT
        windowDelegatorCanvas_->drawIRect(dirtyRectIt,paint);
        #endif/*SHOW_DIRTY_RECT*/
      }
      RNS_LOG_ERROR("Draw Current Command :pictureCommandKey :: "<<pictureCommandKey<< "Map Size : "<<drawHistorybin_.size());
    }
  }
  if(backBuffer_)  backBuffer_->flushAndSubmit();
  if(windowContext_) {
    RNS_LOG_INFO(" DIRTY RECT SIZE :: "<<dirtyRect.size());
    windowContext_->swapBuffers(dirtyRect);
  }
}

void WindowDelegator::setWindowTittle(const char* titleString) {
  if(window_) window_->setTitle(titleString);
}

void WindowDelegator::onExposeHandler(RnsShell::Window* window) {

  if(window  == window_) {
    sem_wait(&semReadyToDraw_);
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

