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

#include <core/css/constants_name.h>
#include <core/render/node/render_object.h>
#include <core/css/constants_value.h>
#include <cmath>
#include <base/ViewUtils.h>
#include <core/render/node/factory/render_type.h>
#include "render_object.h"

namespace WeexCore {

  class RenderList : public RenderObject {

    bool mIsPreCalculateCellWidth = false;
    int mColumnCount = COLUMN_COUNT_NORMAL;
    float mColumnWidth = AUTO_VALUE;
    float mAvailableWidth = 0;
    float mColumnGap = COLUMN_GAP_NORMAL;
    bool mIsSetFlex = false;
    std::vector<RenderObject*> cellSlots;
    std::vector<RenderObject*> cellSlotsCopys;
    float mLeftGap = 0;
    float mRightGap = 0;

  public:
      ~RenderList(){

          if(cellSlotsCopys.size() > 0){
              for(auto it = cellSlotsCopys.begin(); it != cellSlotsCopys.end(); ++it){
                  RenderObject* child =  *it;
                  if(child){
                      delete child;
                      child = nullptr;
                  }
              }
              cellSlotsCopys.clear();
          }

          if(cellSlots.size() > 0){
              for(auto it = cellSlots.begin(); it != cellSlots.end(); ++it){
                  RenderObject* child =  *it;
                  if(child){
                      delete child;
                      child = nullptr;
                  }
              }
              cellSlots.clear();
          }
      }
      
  public:
      void addCellSlotCopyTrack(RenderObject* cellSlot){
          cellSlot->setParent(this, cellSlot);
          cellSlotsCopys.push_back(cellSlot);
      }
      
      std::vector<RenderObject*> &CellSlots(){
          return cellSlots;
      }
      
