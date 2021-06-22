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
]

vars = {
  'react_native_revision': 'e599d6c5d338c1b4d1a0d988e0d9ff83c179fb54',
  'react_native_tvos_revision': 'tvos-v0.64.2',

  'skia_revision': 'chrome/m86',

  # Note this revision should be updated with
  # third_party/boringssl/roll_boringssl.py, not roll-dep.
  'boringssl_revision': '430a7423039682e4bbc7b522e3b57b2c8dca5e3b',

  # buildtools
  'gn_version': 'git_revision:7d7e8deea36d126397bda2cf924682504271f0e1',
  'clang_format_revision': '96636aa0e9f047f17447f2d45a094d0b59ed7917',
  'libcxx_revision':       'd9040c75cfea5928c804ab7c235fed06a63f743a',
  'libcxxabi_revision':    '196ba1aaa8ac285d94f4ea8d9836390a45360533',
  'libunwind_revision':    'd999d54f4bca789543a2eb6c995af2d9b5a1f3ed',

  'react_native_git': 'https://github.com/facebook/react-native.git',
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
}

deps = {
  # 'src/react-native'                      : Var('react_native_git') + '@' + Var('react_native_revision'),
  # react-native patch to fix cxx textlayoutmanager build break
  #'src/react-native'                      : 'https://github.com/Kudo/react-native.git' + '@' + 'deb66012fe550d536420931e47f5573e798e048a',
  # react-native-tvos + patch to fix gcc compilation build break
  'src/react-native'                      : 'https://github.com/nagra-opentv/react-native-tvos.git' + '@' + Var('react_native_tvos_revision'),
  
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
  'src/third_party/freetype/src'          : Var('chromium_git') + '/chromium/src/third_party/freetype2.git' + '@' + 'a4434747558d872c55e55ce428019a8e15d222dc',
  'src/third_party/harfbuzz/src'          : Var('chromium_git') + '/external/github.com/harfbuzz/harfbuzz.git' + '@' + 'e3af529e511ca492284cdd9f4584666b88a9e00f',
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

  # buildtools
  'src/third_party/depot_tools': Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' + '0bfbd890c3e2f3aa734119507d14162248409664',
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
    'action': ['python', 'src/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python', 'src/build/mac_toolchain.py',
               '--xcode-version', Var('mac_xcode_version')],
  },
  {
    # Update the prebuilt clang toolchain.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py'],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'python',
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
    'action': [ 'python',
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
    'action': [ 'python',
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
