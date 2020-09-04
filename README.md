# react-native-skia
React Native renderer based on [Skia](https://skia.org/)

![Screenshot](https://pbs.twimg.com/media/Eey1WFdUMAE5qMF.png:small)

## Project State
The project is still in proof of concept and requires lots of work.

This board records TODO items for the project to production level

https://github.com/Kudo/react-native-skia/projects/1

If you have interesting for this project, please JOIN US to make it come true!

### Patches from React Native upstream

Although the project currently is a react-native fork, the change from upstream is less, all in this [commit](https://github.com/Kudo/react-native-skia/commit/d870ec0f98ad2b4cfa20e5cfda0f0ce15d175b5c).
Besides, the new commits for this project are rebased from upstream.
So all new commits should be listed as:

`git log $(git merge-base origin/upstream master)..HEAD`

## Supported platforms

Only macOS is supported in the mean time, but the most implementation is cross platform.
It should be easy to extend for other Skia supported platforms.

### macOS setup instructions

#### Prerequisites

- macOS
- [buck](https://buck.build/)

#### Build instructions

1. Git clone this repository and sync node modules by `yarn`

2. Download prebuilt libraries from
https://github.com/Kudo/react-native-skia/releases/download/bin/third-party-prebuilt-lib-macosx.tar.xz

```shell
cd third-party-oss
tar xvf third-party-prebuilt-lib-macosx.tar.xz
```

3. Build

```shell
mkdir build
node cli.js bundle --platform ios --dev false --entry-file ReactSkia/SimpleViewApp.js --bundle-output build/SimpleViewApp.bundle --assets-dest build
buck build //ReactSkia:ReactSkiaApp --out build/ReactSkia
```

4. Run

```shell
cd build
./ReactSkia
```
