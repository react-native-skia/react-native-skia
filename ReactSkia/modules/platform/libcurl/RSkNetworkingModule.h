/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <better/map.h>


#include "ReactSkia/modules/RSkNetworkingModuleBase.h"

namespace facebook {
namespace react {
class RSkNetworkingModule:  public RSkNetworkingModuleBase {
  public:
        RSkNetworkingModule(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);
        ~ RSkNetworkingModule();

        jsi::Value sendRequest(
            folly::dynamic) override;

        better::map <int , CURL*> connectionList_;

  private:
        static uint64_t nextUniqueId();

};
}// namespace react
}// namespace facebook
