/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

namespace RnsShell {

class Application {
public:
    static Application* Create(int argc, char** argv);
    virtual ~Application() {}

    virtual void onResize(int width, int height) = 0;
};

}   // namespace RnsShell
