#!/usr/bin/env python3
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script will check out llvm and clang, and then package the results up
to a tgz file."""

from __future__ import print_function

import argparse
import fnmatch
import itertools
import multiprocessing.dummy
import os
import platform
import shutil
import subprocess
import sys
import tarfile
import time

from update import RELEASE_VERSION, STAMP_FILE

# Path constants.
THIS_DIR = os.path.dirname(__file__)
CHROMIUM_DIR = os.path.abspath(os.path.join(THIS_DIR, '..', '..', '..'))
THIRD_PARTY_DIR = os.path.join(THIS_DIR, '..', '..', '..', 'third_party')
BUILDTOOLS_DIR = os.path.join(THIS_DIR, '..', '..', '..', 'buildtools')
LLVM_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm')
LLVM_BOOTSTRAP_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm-bootstrap')
LLVM_BOOTSTRAP_INSTALL_DIR = os.path.join(THIRD_PARTY_DIR,
                                          'llvm-bootstrap-install')
LLVM_BUILD_DIR = os.path.join(THIRD_PARTY_DIR, 'llvm-build')
LLVM_RELEASE_DIR = os.path.join(LLVM_BUILD_DIR, 'Release+Asserts')
EU_STRIP = os.path.join(BUILDTOOLS_DIR, 'third_party', 'eu-strip', 'bin',
                        'eu-strip')


def Tee(output, logfile):
  logfile.write(output)
  print(output, end=' ')


def TeeCmd(cmd, logfile, fail_hard=True):
  """Runs cmd and writes the output to both stdout and logfile."""
  # Reading from PIPE can deadlock if one buffer is full but we wait on a
  # different one.  To work around this, pipe the subprocess's stderr to
  # its stdout buffer and don't give it a stdin.
  # shell=True is required in cmd.exe since depot_tools has an svn.bat, and
  # bat files only work with shell=True set.
  proc = subprocess.Popen(cmd, bufsize=1, shell=sys.platform == 'win32',
                          stdin=open(os.devnull), stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)
  for line in iter(proc.stdout.readline,''):
    Tee(str(line.decode()), logfile)
    if proc.poll() is not None:
      break
  exit_code = proc.wait()
  if exit_code != 0 and fail_hard:
    print('Failed:', cmd)
    sys.exit(1)


def PrintTarProgress(tarinfo):
  print('Adding', tarinfo.name)
  return tarinfo


def GetExpectedStamp():
  rev_cmd = [sys.executable, os.path.join(THIS_DIR, 'update.py'),
             '--print-revision']
  return str(subprocess.check_output(rev_cmd).decode()).rstrip()


def GetGsutilPath():
  if not 'find_depot_tools' in sys.modules:
    sys.path.insert(0, os.path.join(CHROMIUM_DIR, 'build'))
    global find_depot_tools
    import find_depot_tools
  depot_path = find_depot_tools.add_depot_tools_to_path()
  if depot_path is None:
    print ('depot_tools are not found in PATH. '
           'Follow the instructions in this document '
           'http://dev.chromium.org/developers/how-tos/install-depot-tools'
           ' to install depot_tools and then try again.')
    sys.exit(1)
  gsutil_path = os.path.join(depot_path, 'gsutil.py')
  return gsutil_path


def RunGsutil(args):
  return subprocess.call([sys.executable, GetGsutilPath()] + args)


def PackageInArchive(directory_path, archive_path):
  bin_dir_path = os.path.join(directory_path, 'bin')
  if sys.platform != 'win32' and os.path.exists(bin_dir_path):
    for f in os.listdir(bin_dir_path):
      file_path = os.path.join(bin_dir_path, f)
      if not os.path.islink(file_path):
        subprocess.call(['strip', file_path])

  with tarfile.open(archive_path, 'w:gz') as tar:
    for f in os.listdir(directory_path):
      tar.add(os.path.join(directory_path, f),
              arcname=f, filter=PrintTarProgress)


def MaybeUpload(do_upload, filename, gcs_platform, extra_gsutil_args=[]):
  gsutil_args = ['cp'] + extra_gsutil_args + [
      '-a', 'public-read', filename,
      'gs://chromium-browser-clang-staging/%s/%s' % (gcs_platform, filename)
  ]
  if do_upload:
    print('Uploading %s to Google Cloud Storage...' % filename)
    exit_code = RunGsutil(gsutil_args)
    if exit_code != 0:
      print("gsutil failed, exit_code: %s" % exit_code)
      sys.exit(exit_code)
  else:
    print('To upload, run:')
    print('gsutil %s' % ' '.join(gsutil_args))


def UploadPDBsToSymbolServer(binaries):
  assert sys.platform == 'win32'
  # Upload PDB and binary to the symbol server on Windows.  Put them into the
  # chromium-browser-symsrv bucket, since chrome devs have that in their
  # _NT_SYMBOL_PATH already. Executable and PDB must be at paths following a
  # certain pattern for the Microsoft debuggers to be able to load them.
  # Executable:
  #  chromium-browser-symsrv/clang-cl.exe/ABCDEFAB01234/clang-cl.ex_
  #    ABCDEFAB is the executable's timestamp in %08X format, 01234 is the
  #    executable's image size in %x format. tools/symsrc/img_fingerprint.py
  #    can compute this ABCDEFAB01234 string for us, so use that.
  #    The .ex_ instead of .exe at the end means that the file is compressed.
  # PDB:
  # gs://chromium-browser-symsrv/clang-cl.exe.pdb/AABBCCDD/clang-cl.exe.pd_
  #   AABBCCDD here is computed from the output of
  #      dumpbin /all mybinary.exe | find "Format: RSDS"
  #   but tools/symsrc/pdb_fingerprint_from_img.py can compute it already, so
  #   again just use that.
  sys.path.insert(0, os.path.join(CHROMIUM_DIR, 'tools', 'symsrc'))
  import img_fingerprint, pdb_fingerprint_from_img

  files = []
  for binary_path in binaries:
    binary_path = os.path.join(LLVM_RELEASE_DIR, binary_path)
    binary_id = img_fingerprint.GetImgFingerprint(binary_path)
    (pdb_id, pdb_path) = pdb_fingerprint_from_img.GetPDBInfoFromImg(binary_path)
    files += [(binary_path, binary_id), (pdb_path, pdb_id)]

    # The build process builds clang.exe and then copies it to clang-cl.exe
    # (both are the same binary and they behave differently on what their
    # filename is).  Hence, the pdb is at clang.pdb, not at clang-cl.pdb.
    # Likewise, lld-link.exe's PDB file is called lld.pdb.

  # Compress and upload.
  def compress(t):
    subprocess.check_call(
      ['makecab', '/D', 'CompressionType=LZX', '/D', 'CompressionMemory=21',
       t[0], '/L', os.path.dirname(t[0])], stdout=open(os.devnull, 'w'))
  multiprocessing.dummy.Pool().map(compress, files)
  for f, f_id in files:
    f_cab = f[:-1] + '_'
    dest = '%s/%s/%s' % (os.path.basename(f), f_id, os.path.basename(f_cab))
    print('Uploading %s to Google Cloud Storage...' % dest)
    gsutil_args = ['cp', '-n', '-a', 'public-read', f_cab,
                   'gs://chromium-browser-symsrv/' + dest]
    exit_code = RunGsutil(gsutil_args)
    if exit_code != 0:
      print("gsutil failed, exit_code: %s" % exit_code)
      sys.exit(exit_code)


def main():
  parser = argparse.ArgumentParser(description='build and package clang')
  parser.add_argument('--upload', action='store_true',
                      help='Upload the target archive to Google Cloud Storage.')
  parser.add_argument('--build-mac-arm', action='store_true',
                      help='Build arm binaries. Only valid on macOS.')
  args = parser.parse_args()

  if args.build_mac_arm and sys.platform != 'darwin':
    print('--build-mac-arm only valid on macOS')
    return 1
  if args.build_mac_arm and platform.machine() == 'arm64':
    print('--build-mac-arm only valid on intel to cross-build arm')
    return 1

  expected_stamp = GetExpectedStamp()
  pdir = 'clang-' + expected_stamp
  print(pdir)

  if sys.platform == 'darwin':
    # When we need to run this script on an arm machine, we need to add a
    # --build-mac-intel switch to pick which clang to build, pick the
    # 'Mac_arm64' here when there's no flag and 'Mac' when --build-mac-intel is
    # passed. Also update the build script to explicitly pass a default triple
    # then.
    if args.build_mac_arm or platform.machine() == 'arm64':
      gcs_platform = 'Mac_arm64'
    else:
      gcs_platform = 'Mac'
  elif sys.platform == 'win32':
    gcs_platform = 'Win'
  else:
    gcs_platform = 'Linux_x64'

  with open('buildlog.txt', 'w') as log:
    Tee('Starting build\n', log)

    # Do a clobber build.
    shutil.rmtree(LLVM_BOOTSTRAP_DIR, ignore_errors=True)
    shutil.rmtree(LLVM_BOOTSTRAP_INSTALL_DIR, ignore_errors=True)
    shutil.rmtree(LLVM_BUILD_DIR, ignore_errors=True)

    build_cmd = [
        sys.executable,
        os.path.join(THIS_DIR, 'build.py'), '--bootstrap', '--disable-asserts',
        '--run-tests', '--pgo'
    ]
    if args.build_mac_arm:
      build_cmd.append('--build-mac-arm')
    if sys.platform != 'darwin':
      build_cmd.append('--thinlto')

    TeeCmd(build_cmd, log)

  stamp = open(STAMP_FILE).read().rstrip()
  if stamp != expected_stamp:
    print('Actual stamp (%s) != expected stamp (%s).' % (stamp, expected_stamp))
    return 1

  shutil.rmtree(pdir, ignore_errors=True)

  # Copy a whitelist of files to the directory we're going to tar up.
  # This supports the same patterns that the fnmatch module understands.
  # '$V' is replaced by RELEASE_VERSION further down.
  exe_ext = '.exe' if sys.platform == 'win32' else ''
  want = [
    'bin/llvm-pdbutil' + exe_ext,
    'bin/llvm-symbolizer' + exe_ext,
    'bin/llvm-undname' + exe_ext,
    # Copy built-in headers (lib/clang/3.x.y/include).
    'lib/clang/$V/include/*',
    'lib/clang/$V/share/asan_*list.txt',
    'lib/clang/$V/share/cfi_*list.txt',
  ]
  if sys.platform == 'win32':
    want.extend([
      'bin/clang-cl.exe',
      'bin/lld-link.exe',
    ])
  else:
    want.extend([
      'bin/clang',

      # Add LLD.
      'bin/lld',

      # Add llvm-ar for LTO.
      'bin/llvm-ar',

      # Include libclang_rt.builtins.a for Fuchsia targets.
      'lib/clang/$V/lib/aarch64-unknown-fuchsia/libclang_rt.builtins.a',
      'lib/clang/$V/lib/x86_64-unknown-fuchsia/libclang_rt.builtins.a',

      # Add llvm-readobj (symlinked from llvm-readelf) for extracting SONAMEs.
      'bin/llvm-readobj',
    ])
    if not args.build_mac_arm:
      # TODO(thakis): Figure out why this doesn't build in --build-mac-arm
      # builds.
      want.append(
          'lib/clang/$V/lib/x86_64-unknown-fuchsia/libclang_rt.profile.a')
    if sys.platform != 'darwin':
      # The Fuchsia asan runtime is only built on non-Mac platforms.
      want.append('lib/clang/$V/lib/x86_64-unknown-fuchsia/libclang_rt.asan.so')
  if sys.platform == 'darwin':
    want.extend([
      # AddressSanitizer runtime.
      'lib/clang/$V/lib/darwin/libclang_rt.asan_iossim_dynamic.dylib',
      'lib/clang/$V/lib/darwin/libclang_rt.asan_osx_dynamic.dylib',

      # OS X and iOS builtin libraries for the _IsOSVersionAtLeast runtime
      # function.
      'lib/clang/$V/lib/darwin/libclang_rt.ios.a',
      'lib/clang/$V/lib/darwin/libclang_rt.iossim.a',
      'lib/clang/$V/lib/darwin/libclang_rt.osx.a',

      # Profile runtime (used by profiler and code coverage).
      'lib/clang/$V/lib/darwin/libclang_rt.profile_iossim.a',
      'lib/clang/$V/lib/darwin/libclang_rt.profile_osx.a',

      # UndefinedBehaviorSanitizer runtime.
      'lib/clang/$V/lib/darwin/libclang_rt.ubsan_iossim_dynamic.dylib',
      'lib/clang/$V/lib/darwin/libclang_rt.ubsan_osx_dynamic.dylib',
    ])
  elif sys.platform.startswith('linux'):
    want.extend([
        # pylint: disable=line-too-long

        # Copy the stdlibc++.so.6 we linked the binaries against.
        'lib/libstdc++.so.6',

        # dwp needed for use_debug_fission.
        'bin/llvm-dwp',

        # Add llvm-objcopy for partition extraction on Android.
        'bin/llvm-objcopy',

        # Add llvm-nm.
        'bin/llvm-nm',

        # AddressSanitizer C runtime (pure C won't link with *_cxx).
        'lib/clang/$V/lib/i386-unknown-linux-gnu/libclang_rt.asan.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.asan.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.asan.a.syms',

        # AddressSanitizer C++ runtime.
        'lib/clang/$V/lib/i386-unknown-linux-gnu/libclang_rt.asan_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.asan_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.asan_cxx.a.syms',

        # AddressSanitizer Android runtime.
        'lib/clang/$V/lib/linux/libclang_rt.asan-aarch64-android.so',
        'lib/clang/$V/lib/linux/libclang_rt.asan-arm-android.so',
        'lib/clang/$V/lib/linux/libclang_rt.asan-i686-android.so',

        # Builtins for Android.
        'lib/clang/$V/lib/linux/libclang_rt.builtins-aarch64-android.a',
        'lib/clang/$V/lib/linux/libclang_rt.builtins-arm-android.a',
        'lib/clang/$V/lib/linux/libclang_rt.builtins-i686-android.a',
        'lib/clang/$V/lib/linux/libclang_rt.builtins-x86_64-android.a',

        # HWASAN Android runtime.
        'lib/clang/$V/lib/linux/libclang_rt.hwasan-aarch64-android.so',

        # MemorySanitizer C runtime (pure C won't link with *_cxx).
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.msan.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.msan.a.syms',

        # MemorySanitizer C++ runtime.
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.msan_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.msan_cxx.a.syms',

        # Profile runtime (used by profiler and code coverage).
        'lib/clang/$V/lib/i386-unknown-linux-gnu/libclang_rt.profile.a',
        'lib/clang/$V/lib/linux/libclang_rt.profile-i686-android.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.profile.a',
        'lib/clang/$V/lib/linux/libclang_rt.profile-aarch64-android.a',
        'lib/clang/$V/lib/linux/libclang_rt.profile-arm-android.a',

        # ThreadSanitizer C runtime (pure C won't link with *_cxx).
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan.a.syms',

        # ThreadSanitizer C++ runtime.
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan_cxx.a.syms',

        # UndefinedBehaviorSanitizer C runtime (pure C won't link with *_cxx).
        'lib/clang/$V/lib/i386-unknown-linux-gnu/libclang_rt.ubsan_standalone.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.ubsan_standalone.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.ubsan_standalone.a.syms',

        # UndefinedBehaviorSanitizer C++ runtime.
        'lib/clang/$V/lib/i386-unknown-linux-gnu/libclang_rt.ubsan_standalone_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.ubsan_standalone_cxx.a',
        'lib/clang/$V/lib/x86_64-unknown-linux-gnu/libclang_rt.ubsan_standalone_cxx.a.syms',

        # UndefinedBehaviorSanitizer Android runtime, needed for CFI.
        'lib/clang/$V/lib/linux/libclang_rt.ubsan_standalone-aarch64-android.so',
        'lib/clang/$V/lib/linux/libclang_rt.ubsan_standalone-arm-android.so',

        # Ignorelist for MemorySanitizer (used on Linux only).
        'lib/clang/$V/share/msan_*list.txt',

        # pylint: enable=line-too-long
    ])
  elif sys.platform == 'win32':
    want.extend([
      # AddressSanitizer C runtime (pure C won't link with *_cxx).
      'lib/clang/$V/lib/windows/clang_rt.asan-x86_64.lib',

      # AddressSanitizer C++ runtime.
      'lib/clang/$V/lib/windows/clang_rt.asan_cxx-x86_64.lib',

      # Thunk for AddressSanitizer needed for static build of a shared lib.
      'lib/clang/$V/lib/windows/clang_rt.asan_dll_thunk-x86_64.lib',

      # AddressSanitizer runtime for component build.
      'lib/clang/$V/lib/windows/clang_rt.asan_dynamic-x86_64.dll',
      'lib/clang/$V/lib/windows/clang_rt.asan_dynamic-x86_64.lib',

      # Thunk for AddressSanitizer for component build of a shared lib.
      'lib/clang/$V/lib/windows/clang_rt.asan_dynamic_runtime_thunk-x86_64.lib',

      # Profile runtime (used by profiler and code coverage).
      'lib/clang/$V/lib/windows/clang_rt.profile-i386.lib',
      'lib/clang/$V/lib/windows/clang_rt.profile-x86_64.lib',

      # UndefinedBehaviorSanitizer C runtime (pure C won't link with *_cxx).
      'lib/clang/$V/lib/windows/clang_rt.ubsan_standalone-x86_64.lib',

      # UndefinedBehaviorSanitizer C++ runtime.
      'lib/clang/$V/lib/windows/clang_rt.ubsan_standalone_cxx-x86_64.lib',
    ])

  # Check all non-glob wanted files exist on disk.
  want = [w.replace('$V', RELEASE_VERSION) for w in want]
  for w in want:
    if '*' in w: continue
    if os.path.exists(os.path.join(LLVM_RELEASE_DIR, w)): continue
    print('wanted file "%s" but it did not exist' % w, file=sys.stderr)
    return 1

  # TODO(thakis): Try walking over want and copying the files in there instead
  # of walking the directory and doing fnmatch() against want.
  for root, dirs, files in os.walk(LLVM_RELEASE_DIR):
    # root: third_party/llvm-build/Release+Asserts/lib/..., rel_root: lib/...
    rel_root = root[len(LLVM_RELEASE_DIR)+1:]
    rel_files = [os.path.join(rel_root, f) for f in files]
    wanted_files = list(set(itertools.chain.from_iterable(
        fnmatch.filter(rel_files, p) for p in want)))
    if wanted_files:
      # Guaranteed to not yet exist at this point:
      os.makedirs(os.path.join(pdir, rel_root))
    for f in wanted_files:
      src = os.path.join(LLVM_RELEASE_DIR, f)
      dest = os.path.join(pdir, f)
      shutil.copy(src, dest)
      # Strip libraries.
      if sys.platform == 'darwin' and f.endswith('.dylib'):
        subprocess.call(['strip', '-x', dest])
      elif (sys.platform.startswith('linux') and
            os.path.splitext(f)[1] in ['.so', '.a']):
        subprocess.call([EU_STRIP, '-g', dest])

  # Set up symlinks.
  if sys.platform != 'win32':
    os.symlink('clang', os.path.join(pdir, 'bin', 'clang++'))
    os.symlink('clang', os.path.join(pdir, 'bin', 'clang-cl'))
    os.symlink('lld', os.path.join(pdir, 'bin', 'ld.lld'))
    os.symlink('lld', os.path.join(pdir, 'bin', 'ld64.lld'))
    os.symlink('lld', os.path.join(pdir, 'bin', 'lld-link'))
    os.symlink('lld', os.path.join(pdir, 'bin', 'wasm-ld'))
    os.symlink('llvm-readobj', os.path.join(pdir, 'bin', 'llvm-readelf'))

  if sys.platform.startswith('linux'):
    os.symlink('llvm-objcopy', os.path.join(pdir, 'bin', 'llvm-strip'))

  # Copy libc++ headers.
  if sys.platform == 'darwin':
    shutil.copytree(os.path.join(LLVM_BOOTSTRAP_INSTALL_DIR, 'include', 'c++'),
                    os.path.join(pdir, 'include', 'c++'))

  # Create main archive.
  PackageInArchive(pdir, pdir + '.tgz')
  MaybeUpload(args.upload, pdir + '.tgz', gcs_platform)

  # Upload build log next to it.
  os.rename('buildlog.txt', pdir + '-buildlog.txt')
  MaybeUpload(args.upload,
              pdir + '-buildlog.txt',
              gcs_platform,
              extra_gsutil_args=['-z', 'txt'])
  os.remove(pdir + '-buildlog.txt')

  # Zip up llvm-code-coverage for code coverage.
  code_coverage_dir = 'llvm-code-coverage-' + stamp
  shutil.rmtree(code_coverage_dir, ignore_errors=True)
  os.makedirs(os.path.join(code_coverage_dir, 'bin'))
  for filename in ['llvm-cov', 'llvm-profdata']:
    shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', filename + exe_ext),
                os.path.join(code_coverage_dir, 'bin'))
  PackageInArchive(code_coverage_dir, code_coverage_dir + '.tgz')
  MaybeUpload(args.upload, code_coverage_dir + '.tgz', gcs_platform)

  # Zip up llvm-objdump and related tools for sanitizer coverage and Supersize.
  objdumpdir = 'llvmobjdump-' + stamp
  shutil.rmtree(objdumpdir, ignore_errors=True)
  os.makedirs(os.path.join(objdumpdir, 'bin'))
  for filename in [
      'llvm-bcanalyzer', 'llvm-cxxfilt', 'llvm-dwarfdump', 'llvm-nm',
      'llvm-objdump'
  ]:
    shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', filename + exe_ext),
                os.path.join(objdumpdir, 'bin'))
  llvmobjdump_stamp_file_base = 'llvmobjdump_build_revision'
  llvmobjdump_stamp_file = os.path.join(objdumpdir, llvmobjdump_stamp_file_base)
  with open(llvmobjdump_stamp_file, 'w') as f:
    f.write(expected_stamp)
    f.write('\n')
  if sys.platform != 'win32':
    os.symlink('llvm-objdump', os.path.join(objdumpdir, 'bin', 'llvm-otool'))
  PackageInArchive(objdumpdir, objdumpdir + '.tgz')
  MaybeUpload(args.upload, objdumpdir + '.tgz', gcs_platform)

  # Zip up clang-tidy for users who opt into it, and Tricium.
  clang_tidy_dir = 'clang-tidy-' + stamp
  shutil.rmtree(clang_tidy_dir, ignore_errors=True)
  os.makedirs(os.path.join(clang_tidy_dir, 'bin'))
  shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', 'clang-tidy' + exe_ext),
              os.path.join(clang_tidy_dir, 'bin'))
  PackageInArchive(clang_tidy_dir, clang_tidy_dir + '.tgz')
  MaybeUpload(args.upload, clang_tidy_dir + '.tgz', gcs_platform)

  if sys.platform == 'darwin':
    # dsymutil isn't part of the main zip, and it gets periodically
    # deployed to CIPD (manually, not as part of clang rolls) for use in the
    # Mac build toolchain.
    dsymdir = 'dsymutil-' + stamp
    shutil.rmtree(dsymdir, ignore_errors=True)
    os.makedirs(os.path.join(dsymdir, 'bin'))
    shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', 'dsymutil'),
                os.path.join(dsymdir, 'bin'))
    PackageInArchive(dsymdir, dsymdir + '.tgz')
    MaybeUpload(args.upload, dsymdir + '.tgz', gcs_platform)

  # Zip up the translation_unit tool.
  translation_unit_dir = 'translation_unit-' + stamp
  shutil.rmtree(translation_unit_dir, ignore_errors=True)
  os.makedirs(os.path.join(translation_unit_dir, 'bin'))
  shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', 'translation_unit' +
                           exe_ext),
              os.path.join(translation_unit_dir, 'bin'))
  PackageInArchive(translation_unit_dir, translation_unit_dir + '.tgz')
  MaybeUpload(args.upload, translation_unit_dir + '.tgz', gcs_platform)

  # Zip up the libclang binaries.
  libclang_dir = 'libclang-' + stamp
  shutil.rmtree(libclang_dir, ignore_errors=True)
  os.makedirs(os.path.join(libclang_dir, 'bin'))
  os.makedirs(os.path.join(libclang_dir, 'bindings', 'python', 'clang'))
  if sys.platform == 'win32':
    shutil.copy(os.path.join(LLVM_RELEASE_DIR, 'bin', 'libclang.dll'),
                os.path.join(libclang_dir, 'bin'))
  for filename in ['__init__.py', 'cindex.py', 'enumerations.py']:
    shutil.copy(os.path.join(LLVM_DIR, 'clang', 'bindings', 'python', 'clang',
                             filename),
                os.path.join(libclang_dir, 'bindings', 'python', 'clang'))
  PackageInArchive(libclang_dir, libclang_dir + '.tgz')
  MaybeUpload(args.upload, libclang_dir + '.tgz', gcs_platform)

  if sys.platform == 'win32' and args.upload:
    binaries = [f for f in want if f.endswith('.exe') or f.endswith('.dll')]
    assert 'bin/clang-cl.exe' in binaries
    assert 'bin/lld-link.exe' in binaries
    start = time.time()
    UploadPDBsToSymbolServer(binaries)
    end = time.time()
    print('symbol upload took', end - start, 'seconds')

  # FIXME: Warn if the file already exists on the server.


if __name__ == '__main__':
  sys.exit(main())
