#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "core/layout/flex_enum.h"
#include <string>

namespace WeexCore {

  const WXCoreFlexDirection &GetWXCoreFlexDirection(const std::string &value);

  const WXCoreJustifyContent &GetWXCoreJustifyContent(const std::string &value);

  const WXCoreAlignItems &GetWXCoreAlignItem(const std::string &value);

  const WXCoreFlexWrap &GetWXCoreFlexWrap(const std::string &value);

  const WXCoreAlignSelf &GetWXCoreAlignSelf(const std::string &value);

  const WXCorePositionType &GetWXCorePositionType(const std::string &value);
}


#endif //WEEX_PROJECT_STYLEGETTER_H
