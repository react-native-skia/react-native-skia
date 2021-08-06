 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <math.h>

#include "include/core/SkRect.h"

#include "ReactSkia/views/common/RSkImageUtils.h"

namespace facebook {
namespace react {

namespace RSkImageUtils {

  SkRect computeTargetRect (Size srcSize,SkRect targetRect,ImageResizeMode resizeMode) {

    float startX{0},startY{0},width{0},height{0};
    float srcAR = srcSize.width / srcSize.height; /* source Aspect Ratio */
    float targetAR = targetRect.width() / targetRect.height(); /* Target Aspect ratio */

    switch(resizeMode) {

      case ImageResizeMode::Cover:
        if(targetAR <= srcAR) { /*target is taller than the source image */
          height = targetRect.height();
          width = height * srcAR;
          startX= targetRect.x() + ((targetRect.width() - width )/2);
          startY= targetRect.y();
        } else {  /* target is wider than the source image */
          width = targetRect.width();
          height = width /srcAR;
          startX= targetRect.x();
          startY= targetRect.y() + ((targetRect.height() - height )/2);
        }
        return SkRect:: MakeXYWH(startX, startY, width, height);

      case ImageResizeMode::Contain:
        if(targetAR <= srcAR) { /*target is taller than the source image*/
          width = targetRect.width();
          height = width / srcAR;
        } else {  /* target is wider than the source image */
          height = targetRect.height();
          width = height * srcAR;
        }
        startX= targetRect.x() + ((targetRect.width() - width )/2);
        startY= targetRect.y() + ((targetRect.height() - height )/2);
        return SkRect:: MakeXYWH(startX, startY, width, height);

      case ImageResizeMode::Center:
        width = srcSize.width;
        height = srcSize.height;
        if(srcSize.height > targetRect.height()) {
          width = targetRect.width();
          height = width / srcAR;
        }
        if(srcSize.width > targetRect.width()) {
          height = targetRect.height();
          width = height * srcAR;
        }
        startX= targetRect.x() + ((targetRect.width() - width )/2);
        startY= targetRect.y()+ ((targetRect.height() - height )/2);
        return SkRect:: MakeXYWH(startX, startY, width, height);

        case ImageResizeMode::Repeat:
        if((srcSize.width <= targetRect.width()) && (srcSize.height <= targetRect.height())) {
          return  SkRect:: MakeXYWH(targetRect.x(), targetRect.y(), srcSize.width, srcSize.height);
        } 
        /* Apply resizemode:contain logic mode to resize in case target is greater */
        if(targetAR <= srcAR) { /*target taller than the source image */
          width = targetRect.width();
          height = width / srcAR;
        } else {  /* target  wider than the source image */
          height = targetRect.height();
          width = height * srcAR;
        }
        return SkRect:: MakeXYWH(targetRect.x(), targetRect.y(), width, height);
      
        case ImageResizeMode::Stretch:
          return targetRect;
      }
  }
} //RSkImageUtils

} // namespace react
} // namespace facebook
