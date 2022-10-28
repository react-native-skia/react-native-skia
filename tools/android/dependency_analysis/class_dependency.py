# Lint as: python3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Implementation of the graph module for a [Java class] dependency graph."""

import re
from typing import Set, Tuple

import graph
import class_json_consts

# Matches w/o parens: (some.package.name).(class)$($optional$nested$class)
JAVA_CLASS_FULL_NAME_REGEX = re.compile(
    r'^(?P<package>.*)\.(?P<class_name>.*?)(\$(?P<nested>.*))?$')


def java_class_params_to_key(package: str, class_name: str):
    """Returns the unique key created from a package and class name."""
    return f'{package}.{class_name}'


def split_nested_class_from_key(key: str) -> Tuple[str, str]:
    """Splits a jdeps class name into its key and nested class, if any."""
    re_match = JAVA_CLASS_FULL_NAME_REGEX.match(key)
    package = re_match.group('package')
    class_name = re_match.group('class_name')
    nested = re_match.group('nested')
    return java_class_params_to_key(package, class_name), nested


class JavaClass(graph.Node):
    """A representation of a Java class.

    Some classes may have nested classes (eg. explicitly, or
    implicitly through lambdas). We treat these nested classes as part of
    the outer class, storing only their names as metadata.
    """
    def __init__(self, package: str, class_name: str):
        """Initializes a new Java class structure.

        The package and class_name are used to create a unique key per class.

        Args:
            package: The package the class belongs to.
            class_name: The name of the class. For nested classes, this is
                the name of the class that contains them.
        """
        super().__init__(java_class_params_to_key(package, class_name))

        self._package = package
        self._class_name = class_name

        self._nested_classes = set()
        self._build_targets = set()

    @property
    def package(self):
        """The package the class belongs to."""
        return self._package

    @property
    def class_name(self):
        """The name of the class.

        For nested classes, this is the name of the class that contains them.
        """
        return self._class_name

    @property
    def nested_classes(self):
        """A set of nested classes contained within this class."""
        return self._nested_classes

    @nested_classes.setter
    def nested_classes(self, other):
        self._nested_classes = other

    @property
    def build_targets(self) -> Set[str]:
        """Which build target(s) contain the class."""
        # TODO(crbug.com/1124836): Make this return a List, sorted by
        # importance.
        return self._build_targets

    @build_targets.setter
    def build_targets(self, other):
        self._build_targets = other

    def add_nested_class(self, nested: str):
        self._nested_classes.add(nested)

    def add_build_target(self, build_target: str) -> None:
        self._build_targets.add(build_target)

    def get_node_metadata(self):
        """Generates JSON metadata for the current node.

        The list of nested classes is sorted in order to help with testing.
        Structure:
        {
            'package': str,
            'class': str,
            'build_targets' [ str, ... ]
            'nested_classes': [ class_key, ... ],
        }
        """
        return {
            class_json_consts.PACKAGE: self.package,
            class_json_consts.CLASS: self.class_name,
            class_json_consts.BUILD_TARGETS: sorted(self.build_targets),
            class_json_consts.NESTED_CLASSES: sorted(self.nested_classes),
        }


class JavaClassDependencyGraph(graph.Graph):
    """A graph representation of the dependencies between Java classes.

    A directed edge A -> B indicates that A depends on B.
    """
    def create_node_from_key(self, key: str):
        """See comment above the regex definition."""
        re_match = JAVA_CLASS_FULL_NAME_REGEX.match(key)
        package = re_match.group('package')
        class_name = re_match.group('class_name')
        return JavaClass(package, class_name)
