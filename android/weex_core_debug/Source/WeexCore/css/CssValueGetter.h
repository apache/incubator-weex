#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "WeexCore/layout/WXCoreFlexEnum.h"
#include <string>

namespace WeexCore {

  WXCoreFlexDirection getWXCoreFlexDirection(std::string value);

  WXCoreJustifyContent getWXCoreJustifyContent(std::string value);

  WXCoreAlignItems getWXCoreAlignItem(std::string value);

  WXCoreFlexWrap getWXCoreFlexWrap(std::string value);

  WXCoreAlignSelf getWXCoreAlignSelf(std::string value);

  WXCorePositionType getWXCorePositionType(std::string value);
}


#endif //WEEX_PROJECT_STYLEGETTER_H
