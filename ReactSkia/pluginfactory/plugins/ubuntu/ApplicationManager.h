/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

using namespace std;

using CgroupMemoryNotificationCallback = std::function<void(int efd)>;

class RNSApplicationManagerInterfaceImpl : public RNSApplicationManagerInterface {
 public:
  RNSApplicationManagerInterfaceImpl(AppCallbackClient &client);
  ~RNSApplicationManagerInterfaceImpl()override;

 private:
  struct CgroupMemoryData {
   public:
    CgroupMemoryData(CgroupMemoryNotificationCallback fucn);
    ~CgroupMemoryData();
    static int addToEpollWait(int epfd, int efd);
    static int removeFromEpollWait(int epfd, int efd);
   private:
    int epfd_;
    int exitEfd_;
    int memEfd_{0};
    std::thread workerThread_;
    CgroupMemoryNotificationCallback notifyFunc {nullptr};

    int configureEventControl(int eventFd, string commandBuffer);
    int startMonitoringCgroupMemoryThreshold(unsigned long long threshold=0);
    int startMonitoringCgroupMemoryPressure(string pressurelevel="low", string propagationMode = "hierarchy");
    void monitorThread();
  };

  CgroupMemoryData& cgMemHandle_;
  void handleMemoryNotification(int efd);
  CgroupMemoryData& initializeCgroupMemory(CgroupMemoryNotificationCallback cb);
  typedef RNSApplicationManagerInterface INHERITED;
};

extern "C" {
  RNSP_EXPORT RNSApplicationManagerInterface * RnsPluginAppStateCreate(AppCallbackClient &client);
}

} // rnsplugin
} // react
} // facebook
