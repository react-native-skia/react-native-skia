//
// Copyright (c) 2014 Sean Farrell
// Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
//  
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
//


#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>

#include <folly/io/async/ScopedEventBaseThread.h>

#include "ReactSkia/utils/RnsLog.h"

#pragma once

using NotificationCompleteVoidCallback = std::function<void()>;

class NotificationCenter {
    private:
        struct ListenerBase {
            ListenerBase() {}

            ListenerBase(unsigned int i)
            : id(i) {}

            virtual ~ListenerBase() {}

            unsigned int id;
        };

        template <typename... Args>
        struct Listener : public ListenerBase {
            Listener() {}

            Listener(unsigned int i, std::function<void (Args...)> c)
            : ListenerBase(i), cb(c) {}

            std::function<void (Args...)> cb{nullptr};
        };

        std::mutex mutex;
        unsigned int last_listener{0};
        std::map<std::string, std::vector<std::shared_ptr<ListenerBase>>> listenersList;

        NotificationCenter(const NotificationCenter&) = delete;  
        const NotificationCenter& operator = (const NotificationCenter&) = delete;
        folly::ScopedEventBaseThread eventNotifierThread_;
    public:
        NotificationCenter()
        : eventNotifierThread_("NotificationCenterThread") {
          eventNotifierThread_.getEventBase()->waitUntilRunning();
        }

        ~NotificationCenter() {}

        static NotificationCenter& defaultCenter();
        static void initializeDefault();

        //Notification center to handle events from subWindows
        static NotificationCenter& subWindowCenter();
        static void initializeSubWindowCenter();

        template <typename... Args>
        unsigned int addListener(std::string eventName, std::function<void (Args...)> cb);

        template <typename... Args>
        unsigned int on(std::string eventName, std::function<void (Args...)> cb);

        void removeListener(unsigned int listener_id);

        template <typename... Args>
        void emit(std::string eventName, Args... args);

};

template <typename... Args>
unsigned int NotificationCenter::addListener(std::string eventName, std::function<void (Args...)> cb) {
    if (!cb) {
        // throw does not work as exception is disbaled with -fno-exceptions 
        //throw std::invalid_argument("NotificationCenter::addListener: No callbak provided.");
        RNS_LOG_INFO("NotificationCenter::addListener: No callback provided.");
        return 0;
    }
    std::lock_guard<std::mutex> lock(mutex);
    unsigned int listener_id = ++last_listener;
    auto addlistenerHandler = [=](){
      std::lock_guard<std::mutex> lock(mutex);

      auto itr = listenersList.find(eventName);

      if( itr!=listenersList.end() ) {
          (itr->second).push_back(std::make_shared<Listener<Args...>>(listener_id, cb));
      } else {
          std::vector<std::shared_ptr<ListenerBase>> vHandle;
          vHandle.push_back(std::make_shared<Listener<Args...>>(listener_id, cb));
          listenersList.insert({eventName,vHandle});
      }
    };
    eventNotifierThread_.getEventBase()->runInEventBaseThread(std::move(addlistenerHandler));
   return listener_id;
}

template <typename... Args>
unsigned int NotificationCenter::on(std::string eventName, std::function<void (Args...)> cb) {
    return addListener(eventName, cb);
}

template <typename... Args>
void NotificationCenter::emit(std::string eventName, Args... args) {
    //Creating dispatch Handler to dispatch the event in the event base folly thread.
    auto dispatchHandler = [=](){
        std::unique_lock<std::mutex> lock(mutex);
        auto itr =listenersList.find(eventName);
        if(itr == listenersList.end()){
            return; // No Listners for this Event
        }
        auto handle = itr->second;

        mutex.unlock();

        for (auto& iter : handle) {
            auto l =  std::dynamic_pointer_cast<Listener<Args...>>(iter);
            if(l->cb != nullptr) {
                l->cb(args...); // Fire callback to all the Listeners
            }
        }
    };//End of dispacthHandler
    eventNotifierThread_.getEventBase()->runInEventBaseThread(std::move(dispatchHandler));
}

