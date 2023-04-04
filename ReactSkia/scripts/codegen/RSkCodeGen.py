#!/usr/bin/python

# Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
import sys
import io
import os

from RSkGenerateThirdPartyNativeModulesCpp import *
from RSkGenerateThirdPartyNativeModulesH import *
from RSkGenerateThirdPartyFabricComponentsProviderCpp import *
from RSkGenerateThirdPartyFabricComponentsProviderH import *
from RSkGenerateThirdPartyViewConfigManagerCpp import *

class PackageInfo():
  "Stores third-pary package details"
  def __init__(self, name, type, libraryVersion, classNames, moduleNames, libraryName):
    self.name = name
    self.type = type
    self.version = libraryVersion
    self.classNames = classNames
    self.moduleNames = moduleNames
    self.libraryName = libraryName

packageMap = {}
packageMapCustom = {}

ModuleLookupMapTemplateCpp = ''
ModuleLookupFuncTemplateH = ''
ComponentLookupMapTemplateCpp = ''
ComponentLookupFuncTemplateH = ''
ViewConfigTemplateCpp = ''

# Function to parse Rsk.project file and create a package info object
def parseRskProjectFile(packageName, rskProjPath, packageMapTable):
  global ModuleLookupMapTemplateCpp
  global ModuleLookupFuncTemplateH
  global ComponentLookupMapTemplateCpp
  global ComponentLookupFuncTemplateH

  if not os.path.exists(rskProjPath):
		print ("Warning :" + rskProjPath + ' notfound')
		return

  rskProjFileHandle = open(rskProjPath)
  codegenData = json.load(rskProjFileHandle)

  if 'codegenConfig' not in codegenData:
		rskProjFileHandle.close()
		return

  codegenConfig = codegenData['codegenConfig']
  packageversion = codegenConfig['version'] if 'version' in codegenConfig else 'UnknownPkgVersion'
  libName = codegenConfig['name'] if 'name' in codegenConfig else 'UnknownPkgName'
  libType = codegenConfig['type'] if 'type' in codegenConfig else 'UnknownPkgType'
  classNames = []
  moduleNames = []
  if 'skia' in codegenConfig:
    if packageName not in packageMapTable:
      classNames =  codegenConfig['skia']['codegenNativeComponentNativeProps'] if 'codegenNativeComponentNativeProps' in codegenConfig['skia'] else []
      moduleNames = codegenConfig['skia']['TurboModuleRegistryGetEnforcingSpec'] if 'TurboModuleRegistryGetEnforcingSpec' in codegenConfig['skia'] else []

      newPkgObj = PackageInfo(packageName, libType, packageversion, classNames, moduleNames , libName)
      packageMapTable[packageName] = newPkgObj;
    else:
      rskProjFileHandle.close()
      return

    for module in moduleNames:
      ModuleLookupMapTemplateCpp += '    {std::string(\"' + module + '\"' +'), ' + module + 'ModuleCls' + '}, // ' + libName + '\n'
      ModuleLookupFuncTemplateH += 'xplat::module::CxxModule* ' + module + 'ModuleCls(void) ' + 'RNS_USED;  // ' + libName + '\n'

    for idx, component in enumerate(classNames):
      ComponentLookupMapTemplateCpp += '    {std::string(\"' + component + '\"' +'), ' + component + 'Cls' + '}, // ' + libName + '\n'
      ComponentLookupFuncTemplateH += 'RSkComponentProvider* ' + component + 'Cls(void) ' + 'RNS_USED;  // ' + libName + '\n'
      if 'viewManagerConfig' in codegenData:
        viewManagerConfigs = codegenData['viewManagerConfig']
        if component in viewManagerConfigs:
          parseViewConfigs(idx, component, viewManagerConfigs[component])
# End of function parseRskProjectFile()

# Function to parse view configs
def parseViewConfigs(index, viewManagerName, viewManagerConfig):
  global ViewConfigTemplateCpp
  if index != 0:
    ViewConfigTemplateCpp += ' else '

  ViewConfigTemplateCpp += 'if(viewManagerName == "'+ viewManagerName +'") {'

  nativePropsTemplate = ''
  directEventTemplate = ''
  bubblingEventTemplate = ''

  if 'nativePropsConfig' in viewManagerConfig:
      nativePropsTemplate = '\n    '
      for nativeProp in viewManagerConfig['nativePropsConfig']:
        nativePropsTemplate += 'nativeProps["'+nativeProp+'"] = true;\n    '

  if 'directEventConfig' in viewManagerConfig:
      directEventTemplate = '\n    '
      for directEvent in viewManagerConfig['directEventConfig']:
        directEventTemplate += 'directEventTypes["top'+directEvent+'"] = folly::dynamic::object("registrationName", "on' +directEvent +'");\n    '

  if 'bubblingEventConfig' in viewManagerConfig:
      bubblingEventTemplate = '\n    '
      for bubbleEvent in viewManagerConfig['bubblingEventConfig']:
        bubblingEventTemplate += 'bubblingEventTypes["top'+bubbleEvent+'"] = folly::dynamic::object("phasedRegistrationNames", folly::dynamic::object("bubbled","on'+bubbleEvent+'")("captured", "on'+bubbleEvent+'"));\n    '

  ViewConfigTemplateCpp +=  nativePropsTemplate + directEventTemplate + bubblingEventTemplate + '\n  }'
