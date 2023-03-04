gclient_gn_args_file = 'src/build/config/gclient_args.gni'
gclient_gn_args = [
  'build_with_chromium',
  'checkout_android',
  'checkout_android_prebuilts_build_tools',
  'checkout_android_native_support',
  'checkout_google_benchmark',
  'checkout_ios_webkit',
  'checkout_nacl',
  'checkout_oculus_sdk',
  'checkout_openxr',
  'mac_xcode_version',
  'generate_location_tags',
  'react_native_platform',
]

vars = {
  # Supported `react_native_platform` in ['default', 'tvos']
  'react_native_platform': 'default',
  'react_native_git_default': 'https://github.com/facebook/react-native.git',
  'react_native_revision_default': 'v0.71.3',
  'react_native_git_tvos': 'https://github.com/nagra-opentv/react-native-tvos.git',
  'react_native_revision_tvos': 'tvos-v0.64.2',

  'skia_revision': 'chrome/m108',
  'libcurl_revision': 'curl-7_77_0',

  # Note this revision should be updated with
  # third_party/boringssl/roll_boringssl.py, not roll-dep.
  'boringssl_revision': '1ee71185a2322dc354bee5e5a0abfb1810a27dc6',

  # buildtools
  'gn_version':            'git_revision:b9c6c19be95a3863e02f00f1fe403b2502e345b6',
  'ninja_version':         'version:2@1.8.2.chromium.3',
  'clang_format_revision': '8b525d2747f2584fc35d8c7e612e66f377858df7',
  'libcxx_revision':       '64d36e572d3f9719c5d75011a718f33f11126851',
  'libcxxabi_revision':    '9572e56a12c88c011d504a707ca94952be4664f9',
  'libunwind_revision':    '1111799723f6a003e6f52202b9bf84387c552081',

  'chromium_git': 'https://chromium.googlesource.com',
  'skia_git': 'https://skia.googlesource.com',

  'build_with_chromium': False,
  'checkout_android': False,
  'checkout_android_prebuilts_build_tools': False,
  'checkout_android_native_support': 'checkout_android or checkout_chromeos',
  'checkout_google_benchmark': False,
  'checkout_ios_webkit': False,
  'checkout_nacl': False,
  'checkout_oculus_sdk' : False,
  'checkout_openxr' : False,
  'mac_xcode_version': 'default',
  'generate_location_tags': False,
  'llvm_force_head_revision': False,
}

