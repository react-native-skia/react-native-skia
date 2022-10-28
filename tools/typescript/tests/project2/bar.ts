// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {foo} from '../project1/foo.js';
import {baz} from '../project3/baz.js';

import {bar} from 'chrome://some-other-source/legacy_file.js';

// Intentionally impoting unused dependency to test the --tsconfig_base flag.
import {num} from 'chrome://some-other-source/foo.js';

function doNothing(): void {
  console.log(foo());
  console.log(bar());
  console.log(baz());
}

doNothing();
