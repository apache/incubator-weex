#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "WeexCore/layout/WXCoreFlexEnum.h"
#include <string>

namespace WeexCore {

  WXCoreFlexDirection getWXCoreFlexDirection(const std::string &value);

  WXCoreJustifyContent getWXCoreJustifyContent(const std::string &value);

  WXCoreAlignItems getWXCoreAlignItem(const std::string &value);

  WXCoreFlexWrap getWXCoreFlexWrap(const std::string &value);

  WXCoreAlignSelf getWXCoreAlignSelf(const std::string &value);

  WXCorePositionType getWXCorePositionType(const std::string &value);
}


#endif //WEEX_PROJECT_STYLEGETTER_H