# End of function parseViewConfigs()

# Function to genrate all package info details
def preparePkgInfo(modulesPath, applicationPath):

  jsonPath = applicationPath + '/' + 'package.json'

  if not os.path.exists(jsonPath):
    return

  pkgJsonFileHandle = open(jsonPath)
  jsonData = json.load(pkgJsonFileHandle)

  if not 'dependencies' in jsonData:
    return

  for pkg in jsonData['dependencies']:
    if 'codegenConfig' in jsonData:
      print("xxxxxxxxxx New architecture package : Codegen Parsing not implemented yet xxxxxxxxxx")
      return
    else:
      rskProjPath = modulesPath + '/' + pkg + '/skia/Rsk.project'
      parseRskProjectFile(pkg, rskProjPath, packageMap)

      subPath = modulesPath + '/' + pkg
      preparePkgInfo(modulesPath, str(subPath)) # Recursively find its package dependencies
    #end of ('codegenConfig' in jsonData) else
  #end of for pkg in jsonData

  # Special cases where apps have their own custom RN packages : app/skia/[package1, package2..]
  if os.path.exists(applicationPath + '/skia'):
    for dirName in os.listdir(applicationPath + '/skia'):
      if os.path.isdir(applicationPath + '/skia/' + dirName):
        parseRskProjectFile(dirName, applicationPath + '/skia/' + dirName +'/Rsk.project', packageMapCustom)

  '''print('---------- Package List ----------')
  for item in packageMap:
    package = packageMap[item]
    print(package.name + '[' + package.type + '][' + package.version + '], library : ' + package.libraryName + ', Components :' +  str(package.classNames) + ', Modules :' + str(package.moduleNames))
  print('------- Custom Package List ------')
  for item in packageMapCustom:
    package = packageMapCustom[item]
    print(package.name + '[' + package.type + '][' + package.version + '], library : ' + package.libraryName + ', Components :' +  str(package.classNames) + ', Modules :' + str(package.moduleNames))
  print('----------------------------------')'''

  pkgJsonFileHandle.close()
# End of function preparePkgInfo()

# Main function
def main():

  appPath = ''
  nodeModulesPath = ''
  codegenConfigFileHandle = open(sys.argv[2])
  codgenConfig = json.load(codegenConfigFileHandle)

  if 'application' in codgenConfig:
    if 'path' in codgenConfig['application']:
      appPath = codgenConfig['application']['path']
      if os.path.isabs(appPath) == False:
        appPath = sys.argv[3] + '/' + appPath

  if os.path.exists(appPath):
    if 'nodeModules' in codgenConfig:
      if 'path' in codgenConfig['nodeModules']:
        nodeModulesPath = codgenConfig['nodeModules']['path']
        if os.path.isabs(nodeModulesPath) == False:
          nodeModulesPath = sys.argv[3] + '/' + nodeModulesPath

    #install application dependent node module to given nodes module path
    yarnInstallCmd = 'yarn install --cwd '+ appPath + ' --modules-folder ' + nodeModulesPath
    os.system(yarnInstallCmd)

    preparePkgInfo(nodeModulesPath, appPath)
  else:
    print('xxxxxxxxxx Application '+ appPath +' Doesnt Exist xxxxxxxxxx')

  generateThirdPartyFabricComponentsProviderCpp(sys.argv[1], ComponentLookupMapTemplateCpp)
  generateThirdPartyFabricComponentsProviderH(sys.argv[1], ComponentLookupFuncTemplateH)
  generateThirdPartyNativeModulesProviderCpp(sys.argv[1], ModuleLookupMapTemplateCpp)
  generateThirdPartyNativeModulesProviderH(sys.argv[1], ModuleLookupFuncTemplateH)
  generateThirdPartyViewConfigManagerCpp(sys.argv[1], ViewConfigTemplateCpp)

  codegenConfigFileHandle.close()

if __name__== "__main__":
  main()
