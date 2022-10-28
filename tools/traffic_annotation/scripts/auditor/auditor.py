#!/usr/bin/env vpython3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import copy
import datetime
import difflib
import logging
import os
import platform
import re
import subprocess
import sys
import traceback
import xml.etree.ElementTree

from enum import Enum, auto
from functools import reduce
from google.protobuf import text_format
from google.protobuf.descriptor import FieldDescriptor
from google.protobuf.message import Message
from pathlib import Path
from typing import NewType, TYPE_CHECKING, Any, Optional, List, Dict, Set, \
    Iterable, Tuple, Union

# Path to the directory where this script is.
SCRIPT_DIR = Path(__file__).resolve().parent

# Absolute path to chrome/src.
SRC_DIR = SCRIPT_DIR.parents[3]

# TODO(nicolaso): Move extractor.py to this folder once the C++ auditor doesn't
# depend on it anymore.
sys.path.insert(0, str(SCRIPT_DIR.parent))
import extractor
from annotation_tools import NetworkTrafficAnnotationTools

if TYPE_CHECKING:
  # For the `mypy` type checker, a hardcoded import that is never used when
  # actually running. The real import is in AuditorUI.import_proto()
  #
  # TODO(nicolaso): Add instructions for running mypy.
  import traffic_annotation_pb2
  from traffic_annotation_pb2 import NetworkTrafficAnnotation as \
      traffic_annotation

UniqueId = NewType("UniqueId", str)
HashCode = NewType("HashCode", int)

# Reserved annotation unique IDs that should only be used in untracked files
# (e.g., test files or files that aren't compiled on this platform).
TEST_IDS = [UniqueId("test"), UniqueId("test_partial")]
MISSING_ID = UniqueId("missing")
NO_ANNOTATION_ID = UniqueId("undefined")
RESERVED_IDS = TEST_IDS + [MISSING_ID, NO_ANNOTATION_ID]

# Host platforms that support running auditor.py.
SUPPORTED_PLATFORMS = ["linux", "windows", "android"]

# These platforms populate the "os_list" field in annotations.xml for
# newly-added annotations (i.e., assume they're present on these platforms).
#
# Android isn't completely supported yet, so exclude it for now.
# TODO(crbug.com/1231780): Revisit this once Android support is complete.
DEFAULT_OS_LIST = ["linux", "windows"]

# Earliest valid milestone for added_in_milestone in annotations.xml.
MIN_MILESTONE = 62

# String that appears at the top of annotations.xml.
XML_COMMENT = """<?xml version="1.0"?>
<!--
Copyright 2017 The Chromium Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Refer to README.md for content description and update process.
-->

<annotations>"""

# Configure logging with timestamp, log level, filename, and line number.
logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s:%(levelname)s:%(filename)s(%(lineno)d)] %(message)s")
logger = logging.getLogger(__name__)


def get_current_platform(build_path: Optional[Path] = None) -> str:
  """Return the target platform of |build_path| based on heuristics."""
  # Use host platform as the source of truth (in most cases).
  current_platform: str = platform.system().lower()

  if current_platform == "linux" and build_path is not None:
    # It could be an Android build directory, being compiled from a Linux host.
    # Look for a target_os="android" line in args.gn.
    try:
      gn_args = (build_path / "args.gn").read_text(encoding="utf-8")
      pattern = re.compile(r"^\s*target_os\s*=\s*\"android\"\s*$", re.MULTILINE)
      if pattern.search(gn_args):
        current_platform = "android"
    except (ValueError, OSError) as e:
      logger.info(e)
      # Maybe the file's absent, or it can't be decoded as UTF-8, or something.
      # It's probably not Android in that case.
      pass

  if current_platform not in SUPPORTED_PLATFORMS:
    raise ValueError("Unsupported platform {}".format(current_platform))

  return current_platform


def twos_complement_8bit(b: int) -> int:
  """Interprets b like a signed 8-bit integer, possibly changing its sign.

  For instance, twos_complement_8bit(204) returns -52."""
  if b >= 256:
    raise ValueError("b must fit inside 8 bits")
  if b & (1 << 7):
    # Negative number, calculate its value using two's-complement.
    return b - (1 << 8)
  else:
    # Positive number, do not touch.
    return b


def iterative_hash(s: str) -> HashCode:
  """Compute the has code of the given string as in:
  net/traffic_annotation/network_traffic_annotation.h

  Args:
    s: str
      The seed, e.g. unique id of traffic annotation.
  Returns: int
    A hash code.
  """
  return HashCode(
      reduce(lambda acc, b: (acc * 31 + twos_complement_8bit(b)) % 138003713,
             s.encode("utf-8"), 0))


def compute_hash_value(text: str) -> HashCode:
  """Same as iterative_hash, but returns -1 for empty strings."""
  return iterative_hash(text) if text else HashCode(-1)


def merge_string_field(src: Message, dst: Message, field: str):
  """Merges the content of one string field into an annotation."""
  if getattr(src, field):
    if getattr(dst, field):
      setattr(dst, field, "{}\n{}".format(getattr(src, field),
                                          getattr(dst, field)))
    else:
      setattr(dst, field, getattr(src, field))


def fill_proto_with_bogus(proto: Message, field_numbers: List[int]):
  """Fill proto with bogus values for the fields identified by field_numbers.
  Uses reflection to fill the proto with the right types."""
  descriptor = proto.DESCRIPTOR
  for field_number in field_numbers:
    field_number = abs(field_number)

    if field_number not in descriptor.fields_by_number:
      raise ValueError("{} is not a valid {} field".format(
          field_number, descriptor.name))

    field = descriptor.fields_by_number[field_number]
    repeated = field.label == FieldDescriptor.LABEL_REPEATED

    if field.type == FieldDescriptor.TYPE_STRING and not repeated:
      setattr(proto, field.name, "[Archived]")
    elif field.type == FieldDescriptor.TYPE_ENUM and not repeated:
      # Assume the 2nd value in the enum is reasonable, since the 1st is
      # UNSPECIFIED.
      setattr(proto, field.name, field.enum_type.values[1].number)
    elif field.type == FieldDescriptor.TYPE_MESSAGE and repeated:
      getattr(proto, field.name).add()
    else:
      raise NotImplementedError("Unimplemented proto field type {} ({})".format(
          field.type, "repeated" if repeated else "non-repeated"))


def extract_annotation_id(line: str) -> Optional[UniqueId]:
  """Returns the annotation id given an '<item id=...' line"""
  m = re.search('id="([^"]+)"', line)
  return UniqueId(m.group(1)) if m else None


# TODO(nicolaso): Move TSV-related functions to their own file (after detangling
# circular imports).
def escape_for_tsv(text: str) -> str:
  """Changes double-quotes to single-quotes, and adds double-quotes around the
  text if it has newlines/tabs."""
  text.replace("\"", "'")
  if "\n" in text or "\t" in text:
    return "\"{}\"".format(text)
  return text


def policy_to_text(chrome_policy: Iterable[Message]) -> str:
  """Unnests the policy name/values from chrome_policy, producing a
  human-readable string.

  For example, this:
    chrome_policy {
      SyncDisabled {
        policy_options {
          mode: MANDATORY
        }
        SyncDisabled: true
      }
    }

  becomes this:
    SyncDisabled: true"""
  items = []
  # Use the protobuf serializer library to print the fields, 2 levels deep.
  for policy in chrome_policy:
    for field, value in policy.ListFields():
      for subfield, subvalue in value.ListFields():
        if subfield.name == "policy_options":
          # Skip the policy_options field.
          continue
        writer = text_format.TextWriter(as_utf8=True)
        text_format.PrintField(subfield,
                               subvalue,
                               writer,
                               as_one_line=True,
                               use_short_repeated_primitives=True)
        items.append(writer.getvalue().strip())
  # We wrote an extra comma at the end, remove it before returning.
  return ", ".join(items)
  return re.sub(r", $", "", writer.getvalue()).strip()


def write_annotations_tsv_file(file_path: Path, annotations: List["Annotation"],
                               missing_ids: List[UniqueId]):
  """Writes a TSV file of all annotations and their contents in file_path."""
  logger.info("Saving annotations to TSV file: {}.".format(file_path))
  Destination = traffic_annotation.TrafficSemantics.Destination
  CookiesAllowed = traffic_annotation.TrafficPolicy.CookiesAllowed

  lines = []
  title = "Unique ID\tLast Update\tSender\tDescription\tTrigger\tData\t" + \
  "Destination\tCookies Allowed\tCookies Store\tSetting\tChrome Policy\t" + \
  "Comments\tSource File\tID Hash Code\tContent Hash Code"

  column_count = title.count("\t")
  for missing_id in missing_ids:
    lines.append(missing_id + "\t" * column_count)

  for annotation in annotations:
    if annotation.type != Annotation.Type.COMPLETE:
      continue

    # TODO(nicolaso): Use StringIO for faster concatenation.

    line = annotation.proto.unique_id
    # Placeholder for Last Update Date, will be updated in the scripts.
    line += "\t"

    # Semantics.
    semantics = annotation.proto.semantics
    semantics_list = [
        semantics.sender,
        escape_for_tsv(semantics.description),
        escape_for_tsv(semantics.trigger),
        escape_for_tsv(semantics.data),
    ]

    for semantic_info in semantics_list:
      line += "\t{}".format(semantic_info)

    destination_names = {
        Destination.WEBSITE: "Website",
        Destination.GOOGLE_OWNED_SERVICE: "Google",
        Destination.LOCAL: "Local",
        Destination.OTHER: "Other",
    }
    if (semantics.destination == Destination.OTHER
        and semantics.destination_other):
      line += "\tOther: {}".format(semantics.destination_other)
    elif semantics.destination in destination_names:
      line += "\t{}".format(destination_names[semantics.destination])
    else:
      raise ValueError("Invalid value for the semantics.destination field")

    # Policy.
    policy = annotation.proto.policy
    if annotation.proto.policy.cookies_allowed == CookiesAllowed.YES:
      line += "\tYes"
    else:
      line += "\tNo"

    line += "\t{}".format(escape_for_tsv(policy.cookies_store))
    line += "\t{}".format(escape_for_tsv(policy.setting))

    # Chrome policies.
    if policy.chrome_policy:
      policies_text = policy_to_text(policy.chrome_policy)
    else:
      policies_text = policy.policy_exception_justification
    line += "\t{}".format(escape_for_tsv(policies_text))

    # Comments.
    line += "\t{}".format(escape_for_tsv(annotation.proto.comments))
    # Source.
    source = annotation.proto.source
    code_search_link = "https://cs.chromium.org/chromium/src/"
    line += "\t{}{}?l={}".format(code_search_link, source.file, source.line)
    # ID Hash code.
    line += "\t{}".format(annotation.unique_id_hash_code)
    # Content Hash code.
    line += "\t{}".format(annotation.get_content_hash_code())
    lines.append(line)

  lines.sort()
  lines.insert(0, title)
  report = "\n".join(lines) + "\n"

  file_path.write_text(report, encoding="utf-8")


