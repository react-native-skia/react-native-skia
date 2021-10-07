# react-native-skia
React Native renderer based on [Skia](https://skia.org/)

![Screenshot](https://pbs.twimg.com/media/Eey1WFdUMAE5qMF.png:small)

## Project State
The project is still in proof of concept and requires lots of work.

This board records TODO items for the project to production level

https://github.com/react-native-skia/react-native-skia/projects/1

If you have interesting for this project, please JOIN US to make it come true!

## Supported platforms

Initial versions support both macOS and Linux.But now we are focusing only Linux (Ubuntu 18) to speed up development.But most implementation is cross platform.
It should be easy to extend for other Skia supported platforms.

### Setup instructions

Setup instruction is common for both macOS and Linux except at step 2, where different arguments will be used for gn configuration.

#### Prerequisites

- Install Chromium [depot_tools](https://chromium.googlesource.com/chromium/src/+/master/docs/mac_build_instructions.md#install)

```shell
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
$ export PATH="$PATH:/path/to/depot_tools"
```

#### Build instructions

1. Get the code by gclient

```shell
$ mkdir react-native-skia && cd react-native-skia
$ gclient config --name src --unmanaged https://github.com/react-native-skia/react-native-skia.git@main
$ gclient sync --with_branch_heads

# We will use react-native-skia/src as working directory in the future steps.
$ cd /path/to/react-native-skia/src
```

2. Generate build files

```shell
$ gn gen --args='mac_deployment_target="10.11.0" mac_min_system_version="10.11.0"' out/Debug

# Or release build
# $ gn gen --args='mac_deployment_target="10.11.0" mac_min_system_version="10.11.0" is_debug=false' out/Release

# For Linux (Ubuntu 18)
$ gn gen --args='use_sysroot=false enable_vulkan=false icu_use_data_file=false gl_use_glx=false is_debug=false is_component_build=true' out/Debug
```

3. Sync JavaScript packages

```shell
$ yarn
```

4. Build the code

```shell
$ ninja -C out/Debug ReactSkiaApp

# Build react-native JavaScript bundle
$ yarn workspace react-native-skia run react-native bundle \
    --platform ios \
    --dev false \
    --entry-file SimpleViewApp.js \
    --bundle-output ../../out/Debug/SimpleViewApp.bundle \
    --assets-dest ../../out/Debug
```

5. Run the program

```shell
$ cd out/Debug && ./ReactSkiaApp
```

### Development flow

#### Sync & build latest code

```shell
$ git pull --rebase
$ gclient sync --with_branch_heads

# Sync node modules
$ yarn

# Incremental build C++ code
$ ninja -C out/Debug ReactSkiaApp
```

#### Third-party code sync

We use gclient and [DEPS file](https://github.com/react-native-skia/react-native-skia/blob/main/DEPS) to manage third party code, including react-native.

Once updating the git revision in DEPS, `gclient sync` could sync the local files accordingly.
The change of DEPS could commit into git for others to have the update.

### Third-party code patches

Here to list my patches for third-party code. The list should be as less as possible.

1. react-native patch for cxx textlayoutmanager build break.
https://github.com/Kudo/react-native/commit/b11c10371dec70ae6607286f23581585a401da09
(Will send a PR and hope to be fixed by upstream, then we don't have custom patch for react-native anymore)

2. react-native patch for fix build break for gcc.
https://github.com/Kudo/react-native/commit/deb66012fe550d536420931e47f5573e798e048a

3. folly to support boringssl
https://github.com/Kudo/folly/commit/2b9b7144fdafcc7c78d09465449f1c3a72386ac5

## Credits

Check [the list of contributors here](https://github.com/Kudo/react-native-skia/graphs/contributors). This project is supported by:


[<img src="https://user-images.githubusercontent.com/46429/113642371-3d328c00-96b2-11eb-9feb-550d003ca7b0.png" width="200" alt="NAGRA">](https://www.nagra.com/)
[NAGRA](https://www.nagra.com/)
