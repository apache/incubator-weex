//
// Created by miomin on 2017/7/27.
//

#ifndef WEEXV8_WXSTYLEUTIL_H
#define WEEXV8_WXSTYLEUTIL_H

#include <wtf/text/WTFString.h>
#include "ConstantsValue.h"
#include "../../yoga/Yoga.h"

using namespace WTF;

YGFlexDirection getYGFlexDirection(String value);

YGJustify getYGJustifyContent(String value);

YGAlign getYGAlignItem(String value);

YGPositionType getYGPositionType(String value);

YGAlign getYGAlignSelf(String value);

YGWrap getYGWrap(String value);

YGOverflow getYGOverflow(String value);

#endif //WEEXV8_WXSTYLEUTIL_H
