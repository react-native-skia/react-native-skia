// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.tools.errorprone.plugin;

import com.google.auto.service.AutoService;
import com.google.errorprone.BugPattern;
import com.google.errorprone.VisitorState;
import com.google.errorprone.bugpatterns.BugChecker;
import com.google.errorprone.matchers.Description;
import com.google.errorprone.matchers.Matcher;
import com.google.errorprone.matchers.Matchers;
import com.sun.source.tree.AnnotationTree;

/**
 * Assert class names of classes with Java tests in them end with "Test".
 *
 * Test classes are identified as those that are annotated with {@link org.junit.runner.RunWith}.
 * This works for both JUnit and Instrumentation tests.
 */
@AutoService(BugChecker.class)
@BugPattern(name = "TestClassNameCheck",
        summary = "Class names of classes with tests in them should end with \"Test\".",
        severity = BugPattern.SeverityLevel.ERROR, linkType = BugPattern.LinkType.CUSTOM,
        link = "http://crbug.com/1029370")
public class TestClassNameCheck extends BugChecker implements BugChecker.AnnotationTreeMatcher {
    static final Matcher<AnnotationTree> TEST_ANNOTATION =
            Matchers.anyOf(Matchers.isType("org.junit.runner.RunWith"));

    /**
     * Match if nullable annotation is of type org.junit.runner.RunWith.
     */
    @Override
    public Description matchAnnotation(AnnotationTree annotationTree, VisitorState visitorState) {
        String filePath = visitorState.getPath().getCompilationUnit().getSourceFile().getName();
        if (TEST_ANNOTATION.matches(annotationTree, visitorState)
                && !filePath.endsWith("Test.java")) {
            return describeMatch(annotationTree);
        }
        return Description.NO_MATCH;
    }
}
