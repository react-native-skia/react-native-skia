 /*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/utils/RnsLog.h"
#include "RNSAssetManager.h"

#define  FILENAME "./assets/RSkAssetMap.json"

namespace rns {
namespace sdk {

std::mutex RNSAssetManager::mutex_;//static memebers are suppose initialise out side the class.
RNSAssetManager* RNSAssetManager::RNSAssetManagerInstance_{nullptr};

RNSAssetManager::RNSAssetManager(){
  fstream assetFile;
  assetFile.open(FILENAME, ios::out | ios::in | ios::app);
  if (assetFile.is_open()) {
    assetFile.seekg (0, assetFile.end);
    int length = assetFile.tellg();
    if(length != 0){
      try{
        assetFile.seekg (0, assetFile.beg);
        char * buffer = new char [length];
        // read data as a block:
        assetFile.read (buffer,length);
        string Str(buffer);
        assetDataFile_ = folly::parseJson(Str);
      }
      catch(exception e){
        RNS_LOG_ERROR("json parsing failed");
      }
    }
    assetFile.close();
  }
}

RNSAssetManager::~RNSAssetManager(){}

RNSAssetManager* RNSAssetManager::instance(){
  std::lock_guard<std::mutex> lock(mutex_);
  if (!RNSAssetManagerInstance_) {
    RNSAssetManagerInstance_ = new RNSAssetManager();
  }
  return RNSAssetManagerInstance_;
}

string RNSAssetManager::getAssetPath(string uri){
  std::string result="";
  if(assetDataFile_.find("assets") == assetDataFile_.items().end()){
    RNS_LOG_ERROR("Couldn't able to Load the asset file");
    return result;
  }
  auto reqAsset = assetDataFile_["assets"]["images"];
  auto pos = reqAsset.find(uri);
  if( pos != reqAsset.items().end() ){
    result = pos->second["path"].asString();
    RNS_LOG_DEBUG("RNSAssetManager getDefaultAssetPATH"<<result);
  }else{
    RNS_LOG_ERROR("Image not found in assets");
  }
  return result;
}

} // namespace sdk
} // namespace rns