// This needs to be generated using codegen
#include <string>
#include <unordered_map>

#include "ReactSkia/utils/RnsLog.h"

#include "RSkThirdPartyNativeModuleProvider.h"

namespace facebook {
namespace react {

RSkLegacyNativeModuleProviderProtocol RSkThirdparyNativeModuleCreatorClassWithName(std::string moduleName) {
  // This function and hash table must be generated by codegen with all the linked thirdparty Native modules
  static std::unordered_map<std::string, RSkLegacyNativeModuleProviderProtocol> NativeModulesClassMap = {
  };
  RNS_LOG_TRACE("Look up for module : " << moduleName << " in thirdparty provider");

  auto p = NativeModulesClassMap.find(moduleName);
  if (p != NativeModulesClassMap.end()) {
    RNS_LOG_INFO("Found module : " << moduleName << " in thirdparty provider");
    auto classFunc = p->second;
    return classFunc;
  }

  RNS_LOG_WARN("Could not find module : " << moduleName << " in thirdparty provider");
  return nullptr;
}

} // namespace react
} // namespace facebook
