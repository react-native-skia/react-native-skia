/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/components/RSkComponentActivityIndicatorManager.h"

#define RNS_ACTIVITY_INDICATOR_ROTATION_ANGLE       (360 / RNS_ANIMATION_FRAME_RATE)

std::mutex mutex_;

namespace facebook {
namespace react {

RSkComponentActivityIndicatorManager *RSkComponentActivityIndicatorManager::activityIndicatorManager_{nullptr};

RSkComponentActivityIndicatorManager::RSkComponentActivityIndicatorManager(){
  animRequest_ = new RnsJsRequestAnimation([this](double timestamp){
    RNS_LOG_DEBUG("[" << this->animRequest_ << "] Register Activity Indicator request Animation callback [" << timestamp << "]");
    handleActivityIndicatorAnimation(timestamp);
  });
}

RSkComponentActivityIndicatorManager* RSkComponentActivityIndicatorManager::getActivityIndicatorManager(){
  std::lock_guard<std::mutex> lock(mutex_);
  if (activityIndicatorManager_ == nullptr){
    activityIndicatorManager_ = new RSkComponentActivityIndicatorManager;
  }
  return activityIndicatorManager_;
}

void RSkComponentActivityIndicatorManager::addComponent(std::weak_ptr<RSkComponent> candidate){
  std::shared_ptr<RSkComponent> component = candidate.lock();

  if(component && component->layer().get()){
    actIndComponentList_.push_back(candidate);

    if(actIndComponentList_.size() == 1){
      animRequest_->start();
    }
  } 
}

void RSkComponentActivityIndicatorManager::removeComponent(Tag tag){
  if(actIndComponentList_.empty() == true){
    return;
  }
  
  std::vector<std::weak_ptr<RSkComponent>>::iterator iter;

  for(iter = actIndComponentList_.begin(); iter != actIndComponentList_.end(); iter++){
    std::shared_ptr<RSkComponent> component = iter->lock();

    if(component == nullptr || component->getComponentData().tag == tag){
      actIndComponentList_.erase(iter);
      break;
    }
  }

  if(actIndComponentList_.size() == 0){
    animRequest_->stop();
  } 
}

void RSkComponentActivityIndicatorManager::handleActivityIndicatorAnimation(double timestamp){
  if(actIndComponentList_.empty() == true){
    return;
  }

  std::vector<std::weak_ptr<RSkComponent>>::iterator iter = actIndComponentList_.begin();
  std::shared_ptr<RSkComponent> firstComponent = iter->lock();

  if(firstComponent){
    std::shared_ptr<RnsShell::Layer> layer = firstComponent->layer();

    if(layer.get() != nullptr){
      layer->client().notifyFlushBegin();

      for( auto & iter : actIndComponentList_){
        std::shared_ptr<RSkComponent> component = iter.lock();

        if(component && component->layer().get() != nullptr){
          component->layer()->transformMatrix.preRotate(RNS_ACTIVITY_INDICATOR_ROTATION_ANGLE);
          component->layer()->invalidate( RnsShell::LayerLayoutInvalidate);
        }
      }

      layer->client().notifyFlushRequired();
    }
  }
}

RSkComponentActivityIndicatorManager::~RSkComponentActivityIndicatorManager(){
  delete activityIndicatorManager_;
  activityIndicatorManager_ = nullptr;
}

} // namespace react
} // namespace facebook
