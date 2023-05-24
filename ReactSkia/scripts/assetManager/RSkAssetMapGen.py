#!/usr/bin/python

# Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
import sys
import io
import os

assetMap = {}

# This is experimental and incomplete. Keep this false for now
imageGrouping = False

# Main function
def main():

  ImageExt = ('.png', '.svg', '.jpg', '.jpeg', 'tiff', 'avif', '.gif', '.bmp', '.webp')    # Add image formats here
  FontExt = ('.ttf', '.otf', '.woff', '.woff2')    # Add font formats here

  print('Scanning ' + sys.argv[1] + ' for assets..\nAssetMap.json will be generated in ' + sys.argv[2] + '/')

  # JSON data:
  assetMapJsonEmptyData = '{ "assets": { "images" : {}, "fonts" : {} } }'

  assetMapJsonData = json.loads(assetMapJsonEmptyData)
  # the result is a JSON string:
  print(json.dumps(assetMapJsonData, indent=2))

  currentWorkingDir = os.getcwd()
  os.chdir(sys.argv[1])
  for root, dirs, files in os.walk("./assets", topdown=True):
    for name in files:
      # Images
      if name.endswith(ImageExt):
        imageKey = os.path.splitext(name)[0]
        imagePath = os.path.join(root, name)
        if imageGrouping: # Experimental only, disabled for now.
          imageGroup = os.path.basename(os.path.dirname(imagePath))
          if imageGroup not in assetMapJsonData["assets"]["images"]: # Image group is not present so add gorup with empty data
            assetMapJsonData["assets"]["images"][imageGroup] = {}
          if imageKey not in assetMapJsonData["assets"]["images"][imageGroup]:
            assetMapJsonData["assets"]["images"][imageGroup][imageKey] = { "path" : imagePath}
        else:
          if imageKey not in assetMapJsonData["assets"]["images"]:
            assetMapJsonData["assets"]["images"][imageKey] = { "path" : imagePath}
      # Fonts
      elif name.endswith(FontExt):
        fontKey = os.path.splitext(name)[0]
        fontPath = os.path.join(root, name)
        assetMapJsonData["assets"]["fonts"][fontKey] = { "path" : fontPath}

  # Serializing json
  jsonOutData = json.dumps(assetMapJsonData, indent=2)

  # Dump JSON string:
  print(jsonOutData)
 
  # Writing to RSkAssetMap.json
  os.chdir(currentWorkingDir)
  with open(sys.argv[2] + '/assets/' + 'RSkAssetMap.json', "w") as outfile:
    outfile.write(jsonOutData)

if __name__== "__main__":
  main()
