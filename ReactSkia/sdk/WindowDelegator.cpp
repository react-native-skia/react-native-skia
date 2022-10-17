/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "NotificationCenter.h"
#include "WindowDelegator.h"

namespace rns {
namespace sdk {

void WindowDelegator::createWindow(SkSize windowSize,std::function<void ()> windowReadyCB,std::function<void ()>forceFullScreenDraw,bool runOnTaskRunner) {

  windowSize_=windowSize;
  windowReadyTodrawCB_=windowReadyCB;
  forceFullScreenDraw_=forceFullScreenDraw;

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
  std::scoped_lock lock(renderCtrlMutex);

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
}

void WindowDelegator::commitDrawCall() {
  if(!windowActive) return;
  if( ownsTaskrunner_ )  {
    if( windowTaskRunner_->running() )
      windowTaskRunner_->dispatch([=](){ renderToDisplay(pictureObj); });
  } else {
    renderToDisplay();
  }
}

inline void WindowDelegator::renderToDisplay() {
  if(!windowActive) return;

  std::scoped_lock lock(renderCtrlMutex);

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  int bufferAge=windowContext_->bufferAge();
  if((bufferAge != 1) && (forceFullScreenDraw_)) {
// Forcing full screen redraw as damage region handling is not done
    RNS_LOG_ERROR("@@@@@@@@ drawPicture : Force Redraw@@@@@@@@@");
    if(fullSCreenPictureObj_.get()) {
        RNS_LOG_INFO("SkPicture ( "  << fullSCreenPictureObj_ << " )For " <<
            fullSCreenPictureObj_.get()->approximateOpCount() << " operations and size : " << fullSCreenPictureObj_.get()->approximateBytesUsed());
    }
    fullSCreenPictureObj_->playback(windowDelegatorCanvas_);
  } else
#endif/*RNS_SHELL_HAS_GPU_SUPPORT*/
  {
    RNS_LOG_ERROR("@@@@@@@@ drawPicture : Normal Draw @@@@@@@@@");
    if(pictureObj.get()) {
        RNS_LOG_INFO("SkPicture ( "  << pictureObj << " )For " <<
                pictureObj.get()->approximateOpCount() << " operations and size : " << pictureObj.get()->approximateBytesUsed());
    }
    pictureObj->playback(windowDelegatorCanvas_);
  }
#endif/*RNS_SHELL_HAS_GPU_SUPPORT*/

  if(backBuffer_)  backBuffer_->flushAndSubmit();
  if(windowContext_) {
    std::vector<SkIRect> emptyRect;// No partialUpdate handled , so passing emptyRect instead of dirtyRect
    windowContext_->swapBuffers(emptyRect);
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

