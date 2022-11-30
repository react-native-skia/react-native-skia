/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <react/renderer/core/propsConversions.h>

#include "ActivityIndicatorProps.h"
namespace facebook {
namespace react {

ActivityIndicatorProps::ActivityIndicatorProps(const ActivityIndicatorProps &sourceProps, const RawProps &rawProps)
    : ViewProps(sourceProps, rawProps),
        animating(convertRawProp(
          rawProps,
          "animating",
          sourceProps.animating,
          {})),
        color(convertRawProp(
          rawProps,
          "color",
          sourceProps.color,
          {})),
        hidesWhenStopped(convertRawProp(
          rawProps,
          "hidesWhenStopped",
          sourceProps.hidesWhenStopped,
          {})),
        size(convertRawProp(
          rawProps,
          "size",
          sourceProps.size,
          {}))
          {}

} // namespace react
} // namespace facebook