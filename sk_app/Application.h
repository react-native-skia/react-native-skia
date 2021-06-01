/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Application_DEFINED
#define Application_DEFINED

#include "folly/io/async/EventBase.h"

namespace sk_app {

class Application {
 public:
  static Application *Create(int argc, char **argv, void *platformData);

  virtual ~Application() {}

  virtual void onIdle() = 0;

  static folly::EventBase &GetMainTaskRunner() {
    return s_event_base;
  }

 private:
  static folly::EventBase s_event_base;
};

} // namespace sk_app

#endif
