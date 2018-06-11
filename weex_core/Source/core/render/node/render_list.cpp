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
#include <cmath>

#include "core/render/node/render_list.h"
#include "core/css/constants_name.h"
#include "base/ViewUtils.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"

namespace WeexCore {

  RenderList::~RenderList() {
    if (this->cell_slots_copys.size() > 0) {
      for (auto it = this->cell_slots_copys.begin(); it != this->cell_slots_copys.end(); ++it) {
        RenderObject *child = *it;
        if (child) {
          delete child;
          child = nullptr;
        }
      }
      this->cell_slots_copys.clear();
    }

    if (this->cell_slots.size() > 0) {
      for (auto it = this->cell_slots.begin(); it != this->cell_slots.end(); ++it) {
        RenderObject *child = *it;
        if (child) {
          delete child;
          child = nullptr;
        }
      }
      this->cell_slots.clear();
    }
  }

  void RenderList::addCellSlotCopyTrack(RenderObject *cellSlot) {
    cellSlot->setParent(this, cellSlot);
    this->cell_slots_copys.push_back(cellSlot);
  }

  std::map<std::string, std::string> *RenderList::GetDefaultStyle() {
    std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

    bool isVertical = true;
    RenderObject *parent = (RenderObject *) getParent();

    if (parent != nullptr && !parent->Type().empty()) {
      if (parent->Type() == kHList) {
        isVertical = false;
      } else if (getOrientation() == HORIZONTAL_VALUE) {
        isVertical = false;
      }
    }

    std::string prop = isVertical ? HEIGHT : WIDTH;

    if (prop == HEIGHT && isnan(getStyleHeight()) && !this->is_set_flex) {
      this->is_set_flex = true;
      style->insert(std::pair<std::string, std::string>(FLEX, "1"));
    } else if (prop == WIDTH && isnan(getStyleWidth()) && !this->is_set_flex) {
      this->is_set_flex = true;
      style->insert(std::pair<std::string, std::string>(FLEX, "1"));
    }

    return style;
  }

  void RenderList::setFlex(const float flex) {
    this->is_set_flex = true;
    WXCoreLayoutNode::setFlex(flex);
  }

  std::map<std::string, std::string> *RenderList::GetDefaultAttr() {
    if (!this->is_pre_calculate_cell_width) {
      preCalculateCellWidth();
    }
    return nullptr;
  }

  void RenderList::preCalculateCellWidth() {
    std::map<std::string, std::string> *attrs = new std::map<std::string, std::string>();
    if (Attributes() != nullptr) {
      this->column_count = getColumnCount();
      this->column_width = getColumnWidth();
      this->column_gap = getColumnGap();

      this->left_gap = getLeftGap();
      this->right_gap = getRightGap();

      this->available_width =
          getStyleWidth() - getWebPxByWidth(getPaddingLeft(), GetRenderPage()->ViewPortWidth()) -
          getWebPxByWidth(getPaddingRight(), GetRenderPage()->ViewPortWidth());

      if (AUTO_VALUE == this->column_count && AUTO_VALUE == this->column_width) {
        this->column_count = COLUMN_COUNT_NORMAL;
        this->column_width =
            (this->available_width - ((this->column_count - 1) * this->column_gap)) /
            this->column_count;
        this->column_width = this->column_width > 0 ? this->column_width : 0;
      } else if (AUTO_VALUE == this->column_width && AUTO_VALUE != this->column_count) {
        this->column_width =
            (this->available_width - this->left_gap - this->right_gap -
             ((this->column_count - 1) * this->column_gap)) /
            this->column_count;
        this->column_width = this->column_width > 0 ? this->column_width : 0;
      } else if (AUTO_VALUE != this->column_width && AUTO_VALUE == this->column_count) {
        this->column_count = (int) round(
            (this->available_width + this->column_gap) / (this->column_width + this->column_gap) -
            0.5f);
        this->column_count = this->column_count > 0 ? this->column_count : 1;
        if (this->column_count <= 0) {
          this->column_count = COLUMN_COUNT_NORMAL;
        }
        this->column_width =
            ((this->available_width + this->column_gap - this->left_gap - this->right_gap) /
             this->column_count) - this->column_gap;

      } else if (AUTO_VALUE != this->column_width && AUTO_VALUE != this->column_count) {
        int columnCount = (int) round(
            (this->available_width + this->column_gap - this->left_gap - this->right_gap) /
            (this->column_width + this->column_gap) -
            0.5f);
        this->column_count = columnCount > this->column_count ? this->column_count : columnCount;
        if (this->column_count <= 0) {
          this->column_count = COLUMN_COUNT_NORMAL;
        }
        this->column_width =
            ((this->available_width + this->column_gap - this->left_gap - this->right_gap) /
             this->column_count) - this->column_gap;
      }

      std::string spanOffsets = calcSpanOffset();

      this->is_pre_calculate_cell_width = true;
      if (getColumnCount() > 0 || getColumnWidth() > 0 ||
          this->column_count > COLUMN_COUNT_NORMAL) {
        attrs->insert(
            std::pair<std::string, std::string>(COLUMN_COUNT, to_string(this->column_count)));
        attrs->insert(std::pair<std::string, std::string>(COLUMN_GAP, to_string(this->column_gap)));
        attrs->insert(
            std::pair<std::string, std::string>(COLUMN_WIDTH, to_string(this->column_width)));
      }
      if (spanOffsets.length() > 0) {
        attrs->insert(std::pair<std::string, std::string>(SPAN_OFFSETS, to_string(spanOffsets)));
      }

      for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
        RenderObject::UpdateAttr(iter->first, iter->second);
      }
    }