    std::map<std::string, std::string> *GetDefaultStyle() {
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

      if (prop == HEIGHT && isnan(getStyleHeight()) && !mIsSetFlex) {
        mIsSetFlex = true;
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      } else if (prop == WIDTH && isnan(getStyleWidth()) && !mIsSetFlex) {
        mIsSetFlex = true;
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }

    inline void setFlex(const float flex) {
        mIsSetFlex = true;
        WXCoreLayoutNode::setFlex(flex);
    }

    float calcFreeSpaceAlongMainAxis(const float &width, const float &height, const float &currentLength) const override {
      return NAN;
    }

    std::map<std::string, std::string> *GetDefaultAttr() {
      if (!mIsPreCalculateCellWidth) {
        preCalculateCellWidth();
      }
      return nullptr;
    }


    inline void preCalculateCellWidth() {
        std::map<std::string, std::string> *attrs = new std::map<std::string, std::string>();
        if (Attributes() != nullptr) {
            mColumnCount = getColumnCount();
            mColumnWidth = getColumnWidth();
            mColumnGap = getColumnGap();

            mLeftGap = getLeftGap();
            mRightGap = getRightGap();

            mAvailableWidth = getStyleWidth()- getWebPxByWidth(getPaddingLeft(), GetRenderPage()->ViewPortWidth()) - getWebPxByWidth(getPaddingRight(), GetRenderPage()->ViewPortWidth());

            if (AUTO_VALUE == mColumnCount && AUTO_VALUE == mColumnWidth) {
                mColumnCount = COLUMN_COUNT_NORMAL;
                mColumnWidth = (mAvailableWidth - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
            } else if (AUTO_VALUE == mColumnWidth && AUTO_VALUE != mColumnCount) {
                mColumnWidth = (mAvailableWidth - mLeftGap - mRightGap - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
            } else if (AUTO_VALUE != mColumnWidth && AUTO_VALUE == mColumnCount) {
                mColumnCount = (int)round((mAvailableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = mColumnCount > 0 ? mColumnCount :1;
                if (mColumnCount <= 0) {
                    mColumnCount = COLUMN_COUNT_NORMAL;
                }
                mColumnWidth =((mAvailableWidth + mColumnGap - mLeftGap - mRightGap) / mColumnCount) - mColumnGap;

            } else if(AUTO_VALUE != mColumnWidth && AUTO_VALUE != mColumnCount) {
                int columnCount = (int)round((mAvailableWidth + mColumnGap - mLeftGap - mRightGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = columnCount > mColumnCount ? mColumnCount :columnCount;
                if (mColumnCount <= 0) {
                    mColumnCount = COLUMN_COUNT_NORMAL;
                }
                mColumnWidth= ((mAvailableWidth + mColumnGap - mLeftGap - mRightGap) / mColumnCount) - mColumnGap;
            }

            std::string spanOffsets = calcSpanOffset();

            mIsPreCalculateCellWidth = true;
            if (getColumnCount() > 0 || getColumnWidth() > 0 || mColumnCount > COLUMN_COUNT_NORMAL) {
                attrs->insert(std::pair<std::string, std::string>(COLUMN_COUNT, to_string(mColumnCount)));
                attrs->insert(std::pair<std::string, std::string>(COLUMN_GAP, to_string(mColumnGap)));
                attrs->insert(std::pair<std::string, std::string>(COLUMN_WIDTH, to_string(mColumnWidth)));
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

    std::string calcSpanOffset() {
        std::string spanOffsets;
        if (mLeftGap > 0 || mRightGap > 0) {
            spanOffsets.append("[");
            for (int i = 0; i < mColumnCount; i++) {
                float spanOffset = mLeftGap + i * ((mColumnWidth + mColumnGap) - (mAvailableWidth + mColumnGap) / mColumnCount);
                spanOffsets.append(to_string(spanOffset));
                if(i != mColumnCount - 1) {
                    spanOffsets.append(",");
                }
            }
            spanOffsets.append("]");
        }
        return spanOffsets;
    }

    float getStyleWidth() {
      float width = getWebPxByWidth(getLayoutWidth(), GetRenderPage()->ViewPortWidth());
      if (isnan(width) || width <= 0){
        if(getParent() != nullptr){
          width = getWebPxByWidth(getParent()->getLayoutWidth(), GetRenderPage()->ViewPortWidth());
        }
        if (isnan(width) || width <= 0){
            width = getWebPxByWidth(RenderObject::getStyleWidth(), GetRenderPage()->ViewPortWidth());
        }
      }
      if (isnan(width) || width <= 0){
        width = GetViewPortWidth();
      }
      return width;
    }


    int AddRenderObject(int index, RenderObject *child) {
        if(Type() == kRenderRecycleList
                && (child->Type() == kRenderCellSlot ||  child->Type() == kRenderCell || child->Type()  == kRenderHeader)){
            child->setParent(this, child);
            cellSlots.insert(cellSlots.end(), child);
            index = -1;
        }else{
            index = RenderObject::AddRenderObject(index, child);
        }

        if (!mIsPreCalculateCellWidth) {
            preCalculateCellWidth();
        }

        if(mColumnWidth != 0 && !isnan(mColumnWidth)) {
            AddRenderObjectWidth(child, false);
        }
        return index;
    }

    void AddRenderObjectWidth(RenderObject *child, const bool updating) {
        if (Type() == kRenderWaterfall || Type() == kRenderRecycleList) {
            if(child->Type() == kRenderHeader || child->Type() == kRenderFooter) {
                child->ApplyStyle(WIDTH, to_string(mAvailableWidth), updating);
            } else if (child->IsSticky()) {
                child->ApplyStyle(WIDTH, to_string(mAvailableWidth), updating);
            } else if (child->Type() == kRenderCell || child->Type() == kRenderCellSlot){
                child->ApplyStyle(WIDTH, to_string(mColumnWidth), updating);
            }
        }
    }

    void UpdateAttr(std::string key, std::string value) {
      RenderObject::UpdateAttr(key, value);

      if(!GetAttr(COLUMN_COUNT).empty() || !GetAttr(COLUMN_GAP).empty() || !GetAttr(COLUMN_WIDTH).empty()){
          preCalculateCellWidth();

        if(mColumnWidth == 0 && isnan(mColumnWidth)) {
          return;
        }

        int count = getChildCount();
        for (Index i = 0; i < count; i++) {
          RenderObject *child = GetChild(i);
          AddRenderObjectWidth(this, true);
        }
      }
    }

    float getColumnCount() {
      std::string columnCount = GetAttr(COLUMN_COUNT);

      if (columnCount.empty() || columnCount == AUTO) {
        return AUTO_VALUE;
      }

      float columnCountValue = getFloat(columnCount.c_str());
      return (columnCountValue > 0 && !isnan(columnCountValue)) ? columnCountValue : AUTO_VALUE;
    }

    float getColumnGap() {
      std::string columnGap = GetAttr(COLUMN_GAP);

      if (columnGap.empty() || columnGap == NORMAL) {
        return COLUMN_GAP_NORMAL;
      }

      float columnGapValue = getFloat(columnGap.c_str());
      return (columnGapValue > 0 && !isnan(columnGapValue)) ? columnGapValue : AUTO_VALUE;
    }

    float getColumnWidth() {
      std::string columnWidth = GetAttr(COLUMN_WIDTH);

      if (columnWidth.empty() || columnWidth == AUTO) {
        return AUTO_VALUE;
      }

      float columnWidthValue = getFloat(columnWidth.c_str());
      return (columnWidthValue > 0 && !isnan(columnWidthValue)) ? columnWidthValue : 0;
    }

      float getLeftGap() {
          std::string leftGap = GetAttr(LEFT_GAP);

          if (leftGap.empty() || leftGap == AUTO) {
              return 0;
          }

          float leftGapValue = getFloat(leftGap.c_str());
          return (leftGapValue > 0 && !isnan(leftGapValue)) ? leftGapValue : 0;
      }

      float getRightGap() {
          std::string rightGap = GetAttr(RIGHT_GAP);

          if (rightGap.empty() || rightGap == AUTO) {
              return 0;
          }

          float rightGapValue = getFloat(rightGap.c_str());
          return (rightGapValue > 0 && !isnan(rightGapValue)) ? rightGapValue : 0;
      }

    int getOrientation(){
      std::string direction = GetAttr(SCROLL_DIRECTION);
      if(HORIZONTAL == direction){
        return HORIZONTAL_VALUE;
      }
      return VERTICAL_VALUE;
    }

  };
}

#endif //WEEX_PROJECT_RENDERLIST_H
