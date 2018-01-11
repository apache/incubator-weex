#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "WeexCore/layout/WXCoreFlexEnum.h"
#include <string>

namespace WeexCore {

  WXCoreFlexDirection GetWXCoreFlexDirection(const std::string &value);

  WXCoreJustifyContent GetWXCoreJustifyContent(const std::string &value);

  WXCoreAlignItems GetWXCoreAlignItem(const std::string &value);

  WXCoreFlexWrap GetWXCoreFlexWrap(const std::string &value);

  WXCoreAlignSelf GetWXCoreAlignSelf(const std::string &value);

  WXCorePositionType GetWXCorePositionType(const std::string &value);
}


#endif //WEEX_PROJECT_STYLEGETTER_H
