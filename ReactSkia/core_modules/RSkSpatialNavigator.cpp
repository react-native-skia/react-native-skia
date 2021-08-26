/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include <string>
#include <algorithm>

#include "RSkSpatialNavigator.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

namespace SpatialNavigator {

RSkSpatialNavigator* RSkSpatialNavigator::sharedSpatialNavigator_{nullptr};
std::mutex RSkSpatialNavigator::mutex_;

RSkSpatialNavigator::RSkSpatialNavigator() {
    std::function<void(rnsKey, rnsKeyAction)> handler = std::bind(&RSkSpatialNavigator::handleKeyEvent, this,
                                                                std::placeholders::_1, // rnsKey
                                                                std::placeholders::_2);  // rnsKeyAction
    eventId_ = NotificationCenter::defaultCenter().addListener("onHWKeyEvent", handler);
}

RSkSpatialNavigator::~RSkSpatialNavigator() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(this == sharedSpatialNavigator_)
        sharedSpatialNavigator_ = nullptr;
}

RSkSpatialNavigator* RSkSpatialNavigator::sharedSpatialNavigator() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sharedSpatialNavigator_ == nullptr) {
        sharedSpatialNavigator_ = new RSkSpatialNavigator();
    }
    return sharedSpatialNavigator_;
}

void RSkSpatialNavigator::sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction) {
    if(eventType.c_str() == nullptr)
        return;
    RNS_LOG_DEBUG("Send : " << eventType  << " To ComponentTag : " << tag );
    NotificationCenter::defaultCenter().emit("RCTTVNavigationEventNotification",
                                                folly::dynamic(folly::dynamic::object("eventType", eventType.c_str())
                                                                              ("eventKeyAction", (int)keyAction)
                                                                              ("tag", tag)
                                                                              ("target", tag)
                                                                              ));
}

/*
 * Add a new candidate to the Navigatable if it meets the following criteria:
 * isTVSelectable, focuable are true
*/
void RSkSpatialNavigator::addToNavList(std::shared_ptr<RSkComponent> newCandidate) {
    Component newCandData = newCandidate.get()->getComponentData();
    auto const &viewProps = *std::static_pointer_cast<ViewProps const>(newCandData.props);
    RNS_LOG_DEBUG("Add " << newCandData.componentName << "[" << newCandData.tag <<"] to Navigatable List : isTVSelectable["
                                    << viewProps.isTVSelectable << "]" << " focusable[" << viewProps.focusable << "] " <<
                                    "hasTVPreferredFocus[" << viewProps.hasTVPreferredFocus << "]" << " accessible[" << viewProps.accessible << "]");
    if (viewProps.isTVSelectable == true || viewProps.focusable == true) {
        navComponentList_.push_back(newCandidate.get());
        if(viewProps.hasTVPreferredFocus == true) {
            if(currentFocus_)
                sendNotificationWithEventType("blur", currentFocus_->getComponentData().tag);

            sendNotificationWithEventType("focus", newCandData.tag);
            currentFocus_ = newCandidate.get();
        }
    }
}

void RSkSpatialNavigator::removeFromNavList(std::shared_ptr<RSkComponent> candidate) {
    Component canData = candidate.get()->getComponentData();
    auto const &viewProps = *std::static_pointer_cast<ViewProps const>(canData.props);
    if (viewProps.isTVSelectable == true || viewProps.focusable == true) {
        CandidateList::iterator it;
        it = std::find(navComponentList_.begin(), navComponentList_.end(), candidate.get());
        if (it != navComponentList_.end()) {
            // Reset data if candidate being removed is currently focused
            if (currentFocus_ == candidate.get()) {
                currentFocus_ = nullptr;
            }
            navComponentList_.erase(it);
        }
    }
}

