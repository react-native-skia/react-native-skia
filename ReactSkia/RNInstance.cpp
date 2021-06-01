#include "ReactSkia/RNInstance.h"

#include "ReactSkia/ComponentViewRegistry.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "ReactSkia/LegacyNativeModuleRegistry.h"
#include "ReactSkia/MountingManager.h"
#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/components/RSkComponentProviderImage.h"
#include "ReactSkia/components/RSkComponentProviderRootView.h"
#include "ReactSkia/components/RSkComponentProviderText.h"
#include "ReactSkia/components/RSkComponentProviderView.h"

#if defined (OS_MACOSX)
#include "ReactSkia/platform/macosx/MainRunLoopEventBeat.h"
#include "ReactSkia/platform/macosx/RuntimeEventBeat.h"
#elif defined (OS_LINUX)
#include "ReactSkia/platform/linux/MainRunLoopEventBeat.h"
#include "ReactSkia/platform/linux/RuntimeEventBeat.h"
#endif
#include "ReactSkia/utils/RnsLog.h"

#include "ReactCommon/TurboModuleBinding.h"
#include "cxxreact/JSBigString.h"
#include "cxxreact/MessageQueueThread.h"
#include "cxxreact/ModuleRegistry.h"
#include "jsi/JSCRuntime.h"
#include "jsireact/JSIExecutor.h"
#include "react/config/ReactNativeConfig.h"
#include "react/renderer/componentregistry/ComponentDescriptorProviderRegistry.h"
#include "react/renderer/components/root/RootShadowNode.h"
#include "react/renderer/scheduler/Scheduler.h"
#include "react/renderer/scheduler/SchedulerToolbox.h"
#include "react/utils/ContextContainer.h"

#include <folly/io/async/ScopedEventBaseThread.h>

namespace facebook {
namespace react {

class JSCExecutorFactory : public JSExecutorFactory {
 public:
  JSCExecutorFactory(JSITurboModuleManager *jsiTurboModuleManager)
      : jsiTurboModuleManager_(jsiTurboModuleManager) {}

  std::unique_ptr<JSExecutor> createJSExecutor(
      std::shared_ptr<ExecutorDelegate> delegate,
      std::shared_ptr<MessageQueueThread> jsQueue) override {
    auto installBindings = [jsiTurboModuleManager =
                                jsiTurboModuleManager_](jsi::Runtime &runtime) {
      // react::Logger iosLoggingBinder = [](const std::string &message,
      // unsigned int logLevel) {
      //   _RCTLogJavaScriptInternal(static_cast<RCTLogLevel>(logLevel),
      //   [NSString stringWithUTF8String:message.c_str()]);
      // };
      // react::bindNativeLogger(runtime, iosLoggingBinder);
      TurboModuleBinding::install(
          runtime, std::move(jsiTurboModuleManager->GetProvider()));
    };
    return std::make_unique<JSIExecutor>(
        facebook::jsc::makeJSCRuntime(),
        delegate,
        JSIExecutor::defaultTimeoutInvoker,
        std::move(installBindings));
  }

 private:
  JSITurboModuleManager *jsiTurboModuleManager_;
};

class MessageQueueThreadImpl : public MessageQueueThread {
 public:
  MessageQueueThreadImpl() : thread_("JSThread") {}

  void runOnQueue(std::function<void()> &&task) override {
    thread_.getEventBase()->runInEventBaseThread(std::move(task));
  }
  void runOnQueueSync(std::function<void()> &&task) override {
    thread_.getEventBase()->runInEventBaseThreadAndWait(std::move(task));
  }
  void quitSynchronous() override {}

