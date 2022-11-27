/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <libgen.h>
#include <string>
#include <iostream>
#include <thread>

#include "ReactSkia/utils/RnsUtils.h"

#if PLATFORM(X11)
#include <X11/Xlib.h>
#elif PLATFORM(LIBWPE)
#include <glib.h>
#include <wpe/wpe.h>
#endif

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"

#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/utils/RnsLog.h"

#include "rns_shell/common/Application.h"
#include "rns_shell/common/Window.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/linux/TaskLoop.h"

using namespace RnsShell;

static bool platformInitialize(char **argv) {
    bool status = false;

    TaskLoop::initializeMain();
    NotificationCenter::initializeDefault();
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
    NotificationCenter::initializeSubWindowCenter();//Intializing Notification center for Events from subWindows
#endif/*FEATURE_ONSCREEN_KEYBOARD*/
#if PLATFORM(LIBWPE) || USE(WEP_RENDERER)
    wpe_renderer_host_create_client(); // Has to be called before wpe_loader_init
    wpe_loader_init(wpe_loader_get_loaded_implementation_library_name());
#endif

    // Google Logging
    {
        FLAGS_minloglevel = 0;
        FLAGS_logtostderr = 1; // When set to 1 all logs printed to console instead of file
        FLAGS_colorlogtostderr = 1;
        FLAGS_v = 0; // 1 for DEBUG, 2 for TRACE

        // Applicable only when FLAGS_logtostderr is set to 0
        FLAGS_stderrthreshold = 0; // Log level matching this and above this will be also printed on stderr.
        FLAGS_log_dir = "/tmp/glog";
        google::InitGoogleLogging(argv[0]);
    }

    // Js bundle path papssed as first argument
    if(argv[1]) {
        char bundlePath[PATH_MAX];
        if(realpath(argv[1], bundlePath) == nullptr) {
            RNS_LOG_FATAL("Failed to load argument path " << argv[1]);
            return EXIT_FAILURE;
        }
        std::string filename = basename(bundlePath);
        std::string dirPath = dirname(bundlePath); // Note that `dirname()` will mutate `bundlePath`
        RNS_LOG_INFO("Load " << filename << ", from " << dirPath);
        chdir(dirPath.c_str()); // Change current directory to app directory
    }
    status = PlatformDisplay::initialize();

    return status;
}

static void platformFinalize() {
    RNS_LOG_NOT_IMPL;
    google::ShutdownGoogleLogging();
}

int main(int argc, char**argv) {

    if(false == platformInitialize(argv)) {
        RNS_LOG_FATAL("Platform Initialize Failed");
        return EXIT_FAILURE;
    }

    Application* app = Application::Create(argc, argv);
    if(app == nullptr) {
        RNS_LOG_FATAL("Couldnt Create Application");
        return EXIT_FAILURE;
    }
    std::thread eventThread(RnsShell::Window::createEventLoop, app);

    // Run main thread task runner
    TaskLoop::main().run();

    delete app;
    platformFinalize();

    eventThread.join();

    return EXIT_SUCCESS;
}
