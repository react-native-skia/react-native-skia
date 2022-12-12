/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkMaskFilterBase.h"

#include "rns_shell/compositor/layers/PictureLayer.h"

#include "react/renderer/components/image/ImageEventEmitter.h"

#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/sdk/CurlNetworking.h"
#include "ReactSkia/views/common/RSkImageUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace facebook {
namespace react {

using namespace RSkImageUtils;

RSkComponentImage::RSkComponentImage(const ShadowView &shadowView)
    : RSkComponent(shadowView) {
      imageEventEmitter_ = std::static_pointer_cast<ImageEventEmitter const>(shadowView.eventEmitter);
}

void RSkComponentImage::OnPaint(SkCanvas *canvas) {
  sk_sp<SkImage> imageData{nullptr};
  string path;
  auto component = getComponentData();
  ImageProps const &imageProps = *std::static_pointer_cast<ImageProps const>(component.props);
  //First to check file entry presence. If not exist, generate imageData.
  do {
    if(networkImageData_) {
      imageData = networkImageData_;
      break;
    }
    if(imageProps.sources.empty()) break;
    imageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(imageProps.sources[0].uri.c_str());
    if(imageData) break;

    if (imageProps.sources[0].type == ImageSource::Type::Local) {
      imageData = getLocalImageData(imageProps.sources[0].uri);
    } else if(imageProps.sources[0].type == ImageSource::Type::Remote) {
      requestNetworkImageData(imageProps.sources[0].uri);
    }
  } while(0);

  Rect frame = component.layoutMetrics.frame;
  SkRect frameRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);

  // Draw order 1.Shadow 2. Background 3.Image Shadow 4. Image 5.Border
  bool hollowFrame = false;
  bool needClipAndRestore =false;
  sk_sp<SkImageFilter> imageFilter;
  auto  layerRef=layer();
  if(layerRef->isShadowVisible) {
    /*Draw Shadow on Frame*/
    hollowFrame=drawShadow(canvas,frame,imageBorderMetrics,
                              imageProps.backgroundColor,
                              layerRef->shadowColor,layerRef->shadowOffset,layerRef->shadowOpacity,
                              layerRef->opacity,
                              layerRef->shadowImageFilter,layerRef->shadowMaskFilter
                          );
  }
  /*Draw Frame BackGround*/
  drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
  if(imageData) {
    SkRect imageTargetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,imageProps.resizeMode);
    SkPaint paint;
    /*Draw Image Shadow on below scenario:
      ------------------------------------
      1. Has visible shadow. but both border & background not avialble [case of ShadowDrawnMode::ShadowOnContent]
      2. Shadow Drawn on Border[case of ShadowDrawnMode::ShadowOnBorder], But Image is either transparent  or smaller than the same
    */
    if(hollowFrame) {
        //TODO: For the content Shadow, currently Shadow drawn for both Border[if avaialble] & Content[Image].
        //      This behaviour to be cross verified with reference.
      drawContentShadow(canvas,frameRect,imageTargetRect,imageData,imageProps,layerRef->shadowOffset,layerRef->shadowColor,layerRef->shadowOpacity);
    }
    /*Draw Image */
    if(( frameRect.width() < imageTargetRect.width()) || ( frameRect.height() < imageTargetRect.height())) {
      needClipAndRestore= true;
    }
    /* clipping logic to be applied if computed Frame is greater than the target.*/
    if(needClipAndRestore) {
        canvas->save();
        canvas->clipRect(frameRect,SkClipOp::kIntersect);
    }
    /* TODO: Handle filter quality based of configuration. Setting Low Filter Quality as default for now*/
    paint.setFilterQuality(DEFAULT_IMAGE_FILTER_QUALITY);
    setPaintFilters(paint,imageProps,imageTargetRect,frameRect,false,imageData->isOpaque());
    canvas->drawImageRect(imageData,imageTargetRect,&paint);
    if(needClipAndRestore) {
      canvas->restore();
    }
    networkImageData_ = nullptr;
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
    // Emitting Load completed Event
    if(hasToTriggerEvent_) sendSuccessEvents();

  } else {
  /* Emitting Image Load failed Event*/
    if(imageProps.sources[0].type != ImageSource::Type::Remote) {
      if(!hasToTriggerEvent_) {
        imageEventEmitter_->onLoadStart();
        hasToTriggerEvent_ = true;
      }
      if(hasToTriggerEvent_) sendErrorEvents();
      RNS_LOG_ERROR("Image not loaded :"<<imageProps.sources[0].uri.c_str());
    }
  }
}


