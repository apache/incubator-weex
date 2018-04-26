/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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
