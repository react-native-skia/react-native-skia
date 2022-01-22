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
  'react_native_default_revision': 'deb66012fe550d536420931e47f5573e798e048a',
  'react_native_tvos_revision': 'tvos-v0.64.2',
  'skia_revision': 'chrome/m86',

  # Note this revision should be updated with
  # third_party/boringssl/roll_boringssl.py, not roll-dep.
  'boringssl_revision': '430a7423039682e4bbc7b522e3b57b2c8dca5e3b',
  'libcurl_revision': 'curl-7_77_0',

  # buildtools
  'gn_version': 'git_revision:90294ccdcf9334ed25a76ac9b67689468e506342',
  'clang_format_revision': '99876cacf78329e5f99c244dbe42ccd1654517a0',
  'libcxx_revision':       '79a2e924d96e2fc1e4b937c42efd08898fa472d7',
  'libcxxabi_revision':    '4c6e0991b109638204f08c93600b008c21f01da5',
  'libunwind_revision':    '99015718c37b30d44c3bcbcc92a03fb85fb85a99',

  'react_native_default_git': 'https://github.com/Kudo/react-native.git',
  'react_native_tvos_git': 'https://github.com/nagra-opentv/react-native-tvos.git',
  'chromium_git': 'https://chromium.googlesource.com',
  'skia_git': 'https://skia.googlesource.com',
  'fuchsia_git': 'https://fuchsia.googlesource.com',

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

  'react_native_platform':'tvos',
  'react_native_platform_git':'react_native_tvos_git',
  'react_native_platform_revision':'react_native_tvos_revision',
}

deps = {
  'src/react-native'                      : Var(Var('react_native_platform_git')) + '@' + Var(Var('react_native_platform_revision')),

  # 'src/folly'                             : 'https://github.com/facebook/folly.git' + '@' + 'v2020.01.13.00',
  # folly custom patch to support boringssl, might not be necessary after we replace folly/async with flutter/fml
  'src/folly'                             : 'https://github.com/Kudo/folly.git' + '@' + '2b9b7144fdafcc7c78d09465449f1c3a72386ac5',

  'src/third_party/boost'                 : 'https://github.com/react-native-community/boost-for-react-native.git' + '@' + 'v1.63.0-0',
  'src/third_party/glog'                  : 'https://github.com/google/glog.git' + '@' + 'v0.3.5',
  'src/third_party/double-conversion'     : 'https://github.com/google/double-conversion.git' + '@' + 'v1.1.6',

  # flutter
  # 'src/flutter'                           : Var('chromium_git') + '/external/github.com/flutter/engine' + '@' + 'd1bc06f032f9d6c148ea6b96b48261d6f545004f',
  'src/third_party/rapidjson'             : Var('fuchsia_git') + '/third_party/rapidjson' + '@' + 'ef3564c5c8824989393b87df25355baf35ff544b',

  # skia
  'src/third_party/skia'                  : Var('skia_git') + '/skia.git' + '@' + Var('skia_revision'),
  'src/third_party/angle'                 : Var('chromium_git') + '/angle/angle.git' + '@' + '745e071241ab4d7dede3019e8bb374ed7e64ed79',
  'src/third_party/freetype/src'          : Var('chromium_git') + '/chromium/src/third_party/freetype2.git' + '@' + '9ed5332fe632ada7de8fd345c9ef579b678c4042',
  'src/third_party/harfbuzz/src'          : Var('chromium_git') + '/external/github.com/harfbuzz/harfbuzz.git' + '@' + '9bd7ba5019381bb8584811ee63a0ba0cee7dca99',
  'src/third_party/icu'                   : Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'dbd3825b31041d782c5b504c59dcfb5ac7dda08c',
  'src/third_party/libjpeg-turbo'         : Var('chromium_git') + '/chromium/deps/libjpeg_turbo.git' + '@' + '64fc43d52351ed52143208ce6a656c03db56462b',
  'src/third_party/nasm'                  : Var('chromium_git') + '/chromium/deps/nasm.git' + '@' + '4fa54ca5f7fc3a15a8c78ac94688e64d3e4e4fa1',
  'src/third_party/libpng'                : Var('skia_git') + '/third_party/libpng.git' + '@' + '386707c6d19b974ca2e3db7f5c61873813c6fe44',
  'src/third_party/libwebp'               : Var('chromium_git') + '/webm/libwebp.git' + '@' + '0fe1a89dbf1930fc2554dbe76adad5d962054ead',
  'src/third_party/wuffs'                 : Var('skia_git') + '/external/github.com/google/wuffs.git' + '@' + '00cc8a50aa0c86b6bcb37e9ece8fb100047cc17b',
  'src/third_party/zlib'                  : Var('chromium_git') + '/chromium/src/third_party/zlib.git' + '@' + 'eaf99a4e2009b0e5759e6070ad1760ac1dd75461',
  'src/third_party/swiftshader'           : 'https://swiftshader.googlesource.com/SwiftShader.git' + '@' + 'cb58662302c8e73845116fc5f14acaa08c037cb2',
  'src/third_party/expat/src'           : 'https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git' + '@' + 'e976867fb57a0cd87e3b0fe05d59e0ed63c6febb',

  # boringssl
  'src/third_party/boringssl/src'         : 'https://boringssl.googlesource.com/boringssl.git' + '@' + Var('boringssl_revision'),
  # libcurl
  'src/third_party/libcurl' : 'https://github.com/curl/curl.git' + '@' + Var('libcurl_revision'),  

  # buildtools
  'src/third_party/depot_tools': Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' + 'a91f399a8a1f3199cb1f91b19bd2c3b88bb42d6c',
  'src/buildtools/clang_format/script':
    Var('chromium_git') + '/chromium/llvm-project/cfe/tools/clang-format.git@' +
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
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'python3',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.sha1',
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
