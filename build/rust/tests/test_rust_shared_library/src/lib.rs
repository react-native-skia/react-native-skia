// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#[cxx::bridge]
mod ffi {
    pub struct SomeStruct {
        a: i32,
    }
    extern "Rust" {
        fn say_hello();
        fn allocate_via_rust() -> Box<SomeStruct>;
        fn add_two_ints_via_rust(x: i32, y: i32) -> i32;
    }
}

pub fn say_hello() {
    println!(
        "Hello, world - from a Rust library. Calculations suggest that 3+4={}",
        add_two_ints_via_rust(3, 4)
    );
}

#[test]
fn test_hello() {
    assert_eq!(7, add_two_ints_via_rust(3, 4));
}

pub fn add_two_ints_via_rust(x: i32, y: i32) -> i32 {
    x + y
}

// The next function is used from the
// AllocatorTest.RustComponentUsesPartitionAlloc unit test.
pub fn allocate_via_rust() -> Box<ffi::SomeStruct> {
    Box::new(ffi::SomeStruct { a: 43 })
}
