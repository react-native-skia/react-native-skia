#!/usr/bin/env python3

# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import defaultdict
import argparse
import itertools
import json
import logging
import os
import re
import sys
import typing
"""This module contains the utilities necessary to read Dtrace result files and
convert them other format for flamegraph generation, such as pprof profiles.
"""

from protos.third_party.pprof import profile_pb2


def pairwise(iterable):
  "s -> (s0,s1), (s1,s2), (s2, s3), ..."
  a, b = itertools.tee(iterable)
  next(b, None)
  return zip(a, b)


class ProfileBuilder:
  """Helper to generate a pprof profile."""

  def __init__(self):
    self._profile = profile_pb2.Profile()
    self._locations = {}
    self._profile.string_table.append("")
    self._strings = {"": 0}
    self._signature_id = self.GetStringId('signature')

  def GetStringId(self, string: str) -> int:
    """Returns the id for `string` into the string_table, creating new entry if
       not already present.
    """
    if string in self._strings:
      return self._strings[string]
    index = len(self._profile.string_table)
    self._strings[string] = index
    self._profile.string_table.append(string)
    return index

  def GetSymbolLocation(self, name: str, system_name: str) -> int:
    """Returns the id for a symbol location defined as `(name, system_name)`,
       creating new entry if not already present.
    """
    if (name, system_name) in self._locations:
      return self._locations[(name, system_name)]

    function_id = len(self._profile.location) + 1
    self._locations[(name, system_name)] = function_id

    function = self._profile.function.add()
    function.id = function_id
    function.name = self.GetStringId(name)
    function.system_name = self.GetStringId(system_name)
    # These fields are given default values since they aren't stored in dtrace.
    function.filename = self.GetStringId("")
    function.start_line = 0

    location = self._profile.location.add()
    location.id = function_id
    line = location.line.add()
    line.function_id = function_id
    return function_id

  def AddComment(self, comment: str):
    self._profile.comment.append(self.GetStringId(comment))

  def AddSampleType(self, type: str, unit: str):
    """
    Adds a sample type that describe stackframes in this profile. See
    protos/third_party/pprof/src/profile.proto for more details.
    """
    assert (len(self._profile.sample) == 0)
    sample_type = self._profile.sample_type.add()
    sample_type.type = self.GetStringId(type)
    sample_type.unit = self.GetStringId(unit)

  def AddSample(self, locations: typing.List[int], values: typing.List[int],
                signature: str):
    """
    Adds a sample in the profile, constructed from a list of locations
    representing the stack, and a list of values for that stack (as many values
    as sample types described by this profile).
    """
    assert (len(self._profile.sample_type) == len(values))
    sample = self._profile.sample.add()
    for value in values:
      sample.value.append(value)
    for location in locations:
      sample.location_id.append(location)
    label = sample.label.add()
    label.key = self._signature_id
    label.str = self.GetStringId(signature)

  def SerializeToString(self) -> str:
    return self._profile.SerializeToString()