sk_sp<SkImage> RSkComponentImage::getLocalImageData(string sourceUri) {
  sk_sp<SkImage> imageData{nullptr};
  sk_sp<SkData> data;
  string path;
  decodedimageCacheData imageCacheData;
  path = generateUriPath(sourceUri.c_str());
  if(!path.c_str()) {
    RNS_LOG_ERROR("Invalid File");
    return nullptr;
  }
  data = SkData::MakeFromFileName(path.c_str());
  if (!data) {
    RNS_LOG_ERROR("Unable to make SkData for path : " << path.c_str());
    return nullptr;
  }
  imageData = SkImage::MakeFromEncoded(data);
  if(imageData) {
    imageCacheData.imageData = imageData;
    imageCacheData.expiryTime = (SkTime::GetMSecs() + DEFAULT_MAX_CACHE_EXPIRY_TIME);//convert min to millisecond 30 min *60 sec *1000
    RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(sourceUri.c_str(), imageCacheData);
  }
  if(!hasToTriggerEvent_) {
    imageEventEmitter_->onLoadStart();
    hasToTriggerEvent_ = true;
  }

#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
    printCacheUsage();
#endif //RNS_IMAGECACHING_DEBUG
  return imageData;
}

inline string RSkComponentImage::generateUriPath(string path) {
  if(path.substr(0, 14) == "file://assets/")
    path = "./" + path.substr(7);
  return path;
}

RnsShell::LayerInvalidateMask RSkComponentImage::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {

    auto const &newimageProps = *std::static_pointer_cast<ImageProps const>(newShadowView.props);
    auto component = getComponentData();
    auto const &oldimageProps = *std::static_pointer_cast<ImageProps const>(component.props);
    RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;

    if((forceUpdate) || (oldimageProps.resizeMode != newimageProps.resizeMode)) {
      imageProps.resizeMode = newimageProps.resizeMode;
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
    }
    if((forceUpdate) || (oldimageProps.tintColor != newimageProps.tintColor )) {
      /* TODO : Needs implementation*/
      imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
    }
    if((forceUpdate) || (oldimageProps.sources[0].uri.compare(newimageProps.sources[0].uri) != 0)) {
      if( isRequestInProgress_ && remoteCurlRequest_){
        // if url is changed, image component is get component property update.
        // cancel the onging request and made new request to network.  
        CurlNetworking::sharedCurlNetworking()->abortRequest(remoteCurlRequest_);
        remoteCurlRequest_ = nullptr;
        //TODO - need to send the onEnd event to APP if it is abort.
        isRequestInProgress_=false;
      }
      imageEventEmitter_->onLoadStart();
      hasToTriggerEvent_ = true;
    }
    return updateMask;
}

void RSkComponentImage::drawAndSubmit() {
  layer()->client().notifyFlushBegin();
  layer()->invalidate( RnsShell::LayerPaintInvalidate);
  if (layer()->type() == RnsShell::LAYER_TYPE_PICTURE) {
    RNS_PROFILE_API_OFF(getComponentData().componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer().get())->setPicture(getPicture()));
  }
  layer()->client().notifyFlushRequired();
}

// callback for remoteImageData
bool RSkComponentImage::processImageData(const char* path, char* response, int size) {
  decodedimageCacheData imageCacheData;
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<ImageProps const>(component.props);
  // Responce callback from network. Get image data, insert in Cache and call Onpaint
  sk_sp<SkImage> remoteImageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(path);
  if(remoteImageData ) {
    if(strcmp(path,imageProps.sources[0].uri.c_str()) == 0) {
      drawAndSubmit();
    }
  } else {
    if(!response) return false;
    sk_sp<SkData> data = SkData::MakeWithCopy(response,size);
    if (!data){
      RNS_LOG_ERROR("Unable to make SkData for path : " << path);
      return false;
    }
    remoteImageData = SkImage::MakeFromEncoded(data);
    if(!remoteImageData) return false;

    //Add in cache if image data is valid
    if(remoteImageData && canCacheData_){
      imageCacheData.imageData = remoteImageData;
      imageCacheData.expiryTime = (SkTime::GetMSecs() + cacheExpiryTime_);//convert sec to milisecond 60 *1000
      RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(path, imageCacheData);
    }
    if(strcmp(path,imageProps.sources[0].uri.c_str()) == 0){
      networkImageData_ = remoteImageData;
      drawAndSubmit();
    }
  }
  return true;
}

