
#ifndef WEEXV8_CONSTANTSVALUE_H
#define WEEXV8_CONSTANTSVALUE_H

#include <string>

namespace WeexCore {

  // flex-direction
  constexpr char ROW[] = "row";
  constexpr char ROW_REVERSE[] = "row-reverse";
  constexpr char COLUMN[] = "column";
  constexpr char COLUMN_REVERSE[] = "column-reverse";

  // flex-wrap
  constexpr char NOWRAP[] = "nowrap";
  constexpr char WRAP[] = "wrap";
  constexpr char WRAP_REVERSE[] = "wrap-reverse";

  // align
  constexpr char FLEX_START[] = "flex-start";
  constexpr char FLEX_END[] = "flex-end";
  constexpr char CENTER[] = "center";
  constexpr char SPACE_BETWEEN[] = "space-between";
  constexpr char SPACE_AROUND[] = "space-around";
  constexpr char STRETCH[] = "stretch";
  constexpr char AUTO[] = "auto";

  // position
  constexpr char RELATIVE[] = "relative";
  constexpr char ABSOLUTE[] = "absolute";
  constexpr char FIXED[] = "fixed";
  constexpr char STICKY[] = "sticky";

  constexpr char NORMAL[] = "normal";
  constexpr int COLUMN_COUNT_NORMAL = 1;
  constexpr float AUTO_VALUE = -1;
  constexpr float COLUMN_GAP_NORMAL = 32;

  constexpr int HORIZONTAL_VALUE = 0;
  constexpr int VERTICAL_VALUE = 1;

  constexpr float kDefaultViewPortWidth = 750.0f;
}
#endif //WEEXV8_CONSTANTSVALUE_H
