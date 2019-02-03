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

#include <math.h>
#include <cmath>
#include <utility>

#include "core/common/view_utils.h"
#include "core/css/constants_name.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"
#include "core/render/node/factory/render_creator.h"

namespace WeexCore {

RenderList::~RenderList() {
  if (this->cell_slots_copys_.size() > 0) {
    for (auto it = this->cell_slots_copys_.begin();
         it != this->cell_slots_copys_.end(); ++it) {
      RenderObject *child = *it;
      if (child) {
        delete child;
        child = nullptr;
      }
    }
    this->cell_slots_copys_.clear();
  }

  if (this->cell_slots_.size() > 0) {
    for (auto it = this->cell_slots_.begin(); it != this->cell_slots_.end();
         ++it) {
      RenderObject *child = *it;
      if (child) {
        delete child;
        child = nullptr;
      }
    }
    this->cell_slots_.clear();
  }
}

void RenderList::AddCellSlotCopyTrack(RenderObject *cell_slot) {
  cell_slot->setParent(this, cell_slot);
  this->cell_slots_copys_.push_back(cell_slot);
}

std::map<std::string, std::string> *RenderList::GetDefaultStyle() {
  std::map<std::string, std::string> *style =
      new std::map<std::string, std::string>();

  bool is_vertical = true;
  RenderObject *parent = static_cast<RenderObject *>(getParent());

  if (parent != nullptr && !parent->type().empty()) {
    if (parent->type() == kHList) {
      is_vertical = false;
    } else if (TakeOrientation() == HORIZONTAL_VALUE) {
      is_vertical = false;
    }
  }

  std::string prop = is_vertical ? HEIGHT : WIDTH;

  if (prop == HEIGHT && isnan(getStyleHeight()) && !this->is_set_flex_) {
    this->is_set_flex_ = true;
    style->insert(std::pair<std::string, std::string>(FLEX, "1"));
  } else if (prop == WIDTH && isnan(TakeStyleWidth()) && !this->is_set_flex_) {
    this->is_set_flex_ = true;
    style->insert(std::pair<std::string, std::string>(FLEX, "1"));
  }

  return style;
}

void RenderList::set_flex(const float flex) {
  this->is_set_flex_ = true;
  WXCoreLayoutNode::set_flex(flex);
}

std::map<std::string, std::string> *RenderList::GetDefaultAttr() {
  if (!this->is_pre_calculate_cell_width_) {
    PreCalculateCellWidth();
  }
  return nullptr;
}

void RenderList::PreCalculateCellWidth() {
  std::map<std::string, std::string> *attrs =
      new std::map<std::string, std::string>();
  if (attributes() != nullptr) {
    this->column_count_ = TakeColumnCount();
    this->column_width_ = TakeColumnWidth();
    this->column_gap_ = TakeColumnGap();

    this->left_gap_ = TakeLeftGap();
    this->right_gap_ = TakeRightGap();

    this->available_width_ =
        TakeStyleWidth() -
        getWebPxByWidth(getPaddingLeft(), RenderManager::GetInstance()->viewport_width(page_id())) -
        getWebPxByWidth(getPaddingRight(), RenderManager::GetInstance()->viewport_width(page_id()));

    if (AUTO_VALUE == this->column_count_ &&
        AUTO_VALUE == this->column_width_) {
      this->column_count_ = COLUMN_COUNT_NORMAL;
      this->column_width_ = (this->available_width_ -
                             ((this->column_count_ - 1) * this->column_gap_)) /
                            this->column_count_;
      this->column_width_ = this->column_width_ > 0 ? this->column_width_ : 0;
    } else if (AUTO_VALUE == this->column_width_ &&
               AUTO_VALUE != this->column_count_) {
      this->column_width_ =
          (this->available_width_ - this->left_gap_ - this->right_gap_ -
           ((this->column_count_ - 1) * this->column_gap_)) /
          this->column_count_;
      this->column_width_ = this->column_width_ > 0 ? this->column_width_ : 0;
    } else if (AUTO_VALUE != this->column_width_ &&
               AUTO_VALUE == this->column_count_) {
      this->column_count_ =
          static_cast<int>(round((this->available_width_ + this->column_gap_) /
                                     (this->column_width_ + this->column_gap_) -
                                 0.5f));
      this->column_count_ = this->column_count_ > 0 ? this->column_count_ : 1;
      if (this->column_count_ <= 0) {
        this->column_count_ = COLUMN_COUNT_NORMAL;
      }
      this->column_width_ = ((this->available_width_ + this->column_gap_ -
                              this->left_gap_ - this->right_gap_) /
                             this->column_count_) -
                            this->column_gap_;

    } else if (AUTO_VALUE != this->column_width_ &&
               AUTO_VALUE != this->column_count_) {
      int column_count =
          static_cast<int>(round((this->available_width_ + this->column_gap_ -
                                  this->left_gap_ - this->right_gap_) /
                                     (this->column_width_ + this->column_gap_) -
                                 0.5f));
      this->column_count_ =
          column_count > this->column_count_ ? this->column_count_ : column_count;
      if (this->column_count_ <= 0) {
        this->column_count_ = COLUMN_COUNT_NORMAL;
      }
      this->column_width_ = ((this->available_width_ + this->column_gap_ -
                              this->left_gap_ - this->right_gap_) /
                             this->column_count_) -
                            this->column_gap_;
    }

    std::string span_offsets = CalculateSpanOffset();

    this->is_pre_calculate_cell_width_ = true;
    if (TakeColumnCount() > 0 || TakeColumnWidth() > 0 ||
        this->column_count_ >= COLUMN_COUNT_NORMAL) {
      attrs->insert(std::pair<std::string, std::string>(
          COLUMN_COUNT, to_string(this->column_count_)));
      attrs->insert(std::pair<std::string, std::string>(
          COLUMN_GAP, to_string(this->column_gap_)));
      attrs->insert(std::pair<std::string, std::string>(
          COLUMN_WIDTH, to_string(this->column_width_)));
    }
    if (span_offsets.length() > 0) {
      attrs->insert(std::pair<std::string, std::string>(
          SPAN_OFFSETS, to_string(span_offsets)));
    }

    for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
      RenderObject::UpdateAttr(iter->first, iter->second);
    }
  }