    RenderPage *page = GetRenderPage();

    if (page != nullptr)
      page->SendUpdateAttrAction(this, attrs);

    if (attrs != nullptr) {
      attrs->clear();
      delete attrs;
      attrs = nullptr;
    }
  }

  std::string RenderList::calcSpanOffset() {
    std::string spanOffsets;
    if (this->left_gap > 0 || this->right_gap > 0) {
      spanOffsets.append("[");
      for (int i = 0; i < this->column_count; i++) {
        float spanOffset = this->left_gap + i * ((this->column_width + this->column_gap) -
                                                 (this->available_width + this->column_gap) /
                                                 this->column_count);
        spanOffsets.append(to_string(spanOffset));
        if (i != this->column_count - 1) {
          spanOffsets.append(",");
        }
      }
      spanOffsets.append("]");
    }
    return spanOffsets;
  }

  float RenderList::getStyleWidth() {
    float width = getWebPxByWidth(getLayoutWidth(), GetRenderPage()->ViewPortWidth());
    if (isnan(width) || width <= 0) {
      if (getParent() != nullptr) {
        width = getWebPxByWidth(getParent()->getLayoutWidth(), GetRenderPage()->ViewPortWidth());
      }
      if (isnan(width) || width <= 0) {
        width = getWebPxByWidth(RenderObject::getStyleWidth(), GetRenderPage()->ViewPortWidth());
      }
    }
    if (isnan(width) || width <= 0) {
      width = GetViewPortWidth();
    }
    return width;
  }

  int RenderList::AddRenderObject(int index, RenderObject *child) {
    if (Type() == kRenderRecycleList
        && (child->Type() == kRenderCellSlot || child->Type() == kRenderCell ||
            child->Type() == kRenderHeader)) {
      child->setParent(this, child);
      this->cell_slots.insert(this->cell_slots.end(), child);
      index = -1;
    } else {
      index = RenderObject::AddRenderObject(index, child);
    }

    if (!this->is_pre_calculate_cell_width) {
      preCalculateCellWidth();
    }

    if (this->column_width != 0 && !isnan(this->column_width)) {
      AddRenderObjectWidth(child, false);
    }
    return index;
  }

  void RenderList::AddRenderObjectWidth(RenderObject *child, const bool updating) {
    if (Type() == kRenderWaterfall || Type() == kRenderRecycleList) {
      if (child->Type() == kRenderHeader || child->Type() == kRenderFooter) {
        child->ApplyStyle(WIDTH, to_string(this->available_width), updating);
      } else if (child->IsSticky()) {
        child->ApplyStyle(WIDTH, to_string(this->available_width), updating);
      } else if (child->Type() == kRenderCell || child->Type() == kRenderCellSlot) {
        child->ApplyStyle(WIDTH, to_string(this->column_width), updating);
      }
    }
  }

  void RenderList::UpdateAttr(std::string key, std::string value) {
    RenderObject::UpdateAttr(key, value);

    if (!GetAttr(COLUMN_COUNT).empty() || !GetAttr(COLUMN_GAP).empty() ||
        !GetAttr(COLUMN_WIDTH).empty()) {
      preCalculateCellWidth();

      if (this->column_width == 0 && isnan(this->column_width)) {
        return;
      }

      int count = getChildCount();
      for (Index i = 0; i < count; i++) {
        RenderObject *child = GetChild(i);
        AddRenderObjectWidth(this, true);
      }
    }
  }

  float RenderList::getColumnCount() {
    std::string columnCount = GetAttr(COLUMN_COUNT);

    if (columnCount.empty() || columnCount == AUTO) {
      return AUTO_VALUE;
    }

    float columnCountValue = getFloat(columnCount.c_str());
    return (columnCountValue > 0 && !isnan(columnCountValue)) ? columnCountValue : AUTO_VALUE;
  }

  float RenderList::getColumnGap() {
    std::string columnGap = GetAttr(COLUMN_GAP);

    if (columnGap.empty() || columnGap == NORMAL) {
      return COLUMN_GAP_NORMAL;
    }

    float columnGapValue = getFloat(columnGap.c_str());
    return (columnGapValue > 0 && !isnan(columnGapValue)) ? columnGapValue : AUTO_VALUE;
  }

  float RenderList::getColumnWidth() {
    std::string columnWidth = GetAttr(COLUMN_WIDTH);

    if (columnWidth.empty() || columnWidth == AUTO) {
      return AUTO_VALUE;
    }

    float columnWidthValue = getFloat(columnWidth.c_str());
    return (columnWidthValue > 0 && !isnan(columnWidthValue)) ? columnWidthValue : 0;
  }

  float RenderList::getLeftGap() {
    std::string leftGap = GetAttr(LEFT_GAP);

    if (leftGap.empty() || leftGap == AUTO) {
      return 0;
    }

    float leftGapValue = getFloat(leftGap.c_str());
    return (leftGapValue > 0 && !isnan(leftGapValue)) ? leftGapValue : 0;
  }

  float RenderList::getRightGap() {
    std::string rightGap = GetAttr(RIGHT_GAP);

    if (rightGap.empty() || rightGap == AUTO) {
      return 0;
    }

    float rightGapValue = getFloat(rightGap.c_str());
    return (rightGapValue > 0 && !isnan(rightGapValue)) ? rightGapValue : 0;
  }

  int RenderList::getOrientation() {
    std::string direction = GetAttr(SCROLL_DIRECTION);
    if (HORIZONTAL == direction) {
      return HORIZONTAL_VALUE;
    }
    return VERTICAL_VALUE;
  }
}
