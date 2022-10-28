/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"
#include "rns_shell/compositor/layers/PictureLayer.h"

#include "react/renderer/components/image/ImageEventEmitter.h"

#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/sdk/CurlNetworking.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkImageUtils.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace RSkImageUtils;

RSkComponentImage::RSkComponentImage(const ShadowView &shadowView)
    : RSkComponent(shadowView) {
      imageEventEmitter_ = std::static_pointer_cast<ImageEventEmitter const>(shadowView.eventEmitter);
}

void RSkComponentImage::OnPaint(SkCanvas *canvas) {
  sk_sp<SkImage> imageData{nullptr};
  string path;
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<ImageProps const>(component.props);
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
      imageData = getLocalImageData(imageProps.sources[0]);
    } else if(imageProps.sources[0].type == ImageSource::Type::Remote) {
      requestNetworkImageData(imageProps.sources[0]);
    }
  } while(0);

  Rect frame = component.layoutMetrics.frame;
  SkRect frameRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);
  // Draw order 1.Shadow 2. Background 3.Image Shadow 4. Image 5.Border
  bool contentShadow = false;
  bool needClipAndRestore =false;
  sk_sp<SkImageFilter> imageFilter;

  if(layer()->shadowFilter) {
    contentShadow=drawShadow(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,layer()->shadowOpacity,layer()->shadowFilter);
  }
  drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);

  if(imageData) {
    SkRect targetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,imageProps.resizeMode);
    SkPaint paint,shadowPaint;
    /*Draw Image Shadow*/
    if(contentShadow) {
      if(imageProps.resizeMode == ImageResizeMode::Repeat) {
        imageFilter = SkImageFilters::Tile(targetRect,frameRect,layer()->shadowFilter);
      }
      if(imageProps.blurRadius > 0) {
        imageFilter = SkImageFilters::Blur(imageProps.blurRadius, imageProps.blurRadius,(imageFilter ? imageFilter : layer()->shadowFilter));
      }
      imageFilter ? shadowPaint.setImageFilter(std::move(imageFilter)) : shadowPaint.setImageFilter(layer()->shadowFilter);

      if(!(isOpaque(layer()->shadowOpacity)))
        canvas->saveLayerAlpha(&frameRect,layer()->shadowOpacity);
      canvas->drawImageRect(imageData, targetRect, &shadowPaint);
      if(!(isOpaque(layer()->shadowOpacity)))
        canvas->restore();
    }
    /*Draw Image */
    if(( frameRect.width() < targetRect.width()) || ( frameRect.height() < targetRect.height()))
      needClipAndRestore= true;
    /* clipping logic to be applied if computed Frame is greater than the target.*/
    if(needClipAndRestore) {
        canvas->save();
        canvas->clipRect(frameRect,SkClipOp::kIntersect);
    }
    /* TODO: Handle filter quality based of configuration. Setting Low Filter Quality as default for now*/
    paint.setFilterQuality(DEFAULT_IMAGE_FILTER_QUALITY);
    if(imageProps.resizeMode == ImageResizeMode::Repeat) {
        imageFilter = (SkImageFilters::Tile(targetRect,frameRect,nullptr));
    }
    if(imageProps.blurRadius > 0)
      imageFilter =  SkImageFilters::Blur(imageProps.blurRadius, imageProps.blurRadius,(imageFilter ? imageFilter : nullptr));
    if(imageFilter) {
      paint.setImageFilter(std::move(imageFilter));
    }
    canvas->drawImageRect(imageData,targetRect,&paint);
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

sk_sp<SkImage> RSkComponentImage::getLocalImageData(ImageSource source) {
  sk_sp<SkImage> imageData{nullptr};
  sk_sp<SkData> data;
  string path;
  decodedimageCacheData imageCacheData;
  path = generateUriPath(source.uri.c_str());
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
    RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(source.uri.c_str(), imageCacheData);
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

inline bool shouldCacheData(std::string cacheControlData) {
  if(cacheControlData.find(RNS_NO_CACHE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_NO_STORE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_MAX_AGE_0_STR) != std::string::npos) return false;

  return true;
}

inline double getCacheMaxAgeDuration(std::string cacheControlData) {
  size_t maxAgePos = cacheControlData.find(RNS_MAX_AGE_STR);
  if(maxAgePos != std::string::npos) {
    size_t maxAgeEndPos = cacheControlData.find(';',maxAgePos);
    return std::stoi(cacheControlData.substr(maxAgePos+8,maxAgeEndPos));
  }
  return DEFAULT_MAX_CACHE_EXPIRY_TIME;
}

void RSkComponentImage::requestNetworkImageData(ImageSource source) {
  auto sharedCurlNetworking = CurlNetworking::sharedCurlNetworking();
  std::shared_ptr<CurlRequest> remoteCurlRequest = std::make_shared<CurlRequest>(nullptr,source.uri,0,"GET");

  folly::dynamic query = folly::dynamic::object();

  //Before network request, reset the cache info with default values
  canCacheData_ = true;
  cacheExpiryTime_ = DEFAULT_MAX_CACHE_EXPIRY_TIME;

  // headercallback lambda fuction
  auto headerCallback =  [this, remoteCurlRequest](void* curlresponseData,void *userdata)->bool {
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest *curlRequest = (CurlRequest *) userdata;

    double responseMaxAgeTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
    // Parse server response headers and retrieve caching details
    auto responseCacheControlData = responseData->headerBuffer.find("Cache-Control");
    if(responseCacheControlData != responseData->headerBuffer.items().end()) {
      std::string responseCacheControlString = responseCacheControlData->second.asString();
      canCacheData_ = shouldCacheData(responseCacheControlString);
      if(canCacheData_) responseMaxAgeTime = getCacheMaxAgeDuration(responseCacheControlString);
    }

    // TODO : Parse request headers and retrieve caching details

    cacheExpiryTime_ = std::min(responseMaxAgeTime,static_cast<double>(DEFAULT_MAX_CACHE_EXPIRY_TIME));
    RNS_LOG_DEBUG("url [" << responseData->responseurl << "] canCacheData[" << canCacheData_ << "] cacheExpiryTime[" << cacheExpiryTime_ << "]");
    return 0;
  };


  // completioncallback lambda fuction
  auto completionCallback =  [this, remoteCurlRequest](void* curlresponseData,void *userdata)->bool {
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest * curlRequest = (CurlRequest *) userdata;
    if((!responseData
        || !processImageData(curlRequest->URL.c_str(),responseData->responseBuffer,responseData->contentSize)) && (hasToTriggerEvent_)) {
      sendErrorEvents();
    }
    //Reset the lamda callback so that curlRequest shared pointer dereffered from the lamda 
    // and gets auto destructored after the completion callback.
    remoteCurlRequest->curldelegator.CURLNetworkingHeaderCallback = nullptr;
    remoteCurlRequest->curldelegator.CURLNetworkingCompletionCallback = nullptr;
    return 0;
  };

  remoteCurlRequest->curldelegator.delegatorData = remoteCurlRequest.get();
  remoteCurlRequest->curldelegator.CURLNetworkingHeaderCallback = headerCallback;
  remoteCurlRequest->curldelegator.CURLNetworkingCompletionCallback=completionCallback;
  if(!hasToTriggerEvent_) {
    imageEventEmitter_->onLoadStart();
    hasToTriggerEvent_ = true;
  }
  sharedCurlNetworking->sendRequest(remoteCurlRequest,query);
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

} // namespace react
} // namespace facebook
