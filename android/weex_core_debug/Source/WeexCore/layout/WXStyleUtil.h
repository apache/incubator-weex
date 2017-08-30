
#ifndef WEEXV8_WXSTYLEUTIL_H
#define WEEXV8_WXSTYLEUTIL_H

#include "ConstantsValue.h"
#include "../../yoga/Yoga.h"
#include <string>

using namespace std;

YGFlexDirection getYGFlexDirection(string value);

YGJustify getYGJustifyContent(string value);

YGAlign getYGAlignItem(string value);

YGPositionType getYGPositionType(string value);

YGAlign getYGAlignSelf(string value);

YGWrap getYGWrap(string value);

YGOverflow getYGOverflow(string value);

#endif //WEEXV8_WXSTYLEUTIL_H
