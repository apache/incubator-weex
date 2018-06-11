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
#ifndef WEEX_PROJECT_RENDERLIST_H
#define WEEX_PROJECT_RENDERLIST_H

#include "core/render/node/render_object.h"
#include "core/css/constants_value.h"

namespace WeexCore {

  class RenderList : public RenderObject {

  public:

    ~RenderList();

    void addCellSlotCopyTrack(RenderObject *cellSlot);

    void setFlex(const float flex);

    std::map <std::string, std::string> *GetDefaultStyle();

    std::map <std::string, std::string> *GetDefaultAttr();

    void preCalculateCellWidth();

    std::string calcSpanOffset();

    float getStyleWidth();

    int AddRenderObject(int index, RenderObject *child);

    void AddRenderObjectWidth(RenderObject *child, const bool updating);

    void UpdateAttr(std::string key, std::string value);

    float getColumnCount();

    float getColumnGap();

    float getColumnWidth();

    float getLeftGap();

    float getRightGap();

    int getOrientation();

    inline float calcFreeSpaceAlongMainAxis(const float &width, const float &height, const float &currentLength) const override {
      return NAN;
    }

    inline std::vector<RenderObject *> &CellSlots() {
      return cell_slots;
    }

  private:

    bool is_pre_calculate_cell_width = false;
    int column_count = COLUMN_COUNT_NORMAL;
    float column_width = AUTO_VALUE;
    float available_width = 0;
    float column_gap = COLUMN_GAP_NORMAL;
    bool is_set_flex = false;
    std::vector<RenderObject *> cell_slots;
    std::vector<RenderObject *> cell_slots_copys;
    float left_gap = 0;
    float right_gap = 0;
  };
}

#endif //WEEX_PROJECT_RENDERLIST_H
