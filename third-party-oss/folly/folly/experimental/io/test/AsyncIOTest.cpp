/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/experimental/io/AsyncIO.h>
#include <folly/experimental/io/test/AsyncBaseTestLib.h>

using folly::AsyncIO;

namespace folly {
namespace test {
INSTANTIATE_TYPED_TEST_CASE_P(AsyncTest, AsyncTest, AsyncIO);
} // namespace test
} // namespace folly
