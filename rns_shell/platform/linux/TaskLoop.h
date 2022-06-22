/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <folly/io/async/EventBase.h>

namespace RnsShell {

using namespace folly;

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

    void dispatch(Func fun);

private:
    EventBase eventBase_;
};

} // namespace RnsShell
