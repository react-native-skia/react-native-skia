#!/usr/bin/env python3
#
# Copyright 2013 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
from collections import defaultdict
import logging
import os
import re
import shutil
import sys
import zipfile

import dex
import dex_jdk_libs
from util import build_utils
from util import diff_utils

sys.path.insert(1, os.path.dirname(os.path.dirname(__file__)))
from pylib.dex import dex_parser

_BLOCKLISTED_EXPECTATION_PATHS = [
    # A separate expectation file is created for these files.
    'clank/third_party/google3/pg_confs/'
]


def _ParseOptions():
  args = build_utils.ExpandFileArgs(sys.argv[1:])
  parser = argparse.ArgumentParser()
  build_utils.AddDepfileOption(parser)
  parser.add_argument('--r8-path',
                      required=True,
                      help='Path to the R8.jar to use.')
  parser.add_argument(
      '--desugar-jdk-libs-json', help='Path to desugar_jdk_libs.json.')
  parser.add_argument('--input-paths',
                      action='append',
                      required=True,
                      help='GN-list of .jar files to optimize.')
  parser.add_argument('--desugar-jdk-libs-jar',
                      help='Path to desugar_jdk_libs.jar.')
  parser.add_argument('--desugar-jdk-libs-configuration-jar',
                      help='Path to desugar_jdk_libs_configuration.jar.')
  parser.add_argument('--output-path', help='Path to the generated .jar file.')
  parser.add_argument(
      '--proguard-configs',
      action='append',
      required=True,
      help='GN-list of configuration files.')
  parser.add_argument(
      '--apply-mapping', help='Path to ProGuard mapping to apply.')
  parser.add_argument(
      '--mapping-output',
      required=True,
      help='Path for ProGuard to output mapping file to.')
  parser.add_argument(
      '--extra-mapping-output-paths',
      help='GN-list of additional paths to copy output mapping file to.')
  parser.add_argument(
      '--classpath',
      action='append',
      help='GN-list of .jar files to include as libraries.')
  parser.add_argument('--main-dex-rules-path',
                      action='append',
                      help='Path to main dex rules for multidex.')
  parser.add_argument(
      '--min-api', help='Minimum Android API level compatibility.')
  parser.add_argument('--enable-obfuscation',
                      action='store_true',
                      help='Minify symbol names')
  parser.add_argument(
      '--verbose', '-v', action='store_true', help='Print all ProGuard output')
  parser.add_argument(
      '--repackage-classes', help='Package all optimized classes are put in.')
  parser.add_argument(
    '--disable-checks',
    action='store_true',
    help='Disable -checkdiscard directives and missing symbols check')
  parser.add_argument('--sourcefile', help='Value for source file attribute')
  parser.add_argument(
      '--force-enable-assertions',
      action='store_true',
      help='Forcefully enable javac generated assertion code.')
  parser.add_argument('--assertion-handler',
                      help='The class name of the assertion handler class.')
  parser.add_argument(
      '--feature-jars',
      action='append',
      help='GN list of path to jars which comprise the corresponding feature.')
  parser.add_argument(
      '--dex-dest',
      action='append',
      dest='dex_dests',
      help='Destination for dex file of the corresponding feature.')
  parser.add_argument(
      '--feature-name',
      action='append',
      dest='feature_names',
      help='The name of the feature module.')
  parser.add_argument(
      '--uses-split',
      action='append',
      help='List of name pairs separated by : mapping a feature module to a '
      'dependent feature module.')
  parser.add_argument(
      '--keep-rules-targets-regex',
      metavar='KEEP_RULES_REGEX',
      help='If passed outputs keep rules for references from all other inputs '
      'to the subset of inputs that satisfy the KEEP_RULES_REGEX.')
  parser.add_argument(
      '--keep-rules-output-path',
      help='Output path to the keep rules for references to the '
      '--keep-rules-targets-regex inputs from the rest of the inputs.')
  parser.add_argument('--warnings-as-errors',
                      action='store_true',
                      help='Treat all warnings as errors.')
  parser.add_argument('--show-desugar-default-interface-warnings',
                      action='store_true',
                      help='Enable desugaring warnings.')
  parser.add_argument('--dump-inputs',
                      action='store_true',
                      help='Use when filing R8 bugs to capture inputs.'
                      ' Stores inputs to r8inputs.zip')
  parser.add_argument(
      '--stamp',
      help='File to touch upon success. Mutually exclusive with --output-path')
  parser.add_argument('--desugared-library-keep-rule-output',
                      help='Path to desugared library keep rule output file.')

  diff_utils.AddCommandLineFlags(parser)
  options = parser.parse_args(args)

  if options.feature_names:
    if options.output_path:
      parser.error('Feature splits cannot specify an output in GN.')
    if not options.actual_file and not options.stamp:
      parser.error('Feature splits require a stamp file as output.')
  elif not options.output_path:
    parser.error('Output path required when feature splits aren\'t used')

  if bool(options.keep_rules_targets_regex) != bool(
      options.keep_rules_output_path):
    parser.error('You must path both --keep-rules-targets-regex and '
                 '--keep-rules-output-path')

  if options.force_enable_assertions and options.assertion_handler:
    parser.error('Cannot use both --force-enable-assertions and '
                 '--assertion-handler')

  options.classpath = build_utils.ParseGnList(options.classpath)
  options.proguard_configs = build_utils.ParseGnList(options.proguard_configs)
  options.input_paths = build_utils.ParseGnList(options.input_paths)
  options.extra_mapping_output_paths = build_utils.ParseGnList(
      options.extra_mapping_output_paths)

  if options.feature_names:
    if 'base' not in options.feature_names:
      parser.error('"base" feature required when feature arguments are used.')
    if len(options.feature_names) != len(options.feature_jars) or len(
        options.feature_names) != len(options.dex_dests):
      parser.error('Invalid feature argument lengths.')

    options.feature_jars = [
        build_utils.ParseGnList(x) for x in options.feature_jars
    ]

  split_map = {}
  if options.uses_split:
    for split_pair in options.uses_split:
      child, parent = split_pair.split(':')
      for name in (child, parent):
        if name not in options.feature_names:
          parser.error('"%s" referenced in --uses-split not present.' % name)
      split_map[child] = parent
  options.uses_split = split_map

  return options


