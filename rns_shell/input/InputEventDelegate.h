/*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "rns_shell/input/export.h"

namespace RnsShell {

struct MouseWheelEvent;

class INPUT_EXPORT InputEventDelegate {
 public:
  InputEventDelegate() = default;
  InputEventDelegate(const InputEventDelegate&) = delete;
  InputEventDelegate& operator=(const InputEventDelegate&) = delete;
  virtual ~InputEventDelegate() = default;

  virtual void DispatchInputEvent(MouseWheelEvent &&event) = 0;
};

} // namespace RnsShell
