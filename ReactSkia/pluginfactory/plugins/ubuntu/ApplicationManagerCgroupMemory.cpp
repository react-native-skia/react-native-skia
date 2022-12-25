/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>

#include "ReactSkia/pluginfactory/plugins/ubuntu/ApplicationManager.h"

#define MAX_EPOLL_EVENTS            10
#define MAX_EPOLL_TIMEOUT           -1 // infinite
#define CGROUP_MEMORY_PRESSURE  "/sys/fs/cgroup/memory/memory.pressure_level"
#define CGROUP_EVENT_CONTROL    "/sys/fs/cgroup/memory/cgroup.event_control"
#define CGROUP_MEMORY_USAGE     "/sys/fs/cgroup/memory/memory.usage_in_bytes"

namespace facebook {
namespace react {
namespace rnsplugin {

int RNSApplicationManagerInterfaceImpl::CgroupMemoryData::addToEpollWait(int epfd, int efd) {
  struct epoll_event ev = {0};
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = efd;
  return(epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev));
}

int RNSApplicationManagerInterfaceImpl::CgroupMemoryData::removeFromEpollWait(int epfd, int efd) {
  struct epoll_event ev = {0};
  ev.events = EPOLLIN;
  ev.data.fd = efd;
  return(epoll_ctl(epfd, EPOLL_CTL_DEL, efd, &ev));
}

RNSApplicationManagerInterfaceImpl::CgroupMemoryData::CgroupMemoryData(CgroupMemoryNotificationCallback cb)
    : notifyFunc(cb) {
  if((epfd_ = epoll_create(1)) < 0) {
    RNS_LOG_ERROR("Couldnt Create Epoll FD" << strerror(errno));
    return;
  }
  if((exitEfd_ = (int)eventfd(0,0)) == -1) {
    RNS_LOG_ERROR("Couldnt Create Event FD : " << strerror(errno));
    return;
  }
  if( addToEpollWait(epfd_, exitEfd_) == -1) { // add this eventfd to poll
    RNS_LOG_ERROR("Failed to add Event FD to Poll : " << strerror(errno));
    return;
  }
  workerThread_ = std::thread([this]() {
    monitorThread();
  });
  memEfd_ = startMonitoringCgroupMemoryPressure("low", "hierarchy");
}

void RNSApplicationManagerInterfaceImpl::CgroupMemoryData::monitorThread() {
  struct epoll_event events[MAX_EPOLL_EVENTS];
  int nfds;

  do {
    if ((nfds = epoll_wait(epfd_, events, MAX_EPOLL_EVENTS, MAX_EPOLL_TIMEOUT)) < 0) {
      RNS_LOG_ERROR("Failed epoll_wait...");
      std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
    RNS_LOG_DEBUG("Received epoll event");

    if(nfds) {
      int i;
      for (i = 0; i < nfds; ++i) { //search for efd on which event occured
        int efd = events[i].data.fd;
        uint64_t u;

        if( exitEfd_ == efd ) {
          RNS_LOG_DEBUG("Got exit notification on efd : " << efd << ", Will exit the thread");
          return;
        }
        if (read(efd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
          RNS_LOG_ERROR("Failed to read event_control on efd : " << efd);
        } else {
          RNS_LOG_DEBUG("Memory event received on efd : " << efd);
          if(notifyFunc)
            notifyFunc(efd);
        }
      }
    }
  }while(true);
  RNS_LOG_INFO("========== Exited from memory monitor thread ==========");
}

RNSApplicationManagerInterfaceImpl::CgroupMemoryData::~CgroupMemoryData() {
  // 1. Write dummy data on exitFd to exit epoll_wait thread
  if(exitEfd_) {
    uint64_t u = 1;
    if( write(exitEfd_, &u, sizeof(uint64_t)) != sizeof(uint64_t) ) {
      RNS_LOG_ERROR("Critical Error, exit without joining thread");
    } else {
      // 2. Wait until thread joins
      if (workerThread_.joinable() ) {
        workerThread_.join();
      }
    }
  }
  // 3. Cleanup fd
  if(epfd_) {
    close(epfd_);
  }
  if(exitEfd_)
    close(exitEfd_);
}

int RNSApplicationManagerInterfaceImpl::CgroupMemoryData::configureEventControl(int eventFd, string commandBuffer) {
  int eventControlFd = -1;
  bool result = false;
  do {
    if ((eventControlFd = open(CGROUP_EVENT_CONTROL, O_WRONLY)) == -1) {
      RNS_LOG_WARN("Failed to open : " << CGROUP_EVENT_CONTROL << " : " << strerror(errno));
      break;
    }
    if (write(eventControlFd, commandBuffer.c_str(), commandBuffer.length()) == -1) {
      RNS_LOG_WARN("Failed to write cgroup.event_control file :" << CGROUP_EVENT_CONTROL << " : " << strerror(errno));
      break;
    }
    if( addToEpollWait(epfd_, eventFd) == -1) {
      RNS_LOG_WARN("Failed to add epoll for :" << CGROUP_EVENT_CONTROL << " : " << strerror(errno));
      break;
    }
    result = true;
  }while(0);

  if (eventControlFd >= 0 && close(eventControlFd) == -1) {
    RNS_LOG_WARN("Failed close " << CGROUP_EVENT_CONTROL);
  }
  return result;
}

int RNSApplicationManagerInterfaceImpl::CgroupMemoryData::startMonitoringCgroupMemoryPressure(string pressurelevel, string propagationMode) {
  char buffer[128];
  int eventFd = -1, memoryFileFd = -1;
  bool result = false;

  do {
    if ((eventFd = (int)eventfd(0, 0)) == -1) {
      RNS_LOG_WARN("Couldnt create Event FD : " << strerror(errno));
      break;
    }
    if ((memoryFileFd = open(CGROUP_MEMORY_PRESSURE, O_RDONLY)) == -1) {
      RNS_LOG_WARN("Failed to open : " << CGROUP_MEMORY_PRESSURE << " : " << strerror(errno));
      break;
    }
    sprintf(buffer, "%d %d %s,%s", eventFd, memoryFileFd, pressurelevel.c_str(), propagationMode.c_str());
    result = configureEventControl(eventFd, buffer);
  }while(0);

  if(!result) {
    if( eventFd >= 0 ) {
      close(eventFd);
      eventFd = -1;
    }
  }

  if (memoryFileFd >= 0 && close(memoryFileFd) == -1) {
    RNS_LOG_WARN("Failed close " << CGROUP_MEMORY_PRESSURE);
  }
  return eventFd;
}

int RNSApplicationManagerInterfaceImpl::CgroupMemoryData::startMonitoringCgroupMemoryThreshold(unsigned long long threshold) {
  char buffer[128];
  int eventFd = -1, memoryFileFd = -1;
  bool result = false;

  do {
    if ((eventFd = (int)eventfd(0, 0)) == -1) {
      RNS_LOG_WARN("Couldnt create Event FD : " << strerror(errno));
      break;
    }
    if ((memoryFileFd = open(CGROUP_MEMORY_USAGE, O_RDONLY)) == -1) {
      RNS_LOG_WARN("Failed to open : " << CGROUP_MEMORY_USAGE << " : " << strerror(errno));
      break;
    }
    sprintf(buffer, "%d %d %llu", eventFd, memoryFileFd, threshold);
    result = configureEventControl(eventFd, buffer);
  }while(0);

  if(!result) {
    if( eventFd >= 0 ) {
      close(eventFd);
      eventFd = -1;
    }
  }

  if (memoryFileFd >= 0 && close(memoryFileFd) == -1) {
    RNS_LOG_WARN("Failed close " << CGROUP_MEMORY_USAGE);
  }
  return eventFd;
}

} // rnsplugin
} // react
} // facebook
