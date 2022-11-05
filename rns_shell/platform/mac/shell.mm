/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <libgen.h>
#include <filesystem>
#include <string>
#include <thread>

#import <AppKit/AppKit.h>

#include <boost/filesystem.hpp>
#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "rns_shell/common/Application.h"
#include "rns_shell/common/Window.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/mac/AppDelegate.h"
#include "rns_shell/platform/mac/TaskLoop.h"

using namespace RnsShell;
namespace fs = boost::filesystem;

static bool platformInitialize(char **argv)
{
  bool status = false;

  TaskLoop::initializeMain();
  NotificationCenter::initializeDefault();
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  NotificationCenter::initializeSubWindowCenter(); // Intializing Notification center for Events from subWindows
#endif // ENABLE(FEATURE_ONSCREEN_KEYBOARD)

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
  if (argv[1]) {
    char bundlePath[PATH_MAX];
    if (realpath(argv[1], bundlePath) == nullptr) {
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

static void platformFinalize()
{
  RNS_LOG_NOT_IMPL;
  google::ShutdownGoogleLogging();
}

int main(int argc, char **argv)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];

  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Create the application menu.
  NSMenu *menuBar = [[NSMenu alloc] initWithTitle:@"AMainMenu"];
  [NSApp setMainMenu:menuBar];

  NSMenuItem *item;
  NSMenu *subMenu;

  item = [[NSMenuItem alloc] initWithTitle:@"Apple" action:nil keyEquivalent:@""];
  [menuBar addItem:item];
  subMenu = [[NSMenu alloc] initWithTitle:@"Apple"];
  [menuBar setSubmenu:subMenu forItem:item];
  [item release];
  item = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
  [subMenu addItem:item];
  [item release];
  [subMenu release];

  // Set AppDelegate to catch certain global events
  AppDelegate *appDelegate = [[AppDelegate alloc] init];
  [NSApp setDelegate:appDelegate];

  if (!platformInitialize(argv)) {
    RNS_LOG_FATAL("Platform Initialize Failed");
    return EXIT_FAILURE;
  }

  Application *app = Application::Create(argc, argv);
  if (app == nullptr) {
    RNS_LOG_FATAL("Couldnt Create Application");
    return EXIT_FAILURE;
  }
  // TODO:
  // std::thread eventThread(RnsShell::Window::createEventLoop, app);

  // Run main thread task runner
  TaskLoop::main().run();

  // This will run until the application finishes launching, then lets us take over
  [NSApp run];

  delete app;

  [NSApp setDelegate:nil];
  [appDelegate release];

  [menuBar release];
  [pool release];

  platformFinalize();

  // TODO:
  // eventThread.join();

  return EXIT_SUCCESS;
}