class _SplitContext:
  def __init__(self, name, output_path, input_jars, work_dir, parent_name=None):
    self.name = name
    self.parent_name = parent_name
    self.input_jars = set(input_jars)
    self.final_output_path = output_path
    self.staging_dir = os.path.join(work_dir, name)
    os.mkdir(self.staging_dir)

  def CreateOutput(self, has_imported_lib=False, keep_rule_output=None):
    found_files = build_utils.FindInDirectory(self.staging_dir)
    if not found_files:
      raise Exception('Missing dex outputs in {}'.format(self.staging_dir))

    if self.final_output_path.endswith('.dex'):
      if has_imported_lib:
        raise Exception(
            'Trying to create a single .dex file, but a dependency requires '
            'JDK Library Desugaring (which necessitates a second file).'
            'Refer to %s to see what desugaring was required' %
            keep_rule_output)
      if len(found_files) != 1:
        raise Exception('Expected exactly 1 dex file output, found: {}'.format(
            '\t'.join(found_files)))
      shutil.move(found_files[0], self.final_output_path)
      return

    # Add to .jar using Python rather than having R8 output to a .zip directly
    # in order to disable compression of the .jar, saving ~500ms.
    tmp_jar_output = self.staging_dir + '.jar'
    build_utils.DoZip(found_files, tmp_jar_output, base_dir=self.staging_dir)
    shutil.move(tmp_jar_output, self.final_output_path)


