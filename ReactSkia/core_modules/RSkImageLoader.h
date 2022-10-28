/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <better/map.h>
#include "ReactCommon/TurboCxxModule.h"
#include "ReactSkia/sdk/CurlNetworking.h"

using namespace std;
namespace facebook {
using namespace xplat::module;

namespace react {

class RSkImageLoaderModule : public CxxModule {
 public:
  RSkImageLoaderModule() = default;
  ~RSkImageLoaderModule();
  virtual auto getConstants() -> std::map<std::string, folly::dynamic>;
  virtual auto getMethods() -> std::vector<Method>;
  virtual std::string getName();

 private:
  typedef better::map <std::string, std::shared_ptr<CurlRequest>> ImageSizeMap;
  ImageSizeMap imageRequestList_;
  void getImageSize(std::string uri, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
  void getImageSizeWithHeaders(std::string uri, folly::dynamic headers, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
  inline void handleRejectBlock( CxxModule::Callback rejectBlock);
  inline void handleResolveBlock(CxxModule::Callback resolveBlock,sk_sp<SkImage> remoteImageData);
  void prefetchImage(std::string uri, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
  void queryCache(folly::dynamic uris, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
};

class RSkImageLoader : public TurboCxxModule {
 public:
  RSkImageLoader(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker);

  ~RSkImageLoader() = default;
 private:
  
};

}//namespace react
}//namespace facebook
