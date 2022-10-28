// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

pub fn say_hello_from_v1() {
    println!("Hello, world - from lib version 1");
}

pub fn say_hello_from_v2() {
    panic!("We expected to say hello from version 2");
}