  RenderPage *page = GetRenderPage();

  if (page != nullptr) page->SendUpdateAttrAction(this, attrs);

  if (attrs != nullptr) {
    attrs->clear();
    delete attrs;
    attrs = nullptr;
  }
}

std::string RenderList::CalculateSpanOffset() {
  std::string span_offsets;
  float divide = available_width_ / column_count_;
  float item_start_pos = 0;
  if (this->left_gap_ > 0 || this->right_gap_ > 0 || column_gap_ > 0) {
    span_offsets.append("[");
    for (int i = 0; i < this->column_count_; i++) {
      if (i == 0) {
        item_start_pos += left_gap_;
      } else {
        item_start_pos += column_gap_ + column_width_;
      }
      float span_offset = item_start_pos - i * divide;
      span_offsets.append(to_string(span_offset));
      if (i != this->column_count_ - 1) {
        span_offsets.append(",");
      }
    }
    span_offsets.append("]");
  }
  return span_offsets;
}

float RenderList::TakeStyleWidth() {
  float width =
      getWebPxByWidth(getLayoutWidth(), RenderManager::GetInstance()->viewport_width(page_id()));
  if (isnan(width) || width <= 0) {
    if (getParent() != nullptr) {
      width = getWebPxByWidth(getParent()->getLayoutWidth(),
                              RenderManager::GetInstance()->viewport_width(page_id()));
    }
    if (isnan(width) || width <= 0) {
      width = getWebPxByWidth(RenderObject::getStyleWidth(),
                              RenderManager::GetInstance()->viewport_width(page_id()));
    }
  }
  if (isnan(width) || width <= 0) {
    width = RenderManager::GetInstance()->viewport_width(page_id());
  }
  return width;
}

int RenderList::AddRenderObject(int index, RenderObject *child) {
  if (type() == kRenderRecycleList &&
      (child->type() == kRenderCellSlot || child->type() == kRenderCell ||
       child->type() == kRenderHeader)) {
    child->setParent(this, child);
    this->cell_slots_.insert(this->cell_slots_.end(), child);
    index = -1;
  } else {
    index = RenderObject::AddRenderObject(index, child);
  }

  if (!this->is_pre_calculate_cell_width_) {
    PreCalculateCellWidth();
  }

  if (this->column_width_ != 0 && !isnan(this->column_width_)) {
    AddRenderObjectWidth(child, false);
  }
  return index;
}

