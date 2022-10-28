#!/usr/bin/env python3
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script is used to build clang binaries. It is used by package.py to
create the prebuilt binaries downloaded by update.py and used by developers.

The expectation is that update.py downloads prebuilt binaries for everyone, and
nobody should run this script as part of normal development.
"""

from __future__ import print_function

import argparse
import glob
import io
import json
import os
import pipes
import platform
import re
import shutil
import subprocess
import sys

from update import (CDS_URL, CHROMIUM_DIR, CLANG_REVISION, LLVM_BUILD_DIR,
                    FORCE_HEAD_REVISION_FILE, PACKAGE_VERSION, RELEASE_VERSION,
                    STAMP_FILE, DownloadUrl, DownloadAndUnpack, EnsureDirExists,
                    ReadStampFile, RmTree, WriteStampFile)

# Path constants. (All of these should be absolute paths.)
THIRD_PARTY_DIR = os.path.join(CHROMIUM_DIR, 'third_party')
LLVM_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm')
COMPILER_RT_DIR = os.path.join(LLVM_DIR, 'compiler-rt')
LLVM_BOOTSTRAP_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm-bootstrap')
LLVM_BOOTSTRAP_INSTALL_DIR = os.path.join(THIRD_PARTY_DIR,
                                          'llvm-bootstrap-install')
LLVM_INSTRUMENTED_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm-instrumented')
LLVM_PROFDATA_FILE = os.path.join(LLVM_INSTRUMENTED_DIR, 'profdata.prof')
LLVM_BUILD_TOOLS_DIR = os.path.abspath(
    os.path.join(LLVM_DIR, '..', 'llvm-build-tools'))
ANDROID_NDK_DIR = os.path.join(
    CHROMIUM_DIR, 'third_party', 'android_ndk')
FUCHSIA_SDK_DIR = os.path.join(CHROMIUM_DIR, 'third_party', 'fuchsia-sdk',
                               'sdk')

BUG_REPORT_URL = ('https://crbug.com and run'
                  ' tools/clang/scripts/process_crashreports.py'
                  ' (only works inside Google) which will upload a report')


win_sdk_dir = None
def GetWinSDKDir():
  """Get the location of the current SDK."""
  global win_sdk_dir
  if win_sdk_dir:
    return win_sdk_dir

  # Don't let vs_toolchain overwrite our environment.
  environ_bak = os.environ

  sys.path.append(os.path.join(CHROMIUM_DIR, 'build'))
  import vs_toolchain
  win_sdk_dir = vs_toolchain.SetEnvironmentAndGetSDKDir()
  msvs_version = vs_toolchain.GetVisualStudioVersion()

  if bool(int(os.environ.get('DEPOT_TOOLS_WIN_TOOLCHAIN', '1'))):
    dia_path = os.path.join(win_sdk_dir, '..', 'DIA SDK', 'bin', 'amd64')
  else:
    if 'GYP_MSVS_OVERRIDE_PATH' not in os.environ:
      vs_path = vs_toolchain.DetectVisualStudioPath()
    else:
      vs_path = os.environ['GYP_MSVS_OVERRIDE_PATH']
    dia_path = os.path.join(vs_path, 'DIA SDK', 'bin', 'amd64')

  os.environ = environ_bak
  return win_sdk_dir


def RunCommand(command, msvc_arch=None, env=None, fail_hard=True):
  """Run command and return success (True) or failure; or if fail_hard is
     True, exit on failure.  If msvc_arch is set, runs the command in a
     shell with the msvc tools for that architecture."""

  if msvc_arch and sys.platform == 'win32':
    command = [os.path.join(GetWinSDKDir(), 'bin', 'SetEnv.cmd'),
               "/" + msvc_arch, '&&'] + command

  # https://docs.python.org/2/library/subprocess.html:
  # "On Unix with shell=True [...] if args is a sequence, the first item
  # specifies the command string, and any additional items will be treated as
  # additional arguments to the shell itself.  That is to say, Popen does the
  # equivalent of:
  #   Popen(['/bin/sh', '-c', args[0], args[1], ...])"
  #
  # We want to pass additional arguments to command[0], not to the shell,
  # so manually join everything into a single string.
  # Annoyingly, for "svn co url c:\path", pipes.quote() thinks that it should
  # quote c:\path but svn can't handle quoted paths on Windows.  Since on
  # Windows follow-on args are passed to args[0] instead of the shell, don't
  # do the single-string transformation there.
  if sys.platform != 'win32':
    command = ' '.join([pipes.quote(c) for c in command])
  print('Running', command)
  if subprocess.call(command, env=env, shell=True) == 0:
    return True
  print('Failed.')
  if fail_hard:
    sys.exit(1)
  return False


def CopyFile(src, dst):
  """Copy a file from src to dst."""
  print("Copying %s to %s" % (src, dst))
  shutil.copy(src, dst)


def CopyDirectoryContents(src, dst):
  """Copy the files from directory src to dst."""
  dst = os.path.realpath(dst)  # realpath() in case dst ends in /..
  EnsureDirExists(dst)
  for f in os.listdir(src):
    CopyFile(os.path.join(src, f), dst)


def CheckoutLLVM(commit, dir):
  """Checkout the LLVM monorepo at a certain git commit in dir. Any local
  modifications in dir will be lost."""

  print('Checking out LLVM monorepo %s into %s' % (commit, dir))

  # Try updating the current repo if it exists and has no local diff.
  if os.path.isdir(dir):
    os.chdir(dir)
    # git diff-index --quiet returns success when there is no diff.
    # Also check that the first commit is reachable.
    if (RunCommand(['git', 'diff-index', '--quiet', 'HEAD'], fail_hard=False)
        and RunCommand(['git', 'fetch'], fail_hard=False)
        and RunCommand(['git', 'checkout', commit], fail_hard=False)):
      return

    # If we can't use the current repo, delete it.
    os.chdir(CHROMIUM_DIR)  # Can't remove dir if we're in it.
    print('Removing %s.' % dir)
    RmTree(dir)

  clone_cmd = ['git', 'clone', 'https://github.com/llvm/llvm-project/', dir]

  if RunCommand(clone_cmd, fail_hard=False):
    os.chdir(dir)
    if RunCommand(['git', 'checkout', commit], fail_hard=False):
      return

  print('CheckoutLLVM failed.')
  sys.exit(1)


def UrlOpen(url):
  # TODO(crbug.com/1067752): Use urllib once certificates are fixed.
  return subprocess.check_output(['curl', '--silent', url],
                                 universal_newlines=True)


def GetLatestLLVMCommit():
  """Get the latest commit hash in the LLVM monorepo."""
  ref = json.loads(
      UrlOpen(('https://api.github.com/repos/'
               'llvm/llvm-project/git/refs/heads/main')))
  assert ref['object']['type'] == 'commit'
  return ref['object']['sha']


def GetCommitDescription(commit):
  """Get the output of `git describe`.

  Needs to be called from inside the git repository dir."""
  git_exe = 'git.bat' if sys.platform.startswith('win') else 'git'
  return subprocess.check_output(
      [git_exe, 'describe', '--long', '--abbrev=8', commit],
      universal_newlines=True).rstrip()


def DeleteChromeToolsShim():
  # TODO: These dirs are no longer used. Remove this code after a while.
  OLD_SHIM_DIR = os.path.join(LLVM_DIR, 'tools', 'zzz-chrometools')
  shutil.rmtree(OLD_SHIM_DIR, ignore_errors=True)
  CHROME_TOOLS_SHIM_DIR = os.path.join(LLVM_DIR, 'llvm', 'tools', 'chrometools')
  shutil.rmtree(CHROME_TOOLS_SHIM_DIR, ignore_errors=True)


def AddCMakeToPath(args):
  """Download CMake and add it to PATH."""
  if args.use_system_cmake:
    return

  if sys.platform == 'win32':
    zip_name = 'cmake-3.17.1-win64-x64.zip'
    dir_name = ['cmake-3.17.1-win64-x64', 'bin']
  elif sys.platform == 'darwin':
    if platform.machine() == 'arm64':
      # TODO(thakis): Move to 3.20 everywhere.
      zip_name = 'cmake-3.20.0-macos-universal.tar.gz'
      dir_name = [
          'cmake-3.20.0-macos-universal', 'CMake.app', 'Contents', 'bin'
      ]
    else:
      zip_name = 'cmake-3.17.1-Darwin-x86_64.tar.gz'
      dir_name = ['cmake-3.17.1-Darwin-x86_64', 'CMake.app', 'Contents', 'bin']
  else:
    zip_name = 'cmake-3.17.1-Linux-x86_64.tar.gz'
    dir_name = ['cmake-3.17.1-Linux-x86_64', 'bin']

  cmake_dir = os.path.join(LLVM_BUILD_TOOLS_DIR, *dir_name)
  if not os.path.exists(cmake_dir):
    DownloadAndUnpack(CDS_URL + '/tools/' + zip_name, LLVM_BUILD_TOOLS_DIR)
  os.environ['PATH'] = cmake_dir + os.pathsep + os.environ.get('PATH', '')


def AddGnuWinToPath():
  """Download some GNU win tools and add them to PATH."""
  if sys.platform != 'win32':
    return

  gnuwin_dir = os.path.join(LLVM_BUILD_TOOLS_DIR, 'gnuwin')
  GNUWIN_VERSION = '14'
  GNUWIN_STAMP = os.path.join(gnuwin_dir, 'stamp')
  if ReadStampFile(GNUWIN_STAMP) == GNUWIN_VERSION:
    print('GNU Win tools already up to date.')
  else:
    zip_name = 'gnuwin-%s.zip' % GNUWIN_VERSION
    DownloadAndUnpack(CDS_URL + '/tools/' + zip_name, LLVM_BUILD_TOOLS_DIR)
    WriteStampFile(GNUWIN_VERSION, GNUWIN_STAMP)

  os.environ['PATH'] = gnuwin_dir + os.pathsep + os.environ.get('PATH', '')

  # find.exe, mv.exe and rm.exe are from MSYS (see crrev.com/389632). MSYS uses
  # Cygwin under the hood, and initializing Cygwin has a race-condition when
  # getting group and user data from the Active Directory is slow. To work
  # around this, use a horrible hack telling it not to do that.
  # See https://crbug.com/905289
  etc = os.path.join(gnuwin_dir, '..', '..', 'etc')
  EnsureDirExists(etc)
  with open(os.path.join(etc, 'nsswitch.conf'), 'w') as f:
    f.write('passwd: files\n')
    f.write('group: files\n')


def AddZlibToPath():
  """Download and build zlib, and add to PATH."""
  zlib_dir = os.path.join(LLVM_BUILD_TOOLS_DIR, 'zlib-1.2.11')
  if os.path.exists(zlib_dir):
    RmTree(zlib_dir)
  zip_name = 'zlib-1.2.11.tar.gz'
  DownloadAndUnpack(CDS_URL + '/tools/' + zip_name, LLVM_BUILD_TOOLS_DIR)
  os.chdir(zlib_dir)
  zlib_files = [
      'adler32', 'compress', 'crc32', 'deflate', 'gzclose', 'gzlib', 'gzread',
      'gzwrite', 'inflate', 'infback', 'inftrees', 'inffast', 'trees',
      'uncompr', 'zutil'
  ]
  cl_flags = [
      '/nologo', '/O2', '/DZLIB_DLL', '/c', '/D_CRT_SECURE_NO_DEPRECATE',
      '/D_CRT_NONSTDC_NO_DEPRECATE'
  ]
  RunCommand(
      ['cl.exe'] + [f + '.c' for f in zlib_files] + cl_flags, msvc_arch='x64')
  RunCommand(
      ['lib.exe'] + [f + '.obj'
                     for f in zlib_files] + ['/nologo', '/out:zlib.lib'],
      msvc_arch='x64')
  # Remove the test directory so it isn't found when trying to find
  # test.exe.
  shutil.rmtree('test')

  os.environ['PATH'] = zlib_dir + os.pathsep + os.environ.get('PATH', '')
  return zlib_dir


def DownloadRPMalloc():
  """Download rpmalloc."""
  rpmalloc_dir = os.path.join(LLVM_BUILD_TOOLS_DIR, 'rpmalloc')
  if os.path.exists(rpmalloc_dir):
    RmTree(rpmalloc_dir)

  # Using rpmalloc bc1923f rather than the latest release (1.4.1) because
  # it contains the fix for https://github.com/mjansson/rpmalloc/pull/186
  # which would cause lld to deadlock.
  # The zip file was created and uploaded as follows:
  # $ mkdir rpmalloc
  # $ curl -L https://github.com/mjansson/rpmalloc/archive/bc1923f436539327707b08ef9751a7a87bdd9d2f.tar.gz \
  #     | tar -C rpmalloc --strip-components=1 -xzf -
  # $ GZIP=-9 tar vzcf rpmalloc-bc1923f.tgz rpmalloc
  # $ gsutil.py cp -n -a public-read rpmalloc-bc1923f.tgz \
  #     gs://chromium-browser-clang/tools/
  zip_name = 'rpmalloc-bc1923f.tgz'
  DownloadAndUnpack(CDS_URL + '/tools/' + zip_name, LLVM_BUILD_TOOLS_DIR)
  rpmalloc_dir = rpmalloc_dir.replace('\\', '/')
  return rpmalloc_dir


def MaybeDownloadHostGcc(args):
  """Download a modern GCC host compiler on Linux."""
  if not sys.platform.startswith('linux') or args.gcc_toolchain:
    return
  gcc_dir = os.path.join(LLVM_BUILD_TOOLS_DIR, 'gcc-10.2.0-trusty')
  if not os.path.exists(gcc_dir):
    DownloadAndUnpack(CDS_URL + '/tools/gcc-10.2.0-trusty.tgz', gcc_dir)
  args.gcc_toolchain = gcc_dir


def VerifyVersionOfBuiltClangMatchesVERSION():
  """Checks that `clang --version` outputs RELEASE_VERSION. If this
  fails, update.RELEASE_VERSION is out-of-date and needs to be updated (possibly
  in an `if args.llvm_force_head_revision:` block inupdate. main() first)."""
  clang = os.path.join(LLVM_BUILD_DIR, 'bin', 'clang')
  if sys.platform == 'win32':
    clang += '-cl.exe'
  version_out = subprocess.check_output([clang, '--version'],
                                        universal_newlines=True)
  version_out = re.match(r'clang version ([0-9.]+)', version_out).group(1)
  if version_out != RELEASE_VERSION:
    print(('unexpected clang version %s (not %s), '
           'update RELEASE_VERSION in update.py')
          % (version_out, RELEASE_VERSION))
    sys.exit(1)


def VerifyZlibSupport():
  """Check that clang was built with zlib support enabled."""
  clang = os.path.join(LLVM_BUILD_DIR, 'bin', 'clang')
  test_file = '/dev/null'
  if sys.platform == 'win32':
    clang += '.exe'
    test_file = 'nul'

  print('Checking for zlib support')
  clang_out = subprocess.check_output([
      clang, '-target', 'x86_64-unknown-linux-gnu', '-gz', '-c', '-###', '-x',
      'c', test_file
  ],
                                      stderr=subprocess.STDOUT,
                                      universal_newlines=True)
  if (re.search(r'--compress-debug-sections', clang_out)):
    print('OK')
  else:
    print(('Failed to detect zlib support!\n\n(driver output: %s)') % clang_out)
    sys.exit(1)


def CopyLibstdcpp(args, build_dir):
  if not args.gcc_toolchain:
    return
  # Find libstdc++.so.6
  libstdcpp = subprocess.check_output([
      os.path.join(args.gcc_toolchain, 'bin', 'g++'),
      '-print-file-name=libstdc++.so.6'
  ],
                                      universal_newlines=True).rstrip()

  # Copy libstdc++.so.6 into the build dir so that the built binaries can find
  # it. Binaries get their rpath set to $origin/../lib/. For clang, lld,
  # etc. that live in the bin/ directory, this means they expect to find the .so
  # in their neighbouring lib/ dir.
  # For unit tests we pass -Wl,-rpath to the linker pointing to the lib64 dir
  # in the gcc toolchain, via LLVM_LOCAL_RPATH below.
  # The two fuzzer tests are weird in that they copy the fuzzer binary from bin/
  # into the test tree under a different name. To make the relative rpath in
  # them work, copy libstdc++ to the copied location for now.
  # There is also a compiler-rt test that copies llvm-symbolizer out of bin/.
  # TODO(thakis): Instead, make the upstream lit.local.cfg.py for these 2 tests
  # check if the binary contains an rpath and if so disable the tests.
  for d in ['lib',
            'test/tools/llvm-isel-fuzzer/lib',
            'test/tools/llvm-opt-fuzzer/lib']:
    EnsureDirExists(os.path.join(build_dir, d))
    CopyFile(libstdcpp, os.path.join(build_dir, d))

  sanitizer_common_tests = os.path.join(build_dir,
                                 'projects/compiler-rt/test/sanitizer_common')
  if os.path.exists(sanitizer_common_tests):
    for d in ['asan-i386-Linux', 'asan-x86_64-Linux', 'lsan-i386-Linux',
              'lsan-x86_64-Linux', 'msan-x86_64-Linux', 'tsan-x86_64-Linux',
              'ubsan-i386-Linux', 'ubsan-x86_64-Linux']:
      libpath = os.path.join(sanitizer_common_tests, d, 'Output', 'lib')
      EnsureDirExists(libpath)
      CopyFile(libstdcpp, libpath)


def gn_arg(v):
  if v == 'True':
    return True
  if v == 'False':
    return False
  raise argparse.ArgumentTypeError('Expected one of %r or %r' % (
      'True', 'False'))


def main():
  parser = argparse.ArgumentParser(description='Build Clang.')
  parser.add_argument('--bootstrap', action='store_true',
                      help='first build clang with CC, then with itself.')
  parser.add_argument('--build-mac-arm', action='store_true',
                      help='Build arm binaries. Only valid on macOS.')
  parser.add_argument('--disable-asserts', action='store_true',
                      help='build with asserts disabled')
  parser.add_argument('--gcc-toolchain', help='what gcc toolchain to use for '
                      'building; --gcc-toolchain=/opt/foo picks '
                      '/opt/foo/bin/gcc')
  parser.add_argument('--pgo', action='store_true', help='build with PGO')
  parser.add_argument('--thinlto',
                      action='store_true',
                      help='build with ThinLTO')
  parser.add_argument('--llvm-force-head-revision', action='store_true',
                      help='build the latest revision')
  parser.add_argument('--run-tests', action='store_true',
                      help='run tests after building')
  parser.add_argument('--skip-build', action='store_true',
                      help='do not build anything')
  parser.add_argument('--skip-checkout', action='store_true',
                      help='do not create or update any checkouts')
  parser.add_argument('--build-dir',
                      help='Override build directory')
  parser.add_argument('--extra-tools', nargs='*', default=[],
                      help='select additional chrome tools to build')
  parser.add_argument('--use-system-cmake', action='store_true',
                      help='use the cmake from PATH instead of downloading '
                      'and using prebuilt cmake binaries')
  parser.add_argument('--with-android', type=gn_arg, nargs='?', const=True,
                      help='build the Android ASan runtime (linux only)',
                      default=sys.platform.startswith('linux'))
  parser.add_argument('--with-fuchsia',
                      type=gn_arg,
                      nargs='?',
                      const=True,
                      help='build the Fuchsia runtimes (linux and mac only)',
                      default=sys.platform.startswith('linux')
                      or sys.platform.startswith('darwin'))
  parser.add_argument('--without-android', action='store_false',
                      help='don\'t build Android ASan runtime (linux only)',
                      dest='with_android')
  parser.add_argument('--without-fuchsia', action='store_false',
                      help='don\'t build Fuchsia clang_rt runtime (linux/mac)',
                      dest='with_fuchsia',
                      default=sys.platform in ('linux2', 'darwin'))
  args = parser.parse_args()

  global CLANG_REVISION, PACKAGE_VERSION, LLVM_BUILD_DIR

  if (args.pgo or args.thinlto) and not args.bootstrap:
    print('--pgo/--thinlto requires --bootstrap')
    return 1
  if args.with_android and not os.path.exists(ANDROID_NDK_DIR):
    print('Android NDK not found at ' + ANDROID_NDK_DIR)
    print('The Android NDK is needed to build a Clang whose -fsanitize=address')
    print('works on Android. See ')
    print('https://www.chromium.org/developers/how-tos/android-build-instructions')
    print('for how to install the NDK, or pass --without-android.')
    return 1

  if args.with_fuchsia and not os.path.exists(FUCHSIA_SDK_DIR):
    print('Fuchsia SDK not found at ' + FUCHSIA_SDK_DIR)
    print('The Fuchsia SDK is needed to build libclang_rt for Fuchsia.')
    print('Install the Fuchsia SDK by adding fuchsia to the ')
    print('target_os section in your .gclient and running hooks, ')
    print('or pass --without-fuchsia.')
    print(
        'https://chromium.googlesource.com/chromium/src/+/main/docs/fuchsia/build_instructions.md'
    )
    print('for general Fuchsia build instructions.')
    return 1

  if args.build_mac_arm and sys.platform != 'darwin':
    print('--build-mac-arm only valid on macOS')
    return 1
  if args.build_mac_arm and platform.machine() == 'arm64':
    print('--build-mac-arm only valid on intel to cross-build arm')
    return 1

  # Don't buffer stdout, so that print statements are immediately flushed.
  # LLVM tests print output without newlines, so with buffering they won't be
  # immediately printed.
  major, _, _, _, _ = sys.version_info
  if major == 3:
    # Python3 only allows unbuffered output for binary streams. This
    # workaround comes from https://stackoverflow.com/a/181654/4052492.
    sys.stdout = io.TextIOWrapper(open(sys.stdout.fileno(), 'wb', 0),
                                  write_through=True)
  else:
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

  # The gnuwin package also includes curl, which is needed to interact with the
  # github API below.
  # TODO(crbug.com/1067752): Use urllib once certificates are fixed, and
  # move this down to where we fetch other build tools.
  AddGnuWinToPath()

  # TODO(crbug.com/929645): Remove once we build on host systems with a modern
  # enough GCC to build Clang.
  MaybeDownloadHostGcc(args)

  if sys.platform == 'darwin':
    isysroot = subprocess.check_output(['xcrun', '--show-sdk-path'],
                                       universal_newlines=True).rstrip()

  if args.build_dir:
    LLVM_BUILD_DIR = args.build_dir

  if args.llvm_force_head_revision:
    checkout_revision = GetLatestLLVMCommit()
  else:
    checkout_revision = CLANG_REVISION

  if not args.skip_checkout:
    CheckoutLLVM(checkout_revision, LLVM_DIR)

  if args.llvm_force_head_revision:
    CLANG_REVISION = GetCommitDescription(checkout_revision)
    PACKAGE_VERSION = '%s-0' % CLANG_REVISION

  print('Locally building clang %s...' % PACKAGE_VERSION)
  WriteStampFile('', STAMP_FILE)
  WriteStampFile('', FORCE_HEAD_REVISION_FILE)

  AddCMakeToPath(args)
  DeleteChromeToolsShim()


  if args.skip_build:
    return 0

  # The variable "lld" is only used on Windows because only there does setting
  # CMAKE_LINKER have an effect: On Windows, the linker is called directly,
  # while elsewhere it's called through the compiler driver, and we pass
  # -fuse-ld=lld there to make the compiler driver call the linker (by setting
  # LLVM_ENABLE_LLD).
  cc, cxx, lld = None, None, None

  cflags = []
  cxxflags = []
  ldflags = []

  targets = 'AArch64;ARM;Mips;PowerPC;SystemZ;WebAssembly;X86'

  projects = 'clang;compiler-rt;lld;clang-tools-extra'

  if sys.platform == 'darwin':
    # clang needs libc++, else -stdlib=libc++ won't find includes
    # (this is needed for bootstrap builds and for building the fuchsia runtime)
    projects += ';libcxx'

  base_cmake_args = [
      '-GNinja',
      '-DCMAKE_BUILD_TYPE=Release',
      '-DLLVM_ENABLE_ASSERTIONS=%s' % ('OFF' if args.disable_asserts else 'ON'),
      '-DLLVM_ENABLE_PROJECTS=' + projects,
      '-DLLVM_TARGETS_TO_BUILD=' + targets,
      '-DLLVM_ENABLE_PIC=OFF',
      '-DLLVM_ENABLE_UNWIND_TABLES=OFF',
      '-DLLVM_ENABLE_TERMINFO=OFF',
      '-DLLVM_ENABLE_Z3_SOLVER=OFF',
      '-DCLANG_PLUGIN_SUPPORT=OFF',
      '-DCLANG_ENABLE_STATIC_ANALYZER=OFF',
      '-DCLANG_ENABLE_ARCMT=OFF',
      '-DBUG_REPORT_URL=' + BUG_REPORT_URL,
      # Don't run Go bindings tests; PGO makes them confused.
      '-DLLVM_INCLUDE_GO_TESTS=OFF',
      # TODO(crbug.com/1113475): Update binutils.
      '-DENABLE_X86_RELAX_RELOCATIONS=NO',
      # See crbug.com/1126219: Use native symbolizer instead of DIA
      '-DLLVM_ENABLE_DIA_SDK=OFF',
      # See crbug.com/1205046: don't build scudo (and others we don't need).
      '-DCOMPILER_RT_SANITIZERS_TO_BUILD=asan;dfsan;msan;hwasan;tsan;cfi',
      # The default value differs per platform, force it off everywhere.
      '-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF',
  ]

  if args.gcc_toolchain:
    # Use the specified gcc installation for building.
    cc = os.path.join(args.gcc_toolchain, 'bin', 'gcc')
    cxx = os.path.join(args.gcc_toolchain, 'bin', 'g++')
    if not os.access(cc, os.X_OK):
      print('Invalid --gcc-toolchain: ' + args.gcc_toolchain)
      return 1
    base_cmake_args += [
        '-DLLVM_LOCAL_RPATH=' + os.path.join(args.gcc_toolchain, 'lib64')
    ]

  if sys.platform == 'darwin':
    # For libc++, we only want the headers.
    base_cmake_args.extend([
        '-DLIBCXX_ENABLE_SHARED=OFF',
        '-DLIBCXX_ENABLE_STATIC=OFF',
        '-DLIBCXX_INCLUDE_TESTS=OFF',
        '-DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF',
    ])

  if args.gcc_toolchain:
    # Force compiler-rt tests to use our gcc toolchain (including libstdc++.so)
    # because the one on the host may be too old.
    base_cmake_args.append(
        '-DCOMPILER_RT_TEST_COMPILER_CFLAGS=--gcc-toolchain=' +
        args.gcc_toolchain + ' -Wl,-rpath,' +
        os.path.join(args.gcc_toolchain, 'lib64') + ' -Wl,-rpath,' +
        os.path.join(args.gcc_toolchain, 'lib32'))

  if sys.platform == 'win32':
    base_cmake_args.append('-DLLVM_USE_CRT_RELEASE=MT')

    # Require zlib compression.
    zlib_dir = AddZlibToPath()
    cflags.append('-I' + zlib_dir)
    cxxflags.append('-I' + zlib_dir)
    ldflags.append('-LIBPATH:' + zlib_dir)

    # Use rpmalloc. For faster ThinLTO linking.
    rpmalloc_dir = DownloadRPMalloc()
    base_cmake_args.append('-DLLVM_INTEGRATED_CRT_ALLOC=' + rpmalloc_dir)

  if sys.platform != 'win32':
    # libxml2 is required by the Win manifest merging tool used in cross-builds.
    base_cmake_args.append('-DLLVM_ENABLE_LIBXML2=FORCE_ON')

  if args.bootstrap:
    print('Building bootstrap compiler')
    if os.path.exists(LLVM_BOOTSTRAP_DIR):
      RmTree(LLVM_BOOTSTRAP_DIR)
    EnsureDirExists(LLVM_BOOTSTRAP_DIR)
    os.chdir(LLVM_BOOTSTRAP_DIR)

    projects = 'clang'
    if args.pgo:
      # Need libclang_rt.profile
      projects += ';compiler-rt'
    if sys.platform != 'darwin':
      projects += ';lld'
    if sys.platform == 'darwin':
      # Need libc++ and compiler-rt for the bootstrap compiler on mac.
      projects += ';libcxx;compiler-rt'

    bootstrap_targets = 'X86'
    if sys.platform == 'darwin':
      # Need ARM and AArch64 for building the ios clang_rt.
      bootstrap_targets += ';ARM;AArch64'
    bootstrap_args = base_cmake_args + [
        '-DLLVM_TARGETS_TO_BUILD=' + bootstrap_targets,
        '-DLLVM_ENABLE_PROJECTS=' + projects,
        '-DCMAKE_INSTALL_PREFIX=' + LLVM_BOOTSTRAP_INSTALL_DIR,
        '-DCMAKE_C_FLAGS=' + ' '.join(cflags),
        '-DCMAKE_CXX_FLAGS=' + ' '.join(cxxflags),
        '-DCMAKE_EXE_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_SHARED_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_MODULE_LINKER_FLAGS=' + ' '.join(ldflags),
        # Ignore args.disable_asserts for the bootstrap compiler.
        '-DLLVM_ENABLE_ASSERTIONS=ON',
    ]
    if sys.platform == 'darwin':
      # On macOS, the bootstrap toolchain needs to have compiler-rt because
      # dsymutil's link needs libclang_rt.osx.a. Only the x86_64 osx
      # libraries are needed though, and only libclang_rt (i.e.
      # COMPILER_RT_BUILD_BUILTINS).
      bootstrap_args.extend([
          '-DCOMPILER_RT_BUILD_BUILTINS=ON',
          '-DCOMPILER_RT_BUILD_CRT=OFF',
          '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
          '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
          '-DCOMPILER_RT_BUILD_ORC=OFF',
          '-DCOMPILER_RT_BUILD_SANITIZERS=OFF',
          '-DCOMPILER_RT_BUILD_XRAY=OFF',
          '-DCOMPILER_RT_ENABLE_IOS=OFF',
          '-DCOMPILER_RT_ENABLE_WATCHOS=OFF',
          '-DCOMPILER_RT_ENABLE_TVOS=OFF',
          ])
      if platform.machine() == 'arm64':
        bootstrap_args.extend(['-DDARWIN_osx_ARCHS=arm64'])
      else:
        bootstrap_args.extend(['-DDARWIN_osx_ARCHS=x86_64'])
    elif args.pgo:
      # PGO needs libclang_rt.profile but none of the other compiler-rt stuff.
      bootstrap_args.extend([
          '-DCOMPILER_RT_BUILD_BUILTINS=OFF',
          '-DCOMPILER_RT_BUILD_CRT=OFF',
          '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
          '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
          '-DCOMPILER_RT_BUILD_ORC=OFF',
          '-DCOMPILER_RT_BUILD_PROFILE=ON',
          '-DCOMPILER_RT_BUILD_SANITIZERS=OFF',
          '-DCOMPILER_RT_BUILD_XRAY=OFF',
          ])

    if cc is not None:  bootstrap_args.append('-DCMAKE_C_COMPILER=' + cc)
    if cxx is not None: bootstrap_args.append('-DCMAKE_CXX_COMPILER=' + cxx)
    if lld is not None: bootstrap_args.append('-DCMAKE_LINKER=' + lld)
    RunCommand(['cmake'] + bootstrap_args + [os.path.join(LLVM_DIR, 'llvm')],
               msvc_arch='x64')
    CopyLibstdcpp(args, LLVM_BOOTSTRAP_DIR)
    CopyLibstdcpp(args, LLVM_BOOTSTRAP_INSTALL_DIR)
    RunCommand(['ninja'], msvc_arch='x64')
    if args.run_tests:
      test_targets = ['check-all']
      if sys.platform == 'darwin' and platform.machine() == 'arm64':
        # TODO(llvm.org/PR49918): Run check-all on mac/arm too.
        test_targets = ['check-llvm', 'check-clang']
      RunCommand(['ninja'] + test_targets, msvc_arch='x64')
    RunCommand(['ninja', 'install'], msvc_arch='x64')

    if sys.platform == 'win32':
      cc = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'clang-cl.exe')
      cxx = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'clang-cl.exe')
      lld = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'lld-link.exe')
      # CMake has a hard time with backslashes in compiler paths:
      # https://stackoverflow.com/questions/13050827
      cc = cc.replace('\\', '/')
      cxx = cxx.replace('\\', '/')
      lld = lld.replace('\\', '/')
    else:
      cc = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'clang')
      cxx = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'clang++')
    if sys.platform.startswith('linux'):
      base_cmake_args.append('-DLLVM_ENABLE_LLD=ON')

    if args.gcc_toolchain:
      # Tell the bootstrap compiler where to find the standard library headers
      # and shared object files.
      cflags.append('--gcc-toolchain=' + args.gcc_toolchain)
      cxxflags.append('--gcc-toolchain=' + args.gcc_toolchain)

    print('Bootstrap compiler installed.')

  if args.pgo:
    print('Building instrumented compiler')
    if os.path.exists(LLVM_INSTRUMENTED_DIR):
      RmTree(LLVM_INSTRUMENTED_DIR)
    EnsureDirExists(LLVM_INSTRUMENTED_DIR)
    os.chdir(LLVM_INSTRUMENTED_DIR)

    projects = 'clang'
    if sys.platform == 'darwin':
      projects += ';libcxx;compiler-rt'

    instrument_args = base_cmake_args + [
        '-DLLVM_ENABLE_PROJECTS=' + projects,
        '-DCMAKE_C_FLAGS=' + ' '.join(cflags),
        '-DCMAKE_CXX_FLAGS=' + ' '.join(cxxflags),
        '-DCMAKE_EXE_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_SHARED_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_MODULE_LINKER_FLAGS=' + ' '.join(ldflags),
        # Build with instrumentation.
        '-DLLVM_BUILD_INSTRUMENTED=IR',
    ]
    # Build with the bootstrap compiler.
    if cc is not None:  instrument_args.append('-DCMAKE_C_COMPILER=' + cc)
    if cxx is not None: instrument_args.append('-DCMAKE_CXX_COMPILER=' + cxx)
    if lld is not None: instrument_args.append('-DCMAKE_LINKER=' + lld)

    RunCommand(['cmake'] + instrument_args + [os.path.join(LLVM_DIR, 'llvm')],
               msvc_arch='x64')
    CopyLibstdcpp(args, LLVM_INSTRUMENTED_DIR)
    RunCommand(['ninja'], msvc_arch='x64')
    print('Instrumented compiler built.')

    # Train by building some C++ code.
    #
    # pgo_training-1.ii is a preprocessed (on Linux) version of
    # src/third_party/blink/renderer/core/layout/layout_object.cc, selected
    # because it's a large translation unit in Blink, which is normally the
    # slowest part of Chromium to compile. Using this, we get ~20% shorter
    # build times for Linux, Android, and Mac, which is also what we got when
    # training by actually building a target in Chromium. (For comparison, a
    # C++-y "Hello World" program only resulted in 14% faster builds.)
    # See https://crbug.com/966403#c16 for all numbers.
    #
    # Although the training currently only exercises Clang, it does involve LLVM
    # internals, and so LLD also benefits when used for ThinLTO links.
    #
    # NOTE: Tidy uses binaries built with this profile, but doesn't seem to
    # gain much from it. If tidy's execution time becomes a concern, it might
    # be good to investigate that.
    #
    # TODO(hans): Enhance the training, perhaps by including preprocessed code
    # from more platforms, and by doing some linking so that lld can benefit
    # from PGO as well. Perhaps the training could be done asynchronously by
    # dedicated buildbots that upload profiles to the cloud.
    training_source = 'pgo_training-1.ii'
    with open(training_source, 'wb') as f:
      DownloadUrl(CDS_URL + '/' + training_source, f)
    train_cmd = [os.path.join(LLVM_INSTRUMENTED_DIR, 'bin', 'clang++'),
                '-target', 'x86_64-unknown-unknown', '-O2', '-g', '-std=c++14',
                 '-fno-exceptions', '-fno-rtti', '-w', '-c', training_source]
    if sys.platform == 'darwin':
      train_cmd.extend(['-stdlib=libc++', '-isysroot', isysroot])
    RunCommand(train_cmd, msvc_arch='x64')

    # Merge profiles.
    profdata = os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'bin', 'llvm-profdata')
    RunCommand([profdata, 'merge', '-output=' + LLVM_PROFDATA_FILE] +
                glob.glob(os.path.join(LLVM_INSTRUMENTED_DIR, 'profiles',
                                       '*.profraw')), msvc_arch='x64')
    print('Profile generated.')

  compiler_rt_args = [
    '-DCOMPILER_RT_BUILD_CRT=OFF',
    '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
    '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
    '-DCOMPILER_RT_BUILD_ORC=OFF',
    '-DCOMPILER_RT_BUILD_PROFILE=ON',
    '-DCOMPILER_RT_BUILD_SANITIZERS=ON',
    '-DCOMPILER_RT_BUILD_XRAY=OFF',
  ]
  if sys.platform == 'darwin':
    compiler_rt_args.extend([
        '-DCOMPILER_RT_BUILD_BUILTINS=ON',
        '-DCOMPILER_RT_ENABLE_IOS=ON',
        '-DCOMPILER_RT_ENABLE_WATCHOS=OFF',
        '-DCOMPILER_RT_ENABLE_TVOS=OFF',
        # armv7 is A5 and earlier, armv7s is A6+ (2012 and later, before 64-bit
        # iPhones). armv7k is Apple Watch, which we don't need.
        '-DDARWIN_ios_ARCHS=armv7;armv7s;arm64',
        '-DDARWIN_iossim_ARCHS=i386;x86_64;arm64',
        # We don't need 32-bit intel support for macOS, we only ship 64-bit.
        '-DDARWIN_osx_ARCHS=arm64;x86_64',
    ])
  else:
    compiler_rt_args.append('-DCOMPILER_RT_BUILD_BUILTINS=OFF')

  # LLVM uses C++11 starting in llvm 3.5. On Linux, this means libstdc++4.7+ is
  # needed, on OS X it requires libc++. clang only automatically links to libc++
  # when targeting OS X 10.9+, so add stdlib=libc++ explicitly so clang can run
  # on OS X versions as old as 10.7.
  deployment_target = ''

  if sys.platform == 'darwin' and args.bootstrap:
    # When building on 10.9, /usr/include usually doesn't exist, and while
    # Xcode's clang automatically sets a sysroot, self-built clangs don't.
    cflags = ['-isysroot', isysroot]
    cxxflags = ['-stdlib=libc++'] + cflags
    ldflags += ['-stdlib=libc++']
    deployment_target = '10.7'

  # If building at head, define a macro that plugins can use for #ifdefing
  # out code that builds at head, but not at CLANG_REVISION or vice versa.
  if args.llvm_force_head_revision:
    cflags += ['-DLLVM_FORCE_HEAD_REVISION']
    cxxflags += ['-DLLVM_FORCE_HEAD_REVISION']

  # Build PDBs for archival on Windows.  Don't use RelWithDebInfo since it
  # has different optimization defaults than Release.
  # Also disable stack cookies (/GS-) for performance.
  if sys.platform == 'win32':
    cflags += ['/Zi', '/GS-']
    cxxflags += ['/Zi', '/GS-']
    ldflags += ['/DEBUG', '/OPT:REF', '/OPT:ICF']

  deployment_env = None
  if deployment_target:
    deployment_env = os.environ.copy()
    deployment_env['MACOSX_DEPLOYMENT_TARGET'] = deployment_target

  print('Building final compiler.')

  default_tools = ['plugins', 'blink_gc_plugin', 'translation_unit']
  chrome_tools = list(set(default_tools + args.extra_tools))
  if cc is not None:  base_cmake_args.append('-DCMAKE_C_COMPILER=' + cc)
  if cxx is not None: base_cmake_args.append('-DCMAKE_CXX_COMPILER=' + cxx)
  if lld is not None: base_cmake_args.append('-DCMAKE_LINKER=' + lld)
  cmake_args = base_cmake_args + compiler_rt_args + [
      '-DCMAKE_C_FLAGS=' + ' '.join(cflags),
      '-DCMAKE_CXX_FLAGS=' + ' '.join(cxxflags),
      '-DCMAKE_EXE_LINKER_FLAGS=' + ' '.join(ldflags),
      '-DCMAKE_SHARED_LINKER_FLAGS=' + ' '.join(ldflags),
      '-DCMAKE_MODULE_LINKER_FLAGS=' + ' '.join(ldflags),
      '-DCMAKE_INSTALL_PREFIX=' + LLVM_BUILD_DIR,
      '-DLLVM_EXTERNAL_PROJECTS=chrometools',
      '-DLLVM_EXTERNAL_CHROMETOOLS_SOURCE_DIR=' +
          os.path.join(CHROMIUM_DIR, 'tools', 'clang'),
      '-DCHROMIUM_TOOLS=%s' % ';'.join(chrome_tools)]
  if args.pgo:
    cmake_args.append('-DLLVM_PROFDATA_FILE=' + LLVM_PROFDATA_FILE)
  if args.thinlto:
    cmake_args.append('-DLLVM_ENABLE_LTO=Thin')
  if sys.platform == 'win32':
    cmake_args.append('-DLLVM_ENABLE_ZLIB=FORCE_ON')

  if sys.platform == 'darwin':
    cmake_args += ['-DCOMPILER_RT_ENABLE_IOS=ON',
                   '-DSANITIZER_MIN_OSX_VERSION=10.7']
    if args.build_mac_arm:
      assert platform.machine() != 'arm64', 'build_mac_arm for cross build only'
      cmake_args += ['-DCMAKE_OSX_ARCHITECTURES=arm64',
                     '-DLLVM_USE_HOST_TOOLS=ON']

  # The default LLVM_DEFAULT_TARGET_TRIPLE depends on the host machine.
  # Set it explicitly to make the build of clang more hermetic, and also to
  # set it to arm64 when cross-building clang for mac/arm.
  if sys.platform == 'darwin':
    if args.build_mac_arm or platform.machine() == 'arm64':
      cmake_args.append('-DLLVM_DEFAULT_TARGET_TRIPLE=arm64-apple-darwin')
    else:
      cmake_args.append('-DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-apple-darwin')
  elif sys.platform.startswith('linux'):
    cmake_args.extend([
        '-DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-gnu',
        '-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON',
    ])
  elif sys.platform == 'win32':
    cmake_args.append('-DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-pc-windows-msvc')

  if os.path.exists(LLVM_BUILD_DIR):
    RmTree(LLVM_BUILD_DIR)
  EnsureDirExists(LLVM_BUILD_DIR)
  os.chdir(LLVM_BUILD_DIR)
  RunCommand(['cmake'] + cmake_args + [os.path.join(LLVM_DIR, 'llvm')],
             msvc_arch='x64', env=deployment_env)
  CopyLibstdcpp(args, LLVM_BUILD_DIR)
  RunCommand(['ninja'], msvc_arch='x64')

  if chrome_tools:
    # If any Chromium tools were built, install those now.
    RunCommand(['ninja', 'cr-install'], msvc_arch='x64')

  if not args.build_mac_arm:
    VerifyVersionOfBuiltClangMatchesVERSION()
    VerifyZlibSupport()

  if sys.platform == 'win32':
    rt_platform = 'windows'
  elif sys.platform == 'darwin':
    rt_platform = 'darwin'
  else:
    assert sys.platform.startswith('linux')
    rt_platform = 'linux'
  rt_lib_dst_dir = os.path.join(LLVM_BUILD_DIR, 'lib', 'clang', RELEASE_VERSION,
                                'lib', rt_platform)
  # Make sure the directory exists; this will not be implicilty created if
  # built with per-target runtime directories.
  if not os.path.exists(rt_lib_dst_dir):
    os.makedirs(rt_lib_dst_dir)

  # Do an out-of-tree build of compiler-rt for 32-bit Win clang_rt.profile.lib.
  if sys.platform == 'win32':
    compiler_rt_build_dir = os.path.join(LLVM_BUILD_DIR, 'compiler-rt')
    if os.path.isdir(compiler_rt_build_dir):
      RmTree(compiler_rt_build_dir)
    os.makedirs(compiler_rt_build_dir)
    os.chdir(compiler_rt_build_dir)
    if args.bootstrap:
      # The bootstrap compiler produces 64-bit binaries by default.
      cflags += ['-m32']
      cxxflags += ['-m32']

    compiler_rt_args = base_cmake_args + [
        '-DCMAKE_C_FLAGS=' + ' '.join(cflags),
        '-DCMAKE_CXX_FLAGS=' + ' '.join(cxxflags),
        '-DCMAKE_EXE_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_SHARED_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCMAKE_MODULE_LINKER_FLAGS=' + ' '.join(ldflags),
        '-DCOMPILER_RT_BUILD_BUILTINS=OFF',
        '-DCOMPILER_RT_BUILD_CRT=OFF',
        '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
        '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
        '-DCOMPILER_RT_BUILD_ORC=OFF',
        '-DCOMPILER_RT_BUILD_PROFILE=ON',
        '-DCOMPILER_RT_BUILD_SANITIZERS=OFF',
        '-DCOMPILER_RT_BUILD_XRAY=OFF',
    ]
    RunCommand(['cmake'] + compiler_rt_args +
               [os.path.join(LLVM_DIR, 'llvm')],
               msvc_arch='x86', env=deployment_env)
    RunCommand(['ninja', 'compiler-rt'], msvc_arch='x86')

    # Copy select output to the main tree.
    rt_lib_src_dir = os.path.join(compiler_rt_build_dir, 'lib', 'clang',
                                  RELEASE_VERSION, 'lib', rt_platform)
    # Static and dynamic libraries:
    CopyDirectoryContents(rt_lib_src_dir, rt_lib_dst_dir)

  if args.with_android:
    # TODO(thakis): Now that the NDK uses clang, try to build all archs in
    # one LLVM build instead of building 3 times.
    toolchain_dir = ANDROID_NDK_DIR + '/toolchains/llvm/prebuilt/linux-x86_64'
    for target_arch in ['aarch64', 'arm', 'i686', 'x86_64']:
      # Build compiler-rt runtimes needed for Android in a separate build tree.
      build_dir = os.path.join(LLVM_BUILD_DIR, 'android-' + target_arch)
      if not os.path.exists(build_dir):
        os.mkdir(os.path.join(build_dir))
      os.chdir(build_dir)
      target_triple = target_arch
      if target_arch == 'arm':
        target_triple = 'armv7'
      api_level = '19'
      if target_arch == 'aarch64' or target_arch == 'x86_64':
        api_level = '21'
      target_triple += '-linux-android' + api_level
      cflags = [
          '--target=' + target_triple,
          '--sysroot=%s/sysroot' % toolchain_dir,
          '--gcc-toolchain=' + toolchain_dir,
          # android_ndk/toolchains/llvm/prebuilt/linux-x86_64/aarch64-linux-android/bin/ld
          # depends on a newer version of libxml2.so than what's available on
          # the bots. To make things work, use our just-built lld as linker.
          '-fuse-ld=lld',
          # The compiler we're building with (just-built clang) doesn't have the
          # compiler-rt builtins; use libgcc to get past the CMake checks.
          '--rtlib=libgcc',
      ]

      android_args = base_cmake_args + [
        '-DCMAKE_C_COMPILER=' + os.path.join(LLVM_BUILD_DIR, 'bin/clang'),
        '-DCMAKE_CXX_COMPILER=' + os.path.join(LLVM_BUILD_DIR, 'bin/clang++'),
        '-DLLVM_CONFIG_PATH=' + os.path.join(LLVM_BUILD_DIR, 'bin/llvm-config'),
        '-DCMAKE_C_FLAGS=' + ' '.join(cflags),
        '-DCMAKE_CXX_FLAGS=' + ' '.join(cflags),
        '-DCMAKE_ASM_FLAGS=' + ' '.join(cflags),
        '-DCOMPILER_RT_BUILD_BUILTINS=ON',
        '-DCOMPILER_RT_BUILD_CRT=OFF',
        '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
        '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
        '-DCOMPILER_RT_BUILD_ORC=OFF',
        '-DCOMPILER_RT_BUILD_PROFILE=ON',
        '-DCOMPILER_RT_BUILD_SANITIZERS=ON',
        '-DCOMPILER_RT_BUILD_XRAY=OFF',
        '-DSANITIZER_CXX_ABI=libcxxabi',
        '-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-u__cxa_demangle',
        '-DANDROID=1']
      RunCommand(['cmake'] + android_args + [COMPILER_RT_DIR])

      libs_want = [
          'lib/linux/libclang_rt.asan-{0}-android.so',
          'lib/linux/libclang_rt.builtins-{0}-android.a',
          'lib/linux/libclang_rt.ubsan_standalone-{0}-android.so',
          'lib/linux/libclang_rt.profile-{0}-android.a',
      ]
      # Only build HWASan for AArch64.
      if target_arch == 'aarch64':
        libs_want += ['lib/linux/libclang_rt.hwasan-{0}-android.so']
      libs_want = [lib.format(target_arch) for lib in libs_want]
      RunCommand(['ninja'] + libs_want)

      # And copy them into the main build tree.
      for p in libs_want:
        shutil.copy(p, rt_lib_dst_dir)

  if args.with_fuchsia:
    # Fuchsia links against libclang_rt.builtins-<arch>.a instead of libgcc.a.
    for target_arch in ['aarch64', 'x86_64']:
      fuchsia_arch_name = {'aarch64': 'arm64', 'x86_64': 'x64'}[target_arch]
      toolchain_dir = os.path.join(
          FUCHSIA_SDK_DIR, 'arch', fuchsia_arch_name, 'sysroot')
      # Build clang_rt runtime for Fuchsia in a separate build tree.
      build_dir = os.path.join(LLVM_BUILD_DIR, 'fuchsia-' + target_arch)
      if not os.path.exists(build_dir):
        os.mkdir(os.path.join(build_dir))
      os.chdir(build_dir)
      target_spec = target_arch + '-unknown-fuchsia'
      if args.build_mac_arm:
        # Just-built clang can't run (it's an arm binary on an intel host), so
        # use the bootstrap compiler instead.
        host_path = LLVM_BOOTSTRAP_INSTALL_DIR
      else:
        host_path = LLVM_BUILD_DIR
      # TODO(thakis): Might have to pass -B here once sysroot contains
      # binaries (e.g. gas for arm64?)
      fuchsia_args = base_cmake_args + [
        '-DCMAKE_C_COMPILER=' + os.path.join(host_path, 'bin/clang'),
        '-DCMAKE_CXX_COMPILER=' + os.path.join(host_path, 'bin/clang++'),
        '-DCMAKE_LINKER=' + os.path.join(host_path, 'bin/clang'),
        '-DCMAKE_AR=' + os.path.join(host_path, 'bin/llvm-ar'),
        '-DLLVM_CONFIG_PATH=' + os.path.join(host_path, 'bin/llvm-config'),
        '-DCMAKE_SYSTEM_NAME=Fuchsia',
        '-DCMAKE_CXX_COMPILER_TARGET=' + target_spec,
        '-DCMAKE_C_COMPILER_TARGET=' + target_spec,
        '-DCMAKE_ASM_COMPILER_TARGET=' + target_spec,
        '-DCOMPILER_RT_BUILD_BUILTINS=ON',
        '-DCOMPILER_RT_BUILD_CRT=OFF',
        '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
        '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
        '-DCOMPILER_RT_BUILD_ORC=OFF',
        '-DCOMPILER_RT_BUILD_PROFILE=OFF',
        '-DCOMPILER_RT_BUILD_SANITIZERS=OFF',
        '-DCOMPILER_RT_BUILD_XRAY=OFF',
        '-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON',
        '-DCMAKE_SYSROOT=%s' % toolchain_dir,
        # TODO(thakis|scottmg): Use PER_TARGET_RUNTIME_DIR for all platforms.
        # https://crbug.com/882485.
        '-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON',

        # These are necessary because otherwise CMake tries to build an
        # executable to test to see if the compiler is working, but in doing so,
        # it links against the builtins.a that we're about to build.
        '-DCMAKE_CXX_COMPILER_WORKS=ON',
        '-DCMAKE_C_COMPILER_WORKS=ON',
        '-DCMAKE_ASM_COMPILER_WORKS=ON',
        ]
      RunCommand(['cmake'] +
                 fuchsia_args +
                 [os.path.join(COMPILER_RT_DIR, 'lib', 'builtins')])
      builtins_a = 'libclang_rt.builtins.a'
      RunCommand(['ninja', builtins_a])

      # And copy it into the main build tree.
      fuchsia_lib_dst_dir = os.path.join(LLVM_BUILD_DIR, 'lib', 'clang',
                                         RELEASE_VERSION, 'lib', target_spec)
      if not os.path.exists(fuchsia_lib_dst_dir):
        os.makedirs(fuchsia_lib_dst_dir)
      CopyFile(os.path.join(build_dir, 'lib', target_spec, builtins_a),
               fuchsia_lib_dst_dir)

      # Build the Fuchsia profile and asan runtimes.  This is done after the rt
      # builtins have been created because the CMake build runs link checks that
      # require that the builtins already exist to succeed.
      # TODO(thakis): Figure out why this doesn't build with the stage0
      # compiler in arm cross builds.
      if target_arch == 'x86_64' and not args.build_mac_arm:
        fuchsia_args.extend([
            '-DCOMPILER_RT_BUILD_BUILTINS=OFF',
            '-DCOMPILER_RT_BUILD_PROFILE=ON',
        ])
        # Build the asan runtime only on non-Mac platforms.  Macs are excluded
        # because the asan install changes library RPATHs which CMake only
        # supports on ELF platforms and MacOS uses Mach-O instead of ELF.
        if sys.platform != 'darwin':
          fuchsia_args.extend([
              '-DCOMPILER_RT_BUILD_SANITIZERS=ON',
              '-DSANITIZER_NO_UNDEFINED_SYMBOLS=OFF',
          ])
        build_phase2_dir = os.path.join(LLVM_BUILD_DIR,
                                         'fuchsia-phase2-' + target_arch)
        if not os.path.exists(build_phase2_dir):
          os.mkdir(os.path.join(build_phase2_dir))
        os.chdir(build_phase2_dir)
        RunCommand(['cmake'] +
                   fuchsia_args +
                   [COMPILER_RT_DIR])
        profile_a = 'libclang_rt.profile.a'
        asan_so = 'libclang_rt.asan.so'
        ninja_command = ['ninja', profile_a]
        if sys.platform != 'darwin':
          ninja_command.append(asan_so)
        RunCommand(ninja_command)
        CopyFile(os.path.join(build_phase2_dir, 'lib', target_spec, profile_a),
                              fuchsia_lib_dst_dir)
        if sys.platform != 'darwin':
          CopyFile(os.path.join(build_phase2_dir, 'lib', target_spec, asan_so),
                   fuchsia_lib_dst_dir)

  # Run tests.
  if (not args.build_mac_arm and
      (args.run_tests or args.llvm_force_head_revision)):
    RunCommand(['ninja', '-C', LLVM_BUILD_DIR, 'cr-check-all'], msvc_arch='x64')

  if not args.build_mac_arm and args.run_tests:
    test_targets = [ 'check-all' ]
    if sys.platform == 'darwin':
      # TODO(thakis): Run check-all on Darwin too, https://crbug.com/959361
      test_targets = [ 'check-llvm', 'check-clang', 'check-lld' ]
    RunCommand(['ninja', '-C', LLVM_BUILD_DIR] + test_targets, msvc_arch='x64')

  WriteStampFile(PACKAGE_VERSION, STAMP_FILE)
  WriteStampFile(PACKAGE_VERSION, FORCE_HEAD_REVISION_FILE)
  print('Clang build was successful.')
  return 0


if __name__ == '__main__':
  sys.exit(main())