def _DeDupeInputJars(split_contexts_by_name):
  """Moves jars used by multiple splits into common ancestors.

  Updates |input_jars| for each _SplitContext.
  """

  def count_ancestors(split_context):
    ret = 0
    if split_context.parent_name:
      ret += 1
      ret += count_ancestors(split_contexts_by_name[split_context.parent_name])
    return ret

  base_context = split_contexts_by_name['base']
  # Sort by tree depth so that ensure children are visited before their parents.
  sorted_contexts = list(split_contexts_by_name.values())
  sorted_contexts.remove(base_context)
  sorted_contexts.sort(key=count_ancestors, reverse=True)

  # If a jar is present in multiple siblings, promote it to their parent.
  seen_jars_by_parent = defaultdict(set)
  for split_context in sorted_contexts:
    seen_jars = seen_jars_by_parent[split_context.parent_name]
    new_dupes = seen_jars.intersection(split_context.input_jars)
    parent_context = split_contexts_by_name[split_context.parent_name]
    parent_context.input_jars.update(new_dupes)
    seen_jars.update(split_context.input_jars)

  def ancestor_jars(parent_name, dest=None):
    dest = dest or set()
    if not parent_name:
      return dest
    parent_context = split_contexts_by_name[parent_name]
    dest.update(parent_context.input_jars)
    return ancestor_jars(parent_context.parent_name, dest)

  # Now that jars have been moved up the tree, remove those that appear in
  # ancestors.
  for split_context in sorted_contexts:
    split_context.input_jars -= ancestor_jars(split_context.parent_name)