class AuditorError:
  class Type(Enum):
    # Annotation syntax is not right.
    SYNTAX = auto()
    # Can't create a MutableNetworkTrafficAnnotationTag from anywhere (except
    # in whitelisted files).
    MUTABLE_TAG = auto()
    # Annotation has some missing fields.
    INCOMPLETE_ANNOTATION = auto()
    # A partial of (branched-)completing annotation is not paired with another
    # annotation to be completed.
    INCOMPLETED_ANNOTATION = auto()
    # Annotation has some inconsistent fields.
    INCONSISTENT_ANNOTATION = auto()
    # Two annotations that are supposed to merge cannot merge.
    MERGE_FAILED = auto()
    # A function is called with the "missing" tag.
    MISSING_TAG_USED = auto()
    # A function is called with the "test" or "test_partial" tag outside of a
    # test file.
    TEST_ANNOTATION = auto()
    # A function is called with NO_TRAFFIC_ANNOTATION_YET tag.
    NO_ANNOTATION = auto()
    # An id has a hash code equal to a reserved word.
    RESERVED_ID_HASH_CODE = auto()
    # An id has a hash code equal to a deprecated one.
    DEPRECATED_ID_HASH_CODE = auto()
    # An id contains an invalid character (not alphanumeric or underscore).
    ID_INVALID_CHARACTER = auto()
    # An id is used in two places without matching conditions. Proper conditions
    # include when 2 annotations are completing each other, or are different
    # branches of the same annotation.
    REPEATED_ID = auto()
    # Annotation does not have a valid second id.
    MISSING_SECOND_ID = auto()
    # Two ids have equal hash codes.
    HASH_CODE_COLLISION = auto()
    # Marked deprecated, but "os_list" is non-empty in annotations.xml.
    DEPRECATED_WITH_OS = auto()
    # "os_list" is invalid in annotations.xml.
    INVALID_OS = auto()
    # "added_in_milestone" is invalid in annotations.xml.
    INVALID_ADDED_IN = auto()
    # annotations.xml requires an update.
    ANNOTATIONS_XML_UPDATE = auto()
    # Annotations should be added to grouping.xml.
    ADD_GROUPING_XML = auto()
    # Annotations should be removed from grouping.xml.
    REMOVE_GROUPING_XML = auto()

  def __init__(self,
               result_type: "AuditorError.Type",
               message: str = "",
               file_path: Optional[Path] = None,
               line: int = 0,
               *extra_details: str):
    self.type = result_type
    self.message = message
    self.file_path = file_path
    self.line = line
    self._details = []

    assert message or result_type in [
        AuditorError.Type.MISSING_TAG_USED, AuditorError.Type.TEST_ANNOTATION,
        AuditorError.Type.NO_ANNOTATION, AuditorError.Type.MISSING_SECOND_ID,
        AuditorError.Type.MUTABLE_TAG, AuditorError.Type.INVALID_OS,
        AuditorError.Type.INVALID_ADDED_IN
    ]

    if message:
      self._details.append(message)
    self._details.extend(extra_details)

  def __str__(self) -> str:
    if self.type == AuditorError.Type.SYNTAX:
      assert self._details
      return ("SYNTAX: Annotation at '{}:{}' has the following syntax"
              " error: {}".format(self.file_path, self.line,
                                  str(self._details[0]).replace("\n", " ")))

    if self.type == AuditorError.Type.MUTABLE_TAG:
      return ("MUTABLE_TAG: Calling CreateMutableNetworkTrafficAnnotationTag() "
              "is not safelisted at '{}:{}'.".format(self.file_path, self.line))

    if self.type == AuditorError.Type.INCOMPLETE_ANNOTATION:
      assert self._details
      return ("INCOMPLETE_ANNOTATION: Annotation at '{}:{}' has the"
              " following missing fields: {}".format(self.file_path, self.line,
                                                     self._details[0]))

    if self.type == AuditorError.Type.INCOMPLETED_ANNOTATION:
      assert self._details
      return ("INCOMPLETED_ANNOTATION: Annotation '{}' is never "
              "completed.".format(self._details[0]))

    if self.type == AuditorError.Type.INCONSISTENT_ANNOTATION:
      assert self._details
      return ("INCONSISTENT_ANNOTATION: Annotation at '{}:{}' has the "
              "following inconsistencies: {}".format(self.file_path, self.line,
                                                     self._details[0]))
    if self.type == AuditorError.Type.MERGE_FAILED:
      assert len(self._details) == 3
      return ("MERGE_FAILED: Annotations '{}' and '{}' cannot be merged due to "
              "the following error(s): {}".format(self._details[1],
                                                  self._details[2],
                                                  self._details[0]))

    if self.type == AuditorError.Type.MISSING_TAG_USED:
      return ("MISSING_TAG_USED: MISSING_TRAFFIC_ANNOTATION tag used in "
              "'{}:{}'.".format(self.file_path, self.line))

    if self.type == AuditorError.Type.TEST_ANNOTATION:
      return ("TEST_ANNOTATION: Annotation for tests used in '{}:{}'.".format(
          self.file_path, self.line))

    if self.type == AuditorError.Type.NO_ANNOTATION:
      return "NO_ANNOTATION: Empty annotation in '{}:{}'.".format(
          self.file_path, self.line)

    if self.type == AuditorError.Type.RESERVED_ID_HASH_CODE:
      assert self._details
      return ("RESERVED_ID_HASH_CODE: Id '{}' in '{}:{}' has a hash code equal "
              "to a reserved word and should be changed.".format(
                  self._details[0], self.file_path, self.line))

    if self.type == AuditorError.Type.DEPRECATED_ID_HASH_CODE:
      assert self._details
      return ("DEPRECATED_ID_HASH_CODE: Id '{}' in '{}:{}' has a hash code "
              "equal to a deprecated id and should be changed.".format(
                  self._details[0], self.file_path, self.line))

    if self.type == AuditorError.Type.HASH_CODE_COLLISION:
      assert len(self._details) == 2
      return ("HASH_CODE_COLLISION: The following annotations have colliding "
              "hash codes and should be updated: '{}', '{}'.".format(
                  self._details[0], self._details[1]))

    if self.type == AuditorError.Type.REPEATED_ID:
      assert len(self._details) == 2
      return ("REPEATED_ID: The following annotations have equal ids and "
              "should be updated: {}, {}.".format(self._details[0],
                                                  self._details[1]))

    if self.type == AuditorError.Type.ID_INVALID_CHARACTER:
      assert self._details
      return ("ID_INVALID_CHARACTER: Id '{}' in '{}:{}' contains an invalid "
              "character.".format(self._details[0], self.file_path, self.line))

    if self.type == AuditorError.Type.MISSING_SECOND_ID:
      return ("MISSING_SECOND_ID: Second id of annotation at '{}:{}' should be "
              "updated, as it has the same hash code as the first one.".format(
                  self.file_path, self.line))

    if self.type == AuditorError.Type.INVALID_OS:
      assert len(self._details) == 2
      return ("INVALID_OS: Invalid OS '{}' in annotation '{}' at {}.".format(
          self._details[0], self._details[1], self.file_path))

    if self.type == AuditorError.Type.DEPRECATED_WITH_OS:
      assert self._details
      return ("DEPRECATED_WITH_OS: Annotation '{}' has a deprecation date and "
              "at least one active OS at {}.".format(self._details[0],
                                                     self.file_path))

    if self.type == AuditorError.Type.INVALID_ADDED_IN:
      assert len(self._details) == 2
      return ("INVALID_ADDED_IN: Invalid or missing added_in_milestone '{}' in "
              "annotation '{}' at {}.".format(self._details[0],
                                              self._details[1], self.file_path))

    if self.type == AuditorError.Type.ADD_GROUPING_XML:
      assert self._details
      return ("ADD_GROUPING_XML: The following annotations should be added "
              "to an existing group in "
              "tools/traffic_annotation/summary/grouping.xml: {}.".format(
                  self._details[0]))

    if self.type == AuditorError.Type.REMOVE_GROUPING_XML:
      assert self._details
      return ("REMOVE_GROUPING_XML: The following annotations are not needed "
              "in tools/traffic_annotation/summary/grouping.xml, and should be "
              "removed: {}.".format(self._details[0]))

    if self.type == AuditorError.Type.ANNOTATIONS_XML_UPDATE:
      assert self._details
      return (
          "'tools/traffic_annotation/summary/annotations.xml' requires update. "
          "It is recommended to run traffic_annotation_auditor locally to do "
          "the updates automatically (please refer to tools/traffic_annotation/"
          "auditor/README.md), but you can also apply the following edit(s) to "
          "do it manually:\n{}\n\n".format(self._details[0]))

    raise NotImplementedError("Unimplemented AuditorError.Type: {}".format(
        self.type.name))

  def __repr__(self) -> str:
    return "AuditorError(\"{}\")".format(str(self))


