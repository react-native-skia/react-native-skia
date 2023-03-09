/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <memory>
#include <dlfcn.h>

namespace facebook {
namespace react {
namespace rnsplugin {

class RnsPluginFactory {
 public:
  RNSP_EXPORT RnsPluginFactory();
  RNSP_EXPORT ~RnsPluginFactory();

  uint8_t MajorVersion() {
    return RSK_INTERFACE_MAJOR_VERSION;
  }
  uint8_t MinorVersion() {
    return RSK_INTERFACE_MINOR_VERSION;
  }
  uint8_t MicroVersion() {
    return RSK_INTERFACE_MICRO_VERSION;
  }

#if ENABLE(RNSP_APP_MANAGER)
  RNSP_EXPORT std::unique_ptr<RNSApplicationManagerInterface> createAppManagerHandle(AppCallbackClient &client) const;
#endif

#if ENABLE(RNSP_PLATFORM_MANAGER)
  RNSP_EXPORT std::unique_ptr<RNSPlatformManagerInterface> createPlatformManagerHandle(PlatformCallbackClient &client) const;
#endif

private:
  void * appMgrPluginHandler_ = nullptr;
  void * platformPluginHandler_ = nullptr;
#if ENABLE(RNSP_APP_MANAGER)
  RNSApplicationManagerInterface_t appManagerCreator_ = nullptr;
#endif
#if ENABLE(RNSP_PLATFORM_MANAGER)
  RNSPlatformManagerInterface_t platformManagerCreator_ = nullptr;
#endif

  static void Reset_dlerror() {
    dlerror();
  }
  static void Check_dlerror() {
    const char * dlsym_error = dlerror();
    if (dlsym_error) {
      throw std::runtime_error(dlsym_error);
    }
  }
};

} // namespace rnsplugin
} // namespace react
} // namespace facebook
