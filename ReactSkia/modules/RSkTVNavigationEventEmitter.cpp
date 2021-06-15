#include "RSkTVNavigationEventEmitter.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
using namespace std;
namespace facebook {
namespace react {

RSkTVNavigationEventEmitter::RSkTVNavigationEventEmitter(
    const std::string &name, 
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : RSkEventEmitter(name, jsInvoker, bridgeInstance){}

void RSkTVNavigationEventEmitter::onTVKeyEvent(rnsKey  eventType,rnsKeyAction eventKeyAction) {
    folly::dynamic parameters = folly::dynamic::object();
    parameters["eventType"] = RNSKeyMap[eventType];
    //parameters["tag"] = 0xFFFF; //TODO need to analyze and value for Tag.
    parameters["eventKeyAction"] = (int)eventKeyAction;
    sendEventWithName(events_[0], folly::dynamic(parameters));
}

void RSkTVNavigationEventEmitter::startObserving() {
    // use the NotificationManager to register for TV Keys
    std::function<void(rnsKey,rnsKeyAction)> handler = std::bind(&RSkTVNavigationEventEmitter::onTVKeyEvent, this, std::placeholders::_1,std::placeholders::_2);
    eventId_ = keyEventNotification.addListener(tvEventName_, handler);
}

void RSkTVNavigationEventEmitter::stopObserving() {
    keyEventNotification.removeListener(eventId_);
}

} //namespace react
} //namespace facebook