class Annotation:
  """An annotation in code, typically extracted from C++.

  Attributes:
    type: An Annotation.Type with the kind of annotation this is.
    proto: A NetworkTrafficAnnotation protobuf message.

    unique_id: The unique ID for this annotation/proto.
    unique_id_hash_code: HashCode of the unique_id.

    second_id: A UniqueId with the other annotation's unique id. This can be the
        completing id for partial annotations, or group id for branched
        completing annotations.
    second_id_hash_code: HashCode of the second_id.

    file: Path to the C++ file that contains the annotation definition.
    line: Line number where the annotation is defined in that C++ file.

    is_loaded_from_archive: True if this annotations was loaded from
        annotations.xml, rather than extracted from C++ code.
    archived_content_hash_code: content_hash_code loaded from annotations.xml.
    archived_added_in_milestone: added_in_milestone from annotations.xml.

    is_merged: True if this annotation was generated by merging 2 other
       incomplete annotations.
  """

  class Type(Enum):
    COMPLETE = (0, "Definition")
    PARTIAL = (1, "Partial")
    COMPLETING = (2, "Completing")
    BRANCHED_COMPLETING = (3, "BranchedCompleting")

    def __str__(self):
      return self.value[1]

    @classmethod
    def from_int(cls, n: int) -> "Annotation.Type":
      for t in Annotation.Type:
        if t.value[0] == n:
          return t
      raise ValueError("{} is not a valid Annotation.Type".format(n))

    @classmethod
    def from_string(cls, name: str) -> "Annotation.Type":
      for t in Annotation.Type:
        if t.value[1] == name:
          return t
      raise ValueError("'{}' is not a valid Annotation.Type".format(name))

  def __init__(self):
    self.type = Annotation.Type.COMPLETING
    self.proto = traffic_annotation_pb2.NetworkTrafficAnnotation()

    # TODO(nicolaso): Remove hash_code="" from annotations.xml, and instead
    # compute its value from unique_id.

    self.second_id: UniqueId = ""
    self.second_id_hash_code: HashCode = -1
    # TODO(nicolaso): Store the second_id instead of its hashcode in
    # annotations.xml. Then, make second_id_hash_code a computed property like
    # unique_id_hash_code.

    # TODO(nicolaso): Remove file and line from the proto in
    # traffic_annotation.proto.
    self.file: Optional[Path] = None
    self.line: int = 0

    self.is_loaded_from_archive = False
    self.archived_content_hash_code: HashCode = -1
    self.archived_added_in_milestone = 0

    self.is_merged = False

  @property
  def unique_id(self) -> UniqueId:
    # Transparently expose the unique_id stored in the proto for convenience.
    return self.proto.unique_id

  @unique_id.setter
  def unique_id(self, unique_id: UniqueId):
    # Transparently expose the unique_id stored in the proto for convenience.
    self.proto.unique_id = unique_id

  @property
  def unique_id_hash_code(self) -> HashCode:
    return compute_hash_value(self.unique_id)

  def get_ids(self) -> List[Tuple[UniqueId, HashCode]]:
    """Returns the ids/hashcodes used by this annotation (up to 2 tuples)."""
    if self.needs_two_ids():
      return [
          (self.unique_id, self.unique_id_hash_code),
          (self.second_id, self.second_id_hash_code),
      ]
    else:
      return [(self.unique_id, self.unique_id_hash_code)]

  @classmethod
  def load_from_archive(cls, archived: "ArchivedAnnotation") -> "Annotation":
    """Loads an annotation based on the data from annotations.xml."""
    annotation = Annotation()
    annotation.is_loaded_from_archive = True
    annotation.type = archived.type
    annotation.unique_id = archived.id
    annotation.file = archived.file_path
    annotation.archived_content_hash_code = archived.content_hash_code
    annotation.archived_added_in_milestone = archived.added_in_milestone

    if annotation.needs_two_ids():
      # We don't have the actual second id, so write a generated value to make
      # it non-empty. This is only relevant in tests.
      annotation.second_id = UniqueId("ARCHIVED_ID_{}".format(
          annotation.second_id))
      annotation.second_id_hash_code = archived.second_id

    fill_proto_with_bogus(annotation.proto.semantics, archived.semantics_fields)

    fill_proto_with_bogus(annotation.proto.policy, archived.policy_fields)

    # cookies_allowed is a special field: negative values indicate NO, and
    # positive values indicate YES.
    CookiesAllowed = traffic_annotation.TrafficPolicy.CookiesAllowed
    policy_fields = archived.policy_fields
    policy_descriptor = annotation.proto.policy.DESCRIPTOR
    cookies_allowed_id = (
        policy_descriptor.fields_by_name["cookies_allowed"].number)
    if +cookies_allowed_id in archived.policy_fields:
      annotation.proto.policy.cookies_allowed = CookiesAllowed.YES
    if -cookies_allowed_id in archived.policy_fields:
      annotation.proto.policy.cookies_allowed = CookiesAllowed.NO

    return annotation

  def create_complete_annotation(self, completing_annotation: "Annotation"
                                 ) -> Tuple["Annotation", List[AuditorError]]:
    """Combines |self| partial annotation with a completing/branched_completing
    annotation and returns the combined complete annotation."""
    if not self.is_completable_with(completing_annotation):
      raise ValueError("{} is not completable with {}".format(
          self.unique_id, completing_annotation.unique_id))

    # To keep the source information meta data, if completing annotation is of
    # type COMPLETING, keep |self| as the main and the other as completing.
    # But if completing annotation is of type BRANCHED_COMPLETING, reverse
    # the order.
    combination = Annotation()
    if completing_annotation.type == Annotation.Type.COMPLETING:
      combination = copy.copy(self)
      combination.proto = traffic_annotation_pb2.NetworkTrafficAnnotation()
      combination.proto.MergeFrom(self.proto)
      other = completing_annotation
    else:
      combination = copy.copy(completing_annotation)
      combination.proto = traffic_annotation_pb2.NetworkTrafficAnnotation()
      combination.proto.MergeFrom(completing_annotation.proto)
      other = self

    combination.is_merged = True
    combination.type = Annotation.Type.COMPLETE
    combination.second_id = UniqueId("")
    combination.second_id_hash_code = HashCode(-1)

    # Update comment.
    merge_string_field(other.proto, combination.proto, "comments")
    assert self.file is not None
    assert completing_annotation.file is not None
    combination.proto.comments += (
        "This annotation is a merge of the following two annotations:\n"
        "'{}' in '{}:{}' and '{}' in '{}:{}'.".format(
            self.unique_id, self.file.as_posix(), self.line,
            completing_annotation.unique_id,
            completing_annotation.file.as_posix(), completing_annotation.line))

    # Copy TrafficSemantics.
    semantics_string_fields = [
        "sender", "description", "trigger", "data", "destination_other"
    ]
    for f in semantics_string_fields:
      merge_string_field(other.proto.semantics, combination.proto.semantics, f)

    # Merge 'destination' field.
    Destination = traffic_annotation.TrafficSemantics.Destination
    if combination.proto.semantics.destination == Destination.UNSPECIFIED:
      combination.proto.semantics.destination = (
          other.proto.semantics.destination)
    elif (other.proto.semantics.destination != Destination.UNSPECIFIED
          and other.proto.semantics.destination !=
          combination.proto.semantics.destination):
      return combination, [
          AuditorError(
              AuditorError.Type.MERGE_FAILED,
              "Annotations contain different semantics::destination values",
              None, 0, self.unique_id, completing_annotation.unique_id)
      ]

    # Copy TrafficPolicy.
    policy_string_fields = [
        "cookies_store", "setting", "policy_exception_justification"
    ]
    for f in policy_string_fields:
      merge_string_field(other.proto.policy, combination.proto.policy, f)

    combination.proto.policy.cookies_allowed = max(
        combination.proto.policy.cookies_allowed,
        other.proto.policy.cookies_allowed)

    combination.proto.policy.chrome_policy.extend(
        other.proto.policy.chrome_policy)

    return combination, []

  def needs_two_ids(self) -> bool:
    """Tells if the annotation requires two ids. All annotations have a unique
    id, but partial annotations also require a completing id, and branched
    completing annotations require a group id."""
    return (self.type in [
        Annotation.Type.PARTIAL, Annotation.Type.BRANCHED_COMPLETING
    ])

  def is_completable_with(self, other) -> bool:
    """Checks to see if this annotation can be completed with the |other|
    annotation, based on their unique ids, types, and extra ids. |self| should
    be of partial type and the |other| either COMPLETING or BRANCHED_COMPLETING
    type."""
    if self.type != Annotation.Type.PARTIAL or not self.second_id:
      return False
    if other.type == Annotation.Type.COMPLETING:
      return self.second_id_hash_code == other.unique_id_hash_code
    if other.type == Annotation.Type.BRANCHED_COMPLETING:
      return self.second_id_hash_code == other.second_id_hash_code
    return False

  def get_semantics_field_numbers(self) -> List[int]:
    """Returns the proto field numbers of TrafficSemantics fields that are
    included in this annotation."""
    return [
        f.number for f in traffic_annotation.TrafficSemantics.DESCRIPTOR.fields
        if getattr(self.proto.semantics, f.name)
    ]

  def get_policy_field_numbers(self) -> List[int]:
    """Returns the proto field numbers of TrafficPolicy fields that are
    included in this annotation."""
    field_numbers = [
        f.number for f in traffic_annotation.TrafficPolicy.DESCRIPTOR.fields
        if getattr(self.proto.policy, f.name)
    ]

    # CookiesAllowed.NO is indicated with a negative value.
    CookiesAllowed = traffic_annotation.TrafficPolicy.CookiesAllowed
    policy_descriptor = self.proto.policy.DESCRIPTOR
    cookies_allowed_id = (
        policy_descriptor.fields_by_name["cookies_allowed"].number)
    if self.proto.policy.cookies_allowed == CookiesAllowed.NO:
      field_numbers.remove(+cookies_allowed_id)
      field_numbers.insert(0, -cookies_allowed_id)

    return field_numbers

  def get_content_hash_code(self) -> HashCode:
    """Computes a hashcode for the annotation content. Source field is not used
    in this computation, as we don't need sensitivity to change in source
    location (file path + line number)."""
    if self.is_loaded_from_archive:
      return self.archived_content_hash_code

    source_free_proto = copy.deepcopy(self.proto)
    source_free_proto.ClearField("source")
    source_free_proto = text_format.MessageToString(source_free_proto,
                                                    as_utf8=False)
    return compute_hash_value(source_free_proto)

  def deserialize(self, serialized_annotation: extractor.Annotation
                  ) -> List[AuditorError]:
    """Deserializes an instance from extractor.Annotation."""
    file_path = Path(serialized_annotation.file_path)
    if file_path.is_absolute():
      file_path = file_path.relative_to(SRC_DIR)
    line_number = serialized_annotation.line_number
    self.file = file_path
    self.line = line_number

    if serialized_annotation.type_name == extractor.AnnotationType.MUTABLE:
      return [
          AuditorError(AuditorError.Type.MUTABLE_TAG, "", file_path,
                       line_number)
      ]

    self.type = Annotation.Type.from_string(
        serialized_annotation.type_name.value)
    self.unique_id = serialized_annotation.unique_id
    self.second_id = serialized_annotation.extra_id
    self.second_id_hash_code = compute_hash_value(self.second_id)

    # Check for reserved IDs first, before trying to parse the Proto.
    if self.unique_id in TEST_IDS:
      return [
          AuditorError(AuditorError.Type.TEST_ANNOTATION, "", file_path,
                       line_number)
      ]

    if self.unique_id == MISSING_ID:
      return [
          AuditorError(AuditorError.Type.MISSING_TAG_USED, "", file_path,
                       line_number)
      ]

    if self.unique_id == NO_ANNOTATION_ID:
      return [
          AuditorError(AuditorError.Type.NO_ANNOTATION, "", file_path,
                       line_number)
      ]

    try:
      text_format.Parse(serialized_annotation.text, self.proto)
    except Exception as e:
      logger.error(str(e))
      return [
          AuditorError(AuditorError.Type.SYNTAX, str(e), file_path, line_number)
      ]

    return []

  def check_complete(self) -> List[AuditorError]:
    """Checks if an annotation has all required fields."""
    CookiesAllowed = traffic_annotation.TrafficPolicy.CookiesAllowed

    unspecifieds = []
    # Check semantic fields.
    semantics_fields = [
        "sender", "description", "trigger", "data", "destination"
    ]
    for field in semantics_fields:
      if not getattr(self.proto.semantics, field):
        unspecifieds.append(field)

    # Check policy fields.
    policy = self.proto.policy
    # cookies_allowed must be specified.
    if policy.cookies_allowed == CookiesAllowed.UNSPECIFIED:
      unspecifieds.append("cookies_allowed")

    # cookies_store is only needed if CookiesAllowed.YES.
    if (not policy.cookies_store
        and policy.cookies_allowed == CookiesAllowed.YES):
      unspecifieds.append("cookies_store")

    # If either of 'chrome_policy' or 'policy_exception_justification' are
    # available, ignore not having the other one.
    if not policy.chrome_policy and not policy.policy_exception_justification:
      unspecifieds.append("chrome_policy")
      unspecifieds.append("policy_exception_justification")

    if unspecifieds:
      error_text = ", ".join(unspecifieds)
      return [
          AuditorError(AuditorError.Type.INCOMPLETE_ANNOTATION, error_text,
                       self.file, self.line)
      ]
    else:
      return []

  def check_consistent(self) -> List[AuditorError]:
    """Checks if annotation fields are consistent."""
    CookiesAllowed = traffic_annotation.TrafficPolicy.CookiesAllowed
    policy = self.proto.policy

    if policy.cookies_allowed == CookiesAllowed.NO and policy.cookies_store:
      return [
          AuditorError(
              AuditorError.Type.INCONSISTENT_ANNOTATION,
              "Cookies store is specified while cookies are not allowed.",
              self.file, self.line)
      ]

    if policy.chrome_policy and policy.policy_exception_justification:
      return [
          AuditorError(
              AuditorError.Type.INCONSISTENT_ANNOTATION,
              "Both chrome policies and policy exception justification are "
              "present.", self.file, self.line)
      ]

    return []


