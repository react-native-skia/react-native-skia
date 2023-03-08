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
    unsigned int index;
    for(auto mapIt = listenersList.begin(); mapIt != listenersList.end(); mapIt++) {
        index=0;
        for(auto vecIt = mapIt->second.begin(); vecIt !=  mapIt->second.end(); vecIt++) {
            if((*vecIt)->id == listener_id) {
                mapIt->second.erase(mapIt->second.begin() + index);
                if(mapIt->second.empty() ) {
                    listenersList.erase(mapIt->first);// dropping map Entry with No Listners
                }
                return;// Listener removed,Exiting..
            }
            index++;
        }
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
