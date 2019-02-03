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
#ifndef CORE_RENDER_NODE_RENDER_LIST_H_
#define CORE_RENDER_NODE_RENDER_LIST_H_

#include <map>
#include <string>
#include <vector>

#include "core/css/constants_value.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

class RenderList : public RenderObject {
 public:
  ~RenderList();

  void set_flex(const float flex) override;

  void AddCellSlotCopyTrack(RenderObject *cell_slot);

  std::map<std::string, std::string> *GetDefaultStyle() override;

  std::map<std::string, std::string> *GetDefaultAttr() override;

  void PreCalculateCellWidth();

  std::string CalculateSpanOffset();

  float TakeStyleWidth();

  int AddRenderObject(int index, RenderObject *child) override;

  void AddRenderObjectWidth(RenderObject *child, const bool updating);

  void AddAttr(std::string key, std::string value) override;

  void UpdateAttr(std::string key, std::string value) override;

  float TakeColumnCount();

  float TakeColumnGap();

  float TakeColumnWidth();

  float TakeLeftGap();

  float TakeRightGap();

  int TakeOrientation();

  inline float CalculateFreeSpaceAlongMainAxis(
      const float &width, const float &height,
      const float &currentLength) const override {
    return NAN;
  }

  inline std::vector<RenderObject *> &CellSlots() { return cell_slots_; }

 private:
  bool is_pre_calculate_cell_width_ = false;
  int column_count_ = COLUMN_COUNT_NORMAL;
  float column_width_ = AUTO_VALUE;
  float available_width_ = 0;
  float column_gap_ = COLUMN_GAP_NORMAL;
  bool is_set_flex_ = false;
  std::vector<RenderObject *> cell_slots_;
  std::vector<RenderObject *> cell_slots_copys_;
  float left_gap_ = 0;
  float right_gap_ = 0;
  std::map<std::string,std::string> mOriginalAttrs;

};
}  // namespace WeexCore

#endif  // CORE_RENDER_NODE_RENDER_LIST_H_
