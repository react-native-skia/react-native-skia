// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class SomeClass;

// Expected rewrite: typedef raw_ptr<SomeClass> SomeClassPtrTypedef.
// TODO(lukasza): Handle rewriting typedefs.
typedef SomeClass* SomeClassPtrTypedef;

// Expected rewrite: using SomeClassPtrTypeAlias = raw_ptr<SomeClass>;
// TODO(lukasza): Handle rewriting type aliases.
using SomeClassPtrTypeAlias = SomeClass*;

struct MyStruct {
  // No rewrite expected here.
  SomeClassPtrTypedef field1;
  SomeClassPtrTypeAlias field2;

  // Only "shallow" rewrite expected here (without unsugaring/inlining the type
  // aliases).  So:
  // Expected rewrite: raw_ptr<SomeClassPtrTypedef> field3;
  SomeClassPtrTypedef* field3;
  // Expected rewrite: raw_ptr<SomeClassPtrTypeAlias> field4;
  SomeClassPtrTypeAlias* field4;
};
