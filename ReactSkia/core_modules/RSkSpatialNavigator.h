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

namespace facebook {
    namespace react {
        class RSkComponent;
    }
}

using namespace facebook::react;

namespace SpatialNavigator {

typedef std::vector<RSkComponent *> CandidateList;

template <class T>
struct sortDirectionComparator;
template <class T>
using SortedCandidateList = sorted_vector_set<T*, sortDirectionComparator<T*>>;

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
    RSkComponent *currentFocus_ = nullptr;

    // List of Navigatable Components
    CandidateList navComponentList_;

    // Private constructor: Singleton class
    RSkSpatialNavigator();

    unsigned int eventId_ = UINT_MAX; // Set to undefined ID
    void navigateInDirection(rnsKey keyEvent);
    void sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction = RNS_KEY_UnknownAction);
    void moveTheFocusInDirection(rnsKey keyEvent, SortedCandidateList<RSkComponent>& overLapping, SortedCandidateList<RSkComponent>& nonOverLapping);

    static bool compareX(RSkComponent *candidate1, RSkComponent *candidate2);
    static bool compareY(RSkComponent *candidate1, RSkComponent *candidate2);
    static bool compareArea(RSkComponent *candidate1, RSkComponent *candidate2);

public:
    static RSkSpatialNavigator* sharedSpatialNavigator();
    ~RSkSpatialNavigator();

    void addToNavList(std::shared_ptr<RSkComponent> candidate);
    void removeFromNavList(std::shared_ptr<RSkComponent> candidate);
    void updateInNavList(std::shared_ptr<RSkComponent> candidate);

    void handleKeyEvent(rnsKey  eventType, rnsKeyAction eventKeyAction);
};

} // namespace SpatialNavigator
