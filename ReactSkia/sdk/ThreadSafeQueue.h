/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace rns {
namespace sdk {

template<typename Type>
class ThreadSafeQueue {

 public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() = default;

  void push(Type const& data) {
    {
      std::lock_guard<std::mutex> lock(queueMutex_);
      dataQueue_.push(data);
    }
    queueSignal_.notify_one();
  }

  bool tryPop(Type& value) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if(dataQueue_.empty()) {
      return false;
    }
    value = dataQueue_.front();
    dataQueue_.pop();
    return true;
  }

  // Blocks when queue is empty
  void pop(Type& value) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    while (dataQueue_.empty()) {
      queueSignal_.wait(lock);
    }

    value = dataQueue_.front();
    dataQueue_.pop();
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return dataQueue_.size();
  }

  bool isEmpty() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return dataQueue_.empty();
  }

  void clear() {
    std::queue<Type> emptyQueue;
    std::lock_guard<std::mutex> lock(queueMutex_);
    std::swap<Type>(dataQueue_, emptyQueue);
  }

  // TODO add timedPop and waitAndPop

 private:
  std::queue<Type> dataQueue_;
  std::condition_variable queueSignal_;
  mutable std::mutex queueMutex_;
};

} // namespace rns
} // namespace sdk
