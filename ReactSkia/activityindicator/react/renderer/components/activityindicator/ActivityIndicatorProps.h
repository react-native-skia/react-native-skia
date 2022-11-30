/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <react/renderer/components/view/ViewProps.h>

namespace facebook {
namespace react {

class ActivityIndicatorProps final : public ViewProps {
 public:
  ActivityIndicatorProps() = default;
  ActivityIndicatorProps(const ActivityIndicatorProps &sourceProps, const RawProps &rawProps);

#pragma mark - Props
  bool animating{true};
  SharedColor color{}; // Framework will provide a default color(Color Value:#999999 As per IOS Documentation).
  bool hidesWhenStopped{true};
  std::string const size{}; // In RNS we will use layout frame width & height instead of this size. With that we can support both string and number size(similar to android implementation).
};

} // namespace react
} // namespace facebook