class ExceptionType(Enum):
  """Valid exception types in safe_list.txt."""
  # Ignore all errors (doesn't check the files at all).
  ALL = "all"
  # Ignore missing annotations.
  MISSING = "missing"
  # Ignore usages of annotation for tests.
  TEST_ANNOTATION = "test_annotation"
  # Ignore CreateMutableNetworkTrafficAnnotationTag().
  MUTABLE_TAG = "mutable_tag"

  @classmethod
  def from_error_type(cls, error_type: AuditorError.Type):
    if error_type == AuditorError.Type.MISSING_TAG_USED:
      return ExceptionType.MISSING
    if error_type == AuditorError.Type.TEST_ANNOTATION:
      return ExceptionType.TEST_ANNOTATION
    if error_type == AuditorError.Type.MUTABLE_TAG:
      return ExceptionType.MUTABLE_TAG
    return None


# Rules from safe_list.txt, extracted and pre-processed.
SafeList = Dict[ExceptionType, List[re.Pattern]]


class FileFilter:
  """Provides the list of files to scan via extractor.py.

  Attributes:
    git_files: The list of files extracted via `git ls-files` (filtered).
    git_file_for_testing: If present, use this .txt file to mock the output of
       `git ls-files`."""

  def __init__(self, accepted_suffixes: List[str]):
    self.git_files: List[Path] = []
    self.git_file_for_testing: Optional[Path] = None
    self.accepted_suffixes = accepted_suffixes

  def get_source_files(self, safe_list: SafeList, prefix: str) -> List[Path]:
    """Returns a filtered list of files in the prefix directory.

    Relevant files:
      - Are tracked by git.
      - Are in a supported programming language (see
        _is_supported_source_file()).
      - Do not match any of the regexen in the ALL category of safe_list.
      - Are inside the directory_name directory."""
    file_paths = []

    if not self.git_files:
      self.get_files_from_git()

    for file_path in self.git_files:
      posix_path = file_path.as_posix()
      if not posix_path.startswith(prefix):
        continue
      if (ExceptionType.ALL in safe_list
          and any(r.match(posix_path) for r in safe_list[ExceptionType.ALL])):
        continue
      file_paths.append(file_path)

    return file_paths

  def _is_supported_source_file(self, file_path: Path) -> bool:
    """Returns true if file_path looks like a non-test C++/Obj-C++ file."""
    # Check file extension.
    if file_path.suffix not in self.accepted_suffixes:
      return False

    # Ignore test files to speed up the tests. They would be only tested when
    # filters are disabled.
    if re.search(r'test$', file_path.stem, re.IGNORECASE):
      return False

    return True

  def get_files_from_git(self) -> None:
    """Populates self.git_files with the output of `git ls-files`.

    Only keeps supported source file (per _is_supported_source_file())."""
    # Change directory to source path to access git and check files.
    original_cwd = os.getcwd()
    os.chdir(SRC_DIR)

    if self.git_file_for_testing is not None:
      # Get list of files from git_list.txt (or similar).
      lines = self.git_file_for_testing.read_text(encoding="utf-8").splitlines()
    else:
      # Get list of files from git.
      if platform.system() == "Windows":
        command_line = ["git.bat", "ls-files"]
      else:
        command_line = ["git", "ls-files"]
      process = subprocess.run(command_line, capture_output=True)
      lines = process.stdout.decode("utf-8").split("\n")

    self.git_files = [
        Path(f) for f in lines if f and self._is_supported_source_file(Path(f))
    ]

    # Now that we're done, undo the chdir().
    os.chdir(original_cwd)


