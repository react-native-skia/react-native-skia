// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>
#include <vector>

#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "tools/json_schema_compiler/test/any.h"

TEST(JsonSchemaCompilerAnyTest, AnyTypePopulate) {
  {
    test::api::any::AnyType any_type;
    auto any_type_value = std::make_unique<base::DictionaryValue>();
    any_type_value->SetString("any", "value");
    EXPECT_TRUE(test::api::any::AnyType::Populate(*any_type_value, &any_type));
    std::unique_ptr<base::Value> any_type_to_value(any_type.ToValue());
    EXPECT_TRUE(any_type_value->Equals(any_type_to_value.get()));
  }
  {
    test::api::any::AnyType any_type;
    auto any_type_value = std::make_unique<base::DictionaryValue>();
    any_type_value->SetInteger("any", 5);
    EXPECT_TRUE(test::api::any::AnyType::Populate(*any_type_value, &any_type));
    std::unique_ptr<base::Value> any_type_to_value(any_type.ToValue());
    EXPECT_TRUE(any_type_value->Equals(any_type_to_value.get()));
  }
}

TEST(JsonSchemaCompilerAnyTest, OptionalAnyParamsCreate) {
  {
    std::vector<base::Value> params_value;
    std::unique_ptr<test::api::any::OptionalAny::Params> params(
        test::api::any::OptionalAny::Params::Create(params_value));
    EXPECT_TRUE(params.get());
    EXPECT_FALSE(params->any_name.get());
  }
  {
    std::vector<base::Value> params_value;
    base::Value param("asdf");
    params_value.push_back(param.Clone());
    std::unique_ptr<test::api::any::OptionalAny::Params> params(
        test::api::any::OptionalAny::Params::Create(params_value));
    ASSERT_TRUE(params);
    ASSERT_TRUE(params->any_name);
    EXPECT_TRUE(params->any_name->Equals(&param));
  }
  {
    std::vector<base::Value> params_value;
    base::Value param(true);
    params_value.push_back(param.Clone());
    std::unique_ptr<test::api::any::OptionalAny::Params> params(
        test::api::any::OptionalAny::Params::Create(params_value));
    ASSERT_TRUE(params);
    ASSERT_TRUE(params->any_name);
    EXPECT_TRUE(params->any_name->Equals(&param));
  }
}