void RSkSpatialNavigator::updateInNavList(std::shared_ptr<RSkComponent> candidate) {
    Component canData = candidate.get()->getComponentData();
    auto const &viewProps = *std::static_pointer_cast<ViewProps const>(canData.props);

    CandidateList::iterator it;
    it = std::find(navComponentList_.begin(), navComponentList_.end(), candidate.get());

    if (it != navComponentList_.end()) {
        // Candidate found in the navigatable list, Reset data if candidate's focusable props have changed
        if (viewProps.isTVSelectable == false && viewProps.focusable == false) {
            if (currentFocus_ == candidate.get()) {
                sendNotificationWithEventType("blur", currentFocus_->getComponentData().tag);
                currentFocus_ = nullptr;
            }
            navComponentList_.erase(it);
        }
    } else {
        // Candidate not found in the current list, Call add method to check if one of props (isTVNavigatable, focusable) might have changed
        addToNavList(candidate);
    }
}

// Comparator function to insert elements into set in custom sort order
// (Item1, Item2) returning true means add Item1 before Item2
// Eg.
// Item1 < Item2 -- Returning true means add Item1 before Item2. Results in ascending
// Item1 > Item2 -- Returning true means add Item1 before Item2. Results in descending
template <class T>
struct sortDirectionComparator {
    sortDirectionComparator(int dir) { this->direction_ = dir; }
    bool operator()(const T& listItem, const T& newItem) const {
        Component listData = listItem->getComponentData();
        Component newData = newItem->getComponentData();
        Rect listCandidate = listData.layoutMetrics.frame;
        Rect newCandidate  = newData.layoutMetrics.frame;

        // Rule 4. If both candidates are having same dimension (x,y,w,h) then select the one with higher zIndex, else higher tag if zIndex is same
        if(listCandidate == newCandidate) {
            //Component with highest zIndex must be added before"
            if(listData.zIndex > newData.zIndex) return true;
            if(newData.zIndex  > listData.zIndex ) return false;
            // Last Resort. If both have same zIndex then items with bigger Tag will have more priority (Last added items will always have bigger tags)
            return listData.tag > newData.tag;
        }
        // Based on dircection following 4 rules apply.
        // Rule 5. Select the Closest candidate in the requested direction.
        // Rule 6. If both candidates have same distance, then select Lower one for Left/Right direction and Left Most for Up/Down direction
        // Rule 7. If both candidates have same value in Rule 6, then select the one with lower width for Left/Right direction and lower height for Up/Down
        // Rule 8. If both candidates have same value in Rule 7, then select the one with lower height for Left/Right direction and lower width for Up/Down
        switch(direction_) {
            case RNS_KEY_Right:
            case RNS_KEY_Left: {
                // Rule 5.
                if(direction_ == RNS_KEY_Right) {
                    if(listCandidate.origin.x < newCandidate.origin.x) return true;
                    if(newCandidate.origin.x  < listCandidate.origin.x) return false;
                }else {
                    if(listCandidate.getMaxX() > newCandidate.getMaxX()) return true;
                    if(newCandidate.getMaxX()  > listCandidate.getMaxX()) return false;
                }
                // Rule 6.
                if(listCandidate.origin.y < newCandidate.origin.y) return true;
                if(newCandidate.origin.y  < listCandidate.origin.y) return false;
                // Rule 7.
                if(listCandidate.size.width < newCandidate.size.width) return true;
                if(newCandidate.size.width  < listCandidate.size.width) return false;
                // Rule 8
                if(listCandidate.size.height < newCandidate.size.height) return true;
                if(newCandidate.size.height  < listCandidate.size.height) return false;

                break;
            }
            case RNS_KEY_Up:
            case RNS_KEY_Down: {
                // Rule 5.
                if(direction_ == RNS_KEY_Up) {
                    if(listCandidate.getMaxY() > newCandidate.getMaxY()) return true;
                    if(newCandidate.getMaxY()  > listCandidate.getMaxY()) return false;
                } else {
                    if(listCandidate.origin.y < newCandidate.origin.y) return true;
                    if(newCandidate.origin.y  < listCandidate.origin.y) return false;
                }
                // Rule 6.
                if(listCandidate.origin.x < newCandidate.origin.x) return true;
                if(newCandidate.origin.x < listCandidate.origin.x) return false;
                // Rule 7.
                if(listCandidate.size.height < newCandidate.size.height) return true;
                if(newCandidate.size.height  < listCandidate.size.height) return false;
                // Rule 8
                if(listCandidate.size.width < newCandidate.size.width) return true;
                if(newCandidate.size.width  < listCandidate.size.width) return false;

                break;
            }
            default:
                break;
        }
        return false;
    }
    int direction_ = 0;
};

