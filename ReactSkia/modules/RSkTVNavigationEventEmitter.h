#include "sdk/NotificationCenter.h"
#include "sdk/RNSKeyCodeMapping.h"
#include "core_modules/RSkEventEmitter.h"
#include "tools/skui/Key.h"

namespace facebook {
namespace react {

class RSkTVNavigationEventEmitter final: public RSkEventEmitter {
    private:
        std::vector<std::string> events_ = {"onHWKeyEvent"};
        std::string tvEventName_ = "RCTTVNavigationEventNotification";
        unsigned int eventId_ = UINT_MAX; // Some undefined ID

        NotificationCenter keyEventNotification;

        void onTVKeyEvent(rnsKey  eventType,rnsKeyAction eventKeyAction);

    public:
        RSkTVNavigationEventEmitter(
            const std::string &name, 
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);

        ~RSkTVNavigationEventEmitter() {}

        std::vector<std::string> supportedEvents() {
            return events_;
        }

        void startObserving();

        void stopObserving();
};

} //namespace react
} //namespace facebook
