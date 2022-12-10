/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#ifdef None
// Workaround conflict between folly's `Struct None` and X11's `#define None 0`
#undef None
#include "folly/io/async/EventBase.h"
#define None 0L
#else
#include "folly/io/async/EventBase.h"
#endif

namespace RnsShell {

class TaskLoop {

public:
    static void initializeMain();
    static TaskLoop& main();
    TaskLoop();
    ~TaskLoop();

    void run();
    bool running();
    void stop();
    void waitUntilRunning();

    void dispatch(folly::Func fun);

private:
    folly::EventBase eventBase_;
};

} // namespace RnsShell