class IdChecker:
  """Performs tests to ensure that annotations have correct ids.

  Attributes:
    reserved_ids: List of IDs that shouldn't be used in code (e.g. test,
        missing, no_traffic_annotation_yet ids).
    deprecated_ids: List of IDs that were used in code before, but shouldn't be
        anymore."""

  def __init__(self, reserved_ids: List[UniqueId],
               deprecated_ids: List[UniqueId]):
    self.reserved_ids = reserved_ids
    self.deprecated_ids = deprecated_ids

    self._annotations: Set[Annotation] = set()

  def check_ids(self, annotations: List[Annotation]) -> List[AuditorError]:
    """Checks annotations for UniqueId-related errors and returns them."""
    self._annotations = set(annotations)
    errors = []

    errors.extend(self._check_ids_format())
    errors.extend(self._check_for_second_ids())
    errors.extend(
        self._check_for_invalid_values(self.reserved_ids,
                                       AuditorError.Type.RESERVED_ID_HASH_CODE))
    errors.extend(
        self._check_for_invalid_values(
            self.deprecated_ids, AuditorError.Type.DEPRECATED_ID_HASH_CODE))
    errors.extend(self._check_for_hash_collisions())
    errors.extend(self._check_for_invalid_repeated_ids())

    return errors

  def _check_ids_format(self) -> List[AuditorError]:
    """Checks if ids only include alphanumeric chars and underscores."""
    errors = []

    for annotation in self._annotations:
      for id, hash_code in annotation.get_ids():
        if not re.match(r"^[0-9a-zA-Z_]*$", id):
          errors.append(
              AuditorError(AuditorError.Type.ID_INVALID_CHARACTER, id,
                           annotation.file, annotation.line))

    return errors

  def _check_for_second_ids(self) -> List[AuditorError]:
    """Checks if annotation that needs 2 ids, have 2 different ids."""
    errors = []

    for annotation in self._annotations:
      if (annotation.needs_two_ids() and
          (not annotation.second_id or
           annotation.second_id_hash_code == annotation.unique_id_hash_code)):
        errors.append(
            AuditorError(AuditorError.Type.MISSING_SECOND_ID, "",
                         annotation.file, annotation.line))

    return errors

  def _check_for_invalid_values(self, invalid_ids: List[UniqueId],
                                error_type: AuditorError.Type
                                ) -> List[AuditorError]:
    """Checks that invalid_ids are not used in annotations.

    If found, returns an error with error_type."""
    errors = []

    for annotation in self._annotations:
      for id, hash_code in annotation.get_ids():
        if id in invalid_ids:
          errors.append(
              AuditorError(error_type, id, annotation.file, annotation.line))

    return errors

  def _check_for_hash_collisions(self) -> List[AuditorError]:
    """Checks that there are no ids with colliding hash values."""
    errors = []
    collisions: Dict[HashCode, UniqueId] = {}

    for annotation in self._annotations:
      for id, hash_code in annotation.get_ids():
        if hash_code not in collisions:
          # If item is loaded from archive, do not keep the second ID for
          # checks. The archive only keeps the hash code, not the second ID
          # itself.
          if (not annotation.is_loaded_from_archive
              or id == annotation.unique_id):
            collisions[hash_code] = id
        else:
          if annotation.is_loaded_from_archive and id == annotation.second_id:
            continue
          if id != collisions[hash_code]:
            errors.append(
                AuditorError(AuditorError.Type.HASH_CODE_COLLISION, id, None, 0,
                             collisions[hash_code]))

    return errors

  def _check_for_invalid_repeated_ids(self) -> List[AuditorError]:
    """Check that there are no invalid repeated ids."""
    errors = []

    first_ids: Dict[HashCode, Annotation] = {}
    second_ids: Dict[HashCode, Annotation] = {}

    # Check if first ids are unique.
    for annotation in self._annotations:
      if annotation.unique_id_hash_code not in first_ids:
        first_ids[annotation.unique_id_hash_code] = annotation
      else:
        errors.append(
            IdChecker._create_repeated_id_error(
                annotation.unique_id, annotation,
                first_ids[annotation.unique_id_hash_code]))

    # If a second id is equal to a first id, the second id should be PARTIAL and
    # the first id should be COMPLETING.
    for annotation in self._annotations:
      if (annotation.needs_two_ids()
          and annotation.second_id_hash_code in first_ids):
        partial = annotation
        completing: Annotation = first_ids[partial.second_id_hash_code]
        if (completing != partial
            and (partial.type != Annotation.Type.PARTIAL
                 or completing.type != Annotation.Type.COMPLETING)):
          errors.append(
              IdChecker._create_repeated_id_error(partial.second_id, partial,
                                                  completing))

    # If two second ids are equal, they should be either PARTIAL or
    # BRANCHED_COMPLETING.
    for annotation in self._annotations:
      if not annotation.needs_two_ids():
        continue
      if annotation.second_id_hash_code not in second_ids:
        second_ids[annotation.second_id_hash_code] = annotation
      else:
        other = second_ids[annotation.second_id_hash_code]
        allowed_types = [
            Annotation.Type.PARTIAL, Annotation.Type.BRANCHED_COMPLETING
        ]
        if (annotation.type not in allowed_types
            or other.type not in allowed_types):
          errors.append(
              self._create_repeated_id_error(annotation.second_id, annotation,
                                             other))

    return errors

  @classmethod
  def _create_repeated_id_error(cls, common_id: UniqueId,
                                annotation1: Annotation,
                                annotation2: Annotation) -> AuditorError:
    """Constructs and returns a REPEATED_ID error."""
    return AuditorError(
        AuditorError.Type.REPEATED_ID,
        "{} in '{}:{}'".format(common_id, annotation1.file, annotation1.line),
        None, 0, "'{}:{}'".format(annotation2.file, annotation2.line))


class ArchivedAnnotation:
  """A record type for annotations.xml entries.

  All values are exactly the same as those stored in annotations.xml, except for
  some type conversions and default values."""

  # Make sure the names and order are exactly the same as the attributes in
  # the XML. This is used to serialize/deserialize the XML.
  FIELDS = [
      "id", "added_in_milestone", "hash_code", "type", "second_id",
      "deprecated", "reserved", "content_hash_code", "os_list",
      "semantics_fields", "policy_fields", "file_path"
  ]

  # Throw an error in Exporter if any of these fields is missing.
  REQUIRED_FIELDS = [
      "id", "hash_code", "type", "file_path", "added_in_milestone"
  ]

  def __init__(self,
               id: UniqueId,
               hash_code: HashCode,
               type: Annotation.Type,
               file_path: Union[Path, str, None],
               added_in_milestone: int,
               second_id: HashCode = HashCode(-1),
               deprecated: str = "",
               reserved: bool = False,
               content_hash_code: HashCode = HashCode(-1),
               os_list: Optional[List[str]] = None,
               semantics_fields: Optional[List[int]] = None,
               policy_fields: Optional[List[int]] = None):
    self.id = id
    self.hash_code = hash_code
    self.type = type
    self.file_path = Path(file_path) if file_path else None
    self.added_in_milestone = added_in_milestone
    self.second_id = second_id
    self.deprecated = deprecated
    self.reserved = reserved
    self.content_hash_code = content_hash_code
    if os_list is None:
      os_list = []
    self.os_list = os_list
    if semantics_fields is None:
      semantics_fields = []
    self.semantics_fields = semantics_fields
    if policy_fields is None:
      policy_fields = []
    self.policy_fields = policy_fields

  def __str__(self):
    return "ArchivedAnnotation({})".format(",".join(
        "{}={}".format(f, repr(getattr(self, f)))
        for f in ArchivedAnnotation.FIELDS))


