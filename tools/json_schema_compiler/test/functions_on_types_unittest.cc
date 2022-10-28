// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/json_schema_compiler/test/functions_on_types.h"

#include <utility>
#include <vector>

#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace functions_on_types = test::api::functions_on_types;

TEST(JsonSchemaCompilerFunctionsOnTypesTest, StorageAreaGetParamsCreate) {
  {
    std::vector<base::Value> params_value;
    std::unique_ptr<functions_on_types::StorageArea::Get::Params> params(
        functions_on_types::StorageArea::Get::Params::Create(params_value));
    ASSERT_TRUE(params);
    EXPECT_FALSE(params->keys);
  }
  {
    std::vector<base::Value> params_value;
    params_value.emplace_back(9);
    std::unique_ptr<functions_on_types::StorageArea::Get::Params> params(
        functions_on_types::StorageArea::Get::Params::Create(params_value));
    EXPECT_FALSE(params);
  }
  {
    std::vector<base::Value> params_value;
    params_value.emplace_back("test");
    std::unique_ptr<functions_on_types::StorageArea::Get::Params> params(
        functions_on_types::StorageArea::Get::Params::Create(params_value));
    ASSERT_TRUE(params);
    ASSERT_TRUE(params->keys);
    EXPECT_EQ("test", *params->keys->as_string);
  }
  {
    base::Value keys_object_value(base::Value::Type::DICTIONARY);
    keys_object_value.SetIntKey("integer", 5);
    keys_object_value.SetStringKey("string", "string");
    std::vector<base::Value> params_value;
    params_value.push_back(keys_object_value.Clone());
    std::unique_ptr<functions_on_types::StorageArea::Get::Params> params(
        functions_on_types::StorageArea::Get::Params::Create(params_value));
    ASSERT_TRUE(params);
    ASSERT_TRUE(params->keys);
    EXPECT_EQ(keys_object_value,
              params->keys->as_object->additional_properties);
  }
}

TEST(JsonSchemaCompilerFunctionsOnTypesTest, StorageAreaGetResultCreate) {
  functions_on_types::StorageArea::Get::Results::Items items;
  items.additional_properties.SetDouble("asdf", 0.1);
  items.additional_properties.SetString("sdfg", "zxcv");
  base::Value results(
      functions_on_types::StorageArea::Get::Results::Create(items));
  ASSERT_TRUE(results.is_list());
  ASSERT_EQ(1u, results.GetList().size());
  EXPECT_EQ(items.additional_properties, results.GetList()[0]);
}

TEST(JsonSchemaCompilerFunctionsOnTypesTest, ChromeSettingGetParamsCreate) {
  base::Value details_value(base::Value::Type::DICTIONARY);
  details_value.SetBoolKey("incognito", true);
  std::vector<base::Value> params_value;
  params_value.push_back(std::move(details_value));
  std::unique_ptr<functions_on_types::ChromeSetting::Get::Params> params(
      functions_on_types::ChromeSetting::Get::Params::Create(params_value));
  EXPECT_TRUE(params.get());
  EXPECT_TRUE(*params->details.incognito);
}
