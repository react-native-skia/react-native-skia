#!/usr/bin/env python
# Copyright 2015 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os.path
import sys
import optparse

_script_path = os.path.realpath(__file__)

sys.path.insert(0, os.path.normpath(_script_path + "/../../json_comment_eater"))
try:
  import json_comment_eater
finally:
  sys.path.pop(0)

sys.path.insert(0, os.path.normpath(_script_path + "/../../json_to_struct"))
try:
  import json_to_struct
finally:
  sys.path.pop(0)

sys.path.insert(
    0,
    os.path.normpath(_script_path + "/../../../components/variations/service"))
try:
  import generate_ui_string_overrider
finally:
  sys.path.pop(0)

_platforms = [
    'android',
    'android_weblayer',
    'android_webview',
    'chromeos',
    'chromeos_lacros',
    'fuchsia',
    'ios',
    'linux',
    'mac',
    'windows',
]

_form_factors = [
    'desktop',
    'phone',
    'tablet',
]

# Convert a platform argument to the matching Platform enum value in
# components/variations/proto/study.proto.
def _PlatformEnumValue(platform):
  assert platform in _platforms
  return 'Study::PLATFORM_' + platform.upper()

def _FormFactorEnumValue(form_factor):
  assert form_factor in _form_factors
  return 'Study::' + form_factor.upper()

def _Load(filename):
  """Loads a JSON file into a Python object and return this object."""
  with open(filename, 'r') as handle:
    result = json.loads(json_comment_eater.Nom(handle.read()))
  return result


def _LoadFieldTrialConfig(filename, platforms, invert):
  """Loads a field trial config JSON and converts it into a format that can be
  used by json_to_struct.
  """
  return _FieldTrialConfigToDescription(_Load(filename), platforms, invert)


def _ConvertOverrideUIStrings(override_ui_strings):
  """Converts override_ui_strings to formatted dicts."""
  overrides = []
  for ui_string, override in override_ui_strings.items():
    overrides.append({
        'name_hash': generate_ui_string_overrider.HashName(ui_string),
        'value': override
    })
  return overrides

def _CreateExperiment(experiment_data,
                      platforms,
                      form_factors,
                      is_low_end_device,
                      invert=False):
  """Creates an experiment dictionary with all necessary information.

  Args:
    experiment_data: An experiment json config.
    platforms: A list of platforms for this trial. This should be
      a subset of |_platforms|.
    form_factors: A list of form factors for this trial. This should be
      a subset of |_form_factors|.
    is_low_end_device: An optional parameter. This can either be True or
      False. None if not specified.
    invert: An optional parameter. If set, inverts the enabled and disabled
      set of experiments. Controlled by a GN flag.

  Returns:
    An experiment dict.
  """
  experiment = {
    'name': experiment_data['name'],
    'platforms': [_PlatformEnumValue(p) for p in platforms],
    'form_factors': [_FormFactorEnumValue(f) for f in form_factors],
  }
  if is_low_end_device is not None:
    experiment['is_low_end_device'] = str(is_low_end_device).lower()
  forcing_flags_data = experiment_data.get('forcing_flag')
  if forcing_flags_data:
    experiment['forcing_flag'] = forcing_flags_data
  min_os_version_data = experiment_data.get('min_os_version')
  if min_os_version_data:
    experiment['min_os_version'] = min_os_version_data
  params_data = experiment_data.get('params')
  if (params_data):
    experiment['params'] = [{'key': param, 'value': params_data[param]}
                          for param in sorted(params_data.keys())];
  enable_features_data = experiment_data.get('enable_features')
  disable_features_data = experiment_data.get('disable_features')
  if enable_features_data or (invert and disable_features_data):
    experiment['enable_features'] = (disable_features_data
                                     if invert else enable_features_data)
  if disable_features_data or (invert and enable_features_data):
    experiment['disable_features'] = (enable_features_data
                                      if invert else disable_features_data)
  override_ui_strings = experiment_data.get('override_ui_strings')
  if override_ui_strings:
    experiment['override_ui_string'] = _ConvertOverrideUIStrings(
        override_ui_strings)
  return experiment


def _CreateTrial(study_name, experiment_configs, platforms, invert):
  """Returns the applicable experiments for |study_name| and |platforms|.

  This iterates through all of the experiment_configs for |study_name|
  and picks out the applicable experiments based off of the valid platforms
  and device type settings if specified.
  """
  experiments = []
  for config in experiment_configs:
    platform_intersection = [p for p in platforms if p in config['platforms']]

    if platform_intersection:
      experiments += [
          _CreateExperiment(e,
                            platform_intersection,
                            config.get('form_factors', []),
                            config.get('is_low_end_device'),
                            invert=invert) for e in config['experiments']
      ]
  return {
    'name': study_name,
    'experiments': experiments,
  }


def _GenerateTrials(config, platforms, invert):
  for study_name in sorted(config.keys()):
    study = _CreateTrial(study_name, config[study_name], platforms, invert)
    # To avoid converting studies with empty experiments (e.g. the study doesn't
    # apply to the target platforms), this generator only yields studies that
    # have non-empty experiments.
    if study['experiments']:
      yield study


def ConfigToStudies(config, platforms, invert):
  """Returns the applicable studies from config for the platforms."""
  return [study for study in _GenerateTrials(config, platforms, invert)]


def _FieldTrialConfigToDescription(config, platforms, invert):
  return {
      'elements': {
          'kFieldTrialConfig': {
              'studies': ConfigToStudies(config, platforms, invert)
          }
      }
  }

def main(arguments):
  parser = optparse.OptionParser(
      description='Generates a struct from a JSON description.',
      usage='usage: %prog [option] -s schema -p platform description')
  parser.add_option('-b', '--destbase',
      help='base directory of generated files.')
  parser.add_option('-d', '--destdir',
      help='directory to output generated files, relative to destbase.')
  parser.add_option('-n', '--namespace',
      help='C++ namespace for generated files. e.g search_providers.')
  parser.add_option('-p', '--platform', action='append', choices=_platforms,
      help='target platform for the field trial, mandatory.')
  parser.add_option('-s', '--schema', help='path to the schema file, '
      'mandatory.')
  parser.add_option('-o', '--output', help='output filename, '
      'mandatory.')
  parser.add_option('-y', '--year',
      help='year to put in the copy-right.')
  parser.add_option(
      '--invert_fieldtrials',
      action='store_true',
      help=
      "Inverts the enabled and disabled experiments for existing field trials.")

  (opts, args) = parser.parse_args(args=arguments)

  if not opts.schema:
    parser.error('You must specify a --schema.')

  if not opts.platform:
    parser.error('You must specify at least 1 --platform.')

  description_filename = os.path.normpath(args[0])
  shortroot = opts.output
  if opts.destdir:
    output_root = os.path.join(os.path.normpath(opts.destdir), shortroot)
  else:
    output_root = shortroot

  if opts.destbase:
    basepath = os.path.normpath(opts.destbase)
  else:
    basepath = ''

  schema = _Load(opts.schema)
  description = _LoadFieldTrialConfig(description_filename, opts.platform,
                                      opts.invert_fieldtrials)
  json_to_struct.GenerateStruct(
      basepath, output_root, opts.namespace, schema, description,
      os.path.split(description_filename)[1], os.path.split(opts.schema)[1],
      opts.year)

if __name__ == '__main__':
  main(sys.argv[1:])