class Exporter:
  """Handles loading and saving ArchivedAnnotations in annotations.xml."""

  SUMMARY_DIR = SCRIPT_DIR.parent.parent / "summary"

  GROUPING_XML_PATH = SCRIPT_DIR.parent.parent / "summary" / "grouping.xml"

  def __init__(self, current_platform: str):
    self.archive: Dict[UniqueId, ArchivedAnnotation] = {}

    assert current_platform in SUPPORTED_PLATFORMS
    self._current_platform = current_platform

    contents = (SRC_DIR / "chrome" / "VERSION").read_text(encoding="utf-8")
    m = re.search(r'MAJOR=(\d+)', contents)
    if not m:
      raise ValueError(
          "Unable to extract MAJOR=... version from chrome/VERSION")
    self._current_milestone = int(m.group(1))

    if self._current_platform == "android":
      # Use a separate file for Android until the CQ/waterfall checks are stable
      # enough, to avoid confusing CL authors.
      #
      # TODO(crbug.com/1231780): Merge this with annotations.xml once the
      # checks are working well on Android.
      self.annotations_xml_path = (Exporter.SUMMARY_DIR /
                                   "annotations_android.xml")
    else:
      self.annotations_xml_path = Exporter.SUMMARY_DIR / "annotations.xml"

  def load_annotations_xml(self) -> None:
    """Loads annotations from annotations.xml into self.archive using
    ArchivedAnnotation objects."""
    logger.info("Parsing {}.".format(
        self.annotations_xml_path.relative_to(SRC_DIR)))

    self.archive = {}

    tree = xml.etree.ElementTree.parse(self.annotations_xml_path)
    root = tree.getroot()

    for item in root.iter("item"):
      assert item.tag == "item"

      # This dictionary will be passed to ArchivedAnnotation's constructor as
      # kwargs.
      kwargs: Dict[str, Any] = dict(item.attrib)

      # Check that all required attribs are present.
      for field in ArchivedAnnotation.REQUIRED_FIELDS:
        if field not in kwargs:
          raise ValueError(
              "Missing attribute '{}' in annotations.xml: {}".format(
                  field, xml.etree.ElementTree.tostring(item, "unicode")))

      # Check for unknown attribs. The constructor for ArchivedAnnotation can
      # do this for us, but the error message is more readable this way.
      unknown_fields = kwargs.keys() - set(ArchivedAnnotation.FIELDS)
      for field in unknown_fields:
        raise ValueError("Invalid attribute '{}' in annotations.xml: {}".format(
            field, xml.etree.ElementTree.tostring(item, "unicode")))

      # Perform some type conversions.
      int_fields = [
          "hash_code", "second_id", "content_hash_code", "added_in_milestone"
      ]
      for field in int_fields:
        if field in kwargs:
          kwargs[field] = int(kwargs[field])

      kwargs["type"] = Annotation.Type.from_int(int(kwargs["type"]))

      if "os_list" in kwargs:
        kwargs["os_list"] = kwargs["os_list"].split(",")

      for field in ["semantics_fields", "policy_fields"]:
        if field in kwargs:
          kwargs[field] = [int(f) for f in kwargs[field].split(",")]

      if "reserved" in kwargs:
        kwargs["reserved"] = True

      # Create the annotation by passing kwargs.
      annotation = ArchivedAnnotation(**kwargs)
      self.archive[annotation.id] = annotation

  def update_annotations(self, annotations: List[Annotation],
                         reserved_ids: List[UniqueId]) -> List[AuditorError]:
    """Updates self.archive with the extracted annotations and reserved ids."""
    assert self.archive

    current_platform_hashcodes: Set[HashCode] = set()

    # Don't include android in the os_list for new annotations, unless we're
    # touching annotations_android.xml.
    default_os_list = DEFAULT_OS_LIST
    if self._current_platform not in default_os_list:
      default_os_list = [self._current_platform]

    for annotation in annotations:
      # annotations.xml only stores raw annotations.
      if annotation.is_merged:
        continue

      # If annotation unique id is already in the imported list, check if other
      # fields have changed.
      if annotation.unique_id in self.archive:
        archived = self.archive[annotation.unique_id]
        archived.second_id = annotation.second_id_hash_code
        archived.file_path = annotation.file
        if not self.matches_current_platform(archived):
          archived.os_list.append(self._current_platform)
        # content_hash_code includes the proto, so this detects most changes.
        archived.content_hash_code = annotation.get_content_hash_code()
      else:
        # If annotation is new, add it and assume it is on all platforms. Tests
        # running on other platforms will request updating this if required.
        new_item = ArchivedAnnotation(
            type=annotation.type,
            id=annotation.unique_id,
            hash_code=annotation.unique_id_hash_code,
            content_hash_code=annotation.get_content_hash_code(),
            os_list=default_os_list,
            added_in_milestone=self._current_milestone,
            file_path=annotation.file)
        if annotation.needs_two_ids():
          new_item.second_id = annotation.second_id_hash_code
        if annotation.type != Annotation.Type.COMPLETE:
          new_item.semantics_fields = annotation.get_semantics_field_numbers()
          new_item.policy_fields = annotation.get_policy_field_numbers()
        self.archive[annotation.unique_id] = new_item

      current_platform_hashcodes.add(annotation.unique_id_hash_code)

    # If a non-reserved annotation is removed from the current platform, update
    # it.
    for unique_id, archived in self.archive.items():
      if (self.matches_current_platform(archived)
          and archived.content_hash_code != -1
          and archived.hash_code not in current_platform_hashcodes):
        archived.os_list.remove(self._current_platform)

    # If there is a new reserved id, add it.
    for reserved_id in reserved_ids:
      if reserved_id not in self.archive:
        self.archive[reserved_id] = ArchivedAnnotation(
            id=reserved_id,
            type=Annotation.Type.COMPLETE,
            added_in_milestone=self._current_milestone,
            hash_code=compute_hash_value(reserved_id),
            reserved=True,
            os_list=default_os_list,
            file_path="")

    # If there are annotations that are not used on any OS, set the deprecation
    # flag.
    for unique_id, archived in self.archive.items():
      if not archived.os_list and not archived.deprecated:
        archived.deprecated = datetime.date.today().strftime("%Y-%m-%d")
        archived.file_path = None
        archived.semantics_fields = []
        archived.policy_fields = []

    return self.check_archived_annotations()

  def matches_current_platform(self, archived: ArchivedAnnotation) -> bool:
    return self._current_platform in archived.os_list

  def _generate_serialized_xml(self) -> str:
    """Generates XML for current report items, for saving to annotations.xml."""
    lines = [XML_COMMENT]
    # Preserve this order, so we always generate the exact same XML string
    # given the same ArchivedAnnotation object.
    for unique_id, archived in self.archive.items():
      node = xml.etree.ElementTree.fromstring('<item/>')

      # Perform the same type conversions as load_annotations_xml(), but in
      # reverse. FIELDS are already in the right order for this <item/> to
      # serialize deterministically.
      for field in ArchivedAnnotation.FIELDS:
        value = getattr(archived, field)
        if value is None and field == "file_path":
          # Always include file_path="", even if it's empty.
          node.attrib[field] = ""
        elif isinstance(value, Path):
          # For file_path="", convert backslashes to slashes.
          node.attrib[field] = value.as_posix()
        elif isinstance(value, str):
          # Remove empty strings.
          if value:
            node.attrib[field] = value
        elif isinstance(value, bool):
          # Boolean is "1" if True, or absent if False.
          if value:
            node.attrib[field] = "1"
        elif isinstance(value, int):
          # Filter out integers that are <= 0.
          if value > 0:
            node.attrib[field] = str(value)
        elif isinstance(value, Annotation.Type):
          # Use the integer value for enums.
          node.attrib[field] = str(value.value[0])
        elif isinstance(value, list):
          # Lists are comma-separated, and absent if empty.
          #
          # N.B. this does not work well for deeper structures. Only 1 level
          # of lists should be used.
          if value:
            node.attrib[field] = ",".join(map(str, value))
        else:
          raise NotImplementedError(
              "Don't know how to serialize value to XML: {} ({})".format(
                  field, value))

      lines.append(" {}".format(xml.etree.ElementTree.tostring(node,
                                                               "unicode")))

    lines.append("</annotations>")
    lines.append("")

    # Replace ' />' with '/>' so output is exactly the same as the C++
    # auditor (i.e., remove the space in self-closing tags).
    # TODO(nicolaso): Remove this dirty 'replace' hack.
    return "\n".join(re.sub(" />$", "/>", l) for l in lines)

  def check_archived_annotations(self) -> List[AuditorError]:
    """Runs tests on the contents of self.archive."""
    assert self.archive
    errors = []

    # Check for annotation hash code duplications.
    used_codes: dict[HashCode, UniqueId] = {}
    for unique_id, archived in self.archive.items():
      if archived.hash_code in used_codes:
        errors.append(
            AuditorError(AuditorError.Type.HASH_CODE_COLLISION,
                         str(archived.hash_code), None, 0, unique_id))
      else:
        used_codes[archived.hash_code] = unique_id

    # Check for coexistence of OS(es) and deprecation date.
    for unique_id, archived in self.archive.items():
      if archived.deprecated and archived.os_list:
        errors.append(
            AuditorError(AuditorError.Type.DEPRECATED_WITH_OS, unique_id,
                         self.annotations_xml_path))

    # Check that listed OSes are valid.
    for unique_id, archived in self.archive.items():
      for os in archived.os_list:
        if os not in SUPPORTED_PLATFORMS:
          errors.append(
              AuditorError(AuditorError.Type.INVALID_OS, "",
                           self.annotations_xml_path, 0, os, unique_id))

    # Check for consistency of "added_in_milestone" attribute.
    for unique_id, archived in self.archive.items():
      if archived.added_in_milestone < MIN_MILESTONE:
        errors.append(
            AuditorError(AuditorError.Type.INVALID_ADDED_IN, "",
                         self.annotations_xml_path, 0,
                         str(archived.added_in_milestone), unique_id))

    return errors

  def save_annotations_xml(self) -> None:
    """Saves self._archive into annotations.xml"""
    logger.info("Saving annotations to {}.".format(
        self.annotations_xml_path.relative_to(SRC_DIR)))
    xml_str = self._generate_serialized_xml()
    self.annotations_xml_path.write_text(xml_str, encoding="utf-8")

  def get_deprecated_ids(self) -> List[UniqueId]:
    """Produces the list of deprecated unique ids. Requires that annotations.xml
    is loaded, i.e. into self.archive."""
    assert self.archive
    return [a.id for a in self.archive.values() if a.deprecated]

  def get_other_platforms_annotation_ids(self) -> List[UniqueId]:
    """Returns a list of annotations that are not defined on this platform."""
    assert self.archive
    return [
        a.id for a in self.archive.values()
        if not a.deprecated and self._current_platform not in a.os_list
    ]

  @classmethod
  def _get_xml_items(cls, xml: str) -> Dict[UniqueId, str]:
    """Returns the list of <item id="..."/> lines in the XML, keyed by their
    id attribute."""
    items: Dict[UniqueId, str] = {}
    for line in xml.split("\n"):
      id = extract_annotation_id(line)
      if id is not None:
        items[id] = line
    return items

  @classmethod
  def _get_xml_differences(cls, old_xml: str, new_xml: str) -> str:
    """Returns the required updates to convert one XML file to another."""
    return ''.join(
        difflib.unified_diff(old_xml.splitlines(keepends=True),
                             new_xml.splitlines(keepends=True)))

  def get_required_updates(self) -> str:
    """Returns the required updates to go from one state to another in
    annotations.xml"""
    logger.info("Computing required updates for {}.".format(
        self.annotations_xml_path.relative_to(SRC_DIR)))

    old_xml = self.annotations_xml_path.read_text(encoding="utf-8")
    new_xml = self._generate_serialized_xml()

    return Exporter._get_xml_differences(old_xml, new_xml)


