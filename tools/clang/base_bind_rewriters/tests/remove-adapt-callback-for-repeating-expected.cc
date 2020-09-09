// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "callback.h"

void Foo(base::OnceClosure) {}

void Bar(int, base::OnceClosure, int) {}

void Test() {
  base::OnceClosure cb = base::BindOnce([] {});
  Foo(base::BindOnce([] {}));
  Bar(1, base::BindOnce([] {}), 1);

  using namespace base;

  OnceClosure cb2 = BindOnce([] {});
  Foo(BindOnce([] {}));
  Bar(1, BindOnce([] {}), 1);

  OnceClosure cb3 = BindOnce([] {});
}
