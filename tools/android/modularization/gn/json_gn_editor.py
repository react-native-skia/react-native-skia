# Lint as: python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
'''Helper script to use GN's JSON interface to make changes.'''

from __future__ import annotations

import contextlib
import copy
import dataclasses
import json
import logging
import os
import pathlib
import re
import shutil
import subprocess
import sys

from typing import Iterator, List, Optional, Tuple

_TOOLS_ANDROID_PATH = pathlib.Path(__file__).parents[2].resolve()
if str(_TOOLS_ANDROID_PATH) not in sys.path:
    sys.path.append(str(_TOOLS_ANDROID_PATH))
from python_utils import git_metadata_utils, subprocess_utils

_SRC_PATH = git_metadata_utils.get_chromium_src_path()

# Refer to parse_tree.cc for GN AST implementation details:
# https://gn.googlesource.com/gn/+/refs/heads/main/src/gn/parse_tree.cc
# These constants should match corresponding entries in parse_tree.cc.
# TODO: Add high-level details for the expected data structure.
NODE_CHILD = 'child'
NODE_TYPE = 'type'
NODE_VALUE = 'value'
BEFORE_COMMENT = 'before_comment'
SUFFIX_COMMENT = 'suffix_comment'
AFTER_COMMENT = 'after_comment'


@contextlib.contextmanager
def _backup_and_restore_file_contents(path: str):
    with open(path) as f:
        contents = f.read()
    try:
        yield
    finally:
        # Ensure that the timestamp is updated since otherwise ninja will not
        # re-build relevant targets with the original file.
        with open(path, 'w') as f:
            f.write(contents)


def _build_targets_output(out_dir: str,
                          targets: List[str],
                          should_print: Optional[bool] = None
                          ) -> Optional[str]:
    env = os.environ.copy()
    if should_print is None:
        should_print = logging.getLogger().isEnabledFor(logging.DEBUG)
    # Ensuring ninja does not attempt to summarize the build results in slightly
    # faster builds. This script does many builds so this time can add up.
    if 'NINJA_SUMMARIZE_BUILD' in env:
        del env['NINJA_SUMMARIZE_BUILD']
    proc = subprocess.Popen(['autoninja', '-C', out_dir] + targets,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            env=env,
                            text=True)
    lines = []
    prev_line = ''
    width = shutil.get_terminal_size().columns
    while proc.poll() is None:
        line = proc.stdout.readline()
        lines.append(line)
        if should_print:
            if prev_line.startswith('[') and line.startswith('['):
                # Shrink the line according to terminal size.
                msg = line.rstrip()
                if len(msg) > width:
                    # 5 = 3 (Ellipsis) + 2 (header)
                    length_to_show = width - 5
                    msg = f'{msg[:2]}...{msg[-length_to_show:]}'
                # \r to return the carriage to the beginning of line, \033[K to
                # replace the normal \n to erase until the end of the line. This
                # allows ninja output for successful targets to overwrite each
                # other.
                msg = f'\r{msg}\033[K'
            elif prev_line.startswith('['):
                # Since the previous line likely did not include a newline, an
                # extra newline is needed to avoid the current line being
                # appended to the previous line.
                msg = f'\n{line}'
            else:
                msg = line
            print(msg, end='')
        prev_line = line
    if proc.returncode != 0:
        return None
    return ''.join(lines)


def _generate_project_json_content(out_dir: str) -> str:
    subprocess_utils.run_command(['gn', 'gen', '--ide=json', out_dir])
    with open(os.path.join(out_dir, 'project.json')) as f:
        return f.read()


@dataclasses.dataclass
class DepList:
    """Represents a dep list assignment in GN."""
    target_name: Optional[str]  # The name of the target containing the list.
    variable_name: str  # Left-hand side variable name the list is assigned to.
    child_nodes: List[dict]  # Right-hand side list of nodes.