def _OptimizeWithR8(options,
                    config_paths,
                    libraries,
                    dynamic_config_data,
                    print_stdout=False):
  with build_utils.TempDir() as tmp_dir:
    if dynamic_config_data:
      dynamic_config_path = os.path.join(tmp_dir, 'dynamic_config.flags')
      with open(dynamic_config_path, 'w') as f:
        f.write(dynamic_config_data)
      config_paths = config_paths + [dynamic_config_path]

    tmp_mapping_path = os.path.join(tmp_dir, 'mapping.txt')
    # If there is no output (no classes are kept), this prevents this script
    # from failing.
    build_utils.Touch(tmp_mapping_path)

    tmp_output = os.path.join(tmp_dir, 'r8out')
    os.mkdir(tmp_output)

    split_contexts_by_name = {}
    if options.feature_names:
      for name, dest_dex, input_jars in zip(options.feature_names,
                                            options.dex_dests,
                                            options.feature_jars):
        parent_name = options.uses_split.get(name)
        if parent_name is None and name != 'base':
          parent_name = 'base'
        split_context = _SplitContext(name,
                                      dest_dex,
                                      input_jars,
                                      tmp_output,
                                      parent_name=parent_name)
        split_contexts_by_name[name] = split_context
    else:
      # Base context will get populated via "extra_jars" below.
      split_contexts_by_name['base'] = _SplitContext('base',
                                                     options.output_path, [],
                                                     tmp_output)
    base_context = split_contexts_by_name['base']

    # R8 OOMs with the default xmx=1G.
    cmd = build_utils.JavaCmd(options.warnings_as_errors, xmx='2G') + [
        '-Dcom.android.tools.r8.experimental.enablewhyareyounotinlining=1',
    ]
    if options.dump_inputs:
      cmd += ['-Dcom.android.tools.r8.dumpinputtofile=r8inputs.zip']
    cmd += [
        '-cp',
        options.r8_path,
        'com.android.tools.r8.R8',
        '--no-data-resources',
        '--output',
        base_context.staging_dir,
        '--pg-map-output',
        tmp_mapping_path,
    ]

    if options.disable_checks:
      # Info level priority logs are not printed by default.
      cmd += ['--map-diagnostics:CheckDiscardDiagnostic', 'error', 'info']
    else:
      cmd += ['--map-diagnostics', 'info', 'warning']
      if not options.warnings_as_errors:
        cmd += ['--map-diagnostics', 'error', 'warning']

    if options.desugar_jdk_libs_json:
      cmd += [
          '--desugared-lib',
          options.desugar_jdk_libs_json,
          '--desugared-lib-pg-conf-output',
          options.desugared_library_keep_rule_output,
      ]

    if options.min_api:
      cmd += ['--min-api', options.min_api]

    if options.assertion_handler:
      cmd += ['--force-assertions-handler:' + options.assertion_handler]
    elif options.force_enable_assertions:
      cmd += ['--force-enable-assertions']

    for lib in libraries:
      cmd += ['--lib', lib]

    for config_file in config_paths:
      cmd += ['--pg-conf', config_file]

    if options.main_dex_rules_path:
      for main_dex_rule in options.main_dex_rules_path:
        cmd += ['--main-dex-rules', main_dex_rule]

    _DeDupeInputJars(split_contexts_by_name)

    # Add any extra inputs to the base context (e.g. desugar runtime).
    extra_jars = set(options.input_paths)
    for split_context in split_contexts_by_name.values():
      extra_jars -= split_context.input_jars
    base_context.input_jars.update(extra_jars)

    for split_context in split_contexts_by_name.values():
      if split_context is base_context:
        continue
      for in_jar in sorted(split_context.input_jars):
        cmd += ['--feature', in_jar, split_context.staging_dir]

    cmd += sorted(base_context.input_jars)

    try:
      stderr_filter = dex.CreateStderrFilter(
          options.show_desugar_default_interface_warnings)
      logging.debug('Running R8')
      build_utils.CheckOutput(cmd,
                              print_stdout=print_stdout,
                              stderr_filter=stderr_filter,
                              fail_on_output=options.warnings_as_errors)
    except build_utils.CalledProcessError as e:
      # Python will print the original exception as well.
      raise Exception(
          'R8 failed. Please see '
          'https://chromium.googlesource.com/chromium/src/+/HEAD/build/'
          'android/docs/java_optimization.md#Debugging-common-failures') from e

    base_has_imported_lib = False
    if options.desugar_jdk_libs_json:
      logging.debug('Running L8')
      existing_files = build_utils.FindInDirectory(base_context.staging_dir)
      jdk_dex_output = os.path.join(base_context.staging_dir,
                                    'classes%d.dex' % (len(existing_files) + 1))
      # Use -applymapping to avoid name collisions.
      l8_dynamic_config_path = os.path.join(tmp_dir, 'l8_dynamic_config.flags')
      with open(l8_dynamic_config_path, 'w') as f:
        f.write("-applymapping '{}'\n".format(tmp_mapping_path))
      # Pass the dynamic config so that obfuscation options are picked up.
      l8_config_paths = [dynamic_config_path, l8_dynamic_config_path]
      if os.path.exists(options.desugared_library_keep_rule_output):
        l8_config_paths.append(options.desugared_library_keep_rule_output)

      base_has_imported_lib = dex_jdk_libs.DexJdkLibJar(
          options.r8_path, options.min_api, options.desugar_jdk_libs_json,
          options.desugar_jdk_libs_jar,
          options.desugar_jdk_libs_configuration_jar, jdk_dex_output,
          options.warnings_as_errors, l8_config_paths)
      if int(options.min_api) >= 24 and base_has_imported_lib:
        with open(jdk_dex_output, 'rb') as f:
          dexfile = dex_parser.DexFile(bytearray(f.read()))
          for m in dexfile.IterMethodSignatureParts():
            print('{}#{}'.format(m[0], m[2]))
        assert False, (
            'Desugared JDK libs are disabled on Monochrome and newer - see '
            'crbug.com/1159984 for details, and see above list for desugared '
            'classes and methods.')

    logging.debug('Collecting ouputs')
    base_context.CreateOutput(base_has_imported_lib,
                              options.desugared_library_keep_rule_output)
    for split_context in split_contexts_by_name.values():
      if split_context is not base_context:
        split_context.CreateOutput()

    shutil.move(tmp_mapping_path, options.mapping_output)
  return split_contexts_by_name