inline void RSkComponentImage::drawContentShadow( SkCanvas *canvas,
                            SkRect frameRect,
                            SkRect imageTargetRect,
                            sk_sp<SkImage> imageData ,
                            const ImageProps &imageProps,
                            SkSize shadowOffset,
                            SkColor shadowColor,
                            float shadowOpacity){
  /*TO DO :When Frame doesn't have background, has border with Jpeg Image and no resize.
    currently drawing shadow for both border and content.
    Need to cross verify with reference and confirm the behaviour.*/

  SkRect shadowBounds;
  SkIRect shadowFrame;
  SkRect  frameBound;
/*On below special cases, content Shadow to be drawn on complete frame/Layout instead on Image/content frame :
  ------------------------------------------------------------------------------------------------------------
   1. The target size of Image > Frame's size. In that case, clipping will be done to contain the image
      within the frame, So shadow to be drawn conidering frame size.
   2. For Repeat mode, Target frame size is the size of the frame itself.[Image will be repeated to fill the frame]
*/
  bool shadowOnFrame=(( frameRect.width() < imageTargetRect.width()) || ( frameRect.height() < imageTargetRect.height())||(imageProps.resizeMode == ImageResizeMode::Repeat));
  if(shadowOnFrame) {
    //Shadow on Frame Boundary
    frameBound=frameRect;
    shadowFrame.setXYWH(frameRect.x() + shadowOffset.width(), frameRect.y() + shadowOffset.height(), frameRect.width(), frameRect.height());
  } else {
    //Shadow on Image/Content Boundary
    frameBound=imageTargetRect;
    shadowFrame.setXYWH(imageTargetRect.x() + shadowOffset.width(), imageTargetRect.y() + shadowOffset.height(), imageTargetRect.width(), imageTargetRect.height());
  }
  SkIRect shadowIBounds=RSkDrawUtils::getShadowBounds(shadowFrame,layer()->shadowMaskFilter,layer()->shadowImageFilter);
  shadowBounds=SkRect::Make(shadowIBounds);

  bool saveLayerDone=false;
//Apply Opacity
  if(shadowOpacity) {
    canvas->saveLayerAlpha(&shadowBounds,shadowOpacity);
    saveLayerDone=true;
  }

  SkPaint shadowPaint;
  setPaintFilters(shadowPaint,imageProps,imageTargetRect,frameRect,true,imageData->isOpaque());

  if(!imageData->isOpaque() ) {
//Apply Shadow for transparent image
    canvas->drawImageRect(imageData, imageTargetRect, &shadowPaint);
  } else {
//Apply Shadow for opaque image
    if(!saveLayerDone) {
      canvas->saveLayer(&shadowBounds,&shadowPaint);
      saveLayerDone =true;
    }
    // clipping done to avoid drawing on non visible area [Area under opque frame]
    canvas->clipRect(frameBound,SkClipOp::kDifference);
    shadowPaint.setColor(shadowColor);
    canvas->drawIRect(shadowFrame, shadowPaint);
  }
  if(saveLayerDone) {
    canvas->restore();
  }
#ifdef SHOW_SHADOW_BOUND
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setColor(SK_ColorGREEN);
  paint.setStrokeWidth(2);
  shadowBounds.join(frameBound);
  canvas->drawRect(shadowBounds,paint);
#endif
}

inline void RSkComponentImage::setPaintFilters (SkPaint &paintObj,const ImageProps &imageProps,
                                                      SkRect imageTargetRect,SkRect frameRect ,
                                                      bool  setFilterForShadow, bool opaqueImage) {
  
  //This function applies appropriate filter on paint to draw Shadow or Image. 

   /*  Image Filter will be used on below scenario :
       -------------------------------------------
      1. For shadow on Image with transparent pixel
      2. For Image draw with Resize mide as "repeat"
      3. For Image Draw with with blur Effect.
   */
  if((setFilterForShadow && !opaqueImage)||
      (! setFilterForShadow &&
         ((imageProps.resizeMode == ImageResizeMode::Repeat) || (imageProps.blurRadius > 0))
      )) {
      sk_sp<SkImageFilter> shadowFilter{nullptr};
      if(setFilterForShadow && (layer()->shadowImageFilter != nullptr) ) {
         shadowFilter=layer()->shadowImageFilter;
      }
      if(imageProps.resizeMode == ImageResizeMode::Repeat) {
         shadowFilter = (SkImageFilters::Tile(imageTargetRect,frameRect,shadowFilter ));
      }
      if(imageProps.blurRadius > 0) {
        shadowFilter = SkImageFilters::Blur(imageProps.blurRadius, imageProps.blurRadius,shadowFilter);
      }
      paintObj.setImageFilter(std::move(shadowFilter));
   } else if(setFilterForShadow && (layer()->shadowMaskFilter != nullptr)) {
      paintObj.setMaskFilter(layer()->shadowMaskFilter);
  }
}