void RSkSpatialNavigator::moveTheFocusInDirection(rnsKey keyEvent,
                                            SortedCandidateList<RSkComponent>& overLapping,
                                            SortedCandidateList<RSkComponent>& nonOverLapping) {

#if defined(THIS_IS_NOT_DEFINED) && (!defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO))
    for (auto candidate = overLapping.begin(); candidate != overLapping.end(); candidate++) {
        RNS_LOG_INFO("OverLapping Tag[" << (*candidate)->getComponentData().tag << "] I[" <<
                        (*candidate)->getComponentData().layoutMetrics.frame.origin.x << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.origin.y << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.getMaxX() << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.getMaxY() << "]");
    }
    for (auto candidate = nonOverLapping.begin(); candidate != nonOverLapping.end(); candidate++) {
        RNS_LOG_INFO("NonOverLapping Tag[" << (*candidate)->getComponentData().tag << "] I[" <<
                        (*candidate)->getComponentData().layoutMetrics.frame.origin.x << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.origin.y << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.getMaxX() << " " <<
                        (*candidate)->getComponentData().layoutMetrics.frame.getMaxY() << "]");
    }
#endif

    RSkComponent* nextFocus = nullptr;
    // Rule 8. By default choose the first one from sorted overLapping set.
    if(overLapping.size() != 0) {
        auto front = overLapping.begin();
        nextFocus = *front;
    }

    // Rule 9. If moving in Up or Down navigation, then check if nonOevrLapping is empty. If not,
    // then choose the closest one between first candidate from both the list.
    if((keyEvent == RNS_KEY_Up || keyEvent == RNS_KEY_Down) && nonOverLapping.size() != 0) {
        auto front = nonOverLapping.begin();
        if(nextFocus) {
            if(keyEvent == RNS_KEY_Up) {
                if(nextFocus->getComponentData().layoutMetrics.frame.getMaxY() < (*front)->getComponentData().layoutMetrics.frame.getMaxY())
                    nextFocus = *front;
            } else if(keyEvent == RNS_KEY_Down) {
                if(nextFocus->getComponentData().layoutMetrics.frame.origin.y > (*front)->getComponentData().layoutMetrics.frame.origin.y)
                    nextFocus = *front;
            }
        } else { // OverLapping set was empty, nothing to compare.
            nextFocus = *front;
        }
    }
    // By now we have either have a  valid nextFocus candidate or there is no valid candidate to focus in the direction
    if(nextFocus && nextFocus->getComponentData().tag != -1 ) {
        sendNotificationWithEventType("blur", currentFocus_->getComponentData().tag);
        sendNotificationWithEventType("focus", nextFocus->getComponentData().tag);
        RNS_LOG_DEBUG("Blur : [" << currentFocus_->getComponentData().tag << "], Focus :[" << nextFocus->getComponentData().tag << "]");
        currentFocus_ = nextFocus;
    }
}