class Auditor:
  """Extracts and validates annotations from the codebase."""

  SAFE_LIST_PATH = (SRC_DIR / "tools" / "traffic_annotation" / "auditor" /
                    "safe_list.txt")
  # TODO(b/203773498): Remove ChromeOS safelist after cleanup.
  CHROME_OS_SAFE_LIST_PATH = (SRC_DIR / "tools" / "traffic_annotation" /
                              "auditor" / "chromeos" / "safe_list.txt")

  def __init__(self, current_platform: str, no_filtering: bool = False):
    self.no_filtering = no_filtering

    self.extracted_annotations: List[Annotation] = []

    self.partial_annotations: List[Annotation] = []
    self.completing_annotations: List[Annotation] = []

    self._safe_list: SafeList = {}

    self.exporter = Exporter(current_platform)

    accepted_suffixes = [".cc", ".mm"]
    if current_platform == "android":
      accepted_suffixes.append(".java")

    self.file_filter = FileFilter(accepted_suffixes)

  def _get_safe_list(self) -> SafeList:
    """Lazily loads safe_list.txt and returns it."""
    if self._safe_list:
      return self._safe_list

    self._safe_list = dict((t, []) for t in ExceptionType)

    # Ignore safe_list.txt while testing.
    if self.file_filter.git_file_for_testing is not None:
      return self._safe_list

    logger.info("Parsing {}.".format(
        Auditor.SAFE_LIST_PATH.relative_to(SRC_DIR)))

    lines = Auditor.SAFE_LIST_PATH.read_text(encoding="utf-8").splitlines()
    if self.exporter._current_platform == "chromeos":
      lines += Auditor.CHROME_OS_SAFE_LIST_PATH.read_text(
          encoding="utf-8").splitlines()

    for line in lines:
      # Ignore comments and empty lines.
      line = line.rstrip()
      if not line or line.startswith("#"):
        continue

      # Expect a type, and at least 1 value on each line.
      tokens = line.split(",")
      assert len(tokens) >= 2, \
              "Unexpected syntax in safe_list.txt, line: {}".format(line)

      exception_type = ExceptionType(tokens[0])
      for token in tokens[1:]:
        token = token.strip()
        # Convert the rest of the line into re.Patterns, marking dots as fixed
        # characters and asterisks as wildcards.
        assert re.match(r'^[0-9a-zA-Z_.*/:@]+$', token), \
            "Unexpected character in safe_list.txt token: '{}'".format(token)
        token = token.replace(".", "\\.")
        token = token.replace("*", ".*")
        self._safe_list[exception_type].append(re.compile(token))

    return self._safe_list

  def _is_safe_listed(self, file_path: Path,
                      exception_type: ExceptionType) -> bool:
    """Returns true if file_path matches the safe list for this exception
    type."""
    safe_list = self._get_safe_list()
    posix_path = file_path.as_posix()
    if any(r.match(posix_path) for r in safe_list[ExceptionType.ALL]):
      return True
    return any(r.match(posix_path) for r in safe_list[exception_type])

  def run_extractor(self, build_path: Path, path_filters: List[str],
                    skip_compdb: bool) -> List[extractor.Annotation]:
    """Run the extractor on the codebase.

    Filters files based on `git ls-files` and compdb. Git lets us avoid
    auto-generated files, and compdb lets us filter files by platform.

    Args:
      build_path: Path
        Path to a directory where Chrome was built (e.g., out/Release)
      path_filters: List[str]
        If this list is empty, parse all .cc/.mm/.java files in the repository.

    Returns:
      A list of all network traffic annotation instances found within a list of
      files.
    """
    safe_list = self._get_safe_list()

    logger.info("Getting list of files from git.")

    # TODO(nicolaso): Both get_source_files() and GetCompDBFiles() take a
    # couple seconds. They have no dependency on each other, so doing them both
    # in parallel may save up to ~2-3 seconds (or not, depending on how much
    # the two would fight for disk IO).

    # TODO(nicolaso): Move FileFilter and `git ls-files` logic to extractor.py,
    # or maybe a separate file?
    files = self.file_filter.get_source_files(safe_list, "")

    # Skip compdb generation while testing to speed up tests.
    if self.file_filter.git_file_for_testing is not None:
      compdb_files = None
    else:
      logger.info("Generating compile_commands.json")
      tools = NetworkTrafficAnnotationTools(str(build_path))
      compdb_files = tools.GetCompDBFiles(not skip_compdb)

    suffixes = '/'.join(self.file_filter.accepted_suffixes)
    if path_filters:
      logger.info("Parsing valid {} files in the Chromium repository, "
                  "that match any of these prefixes: {}".format(
                      suffixes, path_filters))
    else:
      logger.info("Parsing all valid {} files in the Chromium "
                  "repository.".format(suffixes))

    all_annotations = []

    for relative_path in files:
      absolute_path = SRC_DIR / relative_path

      # Skip files based on compdb and path_filters. Java files aren't in
      # compile_commands.json, so don't check those.
      if (absolute_path.suffix != ".java" and compdb_files is not None
          and str(absolute_path) not in compdb_files):
        continue
      if (path_filters
          and not self._path_filters_match(path_filters, relative_path)):
        continue

      # Pre-filter files based on their content, using a fast regex. When files
      # are already in memory from the disk cache, this saves ~10 seconds.
      if (not self.no_filtering
          and not extractor.may_contain_annotations(absolute_path)):
        continue

      # Extract annotations from the .cc/.mm/.java file. This will throw a
      # SourceCodeParsingError if the format is invalid.
      annotations = extractor.extract_annotations(absolute_path)
      if annotations:
        all_annotations.extend(annotations)

    return all_annotations

  def _filter_errors(self, file_path: Path,
                     errors: List[AuditorError]) -> List[AuditorError]:
    """Returns a new list, with safe-listed errors for this file filtered
    out."""
    filtered_errors = []
    for error in errors:
      exception_type = ExceptionType.from_error_type(error.type)
      if (exception_type is None
          or not self._is_safe_listed(file_path, exception_type)):
        filtered_errors.append(error)
    return filtered_errors

  def parse_extractor_output(self, all_annotations: List[extractor.Annotation]
                             ) -> List[AuditorError]:
    """Parses the output of extractor.extract_annotations()."""
    all_errors = []

    for serialized_annotation in all_annotations:
      annotation = Annotation()
      errors = annotation.deserialize(serialized_annotation)
      assert annotation.file is not None
      filtered_errors = self._filter_errors(annotation.file, errors)

      if errors and not filtered_errors:
        # There were errors, but they were all filtered out. Skip this
        # annotation, and don't put it in self.extracted_annotations.
        pass
      elif filtered_errors:
        all_errors.extend(filtered_errors)
      else:
        self.extracted_annotations.append(annotation)

    return all_errors

  def _check_complete_annotation(self,
                                 annotation: Annotation) -> List[AuditorError]:
    """Validate the contents of a COMPLETE annotation."""
    assert annotation.type == Annotation.Type.COMPLETE

    errors = annotation.check_complete()

    if not errors:
      errors = annotation.check_consistent()

    return errors

  def check_annotation_contents(self) -> List[AuditorError]:
    """Checks that all annotation contents are valid.

    Complete annotations should have all required fields and be consistent, and
    incomplete annotations should be completed with each other.

    Also merges incompleting annotations, and adds them to
    self.extracted_annotations."""

    all_errors = []

    partial_annotations: List[Annotation] = []
    completing_annotations: List[Annotation] = []

    # Process complete annotations and separate the others.
    for annotation in self.extracted_annotations:
      if annotation.type == Annotation.Type.COMPLETE:
        # Instances loaded from archive are already checked before archiving.
        if annotation.is_loaded_from_archive:
          continue
        all_errors.extend(self._check_complete_annotation(annotation))
      elif annotation.type == Annotation.Type.PARTIAL:
        partial_annotations.append(annotation)
      else:
        completing_annotations.append(annotation)

    new_annotations: List[Annotation] = []
    used_completing_annotations: Set[Annotation] = set()

    # Combine partial and completing annotations together.
    for partial in partial_annotations:
      found_a_pair = False
      for completing in completing_annotations:
        if partial.is_completable_with(completing):
          found_a_pair = True
          used_completing_annotations.add(completing)

          # Instances loaded from archive are already checked before archiving.
          if (partial.is_loaded_from_archive
              and completing.is_loaded_from_archive):
            break

          complete, errors = partial.create_complete_annotation(completing)
          if not errors:
            errors = self._check_complete_annotation(complete)
          if not errors:
            new_annotations.append(complete)
          else:
            all_errors.extend(errors)

      # Check that the partial annotation was completed by another.
      if not found_a_pair:
        all_errors.append(
            AuditorError(AuditorError.Type.INCOMPLETED_ANNOTATION,
                         partial.unique_id))

    # Check that completing annotations all complete another annotation.
    for completing in completing_annotations:
      if completing not in used_completing_annotations:
        all_errors.append(
            AuditorError(AuditorError.Type.INCOMPLETED_ANNOTATION,
                         completing.unique_id))

    self.extracted_annotations.extend(new_annotations)

    return all_errors

  def _get_grouping_xml_ids(self, grouping_xml_path=Exporter.GROUPING_XML_PATH
                            ) -> Set[UniqueId]:
    logger.info("Parsing {}.".format(grouping_xml_path.relative_to(SRC_DIR)))

    grouping_xml_ids = set()
    tree = xml.etree.ElementTree.parse(grouping_xml_path)
    root = tree.getroot()

    for item in root.iter("traffic_annotation"):
      assert item.tag == "traffic_annotation"
      if "unique_id" not in item.attrib:
        raise ValueError(
            "Missing attribute 'unique_id' in annotations.xml: {}".format(
                xml.etree.ElementTree.tostring(item, "unicode")))
      grouping_xml_ids.add(UniqueId(item.attrib["unique_id"]))

    return grouping_xml_ids

  def check_grouping_xml(self) -> List[AuditorError]:
    #TODO(b/203822700): Add grouping.xml for chromeos.
    if self.exporter._current_platform == "chromeos":
      logger.info("Skipping grouping.xml check for chromeos")
      return []

    grouping_xml_ids = self._get_grouping_xml_ids()

    logger.info("Computing required updates for {}.".format(
        Exporter.GROUPING_XML_PATH.relative_to(SRC_DIR)))

    # Compare with the annotation ids.
    extracted_ids = set()
    accepted_types = [Annotation.Type.PARTIAL, Annotation.Type.COMPLETE]
    for unique_id, archived in self.exporter.archive.items():
      if (archived.type in accepted_types and not archived.deprecated
          and not archived.reserved):
        extracted_ids.add(archived.id)

    errors = []

    # These ids should be added to grouping.xml.
    ids_to_add = extracted_ids - grouping_xml_ids
    if ids_to_add:
      errors.append(
          AuditorError(AuditorError.Type.ADD_GROUPING_XML,
                       ", ".join(sorted(ids_to_add))))

    # These ids should be removed from grouping.xml.
    ids_to_remove = grouping_xml_ids - extracted_ids
    if ids_to_remove:
      errors.append(
          AuditorError(AuditorError.Type.REMOVE_GROUPING_XML,
                       ", ".join(sorted(ids_to_remove))))

    return errors

  def _path_filters_match(self, path_filters: List[str], file_path: Path):
    """Checks if path_filters include the given file_path, or there are path
    filters which are folders (no "." in their name) and match the file
    name."""
    posix_path = file_path.as_posix()
    return (posix_path in path_filters
            or any("." not in f and posix_path.startswith(f)
                   for f in path_filters))

  def _add_missing_annotations(self, path_filters: List[str]):
    """Adds all archived annotations (from annotations.xml) that match the
    following features, to self.extracted_annotations:

    1- Not deprecated
    2- OS list includes current platform.
    3- Has a path (is not a reserved word).
    4- Path does not match an item in path_filters."""
    for unique_id, archived in self.exporter.archive.items():
      if (not archived.deprecated
          and self.exporter.matches_current_platform(archived)
          and archived.file_path is not None
          and not self._path_filters_match(path_filters, archived.file_path)):
        self.extracted_annotations.append(
            Annotation.load_from_archive(archived))

  def run_all_checks(self, path_filters: List[str],
                     report_xml_updates: bool) -> List[AuditorError]:
    """Performs all checks on extracted annotations, and writes annotations.xml.

    If test_only is True, returns the changes that would be made to
    annotations.xml as AuditorErrors, instead of writing them directly to the
    file."""
    errors = []

    self.exporter.load_annotations_xml()
    if path_filters:
      self._add_missing_annotations(path_filters)

    suffixes = '/'.join(self.file_filter.accepted_suffixes)
    logger.info("Checking the validity of annotations extracted from {} "
                "files.".format(suffixes))

    deprecated_ids = self.exporter.get_deprecated_ids()
    id_checker = IdChecker(RESERVED_IDS, deprecated_ids)
    errors.extend(id_checker.check_ids(self.extracted_annotations))

    # Only check annotation contents if IDs are all OK, because if there are
    # id errors, there might be some mismatching annotations and irrelevant
    # content errors.
    if not errors:
      errors.extend(self.check_annotation_contents())

    if not errors:
      errors.extend(
          self.exporter.update_annotations(self.extracted_annotations,
                                           RESERVED_IDS))
      errors.extend(self.check_grouping_xml())

    # If report_xml_updates is true, look at the contents of annotations.xml. If
    # it needs an update, add an ANNOTATIONS_XML_UPDATE error.
    if report_xml_updates:
      updates = self.exporter.get_required_updates()
      if updates:
        errors.append(
            AuditorError(AuditorError.Type.ANNOTATIONS_XML_UPDATE, updates))

    return errors