def _OutputKeepRules(r8_path, input_paths, classpath, targets_re_string,
                     keep_rules_output):
  cmd = build_utils.JavaCmd(False) + [
      '-cp', r8_path, 'com.android.tools.r8.tracereferences.TraceReferences',
      '--map-diagnostics:MissingDefinitionsDiagnostic', 'error', 'warning',
      '--keep-rules', '--output', keep_rules_output
  ]
  targets_re = re.compile(targets_re_string)
  for path in input_paths:
    if targets_re.search(path):
      cmd += ['--target', path]
    else:
      cmd += ['--source', path]
  for path in classpath:
    cmd += ['--lib', path]

  build_utils.CheckOutput(cmd, print_stderr=False, fail_on_output=False)


def _CheckForMissingSymbols(r8_path, dex_files, classpath, warnings_as_errors,
                            error_title):
  cmd = build_utils.JavaCmd(warnings_as_errors) + [
      '-cp', r8_path, 'com.android.tools.r8.tracereferences.TraceReferences',
      '--map-diagnostics:MissingDefinitionsDiagnostic', 'error', 'warning',
      '--check'
  ]

  for path in classpath:
    cmd += ['--lib', path]
  for path in dex_files:
    cmd += ['--source', path]

  failed_holder = [False]

  def stderr_filter(stderr):
    ignored_lines = [
        # Summary contains warning count, which our filtering makes wrong.
        'Warning: Tracereferences found',

        # TODO(agrieve): Create interface jars for these missing classes rather
        #     than allowlisting here.
        'dalvik.system',
        'libcore.io',
        'sun.misc.Unsafe',

        # Found in: com/facebook/fbui/textlayoutbuilder/StaticLayoutHelper
        'android.text.StaticLayout.<init>',

        # Explicictly guarded by try (NoClassDefFoundError) in Flogger's
        # PlatformProvider.
        'com.google.common.flogger.backend.google.GooglePlatform',
        'com.google.common.flogger.backend.system.DefaultPlatform',

        # TODO(agrieve): Exclude these only when use_jacoco_coverage=true.
        'java.lang.instrument.ClassFileTransformer',
        'java.lang.instrument.IllegalClassFormatException',
        'java.lang.instrument.Instrumentation',
        'java.lang.management.ManagementFactory',
        'javax.management.MBeanServer',
        'javax.management.ObjectInstance',
        'javax.management.ObjectName',
        'javax.management.StandardMBean',

        # Explicitly guarded by try (NoClassDefFoundError) in Firebase's
        # KotlinDetector: com.google.firebase.platforminfo.KotlinDetector.
        'kotlin.KotlinVersion',
    ]

    had_unfiltered_items = '  ' in stderr
    stderr = build_utils.FilterLines(
        stderr, '|'.join(re.escape(x) for x in ignored_lines))
    if stderr:
      if 'Missing' in stderr:
        failed_holder[0] = True
        stderr = 'TraceReferences failed: ' + error_title + """
Tip: Build with:
        is_java_debug=false
        treat_warnings_as_errors=false
        enable_proguard_obfuscation=false
     and then use dexdump to see which class(s) reference them.

     E.g.:
       third_party/android_sdk/public/build-tools/*/dexdump -d \
out/Release/apks/YourApk.apk > dex.txt
""" + stderr

        if 'FragmentActivity' in stderr:
          stderr += """
You may need to update build configs to run FragmentActivityReplacer for
additional targets. See
https://chromium.googlesource.com/chromium/src.git/+/main/docs/ui/android/bytecode_rewriting.md.
"""
      elif had_unfiltered_items:
        # Left only with empty headings. All indented items filtered out.
        stderr = ''
    return stderr

  build_utils.CheckOutput(cmd,
                          print_stdout=True,
                          stderr_filter=stderr_filter,
                          fail_on_output=warnings_as_errors)
  return failed_holder[0]


