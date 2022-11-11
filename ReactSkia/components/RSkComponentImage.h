/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "include/core/SkRect.h"

#include "react/renderer/components/image/ImageShadowNode.h"

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"
#include "ReactSkia/sdk/CurlNetworking.h"

#define DEFAULT_IMAGE_FILTER_QUALITY kLow_SkFilterQuality /*Skia's Defualt is kNone_SkFilterQuality*/
#define DEFAULT_MAX_CACHE_EXPIRY_TIME 1800000 // 30mins in milliseconds 1800000
#define RNS_NO_CACHE_STR "no-cache"
#define RNS_NO_STORE_STR "no-store"
#define RNS_MAX_AGE_0_STR "max-age=0"
#define RNS_MAX_AGE_STR "max-age"
namespace facebook {
namespace react {

struct ImgProps{
    ImageResizeMode resizeMode;
    SkColor tintColor;
};

class RSkComponentImage final : public RSkComponent {
 public:
  RSkComponentImage(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
  ~RSkComponentImage();
 private :
  ImgProps imageProps;
  std::shared_ptr<CurlRequest> remoteCurlRequest_{nullptr};
  sk_sp<SkImage> networkImageData_;
  bool isRequestinProgress_{false};
  bool hasToTriggerEvent_{false};
  bool canCacheData_{true};
  double cacheExpiryTime_{DEFAULT_MAX_CACHE_EXPIRY_TIME};
  std::shared_ptr<ImageEventEmitter const> imageEventEmitter_;

  sk_sp<SkImage> getLocalImageData(ImageSource source);
  void requestNetworkImageData(ImageSource source);

  inline string generateUriPath(string path);
  void drawAndSubmit();
  bool processImageData(const char* path, char* response, int size);
  inline void sendErrorEvents();
  inline void sendSuccessEvents();
 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
