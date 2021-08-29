/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RSkViewProps.h"

#include "react/renderer/core/propsConversions.h"

namespace facebook {
namespace react {

RSkViewProps::RSkViewProps(
    RSkViewProps const &sourceProps,
    RawProps const &rawProps)
    : ViewProps(sourceProps, rawProps),
      isSkiaProp(convertRawProp(
          rawProps,
          "isSkiaProp",
          sourceProps.isSkiaProp,
          false)) {}

#pragma mark - DebugStringConvertible

#if RN_DEBUG_STRING_CONVERTIBLE
SharedDebugStringConvertibleList RSkViewProps::getDebugProps() const {
  return ViewProps::getDebugProps();
}
#endif

} // namespace react
} // namespace facebook