void RSkComponentImage::requestNetworkImageData(string sourceUri) {
  remoteCurlRequest_ = std::make_shared<CurlRequest>(nullptr,sourceUri,0,"GET");
  
  folly::dynamic query = folly::dynamic::object();

  //Before network request, reset the cache info with default values
  canCacheData_ = true;
  cacheExpiryTime_ = DEFAULT_MAX_CACHE_EXPIRY_TIME;

  // headercallback lambda fuction
  auto headerCallback =  [this, weakThis = this->weak_from_this()](void* curlresponseData,void *userdata)->size_t {
    auto isAlive = weakThis.lock();
    if(!isAlive) {
       RNS_LOG_WARN("This object is already destroyed. ignoring the completion callback");
       return 0;
     }

    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest *curlRequest = (CurlRequest *) userdata;

    double responseMaxAgeTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
    // Parse server response headers and retrieve caching details
    auto responseCacheControlData = responseData->headerBuffer.find("Cache-Control");
    if(responseCacheControlData != responseData->headerBuffer.items().end()) {
      std::string responseCacheControlString = responseCacheControlData->second.asString();
      canCacheData_ = remoteCurlRequest_->shouldCacheData();
      if(canCacheData_) cacheExpiryTime_ = responseData->cacheExpiryTime;
    }
    RNS_LOG_DEBUG("url [" << responseData->responseurl << "] canCacheData[" << canCacheData_ << "] cacheExpiryTime[" << cacheExpiryTime_ << "]");
    return 0;
  };


  // completioncallback lambda fuction
  auto completionCallback =  [this, weakThis = this->weak_from_this()](void* curlresponseData,void *userdata)->bool {
    auto isAlive = weakThis.lock();
    if(!isAlive) {
      RNS_LOG_WARN("This object is already destroyed. ignoring the completion callback");
      return 0;
    }
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest * curlRequest = (CurlRequest *) userdata;
    if((!responseData
        || !processImageData(curlRequest->URL.c_str(),responseData->responseBuffer,responseData->contentSize)) && (hasToTriggerEvent_)) {
      sendErrorEvents();
    }
    isRequestInProgress_=false;
    remoteCurlRequest_ = nullptr;
    return 0;
  };

  remoteCurlRequest_->curldelegator.delegatorData = remoteCurlRequest_.get();
  remoteCurlRequest_->curldelegator.CURLNetworkingHeaderCallback = headerCallback;
  remoteCurlRequest_->curldelegator.CURLNetworkingCompletionCallback=completionCallback;
  if(!hasToTriggerEvent_) {
    imageEventEmitter_->onLoadStart();
    hasToTriggerEvent_ = true;
  }
  CurlNetworking::sharedCurlNetworking()->sendRequest(remoteCurlRequest_,query);
  isRequestInProgress_ = true;
}

inline void RSkComponentImage::sendErrorEvents() {
  imageEventEmitter_->onError();
  imageEventEmitter_->onLoadEnd();
  hasToTriggerEvent_ = false;
}

inline void RSkComponentImage::sendSuccessEvents() {
  imageEventEmitter_->onLoad();
  imageEventEmitter_->onLoadEnd();
  hasToTriggerEvent_ = false;
}

RSkComponentImage::~RSkComponentImage(){
  // Image component is request send to network by then component is deleted.
  // still the network component will process the request, by calling abort.
  // will reduces the load on network and improve the performance.
  if(isRequestInProgress_ && remoteCurlRequest_){
    //TODO - need to send the onEnd event to APP if it is abort. 
    CurlNetworking::sharedCurlNetworking()->abortRequest(remoteCurlRequest_);
    isRequestInProgress_=false;
  }
}

} // namespace react
} // namespace facebook