class DTraceParser:
  """Parses and merges chromium Dtrace profiles.

  Typical usage example:

  parser = DTraceParser()
  parser.ParseDir('./samples/')
  parser.ExportToPprof(builder)
  """

  def __init__(self, sample_type: str = 'cpu_time'):
    """
    Args:
      output_filename: The path of the file in which results are written.
    """
    self._stack_weights = defaultdict(int)
    self._signatures = defaultdict(str)
    self._stack_frames = {}
    self._sample_type = sample_type
    self._post_processing_applied = False
    self._shorten_stack_samples = False

  def ParseFile(self, stack_file: typing.TextIO):
    """Parses dtrace `stack_file` and adds the data to this profile.
    """
    assert (self._post_processing_applied == False)
    stack_frames = []
    for line, next_line in pairwise(stack_file):
      line_content = line.strip()
      if not line_content:
        continue

      # If the next line is non-empty it's not the last in the stack.
      if next_line.strip():
        # Matches lines like: "0x17e018987e"
        if line_content.startswith("0x"):
          function = line_content
          module = "unsymbolized module"
        else:
          module, function = line_content.split('`', 1)

          # Matches lines with offset like: "module`function+0xf6"
          if len(function.split('+0x')) == 2:
            [function, offset] = function.split('+0x')

        stack_frames.append((module, function))
      else:
        if len(stack_frames) == 0:
          continue
        weight = int(line_content)
        stack_string = ";".join(
            [f'{module}`{function}' for (module, function) in stack_frames])
        self._stack_weights[stack_string] += weight
        self._stack_frames[stack_string] = stack_frames
        stack_frames = []
    if not self._stack_frames:
      logging.error("No results found, check directory contents")
      sys.exit(-1)

  def ParseDir(self, stack_dir: str):
    """Parses all dtrace files in `stack_dir` and adds the data to this profile.

    Args:
      stack_dir: The directory where Dtrace profile results can be found.

    Raises:
      SystemExit: When no results are found in stack_dir.
    """
    for root, dirs, files in os.walk(stack_dir):
      for stack_filename in files:
        with open(os.path.join(stack_dir, stack_filename),
                  newline='',
                  encoding="ISO-8859-1") as stack_file:
          self.ParseFile(stack_file)

    if not self._stack_frames:
      logging.error("No results found, check directory contents")
      sys.exit(-1)

  def ConvertToPprof(self, profile_builder: ProfileBuilder):
    """Converts this profile to pprof by writing to `profile_builder`.
    """
    profile_builder.AddSampleType(self._sample_type, "counts")
    for key in self._stack_frames:
      frames = self._stack_frames[key]
      weight = self._stack_weights[key]
      signature = self._signatures[key]
      sample_locations = []
      for (module, function) in frames:
        sample_locations.append(
            profile_builder.GetSymbolLocation(function, module))
      profile_builder.AddSample(sample_locations, [weight], signature)

  def ConvertToCollapse(self, output_filename: str):
    """Converts this profile to the "collapsed stack" format. In contrast to the
    Dtrace format full stacks are writtent on a single line. At first the
    different are separated by semi-colons and a space separates the weight
    associated with the function.
    Example:

    base::foo;content::bar;biz::baz 17
    base::biz;content::boo;biz::bim 23
    ...

    """
    os.makedirs(f"{os.path.dirname(os.path.abspath(output_filename))}",
                exist_ok=True)

    with open(output_filename, 'w') as f:
      for key in self._stack_frames:
        frames = self._stack_frames[key]
        frames_string = ';'.join(
            [function for (module, function) in reversed(frames)])
        weight = self._stack_weights[key]
        # Reform the line in stacked format and write it out.
        f.write(f"{frames_string} {weight}\n")

  def GetSamplesListForTesting(self):
    samples = []
    for key in self._stack_frames:
      frames = self._stack_frames[key]
      weight = self._stack_weights[key]
      samples.append({"frames": frames, "weight": weight})
    return samples

  def AddSamplesForTesting(self, samples):
    for sample in samples:
      stack_frames = sample['frames']
      stack_string = ";".join(
          [f'{module}`{function}' for (module, function) in stack_frames])
      self._stack_frames[stack_string] = stack_frames
      self._stack_weights[stack_string] += sample['weight']

  def ShortenStack(self, stack: typing.List[typing.Tuple[str, str]]):
    """Drop some frames that don't offer any valuable information. The part
    above/before the frame is trimmed. This means that the base of the stack
    can be dropped but no frame can "skipped".

    Example (dropping biz):

    foo;bar;biz;boo --> boo
    foo;biz;bar;boo --> bar;boo

    Args:
      stack: An array of strings that represent each frame of a stack trace.

    Returns: The input array with zero or more elements removed.
    """

    message_pump_roots = [
        "base::MessagePumpNSRunLoop::DoRun", "base::MessagePumpDefault::Run",
        "base::MessagePumpKqueue::Run", "base::MessagePumpCFRunLoopBase::Run",
        "base::MessagePumpNSApplication::DoRun", "base::mac::CallWithEHFrame",
        "base::internal::WorkerThread::RunPooledWorker",
        "base::internal::WorkerThread::RunBackgroundPooledWorker"
    ]

    first_ignored_index = -1
    for i, (module, function) in enumerate(stack):
      if any(
          function.startswith(message_pump_root)
          for message_pump_root in message_pump_roots):
        # If any of the markers is present in the function it means everything
        # under the frame should be dropped from the stack.
        first_ignored_index = i
        break

    if first_ignored_index != -1:
      return stack[:first_ignored_index]
    else:
      return stack

  def ApplySignatures(self, stack: typing.List[typing.Tuple[str, str]]):
    """Matches and return known signatures to given stackframe.
    """
    if len(stack) >= 512:
      return '_OVERFLOWED_'
    for module, function in stack:
      if function.startswith(
          'safe_browsing::(anonymous namespace)::PlaybackOnBackgroundThread'):
        return 'safe_browsing:VisualSignatures'
      if function.startswith(
          'safe_browsing::(anonymous namespace)::OnModelInputCreated'):
        return 'safe_browsing:VisualSignatures'
      if function.startswith('ParkableStringImpl::CompressInBackground'):
        return 'ParkableString'
    return 'unknown'

  def EnableShortenStackSamples(self):
    self._shorten_stack_samples = True

  def PostProcessStackSamples(self):
    """Applies filtering and enhancing to self.samples().  This function can
    only be called once.

    Raises:
      SystemExit: If this function is called twice on the same object.
    """

    if self._post_processing_applied:
      logging.error("Post processing cannot be applied twice")
      sys.exit(-1)
    self._post_processing_applied = True

    for key in self._stack_frames:
      # Filter out the frames we don't care about and all those under it.
      if self._shorten_stack_samples:
        self._stack_frames[key] = self.ShortenStack(self._stack_frames[key])
      # Signatures are always added since they are non destructive.
      self._signatures[key] = self.ApplySignatures(self._stack_frames[key])


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
      description='Flip stack order of a collapsed stack file.')
  parser.add_argument("--stack_dir",
                      help="Collapsed stack file.",
                      required=True)
  parser.add_argument("--output",
                      help="The file to write the collapsed stacks into.")
  parser.add_argument('--format',
                      dest='format',
                      action='store',
                      choices=["pprof", "collapsed"],
                      default="pprof",
                      help="Output format to generate.")
  parser.add_argument('--shorten',
                      action='store_true',
                      help="Shorten stacks by removing.")
  args = parser.parse_args()
  logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

  profile_mode = 'cpu_time'
  if 'wakeups' in args.stack_dir:
    profile_mode = 'wakeups'
  parser = DTraceParser(profile_mode)
  parser.ParseDir(args.stack_dir)
  if args.shorten:
    parser.EnableShortenStackSamples()
  parser.PostProcessStackSamples()

  data_dir = os.path.abspath(os.path.join(args.stack_dir, os.pardir))
  metadata_path = os.path.join(data_dir, "metadata.json")
  if not os.path.isfile(metadata_path):
    logging.error(f"Could not find metadata.json.")
    sys.exit(-1)
  with open(metadata_path, 'r') as metadata_file:
    metadata = json.load(metadata_file)

  output_filename = args.output
  if args.format == "pprof":
    profile_builder = ProfileBuilder()
    profile_builder.AddComment(json.dumps(metadata, indent=2))
    profile_builder.AddComment(f"Profile mode: {profile_mode}")
    parser.ConvertToPprof(profile_builder)
    if output_filename is None:
      output_filename = os.path.join(data_dir, f"profile_{profile_mode}.pb")
    with open(output_filename, "wb") as output_file:
      output_file.write(profile_builder.SerializeToString())
  else:
    if output_filename is None:
      output_filename = os.path.join(data_dir,
                                     f"profile_{profile_mode}.collapsed")
    parser.ConvertToCollapse(output_filename)
  logging.info(f'Outputing profile in {os.path.abspath(output_filename)}')
