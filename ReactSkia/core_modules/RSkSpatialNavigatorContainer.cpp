/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/


#include "RSkSpatialNavigatorContainer.h"
#include "ReactSkia/components/RSkComponent.h"

namespace SpatialNavigator {

void Container::addComponent(RSkComponent *newCandidate) {
  navComponentList_.push_back(newCandidate);
  RSkSpatialNavigator::sharedSpatialNavigator()->updateSpatialNavigatorState(ComponentAdded, newCandidate);
  RNS_LOG_INFO("Added " << newCandidate->getComponentData().componentName << "[" << newCandidate->getComponentData().tag << "]"
                << " To container : " << this );
}

void Container::mergeComponent(CandidateList candidates) {
  navComponentList_.insert(navComponentList_.end(), candidates.begin(), candidates.end());
  RNS_LOG_INFO("Merging " << candidates.size() << " candidates to container : " << this << ", New Size : " << navComponentList_.size());
  candidates.clear();
}

void Container::removeComponent(RSkComponent *candidate) {
  CandidateList::iterator it;

  it = std::find(navComponentList_.begin(), navComponentList_.end(), candidate);
  if (it != navComponentList_.end()) {
    RSkSpatialNavigator::sharedSpatialNavigator()->updateSpatialNavigatorState(ComponentRemoved, candidate);
    navComponentList_.erase(it);
  }
}

void Container::updateComponent(RSkComponent *candidate) {
    CandidateList::iterator it;
    it = std::find(navComponentList_.begin(), navComponentList_.end(), candidate);

    if (it != navComponentList_.end()) {
        // Candidate found in the navigatable list but candidate's focusable props have changed.
        if (!candidate->isFocusable()) {
            RSkSpatialNavigator::sharedSpatialNavigator()->updateSpatialNavigatorState(ComponentUpdated, candidate);
            navComponentList_.erase(it);
        }
    } else {
        // Candidate not found in the current list, Call add method to check if one of props (isTVNavigatable, focusable) might have changed
        if(candidate->isFocusable())
          addComponent(candidate);
    }
}

RSkComponent* Container::firstInContainer(bool visible, bool skipChildren) {

  if(navComponentList_.size() == 0)
    return nullptr;

  RSkComponent *firstCandidate = navComponentList_.front();
  if(visible && !isVisible(firstCandidate)) { // If visible candidate was requested, return nullptr if it is not in visible area
    RNS_LOG_DEBUG("Fully Visible candidate was requested but " << firstCandidate << " is not or partially visible");
    return nullptr ;
  } else if(!visible) {
    RNS_LOG_NOT_IMPL; // Should handle manual scroll
    return nullptr;
  }

  if(skipChildren == false) {
    if(firstCandidate && firstCandidate->isContainer()) {
      RSkComponent* first = firstCandidate->firstInContainer(visible, skipChildren);
      if(first != nullptr) // If No focusable canditaes in conatiner then we will return container itself as focusable component
        firstCandidate = first;
    }
  }
  return firstCandidate;
}

#if defined(TARGET_OS_TV) && TARGET_OS_TV
RSkComponent* Container::preferredFocusInContainer() {
  RSkComponent *preferredFocus = nullptr;
  std::vector<RSkComponent*>::reverse_iterator i;
  CandidateList &navCompList = navComponentList_;

  for (i = navCompList.rbegin(); i != navCompList.rend(); ++i ) {
    preferredFocus = *i;

    if(preferredFocus->isContainer())
      preferredFocus = preferredFocus->preferredFocusInContainer();

    if(preferredFocus == nullptr) // Didn't find any preferred item in child(container). Check in next sibling.
      continue;

    auto const &viewProps = *std::static_pointer_cast<ViewProps const>(preferredFocus->getComponentData().props);
    if(viewProps.hasTVPreferredFocus == true)
      break;
  }

  if(i == navCompList.rend()) { // No preferred item in this container.
    preferredFocus = nullptr;
  }
  return preferredFocus;
}
#endif //TARGET_OS_TV

} // namespace SpatialNavigator
