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

#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <list>
#include <algorithm>

#pragma once

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

            std::function<void (Args...)> cb;
        };

        std::mutex mutex;
        unsigned int last_listener;
        std::multimap<std::string, std::shared_ptr<ListenerBase>> listeners;

        NotificationCenter(const NotificationCenter&) = delete;  
        const NotificationCenter& operator = (const NotificationCenter&) = delete;
    public:
        NotificationCenter() {
            NotificationCenter::last_listener = 0;
        }

        ~NotificationCenter() {}

        static NotificationCenter& defaultCenter();
        static void initializeDefault();

        template <typename... Args>
        unsigned int addListener(std::string eventName, std::function<void (Args...)> cb);

        //unsigned int addListener(std::string eventName, std::function<void ()> cb);

        template <typename... Args>
        unsigned int on(std::string eventName, std::function<void (Args...)> cb);
        
        //unsigned int on(std::string eventName, std::function<void ()> cb);
        void removeListener(unsigned int listener_id);

        template <typename... Args>
        void emit(std::string eventName, Args... args);

};

template <typename... Args>
unsigned int NotificationCenter::addListener(std::string eventName, std::function<void (Args...)> cb) {
    if (!cb) {
        // throw does not work as exception is disbaled with -fno-exceptions 
        //throw std::invalid_argument("NotificationCenter::addListener: No callbak provided.");

        std::cout << "NotificationCenter::addListener: No callback provided.";
    }
    std::lock_guard<std::mutex> lock(mutex);

    unsigned int listener_id = ++last_listener;
    listeners.insert(std::make_pair(eventName, std::make_shared<Listener<Args...>>(listener_id, cb)));

    return listener_id;        
}

template <typename... Args>
unsigned int NotificationCenter::on(std::string eventName, std::function<void (Args...)> cb) {
    return addListener(eventName, cb);
}

template <typename... Args>
void NotificationCenter::emit(std::string eventName, Args... args) {
    std::list<std::shared_ptr<Listener<Args...>>> handlers;
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto range = listeners.equal_range(eventName);
        handlers.resize(std::distance(range.first, range.second));
        std::transform(range.first, range.second, handlers.begin(), [] (std::pair<const std::string, std::shared_ptr<ListenerBase>> p) {
            auto l = std::dynamic_pointer_cast<Listener<Args...>>(p.second);
            
            return l;
        });
    }

    for (auto& h : handlers) {
        h->cb(args...);
    }        
}