class BuildFile:
    """Represents the contents of a BUILD.gn file."""
    def __init__(self,
                 build_gn_path: str,
                 root_gn_path: str,
                 *,
                 dryrun: bool = False):
        self._root = root_gn_path
        self._rel_path = os.path.relpath(build_gn_path, root_gn_path)
        self._gn_rel_path = '//' + os.path.dirname(self._rel_path)
        self._full_path = os.path.abspath(build_gn_path)
        self._skip_write_content = dryrun

    def __enter__(self):
        output = subprocess_utils.run_command(
            ['gn', 'format', '--dump-tree=json', self._full_path])
        self._content = json.loads(output)
        self._original_content = json.dumps(self._content)
        return self

    def __exit__(self, exc, value, tb):
        if not self._skip_write_content:
            self.write_content_to_file()

    # See: https://gist.github.com/sgraham/bd9ffee312f307d5f417019a9c0f0777
    def _find_all(self, match_fn):
        results = []

        def get_target_name(node) -> Optional[str]:
            """Example format (with irrelevant fields omitted):
            {
                "child": [ {
                    "child": [ {
                        "type": "LITERAL",
                        "value": "\"hello_world_java\""
                    } ],
                    "type": "LIST"
                }, {
                    ...
                } ],
                "type": "FUNCTION",
                "value": "java_library"
            }

            Example return: hello_world_java
            """
            if node.get(NODE_TYPE) != 'FUNCTION':
                return None
            children = node.get(NODE_CHILD)
            if not children:
                return None
            first_child = children[0]
            if first_child.get(NODE_TYPE) != 'LIST':
                return None
            grand_children = first_child.get(NODE_CHILD)
            if not grand_children:
                return None
            grand_child = grand_children[0]
            if grand_child.get(NODE_TYPE) != 'LITERAL':
                return None
            name = grand_child.get(NODE_VALUE)
            if name.startswith('"'):
                return name[1:-1]
            return name

        def recursive_find(root, last_known_target=None):
            target_name = get_target_name(root) or last_known_target
            matched = match_fn(root)
            if matched is not None:
                results.append((target_name, matched))
                return
            children = root.get(NODE_CHILD)
            if children:
                for child in children:
                    recursive_find(child, last_known_target=target_name)

        recursive_find(self._content)
        return results

    def _normalize(self, name: str):
        """Returns the absolute GN path to the target with |name|.

        This method normalizes target names, assuming that relative targets are
        referenced based on the current file, allowing targets to be compared
        by name to determine whether they are the same or not.

        Given the current file is chrome/android/BUILD.gn:

        # Removes surrounding quotation marks.
        "//chrome/android:chrome_java" -> //chrome/android:chrome_java

        # Makes relative paths absolute.
        :chrome_java -> //chrome/android:chrome_java

        # Spells out GN shorthands for basenames.
        //chrome/android -> //chrome/android:android
        """
        if not name:
            return ''
        if name.startswith('"'):
            name = name[1:-1]
        if not name.startswith('//'):
            name = self._gn_rel_path + name
        if not ':' in name:
            name += ':' + os.path.basename(name)
        return name

    def _find_all_list_assignments(self):
        def match_list_assignments(node):
            r"""Matches and returns the list being assigned.

            Binary node
             /       \
            /         \
            name      list of nodes

            Returns the pair (name, list of nodes)
            """
            if node.get(NODE_TYPE) != 'BINARY':
                return None
            children = node.get(NODE_CHILD)
            assert len(children) == 2, (
                'Binary nodes should have two child nodes, but the node is: '
                f'{node}')
            left_child, right_child = children
            if left_child.get(NODE_TYPE) != 'IDENTIFIER':
                return None
            name = left_child.get(NODE_VALUE)
            if right_child.get(NODE_TYPE) != 'LIST':
                return None
            list_of_nodes = right_child.get(NODE_CHILD)
            return name, list_of_nodes

        return self._find_all(match_list_assignments)

    def _find_all_deps_lists(self) -> Iterator[DepList]:
        list_tuples = self._find_all_list_assignments()
        for target_name, (var_name, node_list) in list_tuples:
            if (var_name == 'deps' or var_name.startswith('deps_')
                    or var_name.endswith('_deps') or '_deps_' in var_name):
                yield DepList(target_name=target_name,
                              variable_name=var_name,
                              child_nodes=node_list)

    def split_deps(self, original_dep_name: str,
                   new_dep_names: List[str]) -> bool:
        split = False
        for new_dep_name in new_dep_names:
            if self._split_dep(original_dep_name, new_dep_name):
                split = True
        return split

    def _split_dep(self, original_dep_name: str, new_dep_name: str) -> bool:
        """Add |new_dep_name| to GN deps that contains |original_dep_name|.

        Supports deps, public_deps, and other deps variables.

        Works for explicitly assigning a list to deps:
        deps = [ ..., "original_dep", ...]
        # Becomes
        deps = [ ..., "original_dep", "new_dep", ...]
        Also works for appending a list to deps:
        public_deps += [ ..., "original_dep", ...]
        # Becomes
        public_deps += [ ..., "original_dep", "new_dep", ...]

        Does not work for assigning or appending variables to deps:
        deps = other_list_of_deps # Does NOT check other_list_of_deps.
        # Becomes (no changes)
        deps = other_list_of_deps

        Does not work with parameter expansion, i.e. $variables.

        Returns whether the new dep was added one or more times.
        """
        for dep_name in (original_dep_name, new_dep_name):
            assert dep_name.startswith('//'), (
                f'Absolute GN path required, starting with //: {dep_name}')

        added_new_dep = False
        normalized_original_dep_name = self._normalize(original_dep_name)
        normalized_new_dep_name = self._normalize(new_dep_name)
        for dep_list in self._find_all_deps_lists():
            original_dep_idx = None
            new_dep_already_exists = False
            for idx, child in enumerate(dep_list.child_nodes):
                dep_name = self._normalize(child.get(NODE_VALUE))
                if dep_name == normalized_original_dep_name:
                    original_dep_idx = idx
                if dep_name == normalized_new_dep_name:
                    new_dep_already_exists = True
            if original_dep_idx is not None and not new_dep_already_exists:
                if dep_list.target_name is None:
                    target_str = self._gn_rel_path
                else:
                    target_str = f'{self._gn_rel_path}:{dep_list.target_name}'
                location = f"{target_str}'s {dep_list.variable_name} variable"
                logging.info(f'Adding {new_dep_name} to {location}')
                new_dep = copy.deepcopy(dep_list.child_nodes[original_dep_idx])
                # Any comments associated with the previous dep would not apply.
                for comment_key in (BEFORE_COMMENT, AFTER_COMMENT,
                                    SUFFIX_COMMENT):
                    new_dep.pop(comment_key, None)  # Remove if exists.
                new_dep[NODE_VALUE] = f'"{new_dep_name}"'
                # Add the new dep after the existing dep to preserve comments
                # before the existing dep.
                dep_list.child_nodes.insert(original_dep_idx + 1, new_dep)
                added_new_dep = True

        return added_new_dep

    def remove_deps(self,
                    dep_names: List[str],
                    out_dir: str,
                    targets: List[str],
                    target_name_filter: Optional[str],
                    inline_mode: bool = False) -> Tuple[bool, str]:
        if not inline_mode:
            deps_to_remove = dep_names
        else:
            # If the first dep cannot be removed (or is not found) then in the
            # case of inlining we can skip this file for the rest of the deps.
            first_dep = dep_names[0]
            if not self._remove_deps([first_dep], out_dir, targets,
                                     target_name_filter):
                return False
            deps_to_remove = dep_names[1:]
        return self._remove_deps(deps_to_remove, out_dir, targets,
                                 target_name_filter)

    def _remove_deps(self, dep_names: List[str], out_dir: str,
                     targets: List[str],
                     target_name_filter: Optional[str]) -> Tuple[bool, str]:
        """Remove |dep_names| if the target can still be built in |out_dir|.

        Supports deps, public_deps, and other deps variables.

        Works for explicitly assigning a list to deps:
        deps = [ ..., "original_dep", ...]
        # Becomes
        deps = [ ..., ...]

        Does not work with parameter expansion, i.e. $variables.

        Returns whether any deps were removed.
        """
        normalized_dep_names = set()
        for dep_name in dep_names:
            assert dep_name.startswith('//'), (
                f'Absolute GN path required, starting with //: {dep_name}')
            normalized_dep_names.add(self._normalize(dep_name))

        removed_dep = False
        for dep_list in self._find_all_deps_lists():
            child_deps_to_remove = [
                c for c in dep_list.child_nodes
                if self._normalize(c.get(NODE_VALUE)) in normalized_dep_names
            ]
            if not child_deps_to_remove:
                continue

            if dep_list.target_name is None:
                target_name_str = self._gn_rel_path
            else:
                target_name_str = f'{self._gn_rel_path}:{dep_list.target_name}'
            if (target_name_filter is not None and
                    re.search(target_name_filter, target_name_str) is None):
                logging.info(f'Skip: Since re.search("{target_name_filter}", '
                             f'"{target_name_str}") is None.')
                continue

            location = f"{target_name_str}'s {dep_list.variable_name} variable"
            expected_json = _generate_project_json_content(out_dir)
            num_to_remove = len(child_deps_to_remove)
            for remove_idx, child_dep in enumerate(child_deps_to_remove):
                child_dep_name = self._normalize(child_dep.get(NODE_VALUE))
                idx_to_remove = dep_list.child_nodes.index(child_dep)
                logging.info(f'({remove_idx + 1}/{num_to_remove}) Found '
                             f'{child_dep_name} in {location}.')
                child_to_remove = dep_list.child_nodes[idx_to_remove]
                can_remove_dep = False
                with _backup_and_restore_file_contents(self._full_path):
                    dep_list.child_nodes.remove(child_to_remove)
                    self.write_content_to_file()
                    # Immediately restore deps_list's original value in case the
                    # following build is interrupted. We don't want the
                    # intermediate untested value to be written as the final
                    # build file.
                    dep_list.child_nodes.insert(idx_to_remove, child_to_remove)
                    if expected_json is not None:
                        # If no changes to project.json was detected, this means
                        # the current target is not part of out_dir's build and
                        # cannot be removed even if the build succeeds.
                        after_json = _generate_project_json_content(out_dir)
                        if expected_json == after_json:
                            # If one change in this list isn't part of the
                            # build, no need to try any other in this list.
                            logging.info('Skip: No changes to project.json.')
                            break

                        # Avoids testing every dep removal for the same list.
                        expected_json = None
                    if self._can_still_build_everything(out_dir, targets):
                        can_remove_dep = True
                if not can_remove_dep:
                    continue

                dep_list.child_nodes.remove(child_to_remove)
                # Comments before a target can apply to the targets after.
                if (BEFORE_COMMENT in child_to_remove
                        and idx_to_remove < len(dep_list.child_nodes)):
                    child_after = dep_list.child_nodes[idx_to_remove]
                    if BEFORE_COMMENT not in child_after:
                        child_after[BEFORE_COMMENT] = []
                    child_after[BEFORE_COMMENT][:] = (
                        child_to_remove[BEFORE_COMMENT] +
                        child_after[BEFORE_COMMENT])
                # Comments after or behind a target don't make sense to re-
                # position, simply ignore AFTER_COMMENT and SUFFIX_COMMENT.
                removed_dep = True
                logging.info(f'Removed {child_dep_name} from {location}.')
        return removed_dep

    def _can_still_build_everything(self, out_dir: str,
                                    targets: List[str]) -> bool:
        output = _build_targets_output(out_dir, targets)
        if output is None:
            logging.info('Ninja failed to build all targets')
            return False
        # If ninja did not re-build anything, then the target changed is not
        # among the targets being built. Avoid this change as it's not been
        # tested/used.
        if 'ninja: no work to do.' in output:
            logging.info('Ninja did not find any targets to build')
            return False
        return True

    def write_content_to_file(self) -> None:
        current_content = json.dumps(self._content)
        if current_content != self._original_content:
            subprocess_utils.run_command(
                ['gn', 'format', '--read-tree=json', self._full_path],
                cmd_input=current_content)