def _CombineConfigs(configs,
                    dynamic_config_data,
                    embedded_configs,
                    exclude_generated=False):
  # Sort in this way so //clank versions of the same libraries will sort
  # to the same spot in the file.
  def sort_key(path):
    return tuple(reversed(path.split(os.path.sep)))

  def format_config_contents(path, contents):
    formatted_contents = []
    if not contents.strip():
      return []

    # Fix up line endings (third_party configs can have windows endings).
    contents = contents.replace('\r', '')
    # Remove numbers from generated rule comments to make file more
    # diff'able.
    contents = re.sub(r' #generated:\d+', '', contents)
    formatted_contents.append('# File: ' + path)
    formatted_contents.append(contents)
    formatted_contents.append('')
    return formatted_contents

  ret = []
  for config in sorted(configs, key=sort_key):
    if exclude_generated and config.endswith('.resources.proguard.txt'):
      continue

    # Exclude some confs from expectations.
    if any(entry in config for entry in _BLOCKLISTED_EXPECTATION_PATHS):
      continue

    with open(config) as config_file:
      contents = config_file.read().rstrip()

    ret.extend(format_config_contents(config, contents))

  for path, contents in sorted(embedded_configs.items()):
    ret.extend(format_config_contents(path, contents))


  if dynamic_config_data:
    ret.append('# File: //build/android/gyp/proguard.py (generated rules)')
    ret.append(dynamic_config_data)
    ret.append('')
  return '\n'.join(ret)


def _CreateDynamicConfig(options):
  # Our scripts already fail on output. Adding -ignorewarnings makes R8 output
  # warnings rather than throw exceptions so we can selectively ignore them via
  # dex.py's ignore list. Context: https://crbug.com/1180222
  ret = ["-ignorewarnings"]

  if options.sourcefile:
    ret.append("-renamesourcefileattribute '%s' # OMIT FROM EXPECTATIONS" %
               options.sourcefile)

  if options.enable_obfuscation:
    ret.append("-repackageclasses ''")
  else:
    ret.append("-dontobfuscate")

  if options.apply_mapping:
    ret.append("-applymapping '%s'" % options.apply_mapping)

  return '\n'.join(ret)


def _ExtractEmbeddedConfigs(jar_path, embedded_configs):
  with zipfile.ZipFile(jar_path) as z:
    proguard_names = []
    r8_names = []
    for info in z.infolist():
      if info.is_dir():
        continue
      if info.filename.startswith('META-INF/proguard/'):
        proguard_names.append(info.filename)
      elif info.filename.startswith('META-INF/com.android.tools/r8/'):
        r8_names.append(info.filename)
      elif info.filename.startswith('META-INF/com.android.tools/r8-from'):
        # Assume our version of R8 is always latest.
        if '-upto-' not in info.filename:
          r8_names.append(info.filename)

    # Give preference to r8-from-*, then r8/, then proguard/.
    active = r8_names or proguard_names
    for filename in active:
      config_path = '{}:{}'.format(jar_path, filename)
      embedded_configs[config_path] = z.read(filename).decode('utf-8').rstrip()


def _ContainsDebuggingConfig(config_str):
  debugging_configs = ('-whyareyoukeeping', '-whyareyounotinlining')
  return any(config in config_str for config in debugging_configs)


def _MaybeWriteStampAndDepFile(options, inputs):
  output = options.output_path
  if options.stamp:
    build_utils.Touch(options.stamp)
    output = options.stamp
  if options.depfile:
    build_utils.WriteDepfile(options.depfile, output, inputs=inputs)


def _IterParentContexts(context_name, split_contexts_by_name):
  while context_name:
    context = split_contexts_by_name[context_name]
    yield context
    context_name = context.parent_name


