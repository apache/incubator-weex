#ifndef WEEX_PROJECT_VIEWUTILS_H
#define WEEX_PROJECT_VIEWUTILS_H

#include "WeexCore/platform/android/base/LogUtils.h"
#include <../WeexCore/env/CoreEnvironment.h>
#include <math.h>

namespace WeexCore {
    static float getRealPxByWidth(float pxValue, float customViewport) {
        float realPx = (pxValue * WXCoreEnvironment::getInstance()->getDeviceWidth() /
                        customViewport);
        float result = realPx > 0.005 && realPx < 1 ? 1 : (float) rint(realPx);
        // LOGE("getRealPxByWidth=%f, result=%f", pxValue, result);
        return result;
    }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
