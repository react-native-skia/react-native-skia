// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <memory>

#include "base/allocator/buildflags.h"
#include "base/allocator/partition_allocator/address_pool_manager_bitmap.h"
#include "base/allocator/partition_allocator/partition_address_space.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "build/rust/tests/test_mixed_source_set/test_mixed_source_set.h"
#include "build/rust/tests/test_rust_source_set/src/lib.rs.h"

TEST(RustTest, CppCallingIntoRust_BasicFFI) {
  EXPECT_EQ(7, add_two_ints_via_rust(3, 4));
}

TEST(RustTest, RustComponentUsesPartitionAlloc) {
  // Verify that PartitionAlloc is consistently used in C++ and Rust.
  auto cpp_allocated_int = std::make_unique<int>();
  SomeStruct* rust_allocated_ptr = allocate_via_rust().into_raw();
  EXPECT_EQ(base::IsManagedByPartitionAlloc(rust_allocated_ptr),
            base::IsManagedByPartitionAlloc(cpp_allocated_int.get()));
  rust::Box<SomeStruct>::from_raw(rust_allocated_ptr);
}

TEST(RustTest, CppCallingIntoRustAndBack_BasicFFI) {
  EXPECT_EQ(10u, add_two_ints_via_rust_then_cpp(6, 4));
}