def _DoTraceReferencesChecks(options, split_contexts_by_name):
  # Set of all contexts that are a parent to another.
  parent_splits_context_names = {
      c.parent_name
      for c in split_contexts_by_name.values() if c.parent_name
  }
  context_sets = [
      list(_IterParentContexts(n, split_contexts_by_name))
      for n in parent_splits_context_names
  ]
  # Visit them in order of: base, base+chrome, base+chrome+thing.
  context_sets.sort(key=lambda x: (len(x), x[0].name))

  # Ensure there are no missing references when considering all dex files.
  error_title = 'DEX contains references to non-existent symbols after R8.'
  dex_files = sorted(c.final_output_path
                     for c in split_contexts_by_name.values())
  if _CheckForMissingSymbols(options.r8_path, dex_files, options.classpath,
                             options.warnings_as_errors, error_title):
    # Failed but didn't raise due to warnings_as_errors=False
    return

  for context_set in context_sets:
    # Ensure there are no references from base -> chrome module, or from
    # chrome -> feature modules.
    error_title = (f'DEX within module "{context_set[0].name}" contains '
                   'reference(s) to symbols within child splits')
    dex_files = [c.final_output_path for c in context_set]
    # Each check currently takes about 3 seconds on a fast dev machine, and we
    # run 3 of them (all, base, base+chrome).
    # We could run them concurrently, to shave off 5-6 seconds, but would need
    # to make sure that the order is maintained.
    if _CheckForMissingSymbols(options.r8_path, dex_files, options.classpath,
                               options.warnings_as_errors, error_title):
      # Failed but didn't raise due to warnings_as_errors=False
      return


def main():
  build_utils.InitLogging('PROGUARD_DEBUG')
  options = _ParseOptions()

  # ProGuard configs that are derived from flags.
  logging.debug('Preparing configs')
  dynamic_config_data = _CreateDynamicConfig(options)

  logging.debug('Looking for embedded configs')
  libraries = []
  for p in options.classpath:
    # TODO(bjoyce): Remove filter once old android support libraries are gone.
    # Fix for having Library class extend program class dependency problem.
    if 'com_android_support' in p or 'android_support_test' in p:
      continue
    # If a jar is part of input no need to include it as library jar.
    if p not in libraries and p not in options.input_paths:
      libraries.append(p)

  embedded_configs = {}
  for jar_path in options.input_paths + libraries:
    _ExtractEmbeddedConfigs(jar_path, embedded_configs)

  # ProGuard configs that are derived from flags.
  merged_configs = _CombineConfigs(options.proguard_configs,
                                   dynamic_config_data,
                                   embedded_configs,
                                   exclude_generated=True)
  print_stdout = _ContainsDebuggingConfig(merged_configs) or options.verbose

  depfile_inputs = options.proguard_configs + options.input_paths + libraries
  if options.expected_file:
    diff_utils.CheckExpectations(merged_configs, options)
    if options.only_verify_expectations:
      build_utils.WriteDepfile(options.depfile,
                               options.actual_file,
                               inputs=depfile_inputs)
      return

  if options.keep_rules_output_path:
    _OutputKeepRules(options.r8_path, options.input_paths, options.classpath,
                     options.keep_rules_targets_regex,
                     options.keep_rules_output_path)
    return

  # TODO(agrieve): Stop appending to dynamic_config_data once R8 natively
  #     supports finding configs the "tools" directory.
  #     https://issuetracker.google.com/227983179
  tools_configs = {
      k: v
      for k, v in embedded_configs.items() if 'com.android.tools' in k
  }
  dynamic_config_data += '\n' + _CombineConfigs([], None, tools_configs)

  split_contexts_by_name = _OptimizeWithR8(options, options.proguard_configs,
                                           libraries, dynamic_config_data,
                                           print_stdout)

  if not options.disable_checks:
    logging.debug('Running tracereferences')
    _DoTraceReferencesChecks(options, split_contexts_by_name)

  for output in options.extra_mapping_output_paths:
    shutil.copy(options.mapping_output, output)

  if options.apply_mapping:
    depfile_inputs.append(options.apply_mapping)

  _MaybeWriteStampAndDepFile(options, depfile_inputs)


if __name__ == '__main__':
  main()
