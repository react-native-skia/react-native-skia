// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include "third_party/skia/include/core/SkTypes.h"

void SkDebugf_FileLine(const char* file, int line, const char* format, ...) {
}

void SkAbort_FileLine(const char* file, int line, const char* format, ...) {
  abort();
}