class AuditorUI:
  """Interface to the Auditor, mostly consisting of the main() function.

  Most attributes are derived from command-line flags."""

  def __init__(self,
               build_path: Path,
               path_filters: List[str],
               no_filtering: bool = True,
               test_only: bool = False,
               error_limit: int = 0,
               annotations_file: Optional[Path] = None,
               skip_compdb: bool = False):
    self.build_path = build_path
    # Convert backslashes to slashes on Windows.
    self.path_filters = [Path(f).as_posix() for f in path_filters]
    self.no_filtering = no_filtering
    self.test_only = test_only
    self.error_limit = error_limit
    self.annotations_file = annotations_file
    self.skip_compdb = skip_compdb

    # Exposed for testing.
    self.traffic_annotation = self.import_compiled_proto()
    self.auditor = Auditor(get_current_platform(self.build_path),
                           self.no_filtering)

  def import_compiled_proto(self) -> Any:
    """Global import from function. |self.build_path| is needed to perform
    this import, hence why it's not a top-level import.

    The compiled proto is located ${self.build_path}/pyproto/ and generated
    as a part of compiling Chrome."""
    # Use the build path to import the compiled traffic annotation proto.
    proto_path = (self.build_path / "pyproto" / "chrome" / "browser" /
                  "privacy")
    sys.path.insert(0, str(proto_path))

    try:
      global traffic_annotation_pb2
      global traffic_annotation
      import traffic_annotation_pb2
      # Used for accessing enum constants.
      from traffic_annotation_pb2 import NetworkTrafficAnnotation as \
        traffic_annotation
      return traffic_annotation
    except ImportError as e:
      logger.critical(
        "Failed to import the compiled traffic annotation proto. Make sure "+ \
        "Chrome is built in '{}' before running this script.".format(
          self.build_path))
      raise

  def main(self) -> int:
    if self.no_filtering and self.path_filters:
      logger.warning("The path_filters input is being ignored.")
      self.path_filters = []

    all_annotations = self.auditor.run_extractor(self.build_path,
                                                 self.path_filters,
                                                 self.skip_compdb)
    errors = []

    errors.extend(self.auditor.parse_extractor_output(all_annotations))

    # If we already have errors from parsing annotations, report them. Otherwise
    # check the extracted annotations and their consistency with previous state.
    if not errors:
      errors.extend(
          self.auditor.run_all_checks(self.path_filters, self.test_only))

    # Write annotations TSV file.
    if self.annotations_file is not None:
      missing_ids = self.auditor.exporter.get_other_platforms_annotation_ids()
      write_annotations_tsv_file(self.annotations_file,
                                 self.auditor.extracted_annotations,
                                 missing_ids)

    # Update annotations.xml if everything else is OK and the auditor is not
    # in test-only mode.
    if not self.test_only and not errors:
      self.auditor.exporter.save_annotations_xml()

    # Postprocess errors and dump to stdout.
    if errors:
      print("[Errors]")
      for i, error in enumerate(errors):
        if self.error_limit and i >= self.error_limit:
          break
        print("  ({})\t{}".format(i + 1, str(error)))
      return 1

    sys.stdout.write("Traffic annotations are all OK.\n")
    return 0


if __name__ == "__main__":
  args_parser = argparse.ArgumentParser(
      description="Traffic Annotation Auditor: Extracts network traffic"
      " annotations from the repository, audits them for errors and coverage,"
      " produces reports, and updates related files.",
      prog="auditor.py",
      usage="%(prog)s [OPTION] ... [path_filters]")
  args_parser.add_argument("--build-path",
                           type=Path,
                           help="Path to the build directory.",
                           required=True)
  args_parser.add_argument(
      "--no-filtering",
      action="store_true",
      help="Optional flag asking the tool"
      " to run on the whole repository without text filtering files.")
  args_parser.add_argument(
      "--test-only",
      help="Optional flag to request just running tests and not"
      " updating any file. If not specified,"
      " 'tools/traffic_annotation/summary/annotations.xml' might get updated.",
      action="store_true")
  args_parser.add_argument(
      "--error-resilient",
      help="Optional flag, stating not to return error in"
      " exit code if auditor fails to perform the tests. This flag can be used"
      " for trybots to avoid spamming when tests cannot run.",
      action="store_true")
  args_parser.add_argument("--limit",
                           default=5,
                           type=int,
                           help="Limit for the maximum number of returned "
                           " errors. Use 0 for unlimited.")
  args_parser.add_argument("--annotations-file",
                           type=Path,
                           help="Optional path to a TSV output file with all"
                           " annotations.")
  args_parser.add_argument(
      "--skip-compdb",
      help="Assume compile_commands exists in the build-path, and is "
      " up-to-date. This speeds up the auditor.",
      action="store_true")
  args_parser.add_argument(
      "path_filters",
      nargs="*",
      help="Optional paths to filter which files the"
      " tool is run on. It can also include deleted files names when auditor is"
      " run on a partial repository. These are ignored if all of the following"
      " are true: Not using --extractor-input, using -no-filtering OR"
      " --all-files, using the python extractor.")

  args = args_parser.parse_args()
  build_path = Path(args.build_path)

  print("Starting traffic annotation auditor. This may take a few minutes.")
  print("If you find a bug in this script, file bugs against the 'Enterprise>"
        "TrafficAnnotations' component and CC nicolaso@chromium.org.")
  auditor_ui = AuditorUI(build_path, args.path_filters, args.no_filtering,
                         args.test_only, args.limit, args.annotations_file,
                         args.skip_compdb)

  try:
    sys.exit(auditor_ui.main())
  except Exception as e:
    if args.error_resilient:
      traceback.print_exc()
      sys.exit(0)
    else:
      raise
