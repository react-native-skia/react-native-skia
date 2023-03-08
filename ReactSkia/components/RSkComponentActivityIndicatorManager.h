/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "rns_shell/compositor/layers/Layer.h"
#include "ReactSkia/components/RSkComponentActivityIndicator.h"
#include "ReactSkia/utils/RnsJsRaf.h"

namespace facebook {
namespace react {
class RSkComponentActivityIndicatorManager {
 public:
  static RSkComponentActivityIndicatorManager *getActivityIndicatorManager();

  void addComponent(std::weak_ptr<RSkComponent> candidate);
  void removeComponent(Tag tag);
  ~RSkComponentActivityIndicatorManager();
  
 private:
  static RSkComponentActivityIndicatorManager *activityIndicatorManager_;
  std::vector<std::weak_ptr<RSkComponent>> actIndComponentList_;
  RnsJsRequestAnimation * animRequest_{nullptr};

  RSkComponentActivityIndicatorManager();
  void handleActivityIndicatorAnimation(double timestamp);
};
} // namespace react
} // namespace facebook