static inline bool isValidCandidate(rnsKey direction, Component& curData, Component& canData) {
    Rect current = curData.layoutMetrics.frame;
    Rect candidate = canData.layoutMetrics.frame;

    // Rule 1. If the candidate has same dimention as the current focussed item then ignore.
    if (candidate == current) {
        RNS_LOG_DEBUG("Skip the candiate which has same dimension(x,y,w,h)");
        return false;
    }

#if 0 // Android is not handling this scenario , so we can disable this code for now to avoid this extra check overhead.
    // Rule 1.a If the candidte is completely inside the current focussed element and if it is below (not visible) then ignore it.
    if(candidate.origin.x >= current.origin.x &&
        candidate.getMaxX() <= current.getMaxX() &&
        candidate.origin.y >= current.origin.y &&
        candidate.getMaxY() <= current.getMaxY() &&
        canData.zIndex <= curData.zIndex &&
        canData.tag < curData.tag) {
            return false;
    }
#endif

    // Rule 2. Candidate must be in the direction of navigation.
    switch(direction) {
        case RNS_KEY_Right:
            return (candidate.origin.x >= current.origin.x); // Must be on right side
        case RNS_KEY_Left:
            return(candidate.getMaxX() <= current.getMaxX()); // Must be on left side
        case RNS_KEY_Up:
            return (candidate.getMaxY() <= current.getMaxY()); // Must be on up side
        case RNS_KEY_Down:
            return(candidate.origin.y >= current.origin.y); // Must be on down side
        default:
            RNS_LOG_WARN("Inavlid diretion Navigation : " << RNSKeyMap[direction]);
            break;
    }
    return false;
}

void RSkSpatialNavigator::setDefaultFocus() {
    RSkComponent *nextFocus = nullptr;
    std::vector<RSkComponent*>::reverse_iterator i;
    RNS_LOG_WARN("No Item is focused currently, select the Last TV preferred element, if not then the first element");
    if(navComponentList_.size() == 0) {
        RNS_LOG_WARN("There is no candidates for navigation....");
        return;
    }

    for (i = navComponentList_.rbegin(); i != navComponentList_.rend(); ++i ) {
        nextFocus = *i;
        auto const &viewProps = *std::static_pointer_cast<ViewProps const>(nextFocus->getComponentData().props);
        if(viewProps.hasTVPreferredFocus == true)
            break;
    }

    // Didn't find any element with hasTVPreferredFocus so choose the first element as default focus
    if(i == navComponentList_.rend()) {
        nextFocus = navComponentList_.front();
    }

    if(nextFocus != nullptr) {
        sendNotificationWithEventType("focus", nextFocus->getComponentData().tag);
        RNS_LOG_DEBUG("Set default Focus :[" << nextFocus->getComponentData().tag << "]");
        currentFocus_ = nextFocus;
    }
    return;
}

