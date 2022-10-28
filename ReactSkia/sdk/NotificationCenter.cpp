//
// Copyright (c) 2014 Sean Farrell
// Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
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

#include <glog/logging.h>

#include "NotificationCenter.h"

static std::unique_ptr<NotificationCenter> defaultCenter_;
static std::unique_ptr<NotificationCenter> subWindowCenter_;//SubWindow Notification center

void NotificationCenter::removeListener(unsigned int listener_id) {
    std::lock_guard<std::mutex> lock(mutex);

    auto i = std::find_if(listeners.begin(), listeners.end(), [&] (std::pair<const std::string, std::shared_ptr<ListenerBase>> p) {
        return p.second->id == listener_id;
    });
    if (i != listeners.end()) {
        listeners.erase(i);
    }
    else {
        // throw does not work as exception is disbaled with -fno-exceptions 
        //throw std::invalid_argument("NotificationCenter::removeListener: Invalid listener id.");

        std::cout << "NotificationCenter::removeListener: Invalid listener id.";
    }
}

NotificationCenter& NotificationCenter::defaultCenter() {
    return *defaultCenter_;
}

void NotificationCenter::initializeDefault() {
    if(defaultCenter_.get() == nullptr)
        defaultCenter_ = std::make_unique<NotificationCenter>();
}

NotificationCenter& NotificationCenter::subWindowCenter() {
    return *subWindowCenter_;
}

void NotificationCenter::initializeSubWindowCenter() {
    if(subWindowCenter_.get() == nullptr)
        subWindowCenter_ = std::make_unique<NotificationCenter>();
}
