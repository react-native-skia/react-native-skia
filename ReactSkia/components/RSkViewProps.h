/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "react/renderer/components/view/ViewProps.h"

namespace facebook {
namespace react {

class RSkViewProps;

using SharedRSkViewProps = std::shared_ptr<RSkViewProps const>;

class RSkViewProps : public ViewProps {
 public:
  RSkViewProps() = default;
  RSkViewProps(RSkViewProps const &sourceProps, RawProps const &rawProps);

#pragma mark - Props

  bool isSkiaProp{false};

#pragma mark - DebugStringConvertible

#if RN_DEBUG_STRING_CONVERTIBLE
  SharedDebugStringConvertibleList getDebugProps() const override;
#endif
};

} // namespace react
} // namespace facebook
