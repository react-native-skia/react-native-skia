## React Native third party files for OSS

### Versioning

- **boost**

```shell
git clone https://github.com/react-native-community/boost-for-react-native.git --depth 1 --branch v1.63.0-0 --single-branch boost
rm -rf boost/.git
```

- **glog**

```shell
git clone https://github.com/google/glog.git --depth 1 --branch v0.3.5 --single-branch
rm -rf glog/.git
```

- **double-conversion**

```shell
git clone https://github.com/google/double-conversion.git --depth 1 --branch v1.1.6 --single-branch
rm -rf double-conversion/.git
```

- **folly**

```shell
git clone https://github.com/facebook/folly.git --depth 1 --branch v2020.01.13.00 --single-branch
rm -rf folly/.git
```

- **openssl** (with prebuilt libraries)

```shell
 git clone https://github.com/openssl/openssl.git --depth 1 --branch OpenSSL_1_1_1g --single-branch openssl-src
 cd openssl-src
 ./Configure darwin64-x86_64-cc --prefix=/tmp/openssl --openssldir=/tmp/openssl
 make all install
 mv /tmp/openssl /path/to/third-party-oss/openssl
```

- **libevent**

Manually copied from `https://chromium.googlesource.com/chromium/src.git/+/refs/heads/master/base/third_party/libevent/`

- **skia** (with prebuilt libraries)

Refer source code & build instructions at [official doc](https://skia.org/user/build)

Version branch: `chrome/m83`

Patch to make sk_app exportable

```patch
diff --git a/BUILD.gn b/BUILD.gn
index bf15adf2cd..9dbde862e2 100644
--- a/BUILD.gn
+++ b/BUILD.gn
@@ -2099,7 +2099,8 @@ if (skia_enable_tools) {
     ]
   }

-  test_lib("sk_app") {
+  component("sk_app") {
+    testonly = true
     public_deps = [
       ":gpu_tool_utils",
       ":skia",
```

```shell
gn gen out/Release  --args='is_debug=false'
ninja -C out/Release
```

Manually copied `include`, `sk_app`, `skui` directories.

`cp out/Release/lib*.a /path/to/third-party-oss/skia/lib`
