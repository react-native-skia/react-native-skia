/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <folly/small_vector.h>
#include <folly/sorted_vector_types.h>

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

using folly::sorted_vector_map;
using folly::sorted_vector_set;

namespace facebook{
namespace react {

class RSkComponent;

namespace SpatialNavigator {

typedef std::vector<RSkComponent *> CandidateList;

template <class T>
struct sortDirectionComparator;
template <class T>
using SortedCandidateList = sorted_vector_set<T*, sortDirectionComparator<T*>>;

enum NavigatorStateOperation {
    ComponentAdded = 1,
    ComponentRemoved,
    ComponentUpdated,
};

class Container;

/*
 * This Class manages Spatial Navigation
 * Manages a list of current set of navigatable components on Screen: navList_
 * Keeps track of the currently selected component: reference_
 * Keeps track of mutations on the Components to manage change in the navigatable set: add, remove, update
 * Listens to H/W Keyboard / RCU Keys (LURD Keys) to select a candidate to trigger focus / blur events
*/

class RSkSpatialNavigator {
private:
    static RSkSpatialNavigator *sharedSpatialNavigator_;
    static std::mutex mutex_;

    // Currently focused component
    RSkComponent *currentFocus_{nullptr};

    // List of Navigatable Components
    CandidateList navComponentList_;

    // Private constructor: Singleton class
    RSkSpatialNavigator();

    Container *rootContainer_{nullptr};
    Container *currentContainer_{nullptr};

    void navigateInDirection(rnsKey keyEvent);
    bool advanceFocusInDirection(Container *container, rnsKey keyEvent);
    RSkComponent* findFocusCandidateInContainer(Container *container, rnsKey keyEvent, bool visibleOnly);
    RSkComponent* pickCandidateInDirection(rnsKey keyEvent, SortedCandidateList<RSkComponent>& overLapping, SortedCandidateList<RSkComponent>& nonOverLapping);
    RSkComponent* findDefaultFocusInContainer(Container *container);
    void sendNotificationWithEventType(std::string eventType, int tag, NotificationCompleteVoidCallback completeCB = nullptr);
#if ENABLE(FEATURE_KEY_THROTTLING)
    void tvEventCompleteCallback();
#endif

    NotificationCompleteVoidCallback completeCallback_ {nullptr};
public:
    static RSkSpatialNavigator* sharedSpatialNavigator();
    ~RSkSpatialNavigator();
    void updateFocusCandidate(RSkComponent *focusCandidate);
    void updateSpatialNavigatorState(NavigatorStateOperation operation, RSkComponent *candidate);

    void handleKeyEvent(rnsKey  eventType, rnsKeyAction eventKeyAction);
    RSkComponent* getCurrentFocusElement();

    void setRootContainer(Container *container) { rootContainer_ = container; }
};

}// namespace SpatialNavigator
}//react
}//facebook
