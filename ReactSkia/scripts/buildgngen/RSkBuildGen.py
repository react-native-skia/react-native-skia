#!/usr/bin/python

# Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
import sys
import io
import os

from RSkGenerateExternalBuildGn import *

class PackageInfo():
  "Stores third-pary package details"
  def __init__(self, name, type, libraryVersion, libraryName):
    self.name = name
    self.type = type
    self.version = libraryVersion
    self.libraryName = libraryName

packageMap = {}
packageMapCustom = {}
buildTargets = ''

# Function to parse Rsk.project file and create a package info object
def parseRskProjectFile(packageName, rskProjPath, packageMapTable):
  if not os.path.exists(rskProjPath):
    #print ("Warning :" + rskProjPath + ' notfound')
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
  if 'skia' in codegenConfig:
    if packageName not in packageMap:
      newPkgObj = PackageInfo(packageName, libType, packageversion, libName)
      packageMapTable[packageName] = newPkgObj;
    else:
      rskProjFileHandle.close()
      return
  rskProjFileHandle.close()
# End of function parseRskProjectFile()

# Function to genrate all package info details
def preparePkgInfo(modulesPath, applicationPath):
  global buildTargets

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
     # preparePkgInfo(modulesPath, str(subPath)) # Recursively find its package dependencies
    #end of ('codegenConfig' in jsonData) else
  #end of for pkg in jsonData

  # Special cases where apps have their own custom RN packages : app/skia/[package1, package2..]
  if os.path.exists(applicationPath + '/skia'):
    for dirName in os.listdir(applicationPath + '/skia'):
      if os.path.isdir(applicationPath + '/skia/' + dirName):
        if os.path.exists(applicationPath + '/skia/' + dirName + '/Rsk.project'):
          parseRskProjectFile(dirName, applicationPath + '/skia/' + dirName + '/Rsk.project', packageMapCustom)

  #print('---------- Package List ----------')
  for item in packageMap:
    package = packageMap[item]
    #print(package.name + '[' + package.type + '][' + package.version + '], library : ' + package.libraryName)
    buildTargets += '  \"'+ modulesPath +'/'+package.name+'/skia:'+package.libraryName+'\",\n  '
  #print('------- Custom Package List ------')
  for item in packageMapCustom:
    package = packageMapCustom[item]
    #print(package.name + '[' + package.type + '][' + package.version + '], library : ' + package.libraryName)
    buildTargets += '  \"'+ applicationPath +'/skia/' + package.name + ':' + package.libraryName+'\",\n  '
  #print('----------------------------------')

  pkgJsonFileHandle.close()
# End of function preparePkgInfo()

# Main function
def main():

  global buildTargets
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

  externalBuildGnPath = sys.argv[1] + '/ReactSkia/external'
  generateBuildGn(externalBuildGnPath, buildTargets)

  codegenConfigFileHandle.close()

if __name__== "__main__":
  main()
