# react-native-skia

React Native renderer based on [Skia](https://skia.org/)

![Screenshot](https://pbs.twimg.com/media/Eey1WFdUMAE5qMF.png:small)

## Project State

The project is still in proof of concept and requires lots of work.

This board records TODO items for the project to production level

https://github.com/react-native-skia/react-native-skia

If you have interesting for this project, please JOIN US to make it come true!

## Supported platforms

Only Linux (Ubuntu 18) is supported in the mean time, but the most implementation is cross platform.
macOS support is available on the initial version v0.1.0.
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

$ gclient config --name src --unmanaged https://github.com/react-native-skia/react-native-skia.git
# Or for tvos platform
# $ gclient config --name src --unmanaged https://github.com/react-native-skia/react-native-skia.git --custom-var="react_native_platform='tvos'"

$ gclient sync --with_branch_heads

# We will use react-native-skia/src as working directory in the future steps.
$ cd /path/to/react-native-skia/src
```

2. Generate build files

```shell
# For macOS
$ gn gen out/Debug
# Or for macOS native arm64 build
# $ gn gen --args='target_cpu = "arm64"' out/Debug
# Or release build
# $ gn gen --args='is_debug=false' out/Release

# For Linux (Ubuntu 18)
$ gn gen --args='use_sysroot=false enable_vulkan=false icu_use_data_file=false skia_use_system_nopoll=true gl_use_glx=false is_debug=false is_component_build=true enable_precompiled_headers=true' out/Debug
    Usage of gen args -
    use_sysroot=false : Default is true, set to false to use system sysroot
    enable_vulkan=false  : Default is true , set to false since we are using opengl
    icu_use_data_file=false : Default is true , set to false to include icu data file as binary instead of loadable icu data file
    skia_use_system_nopoll=true  : Default is false , set to true to use system nopoll package
    gl_use_glx=false  : Default is true , set to false to use egl
    gl_has_gpu=false  : Default is true , set to false to build on NON-GPU platform or to verify software rendering on desktop.
    is_debug=false : Default is true , set to false to build with optimizations
    is_component_build=true : Default is "is_debug" value , set to true to build components as shared libraries
    enable_precompiled_headers : Default is false on Linux, set to true to fix the react-native build errors
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
$ cd out/Debug && ./ReactSkia
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

- folly to support boringssl
- boost-for-react-native update to boost 1.80.0 and add context libs

## Credits

Check [the list of contributors here](https://github.com/react-native/react-native-skia/graphs/contributors). This project is supported by:

[<img src="https://user-images.githubusercontent.com/46429/113642371-3d328c00-96b2-11eb-9feb-550d003ca7b0.png" width="200" alt="NAGRA">](https://www.nagra.com/)
[NAGRA](https://www.nagra.com/)
