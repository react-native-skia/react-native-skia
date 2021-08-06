/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "react/renderer/components/image/ImageShadowNode.h"

namespace facebook {
namespace react {

namespace RSkImageUtils{
  
  SkRect computeTargetRect (Size srcSize,SkRect targetRect,ImageResizeMode resizeMode); 

} //namespace RSkImageUtils

} // namespace react
} // namespace facebook
