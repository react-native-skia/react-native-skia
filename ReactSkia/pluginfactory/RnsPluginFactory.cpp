/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "RnsPlugin.h"
#include "RnsPluginFactory.h"

namespace facebook {
namespace react {
namespace rnsplugin {

RnsPluginFactory::RnsPluginFactory(){

#if ENABLE(RNSP_APP_MANAGER)
  RNS_LOG_INFO("========== Loading Application Manager Plugin : " << RNSP_APP_MANAGER_PLUGIN << " for RNS ==========");
  try {
    appMgrPluginHandler_ = dlopen(RNSP_APP_MANAGER_PLUGIN, RTLD_NOW);
    if (nullptr == appMgrPluginHandler_) {
      RNS_LOG_ERROR("Could not Load Application Manager Plugin : " << RNSP_APP_MANAGER_PLUGIN);
      Check_dlerror();
    }
    appManagerCreator_ = reinterpret_cast<RNSApplicationManagerInterface_t>(dlsym(appMgrPluginHandler_, "RnsPluginAppManagerCreate"));
    if (nullptr == appManagerCreator_) {
      RNS_LOG_ERROR("Could not find symbol RnsPluginAppManagerCreate in plugin");
    }
    Check_dlerror();
  } catch (const std::exception& ex){
    RNS_LOG_ERROR(" Platform Application Manager Plugin Loading Error :  " << ex.what());
  }
#endif // RNSP_APP_MANAGER
}

#if ENABLE(RNSP_APP_MANAGER)
std::unique_ptr<RNSApplicationManagerInterface> RnsPluginFactory::createAppManagerHandle(AppCallbackClient &client) const {
  if(appManagerCreator_ == nullptr)
    return nullptr;
  return std::unique_ptr<RNSApplicationManagerInterface>(appManagerCreator_(client));
}
#endif // RNSP_APP_MANAGER

RnsPluginFactory::~RnsPluginFactory() {
#if ENABLE(RNSP_APP_MANAGER)
  if (appMgrPluginHandler_) {
    dlclose(appMgrPluginHandler_);
  }
#endif // RNSP_APP_MANAGER
}

} // namespace rnsplugin
} // namespace react
} // namespace facebook

