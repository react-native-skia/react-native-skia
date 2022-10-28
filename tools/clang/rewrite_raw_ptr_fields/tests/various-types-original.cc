// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

namespace my_namespace {

class SomeClass {
 public:
  void Method(char) {}
  int data_member;
};

template <typename T>
struct SomeTemplate {
  T t;
};

struct MyStruct {
  // Expected rewrite: raw_ptr<raw_ptr<SomeClass>> double_ptr;
  // TODO(lukasza): Handle recursion/nesting.
  SomeClass** double_ptr;

  // Expected rewrite: raw_ptr<void> void_ptr;
  void* void_ptr;

  // |bool*| used to be rewritten as |raw_ptr<_Bool>| which doesn't compile:
  // use of undeclared identifier '_Bool'.
  //
  // Expected rewrite: raw_ptr<bool> bool_ptr;
  bool* bool_ptr;
  // Expected rewrite: raw_ptr<const bool> bool_ptr;
  const bool* const_bool_ptr;

  // Pointers to templates.
  // Expected rewrite: raw_ptr<std::string> string_ptr;
  std::string* string_ptr;
  // Expected rewrite: raw_ptr<std::vector<char>> vector_ptr;
  std::vector<char>* vector_ptr;
  // Expected rewrite: raw_ptr<SomeTemplate<char>> template_ptr;
  SomeTemplate<char>* template_ptr;

  // Some types may be spelled in various, alternative ways.  If possible, the
  // rewriter should preserve the original spelling.
  //
  // Spelling of integer types.
  //
  // Expected rewrite: raw_ptr<int> ...
  int* int_spelling1;
  // Expected rewrite: raw_ptr<signed int> ...
  // TODO(lukasza): Fix?  Today this is rewritten into: raw_ptr<int> ...
  signed int* int_spelling2;
  // Expected rewrite: raw_ptr<long int> ...
  // TODO(lukasza): Fix?  Today this is rewritten into: raw_ptr<long> ...
  long int* int_spelling3;
  // Expected rewrite: raw_ptr<unsigned> ...
  // TODO(lukasza): Fix?  Today this is rewritten into: raw_ptr<unsigned int>
  unsigned* int_spelling4;
  // Expected rewrite: raw_ptr<int32_t> ...
  int32_t* int_spelling5;
  // Expected rewrite: raw_ptr<int64_t> ...
  int64_t* int_spelling6;
  // Expected rewrite: raw_ptr<int_fast32_t> ...
  int_fast32_t* int_spelling7;
  //
  // Spelling of structs and classes.
  //
  // Expected rewrite: raw_ptr<SomeClass> ...
  SomeClass* class_spelling1;
  // Expected rewrite: raw_ptr<class SomeClass> ...
  class SomeClass* class_spelling2;
  // Expected rewrite: raw_ptr<my_namespace::SomeClass> ...
  my_namespace::SomeClass* class_spelling3;

  // No rewrite of function pointers expected, because they won't ever be either
  // A) allocated by PartitionAlloc or B) derived from RawPtrSupport.  In
  // theory |member_data_ptr| below can be A or B, but it can't be expressed as
  // non-pointer T used as a template argument of raw_ptr<>.
  int (*func_ptr)();
  void (SomeClass::*member_func_ptr)(char);  // ~ pointer to SomeClass::Method
  int SomeClass::*member_data_ptr;  // ~ pointer to SomeClass::data_member
  typedef void (*func_ptr_typedef)(char);
  func_ptr_typedef func_ptr_typedef_field;

  // Typedef-ed or type-aliased pointees should participate in the rewriting. No
  // desugaring of the aliases is expected.
  typedef SomeClass SomeClassTypedef;
  using SomeClassAlias = SomeClass;
  typedef void (*func_ptr_typedef2)(char);
  // Expected rewrite: raw_ptr<SomeClassTypedef> ...
  SomeClassTypedef* typedef_ptr;
  // Expected rewrite: raw_ptr<SomeClassAlias> ...
  SomeClassAlias* alias_ptr;
  // Expected rewrite: raw_ptr<func_ptr_typedef2> ...
  func_ptr_typedef2* ptr_to_function_ptr;

  // Typedefs and type alias definitions should not be rewritten.
  //
  // No rewrite expected (for now - in V1 we only rewrite field decls).
  typedef SomeClass* SomeClassPtrTypedef;
  // No rewrite expected (for now - in V1 we only rewrite field decls).
  using SomeClassPtrAlias = SomeClass*;

  // Char pointer fields should be rewritten, unless they are on the
  // --field-filter-file blocklist.  See also gen-char-test.cc for tests
  // covering generating the blocklist.
  //
  // Expected rewrite: raw_ptr<char>, etc.
  char* char_ptr;
  const char* const_char_ptr;
  wchar_t* wide_char_ptr;
  const wchar_t* const_wide_char_ptr;

  // |array_of_ptrs| is an array 123 of pointer to SomeClass.
  // No rewrite expected (this is not a pointer - this is an array).
  SomeClass* ptr_array[123];

  // |ptr_to_array| is a pointer to array 123 of const SomeClass.
  //
  // This test is based on EqualsFramesMatcher from
  // //net/websockets/websocket_channel_test.cc
  //
  // No rewrite expected (this *is* a pointer, but generating a correct
  // replacement is tricky, because the |replacement_range| needs to cover
  // "[123]" that comes *after* the field name).
  const SomeClass (*ptr_to_array)[123];
};

extern "C" {
struct OtherForeignStruct;
struct ForeignStruct {
  // We should not rewrite foreign, extern "C" structs.
  OtherForeignStruct* ptr;
};
}

}  // namespace my_namespace
