 /*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <fstream>
#include <iostream>
#include <mutex>
#include <folly/json.h>
using namespace std;
namespace rns {
namespace sdk {

class RNSAssetManager{
private:
  RNSAssetManager();
  static RNSAssetManager *RNSAssetManagerInstance_;
  static std::mutex mutex_;
  folly::dynamic assetDataFile_ = folly::dynamic::object;
public:  
  static RNSAssetManager* instance();
  string getAssetPath(string uri);
  ~RNSAssetManager();
};

} // namespace sdk
} // namespace rns
