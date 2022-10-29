// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Not allowed for code under disallowed/ but temporarily allowed
// specifically for test code under allowed/.  This regression tests a
// bug where we were taking shallow copies of rules when generating
// rules for subdirectories, so all rule objects were getting the same
// dictionary for specific rules.
#include "buildtools/checkdeps/testdata/disallowed/temp_allowed_for_tests.h"
