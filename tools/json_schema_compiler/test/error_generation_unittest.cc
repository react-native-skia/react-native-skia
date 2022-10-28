// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/json_schema_compiler/test/error_generation.h"

#include <memory>
#include <vector>

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "tools/json_schema_compiler/test/test_util.h"

namespace errors = test::api::error_generation;
using base::Value;
using json_schema_compiler::test_util::Dictionary;
using json_schema_compiler::test_util::List;

template <typename T>
std::u16string GetPopulateError(const Value& value) {
  std::u16string error;
  T test_type;
  T::Populate(value, &test_type, &error);
  return error;
}

testing::AssertionResult EqualsUtf16(const std::string& expected,
                                     const std::u16string& actual) {
  if (base::ASCIIToUTF16(expected) == actual)
    return testing::AssertionSuccess();
  return testing::AssertionFailure() << "\n    actual:     " << actual
                                     << "\n    expected:   " << expected;
}

// GenerateTypePopulate errors

TEST(JsonSchemaCompilerErrorTest, RequiredPropertyPopulate) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>("bling"));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::TestType>(*value)));
  }
  {
    auto value = std::make_unique<Value>(Value::Type::BINARY);
    EXPECT_TRUE(EqualsUtf16("expected dictionary, got binary",
                            GetPopulateError<errors::TestType>(*value)));
  }
}

TEST(JsonSchemaCompilerErrorTest, UnexpectedTypePopulation) {
  {
    auto value = std::make_unique<base::ListValue>();
    EXPECT_TRUE(EqualsUtf16(
        "", GetPopulateError<errors::ChoiceType::Integers>(*value)));
  }
  {
    auto value = std::make_unique<Value>(Value::Type::BINARY);
    EXPECT_TRUE(
        EqualsUtf16("expected integers or integer, got binary",
                    GetPopulateError<errors::ChoiceType::Integers>(*value)));
  }
}

// GenerateTypePopulateProperty errors

TEST(JsonSchemaCompilerErrorTest, TypeIsRequired) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("integers", std::make_unique<Value>(5));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::ChoiceType>(*value)));
  }
  {
    auto value = std::make_unique<base::DictionaryValue>();
    EXPECT_TRUE(EqualsUtf16("'integers' is required",
                            GetPopulateError<errors::ChoiceType>(*value)));
  }
}

// GenerateParamsCheck errors

TEST(JsonSchemaCompilerErrorTest, TooManyParameters) {
  {
    std::vector<Value> params_value;
    params_value.emplace_back(5);
    std::u16string error;
    EXPECT_TRUE(errors::TestFunction::Params::Create(params_value, &error));
  }
  {
    std::vector<Value> params_value;
    params_value.emplace_back(5);
    params_value.emplace_back(5);
    std::u16string error;
    EXPECT_FALSE(errors::TestFunction::Params::Create(params_value, &error));
    EXPECT_TRUE(EqualsUtf16("expected 1 arguments, got 2", error));
  }
}

// GenerateFunctionParamsCreate errors

TEST(JsonSchemaCompilerErrorTest, ParamIsRequired) {
  {
    std::vector<Value> params_value;
    params_value.emplace_back(5);
    std::u16string error;
    EXPECT_TRUE(errors::TestFunction::Params::Create(params_value, &error));
  }
  {
    std::vector<Value> params_value;
    params_value.emplace_back();
    std::u16string error;
    EXPECT_FALSE(errors::TestFunction::Params::Create(params_value, &error));
    EXPECT_TRUE(EqualsUtf16("'num' is required", error));
  }
}

// GeneratePopulateVariableFromValue errors

TEST(JsonSchemaCompilerErrorTest, WrongPropertyValueType) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>("yes"));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::TestType>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>(1.1));
    EXPECT_TRUE(EqualsUtf16("'string': expected string, got double",
                            GetPopulateError<errors::TestType>(*value)));
  }
}

TEST(JsonSchemaCompilerErrorTest, WrongParameterCreationType) {
  {
    std::u16string error;
    std::vector<Value> params_value;
    params_value.emplace_back("Yeah!");
    EXPECT_TRUE(errors::TestString::Params::Create(params_value, &error));
  }
  {
    std::vector<Value> params_value;
    params_value.emplace_back(5);
    std::u16string error;
    EXPECT_FALSE(
        errors::TestTypeInObject::Params::Create(params_value, &error));
    EXPECT_TRUE(EqualsUtf16("'paramObject': expected dictionary, got integer",
        error));
  }
}

TEST(JsonSchemaCompilerErrorTest, WrongTypeValueType) {
  {
    auto value = std::make_unique<base::DictionaryValue>();
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::ObjectType>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("otherType", std::make_unique<Value>(1.1));
    errors::ObjectType out;
    std::u16string error;
    EXPECT_FALSE(errors::ObjectType::Populate(*value, &out, &error));
    EXPECT_TRUE(EqualsUtf16("'otherType': expected dictionary, got double",
        error));
    EXPECT_EQ(NULL, out.other_type.get());
  }
}