deps = {
  'src/react-native'                      : Var('react_native_git_' + Var('react_native_platform')) + '@' + Var('react_native_revision_' + Var('react_native_platform')),

  'src/folly'                             : 'https://github.com/Kudo/folly.git' + '@' + 'b8f14802ab5c10fad30bfc952b3243fb932ea34a',

  'src/third_party/boost'                 : 'https://github.com/Kudo/boost-for-react-native.git' + '@' + '31c7a87aa429d74758dc0ecf3cfd03b44107da28',
  'src/third_party/glog'                  : 'https://github.com/google/glog.git' + '@' + 'v0.3.5',
  'src/third_party/double-conversion'     : 'https://github.com/google/double-conversion.git' + '@' + 'v1.1.6',
  'src/third_party/fmt'                   : 'https://github.com/fmtlib/fmt.git' + '@' + '9.1.0',

  # skia
  'src/third_party/skia'                  : Var('skia_git') + '/skia.git' + '@' + Var('skia_revision'),
  'src/third_party/angle'                 : Var('chromium_git') + '/angle/angle.git' + '@' + '745e071241ab4d7dede3019e8bb374ed7e64ed79',
  'src/third_party/freetype/src'          : Var('chromium_git') + '/chromium/src/third_party/freetype2.git' + '@' + '0b62c1e43dc4b0e3c50662aac757e4f7321e5466',
  'src/third_party/harfbuzz-ng/src'       : Var('chromium_git') + '/external/github.com/harfbuzz/harfbuzz.git' + '@' + '56c467093598ec559a7148b61e112e9de52b7076',
  'src/third_party/icu'                   : Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'da07448619763d1cde255b361324242646f5b268',
  'src/third_party/libjpeg-turbo'         : Var('chromium_git') + '/chromium/deps/libjpeg_turbo.git' + '@' + 'ed683925e4897a84b3bffc5c1414c85b97a129a3',
  'src/third_party/nasm'                  : Var('chromium_git') + '/chromium/deps/nasm.git' + '@' + '9215e8e1d0fe474ffd3e16c1a07a0f97089e6224',
  'src/third_party/libwebp/src'           : Var('chromium_git') + '/webm/libwebp.git' + '@' + '7366f7f394af26de814296152c50e673ed0a832f',
  'src/third_party/wuffs'                 : Var('skia_git') + '/external/github.com/google/wuffs-mirror-release-c.git' + '@' + 'a8205c2fe7564b12fea81ee028ba670112cc7719',
  'src/third_party/swiftshader'           : 'https://swiftshader.googlesource.com/SwiftShader.git' + '@' + 'b22b1b1f2dddcf5eacc8d2a37e7d27f650e1c1e2',
  'src/third_party/expat/src'           : 'https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git' + '@' + '53fbdf5b8925a426e1b41a9e09b833986b87524e',

  # boringssl
  'src/third_party/boringssl/src'         : 'https://boringssl.googlesource.com/boringssl.git' + '@' + Var('boringssl_revision'),
  # libcurl
  'src/third_party/libcurl' : 'https://github.com/curl/curl.git' + '@' + Var('libcurl_revision'),  

  # buildtools
  'src/third_party/depot_tools': Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' + '2c0a8c736a59044e4acc7be9e172343adc5c4310',
  'src/buildtools/clang_format/script':
    Var('chromium_git') + '/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@' +
    Var('clang_format_revision'),
  'src/buildtools/linux64': {
    'packages': [
      {
        'package': 'gn/gn/linux-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux"',
  },
  'src/buildtools/mac': {
    'packages': [
      {
        'package': 'gn/gn/mac-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac"',
  },
  'src/third_party/ninja': {
    'packages': [
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': Var('ninja_version'),
      }
    ],
    'dep_type': 'cipd',
  },
  'src/buildtools/third_party/libc++/trunk':
    Var('chromium_git') +
    '/external/github.com/llvm/llvm-project/libcxx.git' + '@' +
    Var('libcxx_revision'),
  'src/buildtools/third_party/libc++abi/trunk':
    Var('chromium_git') +
    '/external/github.com/llvm/llvm-project/libcxxabi.git' + '@' +
    Var('libcxxabi_revision'),
  'src/buildtools/third_party/libunwind/trunk':
    Var('chromium_git') +
    '/external/github.com/llvm/llvm-project/libunwind.git' + '@' +
    Var('libunwind_revision'),
  'src/buildtools/win': {
    'packages': [
      {
        'package': 'gn/gn/windows-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win"',
  },
}

hooks = [
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python3', 'src/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac or checkout_ios',
    'action': ['python3', 'src/build/mac_toolchain.py'],
  },
  {
    # Update the prebuilt clang toolchain.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'condition': 'not llvm_force_head_revision',
    'action': ['python3', 'src/tools/clang/scripts/update.py'],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'python3',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac_x64',
    'pattern': '.',
    'condition': 'host_os == "mac" and host_cpu == "x64"',
    'action': [ 'python3',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.x64.sha1',
                '-o', 'src/buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_mac_arm64',
    'pattern': '.',
    'condition': 'host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'python3',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.arm64.sha1',
                '-o', 'src/buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'python3',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/linux64/clang-format.sha1',
    ],
  },
]

recursedeps = [
  # ANGLE manages DEPS that it also owns the build files for, such as dEQP.
  # "src/third_party/angle",
]