 private:
  folly::ScopedEventBaseThread thread_;
};

RNInstance::RNInstance() {
  InitializeJSCore();
  RegisterComponents();
  InitializeFabric();
}

RNInstance::~RNInstance() {}

void RNInstance::Start(RSkSurfaceWindow *surface) {
  mountingManager_->BindSurface(surface);
  SurfaceId surfaceId = 1;
  fabricScheduler_->startSurface(
      surfaceId,
      "SimpleViewApp",
      folly::dynamic::object(),
      surface->GetLayoutConstraints(),
      {}, // layoutContext,
      {} // mountingOverrideDelegate
  );
  fabricScheduler_->renderTemplateToSurface(surfaceId, {});

  // NOTE(kudo): Does adding RootView here make sense !?
  auto *provider = componentViewRegistry_->GetProvider(RootComponentName);
  surface->compositor()->setRootLayer(provider->CreateComponent({}));
}

void RNInstance::Stop() {
  SurfaceId surfaceId = 1;
  fabricScheduler_->stopSurface(surfaceId);
}

void RNInstance::InitializeJSCore() {
  instance_ = std::make_unique<Instance>();
  turboModuleManager_ =
      std::make_unique<JSITurboModuleManager>(instance_.get());
  auto cb = std::make_unique<InstanceCallback>();
  auto factory =
      std::make_shared<JSCExecutorFactory>(turboModuleManager_.get());
  instance_->initializeBridge(
      std::make_unique<InstanceCallback>(),
      std::make_shared<JSCExecutorFactory>(turboModuleManager_.get()),
      std::make_shared<MessageQueueThreadImpl>(),
      std::make_shared<LegacyNativeModuleRegistry>());

  // NOTE(kudo): Workaround for TurboModules being fully initialized
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  try {
    auto source = JSBigFileString::fromPath("SimpleViewApp.bundle");
    instance_->loadScriptFromString(
        std::move(source), "SimpleViewApp.bundle", true);
  } catch (const jsi::JSError &ex) {
    std::string exc = ex.what();
    RNS_LOG_ERROR("JS ERROR : " << exc);
  } catch (const std::system_error& ex) {
    std::string exc = ex.what();
    RNS_LOG_ERROR("SYSTEM ERROR : " << exc);
  }
}

void RNInstance::InitializeFabric() {
  facebook::react::ContextContainer::Shared contextContainer =
      std::make_shared<facebook::react::ContextContainer const>();
  std::shared_ptr<const facebook::react::ReactNativeConfig> reactNativeConfig =
      std::make_shared<facebook::react::EmptyReactNativeConfig const>();

  contextContainer->insert("ReactNativeConfig", reactNativeConfig);

  auto runtimeExecutor = instance_->getRuntimeExecutor();

  auto toolbox = SchedulerToolbox{};
  toolbox.contextContainer = contextContainer;
  toolbox.componentRegistryFactory =
      [this](
          EventDispatcher::Weak const &eventDispatcher,
          ContextContainer::Shared const &contextContainer)
      -> SharedComponentDescriptorRegistry {
    return componentViewRegistry_->CreateComponentDescriptorRegistry(
        ComponentDescriptorParameters{eventDispatcher, nullptr, nullptr});
  };
  toolbox.runtimeExecutor = runtimeExecutor;
  // toolbox.mainRunLoopObserverFactory = [](RunLoopObserver::Activity
  // activities,
  //                                         RunLoopObserver::WeakOwner const
  //                                         &owner) {
  //   return std::make_unique<MainRunLoopObserver>(activities, owner);
  // };
  toolbox.synchronousEventBeatFactory =
      [runtimeExecutor](EventBeat::SharedOwnerBox const &ownerBox) {
        return std::make_unique<MainRunLoopEventBeat>(
            ownerBox, runtimeExecutor);
      };

  toolbox.asynchronousEventBeatFactory =
      [runtimeExecutor](EventBeat::SharedOwnerBox const &ownerBox) {
        return std::make_unique<RuntimeEventBeat>(ownerBox, runtimeExecutor);
      };

  mountingManager_ =
      std::make_unique<MountingManager>(componentViewRegistry_.get());
  fabricScheduler_ =
      std::make_shared<Scheduler>(toolbox, nullptr, mountingManager_.get());
}

void RNInstance::RegisterComponents() {
  componentViewRegistry_ = std::make_unique<ComponentViewRegistry>();
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderRootView>());
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderView>());
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderImage>());
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderText>());
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderRawText>());
  componentViewRegistry_->Register(
      std::make_unique<RSkComponentProviderParagraph>());
}

} // namespace react
} // namespace facebook
