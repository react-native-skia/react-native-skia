#!/usr/bin/env python3
# Copyright 2020 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""MB - the Meta-Build wrapper around GN.

MB is a wrapper script for GN that can be used to generate build files
for sets of canned configurations and analyze them.
"""

from __future__ import absolute_import
from __future__ import print_function

import argparse
import ast
import collections
import errno
import json
import os
import pipes
import platform
import re
import shutil
import sys
import subprocess
import tempfile
import traceback
import zipfile

if sys.version_info.major == 2:
  from urllib2 import urlopen
else:
  from urllib.request import urlopen

CHROMIUM_SRC_DIR = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
sys.path = [os.path.join(CHROMIUM_SRC_DIR, 'build')] + sys.path
sys.path.insert(0, os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '..'))

import gn_helpers
from mb.lib import validation


def DefaultVals():
  """Default mixin values"""
  return {
      'args_file': '',
      'gn_args': '',
  }


def PruneVirtualEnv():
  # Set by VirtualEnv, no need to keep it.
  os.environ.pop('VIRTUAL_ENV', None)

  # Set by VPython, if scripts want it back they have to set it explicitly.
  os.environ.pop('PYTHONNOUSERSITE', None)

  # Look for "activate_this.py" in this path, which is installed by VirtualEnv.
  # This mechanism is used by vpython as well to sanitize VirtualEnvs from
  # $PATH.
  os.environ['PATH'] = os.pathsep.join([
    p for p in os.environ.get('PATH', '').split(os.pathsep)
    if not os.path.isfile(os.path.join(p, 'activate_this.py'))
  ])


def main(args):
  # Prune all evidence of VPython/VirtualEnv out of the environment. This means
  # that we 'unwrap' vpython VirtualEnv path/env manipulation. Invocations of
  # `python` from GN should never inherit the gn.py's own VirtualEnv. This also
  # helps to ensure that generated ninja files do not reference python.exe from
  # the VirtualEnv generated from depot_tools' own .vpython file (or lack
  # thereof), but instead reference the default python from the PATH.
  PruneVirtualEnv()

  mbw = MetaBuildWrapper()
  return mbw.Main(args)


class MetaBuildWrapper:
  def __init__(self):
    self.chromium_src_dir = CHROMIUM_SRC_DIR
    self.default_config = os.path.join(self.chromium_src_dir, 'tools', 'mb',
                                       'mb_config.pyl')
    self.default_isolate_map = os.path.join(self.chromium_src_dir, 'testing',
                                            'buildbot', 'gn_isolate_map.pyl')
    self.executable = sys.executable
    self.platform = sys.platform
    self.sep = os.sep
    self.args = argparse.Namespace()
    self.configs = {}
    self.public_artifact_builders = None
    self.builder_groups = {}
    self.mixins = {}
    self.isolate_exe = 'isolate.exe' if self.platform.startswith(
        'win') else 'isolate'
    self.use_luci_auth = False
    self.rts_out_dir = self.PathJoin('gen', 'rts')
    self.banned_from_rts = set()

  def PostArgsInit(self):
    self.use_luci_auth = getattr(self.args, 'luci_auth', False)

    if 'config_file' in self.args and self.args.config_file is None:
      self.args.config_file = self.default_config

    if 'expectations_dir' in self.args and self.args.expectations_dir is None:
      self.args.expectations_dir = os.path.join(
          os.path.dirname(self.args.config_file), 'mb_config_expectations')

    banned_from_rts_map = json.loads(
        self.ReadFile(
            self.PathJoin(self.chromium_src_dir, 'tools', 'mb',
                          'rts_banned_suites.json')))
    self.banned_from_rts.update(banned_from_rts_map.get('*', set()))

    if getattr(self.args, 'builder', None):
      self.banned_from_rts.update(
          banned_from_rts_map.get(self.args.builder, set()))

  def Main(self, args):
    self.ParseArgs(args)
    self.PostArgsInit()
    try:
      ret = self.args.func()
      if ret != 0:
        self.DumpInputFiles()
      return ret
    except KeyboardInterrupt:
      self.Print('interrupted, exiting')
      return 130
    except Exception:
      self.DumpInputFiles()
      s = traceback.format_exc()
      for l in s.splitlines():
        self.Print(l)
      return 1

  def ParseArgs(self, argv):
    def AddCommonOptions(subp):
      group = subp.add_mutually_exclusive_group()
      group.add_argument(
          '-m',  '--builder-group',
          help='builder group name to look up config from')
      subp.add_argument('-b', '--builder',
                        help='builder name to look up config from')
      subp.add_argument('-c', '--config',
                        help='configuration to analyze')
      subp.add_argument('--phase',
                        help='optional phase name (used when builders '
                             'do multiple compiles with different '
                             'arguments in a single build)')
      subp.add_argument('-i', '--isolate-map-file', metavar='PATH',
                        help='path to isolate map file '
                             '(default is %(default)s)',
                        default=[],
                        action='append',
                        dest='isolate_map_files')
      subp.add_argument('-n', '--dryrun', action='store_true',
                        help='Do a dry run (i.e., do nothing, just print '
                             'the commands that will run)')
      subp.add_argument('-v', '--verbose', action='store_true',
                        help='verbose logging')
      subp.add_argument('--root', help='Path to GN source root')
      subp.add_argument('--dotfile', help='Path to GN dotfile')
      AddExpansionOptions(subp)

    def AddExpansionOptions(subp):
      # These are the args needed to expand a config file into the full
      # parsed dicts of GN args.
      subp.add_argument('-f',
                        '--config-file',
                        metavar='PATH',
                        help=('path to config file '
                              '(default is mb_config.pyl'))
      subp.add_argument('-g', '--goma-dir', help='path to goma directory')
      subp.add_argument('--android-version-code',
                        help='Sets GN arg android_default_version_code')
      subp.add_argument('--android-version-name',
                        help='Sets GN arg android_default_version_name')
      subp.add_argument('--rts',
                        default=None,
                        help='which regression test selection model to use'
                        ' For more info about RTS, please see'
                        ' //docs/testing/regression-test-selection.md')
      subp.add_argument('--use-rts',
                        action='store_true',
                        default=False,
                        help='Deprecated argument for enabling RTS')

      # TODO(crbug.com/1060857): Remove this once swarming task templates
      # support command prefixes.
      luci_auth_group = subp.add_mutually_exclusive_group()
      luci_auth_group.add_argument(
          '--luci-auth',
          action='store_true',
          help='Run isolated commands under `luci-auth context`.')
      luci_auth_group.add_argument(
          '--no-luci-auth',
          action='store_false',
          dest='luci_auth',
          help='Do not run isolated commands under `luci-auth context`.')

    parser = argparse.ArgumentParser(
      prog='mb', description='mb (meta-build) is a python wrapper around GN. '
                             'See the user guide in '
                             '//tools/mb/docs/user_guide.md for detailed usage '
                             'instructions.')

    subps = parser.add_subparsers()

    subp = subps.add_parser('analyze',
                            description='Analyze whether changes to a set of '
                                        'files will cause a set of binaries to '
                                        'be rebuilt.')
    AddCommonOptions(subp)
    subp.add_argument('path',
                      help='path build was generated into.')
    subp.add_argument('input_path',
                      help='path to a file containing the input arguments '
                           'as a JSON object.')
    subp.add_argument('output_path',
                      help='path to a file containing the output arguments '
                           'as a JSON object.')
    subp.add_argument('--json-output',
                      help='Write errors to json.output')
    subp.set_defaults(func=self.CmdAnalyze)

    subp = subps.add_parser('export',
                            description='Print out the expanded configuration '
                            'for each builder as a JSON object.')
    AddExpansionOptions(subp)
    subp.set_defaults(func=self.CmdExport)

    subp = subps.add_parser('get-swarming-command',
                            description='Get the command needed to run the '
                            'binary under swarming')
    AddCommonOptions(subp)
    subp.add_argument('--no-build',
                      dest='build',
                      default=True,
                      action='store_false',
                      help='Do not build, just isolate')
    subp.add_argument('--as-list',
                      action='store_true',
                      help='return the command line as a JSON-formatted '
                      'list of strings instead of single string')
    subp.add_argument('path',
                      help=('path to generate build into (or use).'
                            ' This can be either a regular path or a '
                            'GN-style source-relative path like '
                            '//out/Default.'))
    subp.add_argument('target', help='ninja target to build and run')
    subp.set_defaults(func=self.CmdGetSwarmingCommand)

    subp = subps.add_parser('train',
                            description='Writes the expanded configuration '
                            'for each builder as JSON files to a configured '
                            'directory.')
    subp.add_argument('-f',
                      '--config-file',
                      metavar='PATH',
                      help='path to config file (default is mb_config.pyl')
    subp.add_argument('--expectations-dir',
                      metavar='PATH',
                      help='path to dir containing expectation files')
    subp.add_argument('-n',
                      '--dryrun',
                      action='store_true',
                      help='Do a dry run (i.e., do nothing, just print '
                      'the commands that will run)')
    subp.add_argument('-v',
                      '--verbose',
                      action='store_true',
                      help='verbose logging')
    subp.set_defaults(func=self.CmdTrain)

    subp = subps.add_parser('gen',
                            description='Generate a new set of build files.')
    AddCommonOptions(subp)
    subp.add_argument('--swarming-targets-file',
                      help='generates runtime dependencies for targets listed '
                           'in file as .isolate and .isolated.gen.json files. '
                           'Targets should be listed by name, separated by '
                           'newline.')
    subp.add_argument('--json-output',
                      help='Write errors to json.output')
    subp.add_argument('--rts-target-change-recall',
                      type=float,
                      help='how much safety is needed when selecting tests. '
                      '0.0 is the lowest and 1.0 is the highest')
    subp.add_argument('path',
                      help='path to generate build into')
    subp.set_defaults(func=self.CmdGen)

    subp = subps.add_parser('isolate-everything',
                            description='Generates a .isolate for all targets. '
                                        'Requires that mb.py gen has already '
                                        'been run.')
    AddCommonOptions(subp)
    subp.set_defaults(func=self.CmdIsolateEverything)
    subp.add_argument('path',
                      help='path build was generated into')
    subp = subps.add_parser('isolate',
                            description='Generate the .isolate files for a '
                                        'given binary.')
    AddCommonOptions(subp)
    subp.add_argument('--no-build', dest='build', default=True,
                      action='store_false',
                      help='Do not build, just isolate')
    subp.add_argument('-j', '--jobs', type=int,
                      help='Number of jobs to pass to ninja')
    subp.add_argument('path',
                      help='path build was generated into')
    subp.add_argument('target',
                      help='ninja target to generate the isolate for')
    subp.set_defaults(func=self.CmdIsolate)

    subp = subps.add_parser('lookup',
                            description='Look up the command for a given '
                                        'config or builder.')
    AddCommonOptions(subp)
    subp.add_argument('--quiet', default=False, action='store_true',
                      help='Print out just the arguments, '
                           'do not emulate the output of the gen subcommand.')
    subp.add_argument('--recursive', default=False, action='store_true',
                      help='Lookup arguments from imported files, '
                           'implies --quiet')
    subp.set_defaults(func=self.CmdLookup)

    subp = subps.add_parser('try',
                            description='Try your change on a remote builder')
    AddCommonOptions(subp)
    subp.add_argument('target',
                      help='ninja target to build and run')
    subp.add_argument('--force', default=False, action='store_true',
                      help='Force the job to run. Ignores local checkout state;'
                      ' by default, the tool doesn\'t trigger jobs if there are'
                      ' local changes which are not present on Gerrit.')
    subp.set_defaults(func=self.CmdTry)

    subp = subps.add_parser(
      'run', formatter_class=argparse.RawDescriptionHelpFormatter)
    subp.description = (
        'Build, isolate, and run the given binary with the command line\n'
        'listed in the isolate. You may pass extra arguments after the\n'
        'target; use "--" if the extra arguments need to include switches.\n'
        '\n'
        'Examples:\n'
        '\n'
        '  % tools/mb/mb.py run -m chromium.linux -b "Linux Builder" \\\n'
        '    //out/Default content_browsertests\n'
        '\n'
        '  % tools/mb/mb.py run out/Default content_browsertests\n'
        '\n'
        '  % tools/mb/mb.py run out/Default content_browsertests -- \\\n'
        '    --test-launcher-retry-limit=0'
        '\n'
    )
    AddCommonOptions(subp)
    subp.add_argument('-j', '--jobs', type=int,
                      help='Number of jobs to pass to ninja')
    subp.add_argument('--no-build', dest='build', default=True,
                      action='store_false',
                      help='Do not build, just isolate and run')
    subp.add_argument('path',
                      help=('path to generate build into (or use).'
                            ' This can be either a regular path or a '
                            'GN-style source-relative path like '
                            '//out/Default.'))
    subp.add_argument('-s', '--swarmed', action='store_true',
                      help='Run under swarming with the default dimensions')
    subp.add_argument('-d', '--dimension', default=[], action='append', nargs=2,
                      dest='dimensions', metavar='FOO bar',
                      help='dimension to filter on')
    subp.add_argument('--internal',
                      action='store_true',
                      help=('Run under the internal swarming server '
                            '(chrome-swarming) instead of the public server '
                            '(chromium-swarm).'))
    subp.add_argument('--tags', default=[], action='append', metavar='FOO:BAR',
                      help='Tags to assign to the swarming task')
    subp.add_argument('--no-default-dimensions', action='store_false',
                      dest='default_dimensions', default=True,
                      help='Do not automatically add dimensions to the task')
    subp.add_argument('target',
                      help='ninja target to build and run')
    subp.add_argument('extra_args', nargs='*',
                      help=('extra args to pass to the isolate to run. Use '
                            '"--" as the first arg if you need to pass '
                            'switches'))
    subp.set_defaults(func=self.CmdRun)

    subp = subps.add_parser('validate',
                            description='Validate the config file.')
    AddExpansionOptions(subp)
    subp.add_argument('--expectations-dir',
                      metavar='PATH',
                      help='path to dir containing expectation files')
    subp.add_argument('--skip-dcheck-check',
                      help='Skip check for dcheck_always_on.',
                      action='store_true')
    subp.set_defaults(func=self.CmdValidate)

    subp = subps.add_parser('zip',
                            description='Generate a .zip containing the files '
                                        'needed for a given binary.')
    AddCommonOptions(subp)
    subp.add_argument('--no-build', dest='build', default=True,
                      action='store_false',
                      help='Do not build, just isolate')
    subp.add_argument('-j', '--jobs', type=int,
                      help='Number of jobs to pass to ninja')
    subp.add_argument('path',
                      help='path build was generated into')
    subp.add_argument('target',
                      help='ninja target to generate the isolate for')
    subp.add_argument('zip_path',
                      help='path to zip file to create')
    subp.set_defaults(func=self.CmdZip)

    subp = subps.add_parser('help',
                            help='Get help on a subcommand.')
    subp.add_argument(nargs='?', action='store', dest='subcommand',
                      help='The command to get help for.')
    subp.set_defaults(func=self.CmdHelp)

    self.args = parser.parse_args(argv)

  def DumpInputFiles(self):

    def DumpContentsOfFilePassedTo(arg_name, path):
      if path and self.Exists(path):
        self.Print("\n# To recreate the file passed to %s:" % arg_name)
        self.Print("%% cat > %s <<EOF" % path)
        contents = self.ReadFile(path)
        self.Print(contents)
        self.Print("EOF\n%\n")

    if getattr(self.args, 'input_path', None):
      DumpContentsOfFilePassedTo(
          'argv[0] (input_path)', self.args.input_path)
    if getattr(self.args, 'swarming_targets_file', None):
      DumpContentsOfFilePassedTo(
          '--swarming-targets-file', self.args.swarming_targets_file)

  def CmdAnalyze(self):
    vals = self.Lookup()
    return self.RunGNAnalyze(vals)

  def CmdExport(self):
    obj = self._ToJsonish()
    s = json.dumps(obj, sort_keys=True, indent=2, separators=(',', ': '))
    self.Print(s)
    return 0

  def CmdTrain(self):
    expectations_dir = self.args.expectations_dir
    if not self.Exists(expectations_dir):
      self.Print('Expectations dir (%s) does not exist.' % expectations_dir)
      return 1
    # Removing every expectation file then immediately re-generating them will
    # clear out deleted groups.
    for f in self.ListDir(expectations_dir):
      self.RemoveFile(os.path.join(expectations_dir, f))
    obj = self._ToJsonish()
    for builder_group, builder in sorted(obj.items()):
      expectation_file = os.path.join(expectations_dir, builder_group + '.json')
      json_s = json.dumps(builder,
                          indent=2,
                          sort_keys=True,
                          separators=(',', ': '))
      self.WriteFile(expectation_file, json_s)
    return 0

  def RtsSelect(self):
    if self.args.rts == 'rts-ml-chromium':
      model_dir = self.PathJoin(self.chromium_src_dir, 'testing', 'rts',
                                self.args.rts, self._CipdPlatform())
      exe = self.PathJoin(model_dir, self.args.rts)
    else:
      model_dir = self.PathJoin(self.chromium_src_dir, 'testing', 'rts',
                                self._CipdPlatform())
      exe = self.PathJoin(model_dir, self.args.rts)

    if self.platform == 'win32':
      exe += '.exe'

    args = [
        exe,
        'select',
        '-gen-inverse',
        '-model-dir', model_dir, \
        '-out', self.PathJoin(self.ToAbsPath(self.args.path), self.rts_out_dir),
        '-checkout', self.chromium_src_dir,
    ]
    if self.args.rts_target_change_recall:
      if (self.args.rts_target_change_recall < 0
          or self.args.rts_target_change_recall > 1):
        self.WriteFailureAndRaise(
            'rts-target-change-recall must be between (0 and 1]', None)
      args += ['-target-change-recall', str(self.args.rts_target_change_recall)]

    ret, _, err = self.Run(args, force_verbose=True)
    if ret != 0:
      self.WriteFailureAndRaise(err, None)

  def CmdGen(self):
    if self.args.rts:
      self.RtsSelect()
    vals = self.Lookup()
    return self.RunGNGen(vals)

  def CmdGetSwarmingCommand(self):
    vals = self.GetConfig()
    command, _ = self.GetSwarmingCommand(self.args.target, vals)
    if self.args.as_list:
      self.Print(json.dumps(command))
    else:
      self.Print(' '.join(command))
    return 0

  def CmdIsolateEverything(self):
    vals = self.Lookup()
    return self.RunGNGenAllIsolates(vals)

  def CmdHelp(self):
    if self.args.subcommand:
      self.ParseArgs([self.args.subcommand, '--help'])
    else:
      self.ParseArgs(['--help'])

  def CmdIsolate(self):
    vals = self.GetConfig()
    if not vals:
      return 1
    if self.args.build:
      ret = self.Build(self.args.target)
      if ret != 0:
        return ret
    return self.RunGNIsolate(vals)

  def CmdLookup(self):
    vals = self.Lookup()
    _, gn_args = self.GNArgs(vals, expand_imports=self.args.recursive)
    if self.args.quiet or self.args.recursive:
      self.Print(gn_args, end='')
    else:
      cmd = self.GNCmd('gen', '_path_')
      self.Print('\nWriting """\\\n%s""" to _path_/args.gn.\n' % gn_args)
      self.PrintCmd(cmd)
    return 0

  def CmdTry(self):
    ninja_target = self.args.target
    if ninja_target.startswith('//'):
      self.Print("Expected a ninja target like base_unittests, got %s" % (
        ninja_target))
      return 1

    _, out, _ = self.Run(['git', 'cl', 'diff', '--stat'], force_verbose=False)
    if out:
      self.Print("Your checkout appears to local changes which are not uploaded"
                 " to Gerrit. Changes must be committed and uploaded to Gerrit"
                 " to be tested using this tool.")
      if not self.args.force:
        return 1

    json_path = self.PathJoin(self.chromium_src_dir, 'out.json')
    try:
      ret, out, err = self.Run(
        ['git', 'cl', 'issue', '--json=out.json'], force_verbose=False)
      if ret != 0:
        self.Print(
          "Unable to fetch current issue. Output and error:\n%s\n%s" % (
            out, err
        ))
        return ret
      with open(json_path) as f:
        issue_data = json.load(f)
    finally:
      if self.Exists(json_path):
        os.unlink(json_path)

    if not issue_data['issue']:
      self.Print("Missing issue data. Upload your CL to Gerrit and try again.")
      return 1

    class LedException(Exception):
      pass

    def run_cmd(previous_res, cmd):
      if self.args.verbose:
        self.Print(('| ' if previous_res else '') + ' '.join(cmd))

      res, out, err = self.Call(cmd, input=previous_res)
      if res != 0:
        self.Print("Err while running '%s'. Output:\n%s\nstderr:\n%s" % (
          ' '.join(cmd), out, err))
        raise LedException()
      return out

    try:
      result = LedResult(None, run_cmd).then(
        # TODO(martiniss): maybe don't always assume the bucket?
        'led', 'get-builder', 'luci.chromium.try:%s' % self.args.builder).then(
        'led', 'edit', '-r', 'chromium_trybot_experimental',
          '-p', 'tests=["%s"]' % ninja_target).then(
        'led', 'edit-system', '--tag=purpose:user-debug-mb-try').then(
        'led', 'edit-cr-cl', issue_data['issue_url']).then(
        'led', 'launch').result
    except LedException:
      self.Print("If this is an unexpected error message, please file a bug"
                 " with https://goto.google.com/mb-try-bug")
      raise

    swarming_data = json.loads(result)['swarming']
    self.Print("Launched task at https://%s/task?id=%s" % (
      swarming_data['host_name'], swarming_data['task_id']))

  def CmdRun(self):
    vals = self.GetConfig()
    if not vals:
      return 1
    if self.args.build:
      self.Print('')
      ret = self.Build(self.args.target)
      if ret:
        return ret

    self.Print('')
    ret = self.RunGNIsolate(vals)
    if ret:
      return ret

    self.Print('')
    if self.args.swarmed:
      cmd, _ = self.GetSwarmingCommand(self.args.target, vals)
      return self._RunUnderSwarming(self.args.path, self.args.target, cmd,
                                    self.args.internal)
    return self._RunLocallyIsolated(self.args.path, self.args.target)

  def CmdZip(self):
    ret = self.CmdIsolate()
    if ret:
      return ret

    zip_dir = None
    try:
      zip_dir = self.TempDir()
      remap_cmd = [
          self.PathJoin(self.chromium_src_dir, 'tools', 'luci-go',
                        self.isolate_exe), 'remap', '-i',
          self.PathJoin(self.args.path, self.args.target + '.isolate'),
          '-outdir', zip_dir
      ]
      ret, _, _ = self.Run(remap_cmd)
      if ret:
        return ret

      zip_path = self.args.zip_path
      with zipfile.ZipFile(
          zip_path, 'w', zipfile.ZIP_DEFLATED, allowZip64=True) as fp:
        for root, _, files in os.walk(zip_dir):
          for filename in files:
            path = self.PathJoin(root, filename)
            fp.write(path, self.RelPath(path, zip_dir))
      return 0
    finally:
      if zip_dir:
        self.RemoveDirectory(zip_dir)

  def _RunUnderSwarming(self, build_dir, target, isolate_cmd, internal):
    if internal:
      cas_instance = 'chrome-swarming'
      swarming_server = 'chrome-swarming.appspot.com'
    else:
      cas_instance = 'chromium-swarm'
      swarming_server = 'chromium-swarm.appspot.com'
    # TODO(dpranke): Look up the information for the target in
    # the //testing/buildbot.json file, if possible, so that we
    # can determine the isolate target, command line, and additional
    # swarming parameters, if possible.
    #
    # TODO(dpranke): Also, add support for sharding and merging results.
    dimensions = []
    for k, v in self._DefaultDimensions() + self.args.dimensions:
      dimensions += ['-d', '%s=%s' % (k, v)]

    archive_json_path = self.ToSrcRelPath(
        '%s/%s.archive.json' % (build_dir, target))
    cmd = [
        self.PathJoin(self.chromium_src_dir, 'tools', 'luci-go',
                      self.isolate_exe),
        'archive',
        '-i',
        self.ToSrcRelPath('%s/%s.isolate' % (build_dir, target)),
        '-cas-instance',
        cas_instance,
        '-dump-json',
        archive_json_path,
    ]

    # Talking to the isolateserver may fail because we're not logged in.
    # We trap the command explicitly and rewrite the error output so that
    # the error message is actually correct for a Chromium check out.
    self.PrintCmd(cmd)
    ret, out, _ = self.Run(cmd, force_verbose=False)
    if ret:
      self.Print('  -> returned %d' % ret)
      if out:
        self.Print(out, end='')
      return ret

    try:
      archive_hashes = json.loads(self.ReadFile(archive_json_path))
    except Exception:
      self.Print(
          'Failed to read JSON file "%s"' % archive_json_path, file=sys.stderr)
      return 1
    try:
      cas_digest = archive_hashes[target]
    except Exception:
      self.Print(
          'Cannot find hash for "%s" in "%s", file content: %s' %
          (target, archive_json_path, archive_hashes),
          file=sys.stderr)
      return 1

    tags = ['-tag=%s' % tag for tag in self.args.tags]

    try:
      json_dir = self.TempDir()
      json_file = self.PathJoin(json_dir, 'task.json')
      cmd = [
          self.PathJoin('tools', 'luci-go', 'swarming'),
          'trigger',
          '-digest',
          cas_digest,
          '-server',
          swarming_server,
          '-tag=purpose:user-debug-mb',
          '-relative-cwd',
          self.ToSrcRelPath(build_dir),
          '-dump-json',
          json_file,
      ]
      if internal:
        cmd += [
            '--realm',
            'chrome:try',
        ]
      cmd += tags + dimensions + ['--'] + list(isolate_cmd)
      if self.args.extra_args:
        cmd += self.args.extra_args
      self.Print('')
      ret, _, _ = self.Run(cmd, force_verbose=True, capture_output=False)
      if ret:
        return ret
      task_json = self.ReadFile(json_file)
      task_id = json.loads(task_json)["tasks"][0]['task_id']
      collect_output = self.PathJoin(json_dir, 'collect_output.json')
      cmd = [
          self.PathJoin('tools', 'luci-go', 'swarming'),
          'collect',
          '-server',
          swarming_server,
          '-task-output-stdout=console',
          '-task-summary-json',
          collect_output,
          task_id,
      ]
      ret, _, _ = self.Run(cmd, force_verbose=True, capture_output=False)
      if ret != 0:
        return ret
      collect_json = json.loads(self.ReadFile(collect_output))
      # The exit_code field might not be included if the task was successful.
      ret = int(
          collect_json.get(task_id, {}).get('results', {}).get('exit_code', 0))
    finally:
      if json_dir:
        self.RemoveDirectory(json_dir)
    return ret

  def _RunLocallyIsolated(self, build_dir, target):
    cmd = [
        self.PathJoin(self.chromium_src_dir, 'tools', 'luci-go',
                      self.isolate_exe),
        'run',
        '-i',
        self.ToSrcRelPath('%s/%s.isolate' % (build_dir, target)),
    ]
    if self.args.extra_args:
      cmd += ['--'] + self.args.extra_args
    ret, _, _ = self.Run(cmd, force_verbose=True, capture_output=False)
    return ret

  def _DefaultDimensions(self):
    if not self.args.default_dimensions:
      return []

    # This code is naive and just picks reasonable defaults per platform.
    if self.platform == 'darwin':
      os_dim = ('os', 'Mac-10.13')
    elif self.platform.startswith('linux'):
      os_dim = ('os', 'Ubuntu-16.04')
    elif self.platform == 'win32':
      os_dim = ('os', 'Windows-10')
    else:
      raise MBErr('unrecognized platform string "%s"' % self.platform)

    return [('pool', 'chromium.tests'),
            ('cpu', 'x86-64'),
            os_dim]

  def _ToJsonish(self):
    """Dumps the config file into a json-friendly expanded dict.

    Returns:
      A dict with builder group -> builder -> all GN args mapping.
    """
    self.ReadConfigFile(self.args.config_file)
    obj = {}
    for builder_group, builders in self.builder_groups.items():
      obj[builder_group] = {}
      for builder in builders:
        config = self.builder_groups[builder_group][builder]
        if not config:
          continue

        def flatten(config):
          flattened_config = FlattenConfig(self.configs, self.mixins, config)
          if flattened_config['gn_args'] == 'error':
            return None
          args = {'gn_args': gn_helpers.FromGNArgs(flattened_config['gn_args'])}
          if flattened_config.get('args_file'):
            args['args_file'] = flattened_config['args_file']
          return args

        if isinstance(config, dict):
          # This is a 'phased' builder. Each key in the config is a different
          # phase of the builder.
          args = {}
          for k, v in config.items():
            flattened = flatten(v)
            if flattened is None:
              continue
            args[k] = flattened
        elif config.startswith('//'):
          args = config
        else:
          args = flatten(config)
          if args is None:
            continue
        obj[builder_group][builder] = args

    return obj

  def CmdValidate(self, print_ok=True):
    errs = []

    self.ReadConfigFile(self.args.config_file)

    # Build a list of all of the configs referenced by builders.
    all_configs = validation.GetAllConfigs(self.builder_groups)

    # Check that every referenced args file or config actually exists.
    for config, loc in all_configs.items():
      if config.startswith('//'):
        if not self.Exists(self.ToAbsPath(config)):
          errs.append('Unknown args file "%s" referenced from "%s".' %
                      (config, loc))
      elif not config in self.configs:
        errs.append('Unknown config "%s" referenced from "%s".' %
                    (config, loc))

    # Check that every config and mixin is referenced.
    validation.CheckAllConfigsAndMixinsReferenced(errs, all_configs,
                                                  self.configs, self.mixins)

    if self.args.config_file == self.default_config:
      validation.EnsureNoProprietaryMixins(errs, self.builder_groups,
                                           self.configs, self.mixins)

    validation.CheckDuplicateConfigs(errs, self.configs, self.mixins,
                                     self.builder_groups, FlattenConfig)

    if not self.args.skip_dcheck_check:
      self._ValidateEach(errs, validation.CheckDebugDCheckOrOfficial)

    if errs:
      raise MBErr(('mb config file %s has problems:\n  ' %
                   self.args.config_file) + '\n  '.join(errs))

    expectations_dir = self.args.expectations_dir
    # TODO(crbug.com/1117577): Force all versions of mb_config.pyl to have
    # expectations. For now, just ignore those that don't have them.
    if self.Exists(expectations_dir):
      jsonish_blob = self._ToJsonish()
      if not validation.CheckExpectations(self, jsonish_blob, expectations_dir):
        raise MBErr("Expectations out of date. Please run 'mb.py train'.")

    validation.CheckKeyOrdering(errs, self.builder_groups, self.configs,
                                self.mixins)
    if errs:
      raise MBErr('mb config file not sorted:\n' + '\n'.join(errs))

    if print_ok:
      self.Print('mb config file %s looks ok.' % self.args.config_file)
    return 0

  def _ValidateEach(self, errs, validate):
    """Checks a validate function against every builder config.

    This loops over all the builders in the config file, invoking the
    validate function against the full set of GN args. Any errors found
    should be appended to the errs list passed in; the validation
    function signature is

        validate(errs:list, gn_args:dict, builder_group:str, builder:str,
                 phase:(str|None))"""

    for builder_group, builders in self.builder_groups.items():
      for builder, config in builders.items():
        if isinstance(config, dict):
          for phase, phase_config in config.items():
            vals = FlattenConfig(self.configs, self.mixins, phase_config)
            if vals['gn_args'] == 'error':
              continue
            try:
              parsed_gn_args, _ = self.GNArgs(vals, expand_imports=True)
            except IOError:
              # The builder must use an args file that was not checked out or
              # generated, so we should just ignore it.
              parsed_gn_args, _ = self.GNArgs(vals, expand_imports=False)
            validate(errs, parsed_gn_args, builder_group, builder, phase)
        else:
          vals = FlattenConfig(self.configs, self.mixins, config)
          if vals['gn_args'] == 'error':
            continue
          try:
            parsed_gn_args, _ = self.GNArgs(vals, expand_imports=True)
          except IOError:
            # The builder must use an args file that was not checked out or
            # generated, so we should just ignore it.
            parsed_gn_args, _ = self.GNArgs(vals, expand_imports=False)
          validate(errs, parsed_gn_args, builder_group, builder, phase=None)

  def GetConfig(self):
    build_dir = self.args.path

    vals = DefaultVals()
    if self.args.builder or self.args.builder_group or self.args.config:
      vals = self.Lookup()
      # Re-run gn gen in order to ensure the config is consistent with the
      # build dir.
      self.RunGNGen(vals)
      return vals

    toolchain_path = self.PathJoin(self.ToAbsPath(build_dir),
                                   'toolchain.ninja')
    if not self.Exists(toolchain_path):
      self.Print('Must either specify a path to an existing GN build dir '
                 'or pass in a -m/-b pair or a -c flag to specify the '
                 'configuration')
      return {}

    vals['gn_args'] = self.GNArgsFromDir(build_dir)
    return vals

  def GNArgsFromDir(self, build_dir):
    args_contents = ""
    gn_args_path = self.PathJoin(self.ToAbsPath(build_dir), 'args.gn')
    if self.Exists(gn_args_path):
      args_contents = self.ReadFile(gn_args_path)

    # Handle any .gni file imports, e.g. the ones used by CrOS. This should
    # be automatically handled by gn_helpers.FromGNArgs (via its call to
    # gn_helpers.GNValueParser.ReplaceImports), but that currently breaks
    # mb_unittest since it mocks out file reads itself instead of using
    # pyfakefs. This results in gn_helpers trying to read a non-existent file.
    # The implementation of ReplaceImports here can be removed once the
    # unittests use pyfakefs.
    def ReplaceImports(input_contents):
      output_contents = ''
      for l in input_contents.splitlines(True):
        if not l.strip().startswith('#') and 'import(' in l:
          import_file = l.split('"', 2)[1]
          import_file = self.ToAbsPath(import_file)
          imported_contents = self.ReadFile(import_file)
          output_contents += ReplaceImports(imported_contents) + '\n'
        else:
          output_contents += l
      return output_contents

    args_contents = ReplaceImports(args_contents)
    args_dict = gn_helpers.FromGNArgs(args_contents)
    # Re-add the quotes around strings so they show up as they would in the
    # args.gn file.
    for k, v in args_dict.items():
      if isinstance(v, str):
        args_dict[k] = '"%s"' % v
    return ' '.join(['%s=%s' % (k, v) for (k, v) in args_dict.items()])

  def Lookup(self):
    self.ReadConfigFile(self.args.config_file)
    try:
      config = self.ConfigFromArgs()
    except MBErr as e:
      # TODO(crbug.com/912681) While iOS bots are migrated to use the
      # Chromium recipe, we want to ensure that we're checking MB's
      # configurations first before going to iOS.
      # This is to be removed once the migration is complete.
      vals = self.ReadIOSBotConfig()
      if not vals:
        raise e
      return vals

    # TODO(crbug.com/912681) Some iOS bots have a definition, with ios_error
    # as an indicator that it's incorrect. We utilize this to check the
    # iOS JSON instead, and error out if there exists no definition at all.
    # This is to be removed once the migration is complete.
    if config == 'ios_error':
      vals = self.ReadIOSBotConfig()
      if not vals:
        raise MBErr('No iOS definition was found. Please ensure there is a '
                    'definition for the given iOS bot under '
                    'mb_config.pyl or a JSON file definition under '
                    '//ios/build/bots.')
      return vals

    if config.startswith('//'):
      if not self.Exists(self.ToAbsPath(config)):
        raise MBErr('args file "%s" not found' % config)
      vals = DefaultVals()
      vals['args_file'] = config
    else:
      if not config in self.configs:
        raise MBErr(
            'Config "%s" not found in %s' % (config, self.args.config_file))
      vals = FlattenConfig(self.configs, self.mixins, config)
    return vals

  def ReadIOSBotConfig(self):
    if not self.args.builder_group or not self.args.builder:
      return {}
    path = self.PathJoin(self.chromium_src_dir, 'ios', 'build', 'bots',
                         self.args.builder_group, self.args.builder + '.json')
    if not self.Exists(path):
      return {}

    contents = json.loads(self.ReadFile(path))
    gn_args = ' '.join(contents.get('gn_args', []))

    vals = DefaultVals()
    vals['gn_args'] = gn_args
    return vals

  def ReadConfigFile(self, config_file):
    if not self.Exists(config_file):
      raise MBErr('config file not found at %s' % config_file)

    try:
      contents = ast.literal_eval(self.ReadFile(config_file))
    except SyntaxError as e:
      raise MBErr('Failed to parse config file "%s": %s' %
                  (config_file, e)) from e

    self.configs = contents['configs']
    self.mixins = contents['mixins']
    self.builder_groups = contents.get('builder_groups')
    self.public_artifact_builders = contents.get('public_artifact_builders')

  def ReadIsolateMap(self):
    if not self.args.isolate_map_files:
      self.args.isolate_map_files = [self.default_isolate_map]

    for f in self.args.isolate_map_files:
      if not self.Exists(f):
        raise MBErr('isolate map file not found at %s' % f)
    isolate_maps = {}
    for isolate_map in self.args.isolate_map_files:
      try:
        isolate_map = ast.literal_eval(self.ReadFile(isolate_map))
        duplicates = set(isolate_map).intersection(isolate_maps)
        if duplicates:
          raise MBErr(
              'Duplicate targets in isolate map files: %s.' %
              ', '.join(duplicates))
        isolate_maps.update(isolate_map)
      except SyntaxError as e:
        raise MBErr('Failed to parse isolate map file "%s": %s' %
                    (isolate_map, e)) from e
    return isolate_maps

  def ConfigFromArgs(self):
    if self.args.config:
      if self.args.builder_group or self.args.builder:
        raise MBErr('Can not specific both -c/--config and --builder-group '
                    'or -b/--builder')

      return self.args.config

    if not self.args.builder_group or not self.args.builder:
      raise MBErr('Must specify either -c/--config or '
                  '(--builder-group and -b/--builder)')

    if not self.args.builder_group in self.builder_groups:
      raise MBErr('Builder group name "%s" not found in "%s"' %
                  (self.args.builder_group, self.args.config_file))

    if not self.args.builder in self.builder_groups[self.args.builder_group]:
      raise MBErr('Builder name "%s"  not found under groups[%s] in "%s"' %
                  (self.args.builder, self.args.builder_group,
                   self.args.config_file))

    config = self.builder_groups[self.args.builder_group][self.args.builder]
    if isinstance(config, dict):
      if self.args.phase is None:
        raise MBErr('Must specify a build --phase for %s on %s' %
                    (self.args.builder, self.args.builder_group))
      phase = str(self.args.phase)
      if phase not in config:
        raise MBErr('Phase %s doesn\'t exist for %s on %s' %
                    (phase, self.args.builder, self.args.builder_group))
      return config[phase]

    if self.args.phase is not None:
      raise MBErr('Must not specify a build --phase for %s on %s' %
                  (self.args.builder, self.args.builder_group))
    return config

  def RunGNGen(self, vals, compute_inputs_for_analyze=False, check=True):
    build_dir = self.args.path

    if check:
      cmd = self.GNCmd('gen', build_dir, '--check')
    else:
      cmd = self.GNCmd('gen', build_dir)
    _, gn_args = self.GNArgs(vals)
    if compute_inputs_for_analyze:
      gn_args += ' compute_inputs_for_analyze=true'

    # Since GN hasn't run yet, the build directory may not even exist.
    self.MaybeMakeDirectory(self.ToAbsPath(build_dir))

    gn_args_path = self.ToAbsPath(build_dir, 'args.gn')
    self.WriteFile(gn_args_path, gn_args, force_verbose=True)

    if getattr(self.args, 'swarming_targets_file', None):
      # We need GN to generate the list of runtime dependencies for
      # the compile targets listed (one per line) in the file so
      # we can run them via swarming. We use gn_isolate_map.pyl to convert
      # the compile targets to the matching GN labels.
      path = self.args.swarming_targets_file
      if not self.Exists(path):
        self.WriteFailureAndRaise('"%s" does not exist' % path,
                                  output_path=None)
      contents = self.ReadFile(path)
      isolate_targets = set(contents.splitlines())

      isolate_map = self.ReadIsolateMap()
      self.RemovePossiblyStaleRuntimeDepsFiles(vals, isolate_targets,
                                               isolate_map, build_dir)

      err, labels = self.MapTargetsToLabels(isolate_map, isolate_targets)
      if err:
        raise MBErr(err)

      gn_runtime_deps_path = self.ToAbsPath(build_dir, 'runtime_deps')
      self.WriteFile(gn_runtime_deps_path, '\n'.join(labels) + '\n')
      cmd.append('--runtime-deps-list-file=%s' % gn_runtime_deps_path)

    ret, output, _ = self.Run(cmd)
    if ret != 0:
      if self.args.json_output:
        # write errors to json.output
        self.WriteJSON({'output': output}, self.args.json_output)
      # If `gn gen` failed, we should exit early rather than trying to
      # generate isolates. Run() will have already logged any error output.
      self.Print('GN gen failed: %d' % ret)
      return ret

    if getattr(self.args, 'swarming_targets_file', None):
      ret = self.GenerateIsolates(vals, isolate_targets, isolate_map, build_dir)

    return ret

  def RunGNGenAllIsolates(self, vals):
    """
    This command generates all .isolate files.

    This command assumes that "mb.py gen" has already been run, as it relies on
    "gn ls" to fetch all gn targets. If uses that output, combined with the
    isolate_map, to determine all isolates that can be generated for the current
    gn configuration.
    """
    build_dir = self.args.path
    ret, output, _ = self.Run(self.GNCmd('ls', build_dir),
                              force_verbose=False)
    if ret != 0:
      # If `gn ls` failed, we should exit early rather than trying to
      # generate isolates.
      self.Print('GN ls failed: %d' % ret)
      return ret

    # Create a reverse map from isolate label to isolate dict.
    isolate_map = self.ReadIsolateMap()
    isolate_dict_map = {}
    for key, isolate_dict in isolate_map.items():
      isolate_dict_map[isolate_dict['label']] = isolate_dict
      isolate_dict_map[isolate_dict['label']]['isolate_key'] = key

    runtime_deps = []

    isolate_targets = []
    # For every GN target, look up the isolate dict.
    for line in output.splitlines():
      target = line.strip()
      if target in isolate_dict_map:
        if isolate_dict_map[target]['type'] == 'additional_compile_target':
          # By definition, additional_compile_targets are not tests, so we
          # shouldn't generate isolates for them.
          continue

        isolate_targets.append(isolate_dict_map[target]['isolate_key'])
        runtime_deps.append(target)

    self.RemovePossiblyStaleRuntimeDepsFiles(vals, isolate_targets,
                                             isolate_map, build_dir)

    gn_runtime_deps_path = self.ToAbsPath(build_dir, 'runtime_deps')
    self.WriteFile(gn_runtime_deps_path, '\n'.join(runtime_deps) + '\n')
    cmd = self.GNCmd('gen', build_dir)
    cmd.append('--runtime-deps-list-file=%s' % gn_runtime_deps_path)
    self.Run(cmd)

    return self.GenerateIsolates(vals, isolate_targets, isolate_map, build_dir)

  def RemovePossiblyStaleRuntimeDepsFiles(self, vals, targets, isolate_map,
                                          build_dir):
    # TODO(crbug.com/932700): Because `gn gen --runtime-deps-list-file`
    # puts the runtime_deps file in different locations based on the actual
    # type of a target, we may end up with multiple possible runtime_deps
    # files in a given build directory, where some of the entries might be
    # stale (since we might be reusing an existing build directory).
    #
    # We need to be able to get the right one reliably; you might think
    # we can just pick the newest file, but because GN won't update timestamps
    # if the contents of the files change, an older runtime_deps
    # file might actually be the one we should use over a newer one (see
    # crbug.com/932387 for a more complete explanation and example).
    #
    # In order to avoid this, we need to delete any possible runtime_deps
    # files *prior* to running GN. As long as the files aren't actually
    # needed during the build, this hopefully will not cause unnecessary
    # build work, and so it should be safe.
    #
    # Ultimately, we should just make sure we get the runtime_deps files
    # in predictable locations so we don't have this issue at all, and
    # that's what crbug.com/932700 is for.
    possible_rpaths = self.PossibleRuntimeDepsPaths(vals, targets, isolate_map)
    for rpaths in possible_rpaths.values():
      for rpath in rpaths:
        path = self.ToAbsPath(build_dir, rpath)
        if self.Exists(path):
          self.RemoveFile(path)

  def _FilterOutUnneededSkylabDeps(self, deps):
    """Filter out the runtime dependencies not used by Skylab.

    Skylab is CrOS infra facilities for us to run hardware tests. These files
    may appear in the test target's runtime_deps but unnecessary for our tests
    to execute in a CrOS device.
    """
    file_ignore_list = [
        re.compile(r'.*build/android.*'),
        re.compile(r'.*build/chromeos.*'),
        re.compile(r'.*build/cros_cache.*'),
        # The following matches anything under //testing/ that isn't under
        # //testing/buildbot/filters/.
        re.compile(r'.*testing/(?!buildbot/filters).*'),
        re.compile(r'.*third_party/chromite.*'),
        # No test target should rely on files in [output_dir]/gen.
        re.compile(r'^gen/.*'),
    ]
    return [f for f in deps if not any(r.match(f) for r in file_ignore_list)]

  def _DedupDependencies(self, deps):
    """Remove the deps already contained by other paths."""

    def _add(root, path):
      cur = path.popleft()
      # Only continue the recursion if the path has child nodes
      # AND the current node is not ended by other existing paths.
      if path and root.get(cur) != {}:
        return _add(root.setdefault(cur, {}), path)
      # Cut this path, because child nodes are already included.
      root[cur] = {}
      return root

    def _list(root, prefix, res):
      for k, v in root.items():
        if v == {}:
          res.append('%s/%s' % (prefix, k))
          continue
        _list(v, '%s/%s' % (prefix, k), res)
      return res

    root = {}
    for d in deps:
      q = collections.deque(d.rstrip('/').split('/'))
      _add(root, q)
    return [p.lstrip('/') for p in _list(root, '', [])]

  def GenerateIsolates(self, vals, ninja_targets, isolate_map, build_dir):
    """
    Generates isolates for a list of ninja targets.

    Ninja targets are transformed to GN targets via isolate_map.

    This function assumes that a previous invocation of "mb.py gen" has
    generated runtime deps for all targets.
    """
    possible_rpaths = self.PossibleRuntimeDepsPaths(vals, ninja_targets,
                                                    isolate_map)

    for target, rpaths in possible_rpaths.items():
      # TODO(crbug.com/932700): We don't know where each .runtime_deps
      # file might be, but assuming we called
      # RemovePossiblyStaleRuntimeDepsFiles prior to calling `gn gen`,
      # there should only be one file.
      found_one = False
      path_to_use = None
      for r in rpaths:
        path = self.ToAbsPath(build_dir, r)
        if self.Exists(path):
          if found_one:
            raise MBErr('Found more than one of %s' % ', '.join(rpaths))
          path_to_use = path
          found_one = True

      if not found_one:
        raise MBErr('Did not find any of %s' % ', '.join(rpaths))

      command, extra_files = self.GetSwarmingCommand(target, vals)
      runtime_deps = self.ReadFile(path_to_use).splitlines()
      runtime_deps = self._DedupDependencies(runtime_deps)
      if 'is_skylab=true' in vals['gn_args']:
        runtime_deps = self._FilterOutUnneededSkylabDeps(runtime_deps)

      canonical_target = target.replace(':','_').replace('/','_')
      ret = self.WriteIsolateFiles(build_dir, command, canonical_target,
                                   runtime_deps, vals, extra_files)
      if ret != 0:
        return ret
    return 0

  def AddFilterFileArg(self, target, build_dir, command, inverted=False):
    filter_file = ('%s_inverted' % target if inverted else target) + '.filter'
    filter_file_path = self.PathJoin(self.rts_out_dir, filter_file)
    abs_filter_file_path = self.ToAbsPath(build_dir, filter_file_path)

    filter_exists = self.Exists(abs_filter_file_path)
    if filter_exists:
      command.append('--test-launcher-filter-file=%s' % filter_file_path)
      self.Print('added RTS filter file to command: %s' % filter_file)

    return filter_exists

  def PossibleRuntimeDepsPaths(self, vals, ninja_targets, isolate_map):
    """Returns a map of targets to possible .runtime_deps paths.

    Each ninja target maps on to a GN label, but depending on the type
    of the GN target, `gn gen --runtime-deps-list-file` will write
    the .runtime_deps files into different locations. Unfortunately, in
    some cases we don't actually know which of multiple locations will
    actually be used, so we return all plausible candidates.

    The paths that are returned are relative to the build directory.
    """

    android = 'target_os="android"' in vals['gn_args']
    ios = 'target_os="ios"' in vals['gn_args']
    fuchsia = 'target_os="fuchsia"' in vals['gn_args']
    win = self.platform == 'win32' or 'target_os="win"' in vals['gn_args']
    possible_runtime_deps_rpaths = {}
    for target in ninja_targets:
      target_type = isolate_map[target]['type']
      label = isolate_map[target]['label']
      stamp_runtime_deps = 'obj/%s.stamp.runtime_deps' % label.replace(':', '/')
      # TODO(https://crbug.com/876065): 'official_tests' use
      # type='additional_compile_target' to isolate tests. This is not the
      # intended use for 'additional_compile_target'.
      if (target_type == 'additional_compile_target' and
          target != 'official_tests'):
        # By definition, additional_compile_targets are not tests, so we
        # shouldn't generate isolates for them.
        raise MBErr('Cannot generate isolate for %s since it is an '
                    'additional_compile_target.' % target)
      if fuchsia or ios or target_type == 'generated_script':
        # iOS and Fuchsia targets end up as groups.
        # generated_script targets are always actions.
        rpaths = [stamp_runtime_deps]
      elif android:
        # Android targets may be either android_apk or executable. The former
        # will result in runtime_deps associated with the stamp file, while the
        # latter will result in runtime_deps associated with the executable.
        label = isolate_map[target]['label']
        rpaths = [
            target + '.runtime_deps',
            stamp_runtime_deps]
      elif (target_type == 'script'
            or isolate_map[target].get('label_type') == 'group'):
        # For script targets, the build target is usually a group,
        # for which gn generates the runtime_deps next to the stamp file
        # for the label, which lives under the obj/ directory, but it may
        # also be an executable.
        label = isolate_map[target]['label']
        rpaths = [stamp_runtime_deps]
        if win:
          rpaths += [ target + '.exe.runtime_deps' ]
        else:
          rpaths += [ target + '.runtime_deps' ]
      elif win:
        rpaths = [target + '.exe.runtime_deps']
      else:
        rpaths = [target + '.runtime_deps']

      possible_runtime_deps_rpaths[target] = rpaths

    return possible_runtime_deps_rpaths

  def RunGNIsolate(self, vals):
    target = self.args.target
    isolate_map = self.ReadIsolateMap()
    err, labels = self.MapTargetsToLabels(isolate_map, [target])
    if err:
      raise MBErr(err)

    label = labels[0]

    build_dir = self.args.path

    command, extra_files = self.GetSwarmingCommand(target, vals)

    # Any warning for an unused arg will get interleaved into the cmd's
    # stdout. When that happens, the isolate step below will fail with an
    # obscure error when it tries processing the lines of the warning. Fail
    # quickly in that case to avoid confusion
    cmd = self.GNCmd('desc', build_dir, label, 'runtime_deps',
                     '--fail-on-unused-args')
    ret, out, _ = self.Call(cmd)
    if ret != 0:
      if out:
        self.Print(out)
      return ret

    runtime_deps = out.splitlines()

    ret = self.WriteIsolateFiles(build_dir, command, target, runtime_deps, vals,
                                 extra_files)
    if ret != 0:
      return ret

    ret, _, _ = self.Run([
        self.PathJoin(self.chromium_src_dir, 'tools', 'luci-go',
                      self.isolate_exe),
        'check',
        '-i',
        self.ToSrcRelPath('%s/%s.isolate' % (build_dir, target)),
    ],
                         capture_output=False)

    return ret

  def WriteIsolateFiles(self, build_dir, command, target, runtime_deps, vals,
                        extra_files):
    isolate_path = self.ToAbsPath(build_dir, target + '.isolate')
    files = sorted(set(runtime_deps + extra_files))

    # Complain if any file is a directory that's inside the build directory,
    # since that makes incremental builds incorrect. See
    # https://crbug.com/912946
    is_android = 'target_os="android"' in vals['gn_args']
    is_cros = ('target_os="chromeos"' in vals['gn_args']
               or 'is_chromeos_device=true' in vals['gn_args'])
    is_msan = 'is_msan=true' in vals['gn_args']
    is_ios = 'target_os="ios"' in vals['gn_args']
    # pylint: disable=consider-using-ternary
    is_mac = ((self.platform == 'darwin' and not is_ios)
              or 'target_os="mac"' in vals['gn_args'])

    err = ''
    for f in files:
      # Skip a few configs that need extra cleanup for now.
      # TODO(https://crbug.com/912946): Fix everything on all platforms and
      # enable check everywhere.
      if is_android:
        break

      # iOS has generated directories in gn data items.
      # Skipping for iOS instead of listing all apps.
      if is_ios:
        break

      # Skip a few existing violations that need to be cleaned up. Each of
      # these will lead to incorrect incremental builds if their directory
      # contents change. Do not add to this list, except for mac bundles until
      # crbug.com/1000667 is fixed.
      # TODO(https://crbug.com/912946): Remove this if statement.
      if ((is_msan and f == 'instrumented_libraries_prebuilt/')
          or f == 'mr_extension/' or  # https://crbug.com/997947
          f.startswith('nacl_test_data/') or
          f.startswith('ppapi_nacl_tests_libs/') or
          (is_cros and f in (  # https://crbug.com/1002509
              'chromevox_test_data/',
              'gen/ui/file_manager/file_manager/',
              'resources/chromeos/',
              'resources/chromeos/accessibility/accessibility_common/',
              'resources/chromeos/accessibility/chromevox/',
              'resources/chromeos/accessibility/select_to_speak/',
              'test_data/chrome/browser/resources/chromeos/accessibility/'
              'accessibility_common/',
              'test_data/chrome/browser/resources/chromeos/accessibility/'
              'chromevox/',
              'test_data/chrome/browser/resources/chromeos/accessibility/'
              'select_to_speak/',
          )) or (is_mac and f in (  # https://crbug.com/1000667
              'Chromium Framework.framework/',
              'Chromium Helper.app/',
              'Chromium.app/',
              'ChromiumUpdater.app/',
              'ChromiumUpdater_test.app/',
              'Content Shell.app/',
              'Google Chrome Framework.framework/',
              'Google Chrome Helper (Alerts).app/',
              'Google Chrome Helper (GPU).app/',
              'Google Chrome Helper (Plugin).app/',
              'Google Chrome Helper (Renderer).app/',
              'Google Chrome Helper.app/',
              'Google Chrome.app/',
              'GoogleUpdater.app/',
              'GoogleUpdater_test.app/',
              'UpdaterTestApp Framework.framework/',
              'UpdaterTestApp.app/',
              'blink_deprecated_test_plugin.plugin/',
              'blink_test_plugin.plugin/',
              'corb_test_plugin.plugin/',
              'obj/tools/grit/brotli_mac_asan_workaround/',
              'ppapi_tests.plugin/',
              'ui_unittests Framework.framework/',
          ))):
        continue

      # This runs before the build, so we can't use isdir(f). But
      # isolate.py luckily requires data directories to end with '/', so we
      # can check for that.
      if not f.startswith('../../') and f.endswith('/'):
        # Don't use self.PathJoin() -- all involved paths consistently use
        # forward slashes, so don't add one single backslash on Windows.
        err += '\n' + build_dir + '/' +  f

    if err:
      self.Print('error: gn `data` items may not list generated directories; '
                 'list files in directory instead for:' + err)
      return 1

    isolate = {
        'variables': {
            'command': command,
            'files': files,
        }
    }
    # For more info about RTS, please see
    # //docs/testing/regression-test-selection.md
    if self.args.rts:
      if target in self.banned_from_rts:
        self.Print('%s is banned for RTS on this builder' % target)
        isolate['variables']['command'] = command
      else:
        inverted_command = command.copy()
        self.AddFilterFileArg(target, build_dir, command, inverted=False)
        isolate['variables']['command'] = command

        inverted_filter_exists = self.AddFilterFileArg(target,
                                                       build_dir,
                                                       inverted_command,
                                                       inverted=True)
        if inverted_filter_exists:
          isolate['variables']['inverted_command'] = inverted_command

    self.WriteFile(isolate_path, json.dumps(isolate, sort_keys=True) + '\n')

    self.WriteJSON(
      {
        'args': [
          '--isolate',
          self.ToSrcRelPath('%s/%s.isolate' % (build_dir, target)),
        ],
        'dir': self.chromium_src_dir,
        'version': 1,
      },
      isolate_path + 'd.gen.json',
    )

    return 0

  def MapTargetsToLabels(self, isolate_map, targets):
    labels = []
    err = ''

    for target in targets:
      if target == 'all':
        labels.append(target)
      elif target.startswith('//'):
        labels.append(target)
      else:
        if target in isolate_map:
          if isolate_map[target]['type'] == 'unknown':
            err += ('test target "%s" type is unknown\n' % target)
          else:
            labels.append(isolate_map[target]['label'])
        else:
          err += ('target "%s" not found in '
                  '//testing/buildbot/gn_isolate_map.pyl\n' % target)

    return err, labels

  def GNCmd(self, subcommand, path, *args):
    if self.platform.startswith('linux'):
      subdir, exe = 'linux64', 'gn'
    elif self.platform == 'darwin':
      subdir, exe = 'mac', 'gn'
    elif self.platform == 'aix6':
      subdir, exe = 'aix', 'gn'
    else:
      subdir, exe = 'win', 'gn.exe'

    gn_path = self.PathJoin(self.chromium_src_dir, 'buildtools', subdir, exe)
    cmd = [gn_path, subcommand]
    if self.args.root:
      cmd += ['--root=' + self.args.root]
    if self.args.dotfile:
      cmd += ['--dotfile=' + self.args.dotfile]
    return cmd + [path] + list(args)

  def GNArgs(self, vals, expand_imports=False):
    """Returns the gn args from vals as a Python dict and a text string.

    If expand_imports is true, any import() lines will be read in and
    valuese them will be included."""
    gn_args = vals['gn_args']

    if self.args.goma_dir:
      gn_args += ' goma_dir="%s"' % self.args.goma_dir

    android_version_code = self.args.android_version_code
    if android_version_code:
      gn_args += ' android_default_version_code="%s"' % android_version_code

    android_version_name = self.args.android_version_name
    if android_version_name:
      gn_args += ' android_default_version_name="%s"' % android_version_name

    if self.args.rts:
      gn_args += ' use_rts=true'

    args_gn_lines = []
    parsed_gn_args = {}

    args_file = vals.get('args_file', None)
    if args_file:
      if expand_imports:
        content = self.ReadFile(self.ToAbsPath(args_file))
        parsed_gn_args = gn_helpers.FromGNArgs(content)
      else:
        args_gn_lines.append('import("%s")' % args_file)

    # Canonicalize the arg string into a sorted, newline-separated list
    # of key-value pairs, and de-dup the keys if need be so that only
    # the last instance of each arg is listed.
    parsed_gn_args.update(gn_helpers.FromGNArgs(gn_args))
    args_gn_lines.append(gn_helpers.ToGNString(parsed_gn_args))

    return parsed_gn_args, '\n'.join(args_gn_lines)

  def GetSwarmingCommand(self, target, vals):
    isolate_map = self.ReadIsolateMap()

    is_android = 'target_os="android"' in vals['gn_args']
    is_fuchsia = 'target_os="fuchsia"' in vals['gn_args']
    is_cros = ('target_os="chromeos"' in vals['gn_args']
               or 'is_chromeos_device=true' in vals['gn_args'])
    is_cros_device = 'is_chromeos_device=true' in vals['gn_args']
    is_ios = 'target_os="ios"' in vals['gn_args']
    # pylint: disable=consider-using-ternary
    is_mac = ((self.platform == 'darwin' and not is_ios)
              or 'target_os="mac"' in vals['gn_args'])
    is_win = self.platform == 'win32' or 'target_os="win"' in vals['gn_args']
    is_lacros = 'chromeos_is_browser_only=true' in vals['gn_args']

    test_type = isolate_map[target]['type']

    if self.use_luci_auth:
      cmdline = ['luci-auth.exe' if is_win else 'luci-auth', 'context', '--']
    else:
      cmdline = []

    if test_type == 'generated_script' or is_ios or is_lacros:
      assert 'script' not in isolate_map[target], (
          'generated_scripts can no longer customize the script path')
      if is_win:
        default_script = 'bin\\run_{}.bat'.format(target)
      else:
        default_script = 'bin/run_{}'.format(target)
      script = isolate_map[target].get('script', default_script)

      # TODO(crbug.com/816629): remove any use of 'args' from
      # generated_scripts.
      cmdline += [script] + isolate_map[target].get('args', [])

      return cmdline, []


    # TODO(crbug.com/816629): Convert all targets to generated_scripts
    # and delete the rest of this function.

    # This should be true if tests with type='windowed_test_launcher' are
    # expected to run using xvfb. For example, Linux Desktop, X11 CrOS and
    # Ozone CrOS builds on Linux (xvfb is not used on CrOS HW or VMs). Note
    # that one Ozone build can be used to run different backends. Currently,
    # tests are executed for the headless and X11 backends and both can run
    # under Xvfb on Linux.
    use_xvfb = (self.platform.startswith('linux') and not is_android
                and not is_fuchsia and not is_cros_device)

    asan = 'is_asan=true' in vals['gn_args']
    msan = 'is_msan=true' in vals['gn_args']
    tsan = 'is_tsan=true' in vals['gn_args']
    cfi_diag = 'use_cfi_diag=true' in vals['gn_args']
    clang_coverage = 'use_clang_coverage=true' in vals['gn_args']
    java_coverage = 'use_jacoco_coverage=true' in vals['gn_args']
    javascript_coverage = 'use_javascript_coverage=true' in vals['gn_args']

    executable = isolate_map[target].get('executable', target)
    executable_suffix = isolate_map[target].get(
        'executable_suffix', '.exe' if is_win else '')

    vpython_exe = 'vpython3'
    extra_files = [
        '../../.vpython3',
        '../../testing/test_env.py',
    ]

    if is_android and test_type != 'script':
      if asan:
        cmdline += [os.path.join('bin', 'run_with_asan'), '--']
      cmdline += [
          vpython_exe, '../../build/android/test_wrapper/logdog_wrapper.py',
          '--target', target, '--logdog-bin-cmd',
          '../../.task_template_packages/logdog_butler'
      ]
      if test_type != 'junit_test':
        cmdline += ['--store-tombstones']
      if clang_coverage or java_coverage:
        cmdline += ['--coverage-dir', '${ISOLATED_OUTDIR}']
    elif is_fuchsia and test_type != 'script':
      # On Fuchsia, the generated bin/run_* test scripts are used both in
      # infrastructure and by developers. test_env.py is intended to establish a
      # predictable environment for automated testing. In particular, it adds
      # CHROME_HEADLESS=1 to the environment for child processes. This variable
      # is a signal to both test and production code that it is running in the
      # context of automated an testing environment, and should not be present
      # for normal developer workflows.
      cmdline += [
          vpython_exe,
          '../../testing/test_env.py',
          os.path.join('bin', 'run_%s' % target),
          '--test-launcher-bot-mode',
          '--logs-dir=${ISOLATED_OUTDIR}',
      ]
    elif is_cros_device and test_type != 'script':
      cmdline += [
          os.path.join('bin', 'run_%s' % target),
          '--logs-dir=${ISOLATED_OUTDIR}',
      ]
    elif use_xvfb and test_type == 'windowed_test_launcher':
      extra_files.append('../../testing/xvfb.py')
      cmdline += [
          vpython_exe,
          '../../testing/xvfb.py',
          './' + str(executable) + executable_suffix,
          '--test-launcher-bot-mode',
          '--asan=%d' % asan,
          '--lsan=%d' % asan,  # Enable lsan when asan is enabled.
          '--msan=%d' % msan,
          '--tsan=%d' % tsan,
          '--cfi-diag=%d' % cfi_diag,
      ]

      if javascript_coverage:
        cmdline += ['--devtools-code-coverage=${ISOLATED_OUTDIR}']
    elif test_type in ('windowed_test_launcher', 'console_test_launcher'):
      cmdline += [
          vpython_exe,
          '../../testing/test_env.py',
          './' + str(executable) + executable_suffix,
          '--test-launcher-bot-mode',
          '--asan=%d' % asan,
          # Enable lsan when asan is enabled except on Windows where LSAN isn't
          # supported.
          # TODO(https://crbug.com/948939): Enable on Mac once things pass.
          # TODO(https://crbug.com/974478): Enable on ChromeOS once things pass.
          '--lsan=%d' % (asan and not is_mac and not is_win and not is_cros),
          '--msan=%d' % msan,
          '--tsan=%d' % tsan,
          '--cfi-diag=%d' % cfi_diag,
      ]
    elif test_type == 'script':
      # If we're testing a CrOS simplechrome build, assume we need to prepare a
      # DUT for testing. So prepend the command to run with the test wrapper.
      if is_cros_device:
        cmdline += [
            os.path.join('bin', 'cros_test_wrapper'),
            '--logs-dir=${ISOLATED_OUTDIR}',
            '--',
        ]
      if is_android:
        extra_files.append('../../build/android/test_wrapper/logdog_wrapper.py')
        cmdline += [
            vpython_exe,
            '../../testing/test_env.py',
            '../../build/android/test_wrapper/logdog_wrapper.py',
            '--script',
            '../../' + self.ToSrcRelPath(isolate_map[target]['script']),
            '--logdog-bin-cmd',
            '../../.task_template_packages/logdog_butler',
        ]
      else:
        cmdline += [
            vpython_exe, '../../testing/test_env.py',
            '../../' + self.ToSrcRelPath(isolate_map[target]['script'])
        ]
    elif test_type == 'additional_compile_target':
      cmdline = [
          './' + str(target) + executable_suffix,
      ]
    else:
      self.WriteFailureAndRaise('No command line for %s found (test type %s).'
                                % (target, test_type), output_path=None)

    cmdline += isolate_map[target].get('args', [])

    return cmdline, extra_files

  def ToAbsPath(self, build_path, *comps):
    return self.PathJoin(self.chromium_src_dir,
                         self.ToSrcRelPath(build_path),
                         *comps)

  def ToSrcRelPath(self, path):
    """Returns a relative path from the top of the repo."""
    if path.startswith('//'):
      return path[2:].replace('/', self.sep)
    return self.RelPath(path, self.chromium_src_dir)

  def RunGNAnalyze(self, vals):
    # Analyze runs before 'gn gen' now, so we need to run gn gen
    # in order to ensure that we have a build directory.
    ret = self.RunGNGen(vals, compute_inputs_for_analyze=True, check=False)
    if ret != 0:
      return ret

    build_path = self.args.path
    input_path = self.args.input_path
    gn_input_path = input_path + '.gn'
    output_path = self.args.output_path
    gn_output_path = output_path + '.gn'

    inp = self.ReadInputJSON(['files', 'test_targets',
                              'additional_compile_targets'])
    if self.args.verbose:
      self.Print()
      self.Print('analyze input:')
      self.PrintJSON(inp)
      self.Print()


    # This shouldn't normally happen, but could due to unusual race conditions,
    # like a try job that gets scheduled before a patch lands but runs after
    # the patch has landed.
    if not inp['files']:
      self.Print('Warning: No files modified in patch, bailing out early.')
      self.WriteJSON({
            'status': 'No dependency',
            'compile_targets': [],
            'test_targets': [],
          }, output_path)
      return 0

    gn_inp = {}
    gn_inp['files'] = ['//' + f for f in inp['files'] if not f.startswith('//')]

    isolate_map = self.ReadIsolateMap()
    err, gn_inp['additional_compile_targets'] = self.MapTargetsToLabels(
        isolate_map, inp['additional_compile_targets'])
    if err:
      raise MBErr(err)

    err, gn_inp['test_targets'] = self.MapTargetsToLabels(
        isolate_map, inp['test_targets'])
    if err:
      raise MBErr(err)
    labels_to_targets = {}
    for i, label in enumerate(gn_inp['test_targets']):
      labels_to_targets[label] = inp['test_targets'][i]

    try:
      self.WriteJSON(gn_inp, gn_input_path)
      cmd = self.GNCmd('analyze', build_path, gn_input_path, gn_output_path)
      ret, output, _ = self.Run(cmd, force_verbose=True)
      if ret != 0:
        if self.args.json_output:
          # write errors to json.output
          self.WriteJSON({'output': output}, self.args.json_output)
        return ret

      gn_outp_str = self.ReadFile(gn_output_path)
      try:
        gn_outp = json.loads(gn_outp_str)
      except Exception as e:
        self.Print("Failed to parse the JSON string GN returned: %s\n%s"
                   % (repr(gn_outp_str), str(e)))
        raise

      outp = {}
      if 'status' in gn_outp:
        outp['status'] = gn_outp['status']
      if 'error' in gn_outp:
        outp['error'] = gn_outp['error']
      if 'invalid_targets' in gn_outp:
        outp['invalid_targets'] = gn_outp['invalid_targets']
      if 'compile_targets' in gn_outp:
        all_input_compile_targets = sorted(
            set(inp['test_targets'] + inp['additional_compile_targets']))

        # If we're building 'all', we can throw away the rest of the targets
        # since they're redundant.
        if 'all' in gn_outp['compile_targets']:
          outp['compile_targets'] = ['all']
        else:
          outp['compile_targets'] = gn_outp['compile_targets']

        # crbug.com/736215: When GN returns targets back, for targets in
        # the default toolchain, GN will have generated a phony ninja
        # target matching the label, and so we can safely (and easily)
        # transform any GN label into the matching ninja target. For
        # targets in other toolchains, though, GN doesn't generate the
        # phony targets, and we don't know how to turn the labels into
        # compile targets. In this case, we also conservatively give up
        # and build everything. Probably the right thing to do here is
        # to have GN return the compile targets directly.
        if any("(" in target for target in outp['compile_targets']):
          self.Print('WARNING: targets with non-default toolchains were '
                     'found, building everything instead.')
          outp['compile_targets'] = all_input_compile_targets
        else:
          outp['compile_targets'] = [
              label.replace('//', '') for label in outp['compile_targets']]

        # Windows has a maximum command line length of 8k; even Linux
        # maxes out at 128k; if analyze returns a *really long* list of
        # targets, we just give up and conservatively build everything instead.
        # Probably the right thing here is for ninja to support response
        # files as input on the command line
        # (see https://github.com/ninja-build/ninja/issues/1355).
        # Android targets use a lot of templates and often exceed 7kb.
        # https://crbug.com/946266
        max_cmd_length_kb = 64 if platform.system() == 'Linux' else 7

        if len(' '.join(outp['compile_targets'])) > max_cmd_length_kb * 1024:
          self.Print('WARNING: Too many compile targets were affected.')
          self.Print('WARNING: Building everything instead to avoid '
                     'command-line length issues.')
          outp['compile_targets'] = all_input_compile_targets


      if 'test_targets' in gn_outp:
        outp['test_targets'] = [
          labels_to_targets[label] for label in gn_outp['test_targets']]

      if self.args.verbose:
        self.Print()
        self.Print('analyze output:')
        self.PrintJSON(outp)
        self.Print()

      self.WriteJSON(outp, output_path)

    finally:
      if self.Exists(gn_input_path):
        self.RemoveFile(gn_input_path)
      if self.Exists(gn_output_path):
        self.RemoveFile(gn_output_path)

    return 0

  def ReadInputJSON(self, required_keys):
    path = self.args.input_path
    output_path = self.args.output_path
    if not self.Exists(path):
      self.WriteFailureAndRaise('"%s" does not exist' % path, output_path)

    try:
      inp = json.loads(self.ReadFile(path))
    except Exception as e:
      self.WriteFailureAndRaise('Failed to read JSON input from "%s": %s' %
                                (path, e), output_path)

    for k in required_keys:
      if not k in inp:
        self.WriteFailureAndRaise('input file is missing a "%s" key' % k,
                                  output_path)

    return inp

  def WriteFailureAndRaise(self, msg, output_path):
    if output_path:
      self.WriteJSON({'error': msg}, output_path, force_verbose=True)
    raise MBErr(msg)

  def WriteJSON(self, obj, path, force_verbose=False):
    try:
      self.WriteFile(path, json.dumps(obj, indent=2, sort_keys=True) + '\n',
                     force_verbose=force_verbose)
    except Exception as e:
      raise MBErr('Error %s writing to the output path "%s"' % (e, path)) from e

  def PrintCmd(self, cmd):
    if self.platform == 'win32':
      shell_quoter = QuoteForCmd
    else:
      shell_quoter = pipes.quote

    if cmd[0] == self.executable:
      cmd = ['python'] + cmd[1:]
    self.Print(*[shell_quoter(arg) for arg in cmd])

  def PrintJSON(self, obj):
    self.Print(json.dumps(obj, indent=2, sort_keys=True))

  def Build(self, target):
    build_dir = self.ToSrcRelPath(self.args.path)
    if self.platform == 'win32':
      # On Windows use the batch script since there is no exe
      ninja_cmd = ['autoninja.bat', '-C', build_dir]
    else:
      ninja_cmd = ['autoninja', '-C', build_dir]
    if self.args.jobs:
      ninja_cmd.extend(['-j', '%d' % self.args.jobs])
    ninja_cmd.append(target)
    ret, _, _ = self.Run(ninja_cmd, capture_output=False)
    return ret

  def Run(self, cmd, env=None, force_verbose=True, capture_output=True):
    # This function largely exists so it can be overridden for testing.
    if self.args.dryrun or self.args.verbose or force_verbose:
      self.PrintCmd(cmd)
    if self.args.dryrun:
      return 0, '', ''

    ret, out, err = self.Call(cmd, env=env, capture_output=capture_output)
    if self.args.verbose or force_verbose:
      if ret != 0:
        self.Print('  -> returned %d' % ret)
      if out:
        # This is the error seen on the logs
        self.Print(out, end='')
      if err:
        self.Print(err, end='', file=sys.stderr)
    return ret, out, err

  # Call has argument input to match subprocess.run
  def Call(
      self,
      cmd,
      env=None,
      capture_output=True,
      input='',
  ):  # pylint: disable=redefined-builtin
    # We are returning the exit code, we don't want an exception thrown
    # for non-zero exit code
    # pylint: disable=subprocess-run-check
    p = subprocess.run(cmd,
                       shell=False,
                       capture_output=capture_output,
                       cwd=self.chromium_src_dir,
                       env=env,
                       text=True,
                       input=input)
    return p.returncode, p.stdout, p.stderr

  def _CipdPlatform(self):
    """Returns current CIPD platform, e.g. linux-amd64.

    Assumes AMD64.
    """
    if self.platform == 'win32':
      return 'windows-amd64'
    if self.platform == 'darwin':
      return 'mac-amd64'
    return 'linux-amd64'

  def ExpandUser(self, path):
    # This function largely exists so it can be overridden for testing.
    return os.path.expanduser(path)

  def Exists(self, path):
    # This function largely exists so it can be overridden for testing.
    return os.path.exists(path)

  def Fetch(self, url):
    # This function largely exists so it can be overridden for testing.
    f = urlopen(url)
    contents = f.read()
    f.close()
    return contents

  def ListDir(self, path):
    # This function largely exists so it can be overridden for testing.
    return os.listdir(path)

  def MaybeMakeDirectory(self, path):
    try:
      os.makedirs(path)
    except OSError as e:
      if e.errno != errno.EEXIST:
        raise

  def PathJoin(self, *comps):
    # This function largely exists so it can be overriden for testing.
    return os.path.join(*comps)

  def Print(self, *args, **kwargs):
    # This function largely exists so it can be overridden for testing.
    print(*args, **kwargs)
    if kwargs.get('stream', sys.stdout) == sys.stdout:
      sys.stdout.flush()

  def ReadFile(self, path):
    # This function largely exists so it can be overriden for testing.
    with open(path) as fp:
      return fp.read()

  def RelPath(self, path, start='.'):
    # This function largely exists so it can be overriden for testing.
    return os.path.relpath(path, start)

  def RemoveFile(self, path):
    # This function largely exists so it can be overriden for testing.
    os.remove(path)

  def RemoveDirectory(self, abs_path):
    if self.platform == 'win32':
      # In other places in chromium, we often have to retry this command
      # because we're worried about other processes still holding on to
      # file handles, but when MB is invoked, it will be early enough in the
      # build that their should be no other processes to interfere. We
      # can change this if need be.
      self.Run(['cmd.exe', '/c', 'rmdir', '/q', '/s', abs_path])
    else:
      shutil.rmtree(abs_path, ignore_errors=True)

  def TempDir(self):
    # This function largely exists so it can be overriden for testing.
    return tempfile.mkdtemp(prefix='mb_')

  def TempFile(self, mode='w'):
    # This function largely exists so it can be overriden for testing.
    return tempfile.NamedTemporaryFile(mode=mode, delete=False)

  def WriteFile(self, path, contents, force_verbose=False):
    # This function largely exists so it can be overriden for testing.
    if self.args.dryrun or self.args.verbose or force_verbose:
      self.Print('\nWriting """\\\n%s""" to %s.\n' % (contents, path))
    with open(path, 'w') as fp:
      return fp.write(contents)


class LedResult:
  """Holds the result of a led operation. Can be chained using |then|."""

  def __init__(self, result, run_cmd):
    self._result = result
    self._run_cmd = run_cmd

  @property
  def result(self):
    """The mutable result data of the previous led call as decoded JSON."""
    return self._result

  def then(self, *cmd):
    """Invoke led, passing it the current `result` data as input.

    Returns another LedResult object with the output of the command.
    """
    return self.__class__(
        self._run_cmd(self._result, cmd), self._run_cmd)


def FlattenConfig(config_pool, mixin_pool, config):
  mixins = config_pool[config]
  vals = DefaultVals()

  visited = []
  FlattenMixins(mixin_pool, mixins, vals, visited)
  return vals


def FlattenMixins(mixin_pool, mixins_to_flatten, vals, visited):
  for m in mixins_to_flatten:
    if m not in mixin_pool:
      raise MBErr('Unknown mixin "%s"' % m)

    visited.append(m)

    mixin_vals = mixin_pool[m]

    if 'args_file' in mixin_vals:
      if vals['args_file']:
        raise MBErr('args_file specified multiple times in mixins '
                    'for mixin %s' % m)
      vals['args_file'] = mixin_vals['args_file']
    if 'gn_args' in mixin_vals:
      if vals['gn_args']:
        vals['gn_args'] += ' ' + mixin_vals['gn_args']
      else:
        vals['gn_args'] = mixin_vals['gn_args']

    if 'mixins' in mixin_vals:
      FlattenMixins(mixin_pool, mixin_vals['mixins'], vals, visited)
  return vals



class MBErr(Exception):
  pass


# See http://goo.gl/l5NPDW and http://goo.gl/4Diozm for the painful
# details of this next section, which handles escaping command lines
# so that they can be copied and pasted into a cmd window.
UNSAFE_FOR_SET = set('^<>&|')
UNSAFE_FOR_CMD = UNSAFE_FOR_SET.union(set('()%'))
ALL_META_CHARS = UNSAFE_FOR_CMD.union(set('"'))


def QuoteForSet(arg):
  if any(a in UNSAFE_FOR_SET for a in arg):
    arg = ''.join('^' + a if a in UNSAFE_FOR_SET else a for a in arg)
  return arg


def QuoteForCmd(arg):
  # First, escape the arg so that CommandLineToArgvW will parse it properly.
  if arg == '' or ' ' in arg or '"' in arg:
    quote_re = re.compile(r'(\\*)"')
    arg = '"%s"' % (quote_re.sub(lambda mo: 2 * mo.group(1) + '\\"', arg))

  # Then check to see if the arg contains any metacharacters other than
  # double quotes; if it does, quote everything (including the double
  # quotes) for safety.
  if any(a in UNSAFE_FOR_CMD for a in arg):
    arg = ''.join('^' + a if a in ALL_META_CHARS else a for a in arg)
  return arg


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