void RenderList::AddRenderObjectWidth(RenderObject *child,
                                      const bool updating) {
  if ((RenderCreator::GetInstance()->IsAffineType(type(), kRenderWaterfall)) || type() == kRenderRecycleList) {
    if (child->type() == kRenderHeader || child->type() == kRenderFooter) {
      child->ApplyStyle(WIDTH, to_string(this->available_width_), updating);
    } else if (child->is_sticky()) {
      child->ApplyStyle(WIDTH, to_string(this->available_width_), updating);
    } else if (child->type() == kRenderCell ||
               child->type() == kRenderCellSlot) {
      child->ApplyStyle(WIDTH, to_string(this->column_width_), updating);
    }
  }
}


void RenderList::AddAttr(std::string key, std::string value) {
  MapInsertOrAssign(&mOriginalAttrs, key, value);
  RenderObject::AddAttr(key, value);
}


void RenderList::UpdateAttr(std::string key, std::string value) {
  MapInsertOrAssign(&mOriginalAttrs, key, value);
  RenderObject::UpdateAttr(key, value);

  if (!GetAttr(COLUMN_COUNT).empty() || !GetAttr(COLUMN_GAP).empty() ||
      !GetAttr(COLUMN_WIDTH).empty()) {
    PreCalculateCellWidth();

    if (this->column_width_ == 0 && isnan(this->column_width_)) {
      return;
    }

    Index count = getChildCount();
    for (Index i = 0; i < count; i++) {
      RenderObject *child = GetChild(i);
      AddRenderObjectWidth(child, true);
    }
  }
}

static const std::string GetMapAttr(std::map<std::string,std::string>* attrs,  const std::string &key) {
  if (attrs == nullptr) return "";
  std::map<std::string, std::string>::iterator iter = attrs->find(key);
  if (iter != attrs->end()) {
    return iter->second;
  } else {
    return "";
  }
}

float RenderList::TakeColumnCount() {
  std::string column_count = GetMapAttr(&mOriginalAttrs, COLUMN_COUNT);

  if (column_count.empty() || column_count == AUTO) {
    return AUTO_VALUE;
  }

  float column_count_value = getFloat(column_count.c_str());
  return (column_count_value > 0 && !isnan(column_count_value)) ? column_count_value
                                                            : AUTO_VALUE;
}

float RenderList::TakeColumnGap() {
  std::string column_gap = GetMapAttr(&mOriginalAttrs, COLUMN_GAP);

  if (column_gap.empty() || column_gap == NORMAL) {
    return COLUMN_GAP_NORMAL;
  }

  float column_gap_value = getFloat(column_gap.c_str());
  return (column_gap_value > 0 && !isnan(column_gap_value)) ? column_gap_value
                                                        : AUTO_VALUE;
}

float RenderList::TakeColumnWidth() {
  std::string column_width = GetMapAttr(&mOriginalAttrs, COLUMN_WIDTH);

  if (column_width.empty() || column_width == AUTO) {
    return AUTO_VALUE;
  }

  float column_width_value = getFloat(column_width.c_str());
  return (column_width_value > 0 && !isnan(column_width_value)) ? column_width_value
                                                            : 0;
}

float RenderList::TakeLeftGap() {
  std::string left_gap =GetMapAttr(&mOriginalAttrs, LEFT_GAP);

  if (left_gap.empty() || left_gap == AUTO) {
    return 0;
  }

  float left_gap_value = getFloat(left_gap.c_str());
  return (left_gap_value > 0 && !isnan(left_gap_value)) ? left_gap_value : 0;
}

float RenderList::TakeRightGap() {
  std::string right_gap =GetMapAttr(&mOriginalAttrs, RIGHT_GAP);

  if (right_gap.empty() || right_gap == AUTO) {
    return 0;
  }

  float right_gap_value = getFloat(right_gap.c_str());
  return (right_gap_value > 0 && !isnan(right_gap_value)) ? right_gap_value : 0;
}

int RenderList::TakeOrientation() {
  std::string direction = GetAttr(SCROLL_DIRECTION);
  if (HORIZONTAL == direction) {
    return HORIZONTAL_VALUE;
  }
  return VERTICAL_VALUE;
}
}  // namespace WeexCore
