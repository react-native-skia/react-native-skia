/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <include/core/SkSize.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkColor.h>

#define MAX_8BIT 255.99
inline SkSize RSkSkSizeFromSize(const facebook::react::Size &size) {

    return {size.width, size.height};
}

/* Converting React transform matrix of form

[ ScaleX SkewY  0      0
  SkewX  ScaleY 0      0
  0      0      ScaleZ Perps
  TransX TrasnY TrasnZ 0 ]

To Skia Matrix of form

[ ScaleX SKewX  TransX
  SkewY  ScaleY TransY
  Pers0  Pers1  Pers2 ]
*/
inline SkMatrix RSkTransformTo2DMatrix(const facebook::react::Transform &transformMatrix) {

    return {SkMatrix::MakeAll((float)transformMatrix.matrix[0],
          (float)transformMatrix.matrix[4],
          (float)transformMatrix.matrix[12],
          (float)transformMatrix.matrix[1],
          (float)transformMatrix.matrix[5],
          (float)transformMatrix.matrix[13],
          (float)transformMatrix.matrix[3],
          (float)transformMatrix.matrix[7],
          (float)transformMatrix.matrix[15])};
}

inline SkColor RSkColorFromSharedColor(facebook::react::SharedColor sharedColor,SkColor defaultColor) {

    if (sharedColor) {
        auto colorValue = colorComponentsFromColor(sharedColor);
        return SkColorSetARGB(colorValue.alpha * MAX_8BIT,colorValue.red * MAX_8BIT,colorValue.green * MAX_8BIT,colorValue.blue * MAX_8BIT);
    }
    return defaultColor;
}

inline facebook::react::Point RCTPointFromSkPoint(const SkPoint &point) {
  return {point.x(), point.y()};
}

inline facebook::react::Size RCTSizeFromSkSize(const SkSize &size) {
  return {size.width(), size.height()};
}
inline bool isOpaque(float opacity) {
  return (opacity >= MAX_8BIT) ? true:false;
}

inline facebook::react::EdgeInsets RCTEdgeInsetsFromSkRect(const SkRect &edgeInsets){
  return {edgeInsets.left(), edgeInsets.top(), edgeInsets.right(), edgeInsets.bottom()};
}
