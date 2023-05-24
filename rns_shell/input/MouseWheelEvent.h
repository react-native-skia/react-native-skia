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

struct INPUT_EXPORT MouseWheelEvent {
  int eventX;
  int eventY;

  int deltaX;
  int deltaY;
};

} // namespace RnsShell
