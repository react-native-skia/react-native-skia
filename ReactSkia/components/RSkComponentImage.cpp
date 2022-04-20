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
    : RSkComponent(shadowView) {}

void RSkComponentImage::OnPaint(SkCanvas *canvas) {
  sk_sp<SkImage> imageData{nullptr};
  string path;
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<ImageProps const>(component.props);

  /*First to check file entry presence . If not exist, generate imageData*/
  if(!imageProps.sources.empty()){
    imageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(imageProps.sources[0].uri.c_str());
    if(!imageData) {
      if (imageProps.sources[0].type == ImageSource::Type::Local) {
        imageData = getLocalImageData(imageProps.sources[0]);
      } else if(imageProps.sources[0].type == ImageSource::Type::Remote) {
        requestNetworkImageData(imageProps.sources[0]);
      }
    }
  }
  auto imageEventEmitter = std::static_pointer_cast<ImageEventEmitter const>(component.eventEmitter);
  if(imageData)
    /* Emitting Load completed Event*/
    imageEventEmitter->onLoad();
  Rect frame = component.layoutMetrics.frame;
  SkRect frameRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);
  /* Draw order 1.Shadow 2. Background 3.Image Shadow 4. Image 5.Border*/
  bool contentShadow = false;
  bool needClipAndRestore =false;
  if(layer()->shadowOpacity && layer()->shadowFilter){
    contentShadow=drawShadow(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,layer()->shadowOpacity,layer()->shadowFilter);
  }
  drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);

  if(imageData) {
    SkRect targetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,imageProps.resizeMode);
    SkPaint paint,shadowPaint;
    /*Draw Image Shadow*/
    if(contentShadow) {
      if(imageProps.resizeMode == ImageResizeMode::Repeat) {
      /*TODO Not applied any shadow for solid Image when resizeMode is "Repeat"*/
        sk_sp<SkImageFilter> imageFilter(SkImageFilters::Tile(targetRect,frameRect,layer()->shadowFilter));
        shadowPaint.setImageFilter(std::move(imageFilter));
      } else {
        shadowPaint.setImageFilter(layer()->shadowFilter);
      }
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
       sk_sp<SkImageFilter> imageFilter(SkImageFilters::Tile(targetRect,frameRect,nullptr));
       paint.setImageFilter(std::move(imageFilter));
    }
    canvas->drawImageRect(imageData,targetRect,&paint);
    if(needClipAndRestore)
        canvas->restore();
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
  } else {
  /* Emitting Image Load failed Event*/
    RNS_LOG_ERROR("Image not loaded :"<<imageProps.sources[0].uri.c_str());
    imageEventEmitter->onError();
  }
}

sk_sp<SkImage> RSkComponentImage::getLocalImageData(ImageSource source) {
  sk_sp<SkImage> imageData{nullptr};
  sk_sp<SkData> data;
  string path;
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
  if(imageData)
    RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(source.uri.c_str(), imageData);

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
      RNS_LOG_NOT_IMPL;
      imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
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
void RSkComponentImage::processImageData(const char* path, char* response, int size) {
  // Responce callback from network. Get image data, insert in Cache and call Onpaint
  sk_sp<SkImage> findImageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(path);
  if(findImageData) {
    drawAndSubmit();
  } else {
    sk_sp<SkData> data = SkData::MakeWithCopy(response,size);
    if (!data){
      RNS_LOG_ERROR("Unable to make SkData for path : " << path);
      return;
    }
    findImageData = SkImage::MakeFromEncoded(data);
      //Add in cache if image data is valid
    if(findImageData && RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(path, findImageData))
      drawAndSubmit();
  }
}

void RSkComponentImage::requestNetworkImageData(ImageSource source) {
  auto sharedCurlNetworking = CurlNetworking::sharedCurlNetworking();
  CurlRequest *curlRequest = new CurlRequest(nullptr,source.uri.c_str(),0,"GET");

  folly::dynamic query = folly::dynamic::object();

  // completioncallback lambda fuction
  auto completionCallback =  [&](void* curlresponseData,void *userdata)->bool {
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest * curlRequest = (CurlRequest *) userdata;
    processImageData(responseData->responseurl,responseData->responseBuffer,responseData->contentSize);
    delete curlRequest;
    return 0;
  };

  curlRequest->curldelegator.delegatorData = curlRequest;
  curlRequest->curldelegator.CURLNetworkingCompletionCallback=completionCallback;
  sharedCurlNetworking->sendRequest(curlRequest,query);
}

} // namespace react
} // namespace facebook