void RSkSpatialNavigator::navigateInDirection(rnsKey keyEvent) {

    // There is no currently focused element, select last TV Preffered component else first component
    if( currentFocus_ == nullptr ) {
        setDefaultFocus();
        return;
    }

    Component curData = currentFocus_->getComponentData();
    Rect currentRect = curData.layoutMetrics.frame;
    RNS_LOG_DEBUG("Current Focus Tag[" << curData.tag << "] I[" <<
                    currentRect.origin.x << " " << currentRect.origin.y << " " << currentRect.getMaxX() << " " << currentRect.getMaxY() << "]");

    // Sorted set which will use sortDirectionComparator to sort while inserting the elements
    SortedCandidateList<RSkComponent> overLapping(keyEvent);
    SortedCandidateList<RSkComponent> nonOverLapping(keyEvent);

    for (auto candidate = navComponentList_.begin(); candidate != navComponentList_.end(); candidate++) {
        Component canData = (*candidate)->getComponentData();
        Rect candidateRect = canData.layoutMetrics.frame;

        RNS_LOG_DEBUG("Possible Candidate Tag[" << canData.tag << "] I[" <<
                    candidateRect.origin.x << " " << candidateRect.origin.y << " " << candidateRect.getMaxX() << " " << candidateRect.getMaxY() << "]");

        if (*candidate == currentFocus_) {
            RNS_LOG_DEBUG("SKip the current focused item");
            continue;
        }
        if (!isValidCandidate(keyEvent, curData, canData))
            continue;

        switch(keyEvent) {
            case RNS_KEY_Right:
            case RNS_KEY_Left: {
                // Rule 3. Must have Projected overlap in Eastern/Western region
                if(!( canData.layoutMetrics.frame.getMaxY() <= curData.layoutMetrics.frame.origin.y
                    || canData.layoutMetrics.frame.origin.y > curData.layoutMetrics.frame.getMaxY())) {
                        RNS_LOG_DEBUG("Add Tag[ " << canData.tag << " ] to overlaping list for " << RNSKeyMap[keyEvent] << " direction");
                        overLapping.insert(*candidate); // Sorted using sortDirectionComparator
                } // For Right and left navigation we consider only candiates which has projected overalp
                break;
            }
            case RNS_KEY_Up:
            case RNS_KEY_Down: {
                // Rule 3. Has either Projected overlap or nonOverlap in Northern/Southern region
                if(!( canData.layoutMetrics.frame.getMaxX() <= curData.layoutMetrics.frame.origin.x
                    || canData.layoutMetrics.frame.origin.x > curData.layoutMetrics.frame.getMaxX())) {
                        RNS_LOG_DEBUG("Add Tag[ " << canData.tag << " ] to overlaping list for " << RNSKeyMap[keyEvent] << " direction");
                        overLapping.insert(*candidate); // Sorted using sortDirectionComparator
                } else {
                    // Rule 3.a For non-overlap, for up direction, only consider the candidates which is completely above current focussed item and
                    // for down direction, only consider the candidates which is completely below current focussed item
                    if(keyEvent == RNS_KEY_Up && canData.layoutMetrics.frame.getMaxY() < curData.layoutMetrics.frame.origin.y) {
                        RNS_LOG_DEBUG("Add Tag[ " << canData.tag << " ] to nonOverlaping list for " << RNSKeyMap[keyEvent] << " direction");
                        nonOverLapping.insert(*candidate); // Sorted using sortDirectionComparator
                    } else if(keyEvent == RNS_KEY_Down && canData.layoutMetrics.frame.origin.y > curData.layoutMetrics.frame.getMaxY()) {
                        RNS_LOG_DEBUG("Add Tag[ " << canData.tag << " ] to nonOverlaping list for " << RNSKeyMap[keyEvent] << " direction");
                        nonOverLapping.insert(*candidate); // Sorted using sortDirectionComparator
                    }
                }
                break;
            }
            default:
                break;
        } // switch
    } // for loop

    // By now we have two sorted set where the top most element in the set is the right candidate to navigate in respective set.
    // Choose right candidate between top most element from overLapping and nonOverLapping set
    RNS_PROFILE_API_OFF("MoveFocus : ", moveTheFocusInDirection(keyEvent, overLapping, nonOverLapping));
}

void RSkSpatialNavigator::handleKeyEvent(rnsKey  eventKeyType, rnsKeyAction eventKeyAction) {

    if(eventKeyAction != RNS_KEY_Press) // Need to act on keyPress only
        return;

    // First send keyevent to TVNavigation Emitter
    sendNotificationWithEventType(RNSKeyMap[eventKeyType], currentFocus_ ? currentFocus_->getComponentData().tag : -1, eventKeyAction);

    // Then based on spatial navigation alogirthm, send blur/focus
    switch(eventKeyType) {
        case RNS_KEY_Up:
        case RNS_KEY_Down:
        case RNS_KEY_Left:
        case RNS_KEY_Right:{
            RNS_PROFILE_API_OFF("NavigateInDirection : ", navigateInDirection(eventKeyType));
            break;
        }
        default:
            break; // Ignore
    }
}

} // namespace SpatialNavigator