TEST(JsonSchemaCompilerErrorTest, UnableToPopulateArray) {
  {
    std::unique_ptr<base::ListValue> params_value =
        List(std::make_unique<Value>(5));
    EXPECT_TRUE(EqualsUtf16(
        "", GetPopulateError<errors::ChoiceType::Integers>(*params_value)));
  }
  {
    std::unique_ptr<base::ListValue> params_value =
        List(std::make_unique<Value>(5), std::make_unique<Value>(false));
    EXPECT_TRUE(EqualsUtf16(
        "Error at key 'integers': Parsing array failed at index 1: expected "
        "integer, got boolean",
        GetPopulateError<errors::ChoiceType::Integers>(*params_value)));
  }
}

TEST(JsonSchemaCompilerErrorTest, BinaryTypeExpected) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("data", std::make_unique<Value>(Value::Type::BINARY));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::BinaryData>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("data", std::make_unique<Value>(1.1));
    EXPECT_TRUE(EqualsUtf16("'data': expected binary, got double",
                            GetPopulateError<errors::BinaryData>(*value)));
  }
}

TEST(JsonSchemaCompilerErrorTest, ListExpected) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("TheArray", std::make_unique<base::ListValue>());
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::ArrayObject>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("TheArray", std::make_unique<Value>(5));
    EXPECT_TRUE(EqualsUtf16("'TheArray': expected list, got integer",
                            GetPopulateError<errors::ArrayObject>(*value)));
  }
}

// GenerateStringToEnumConversion errors

TEST(JsonSchemaCompilerErrorTest, BadEnumValue) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("enumeration", std::make_unique<Value>("one"));
    EXPECT_TRUE(
        EqualsUtf16("", GetPopulateError<errors::HasEnumeration>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("enumeration", std::make_unique<Value>("bad sauce"));
    EXPECT_TRUE(
        EqualsUtf16("'Enumeration': expected \"one\" or \"two\" "
                    "or \"three\", got \"bad sauce\"",
                    GetPopulateError<errors::HasEnumeration>(*value)));
  }
}

TEST(JsonSchemaCompilerErrorTest, ErrorOnOptionalFailure) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>("bling"));
    EXPECT_TRUE(
        EqualsUtf16("", GetPopulateError<errors::OptionalTestType>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>(1));

    errors::OptionalTestType out;
    std::u16string error;
    EXPECT_FALSE(errors::OptionalTestType::Populate(*value, &out, &error));
    EXPECT_TRUE(EqualsUtf16("'string': expected string, got integer",
        error));
    EXPECT_EQ(NULL, out.string.get());
  }
}

TEST(JsonSchemaCompilerErrorTest, OptionalBinaryTypeFailure) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("data", std::make_unique<Value>(Value::Type::BINARY));
    EXPECT_TRUE(
        EqualsUtf16("", GetPopulateError<errors::OptionalBinaryData>(*value)));
  }
  {
    // There's a bug with silent failures if the key doesn't exist.
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("data", std::make_unique<Value>(1));

    errors::OptionalBinaryData out;
    std::u16string error;
    EXPECT_FALSE(errors::OptionalBinaryData::Populate(*value, &out, &error));
    EXPECT_TRUE(EqualsUtf16("'data': expected binary, got integer",
        error));
    EXPECT_EQ(NULL, out.data.get());
  }
}

TEST(JsonSchemaCompilerErrorTest, OptionalArrayTypeFailure) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("TheArray", std::make_unique<base::ListValue>());
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::ArrayObject>(*value)));
  }
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("TheArray", std::make_unique<Value>(5));
    errors::ArrayObject out;
    std::u16string error;
    EXPECT_FALSE(errors::ArrayObject::Populate(*value, &out, &error));
    EXPECT_TRUE(EqualsUtf16("'TheArray': expected list, got integer",
        error));
    EXPECT_EQ(NULL, out.the_array.get());
  }
}

TEST(JsonSchemaCompilerErrorTest, OptionalUnableToPopulateArray) {
  {
    std::unique_ptr<base::ListValue> params_value =
        List(std::make_unique<Value>(5));
    EXPECT_TRUE(EqualsUtf16(
        "",
        GetPopulateError<errors::OptionalChoiceType::Integers>(*params_value)));
  }
  {
    std::unique_ptr<base::ListValue> params_value =
        List(std::make_unique<Value>(5), std::make_unique<Value>(false));
    errors::OptionalChoiceType::Integers out;
    std::u16string error;
    EXPECT_FALSE(errors::OptionalChoiceType::Integers::Populate(*params_value,
                                                                &out, &error));
    EXPECT_TRUE(
        EqualsUtf16("Error at key 'integers': Parsing array failed at index 1: "
                    "expected integer, got boolean",
                    error));
    EXPECT_EQ(NULL, out.as_integer.get());
  }
}

TEST(JsonSchemaCompilerErrorTest, TooManyKeys) {
  {
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>("yes"));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::TestType>(*value)));
  }
  {
    // We simply ignore extra keys.
    std::unique_ptr<base::DictionaryValue> value =
        Dictionary("string", std::make_unique<Value>("yes"), "ohno",
                   std::make_unique<Value>("many values"));
    EXPECT_TRUE(EqualsUtf16("", GetPopulateError<errors::TestType>(*value)));
  }
}
