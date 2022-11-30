/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <react/renderer/components/view/ConcreteViewShadowNode.h>

#include "ActivityIndicatorProps.h"

namespace facebook {
namespace react {

extern const char ActivityIndicatorComponentName[];

/*
 * `ShadowNode` for <ActivityIndicator> component.
 */

using ActivityIndicatorShadowNode = ConcreteViewShadowNode<ActivityIndicatorComponentName,
                                                            ActivityIndicatorProps>;

} // namespace react
} // namespace facebook