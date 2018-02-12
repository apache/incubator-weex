#ifndef WEEX_PROJECT_RENDERLIST_H
#define WEEX_PROJECT_RENDERLIST_H

#include <WeexCore/css/ConstantsName.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/css/ConstantsValue.h>
#include <WeexCore/platform/android/base/LogUtils.h>
#include "RenderObject.h"

namespace WeexCore {

  class RenderList : public RenderObject {

    bool mIsPreCalculateCellWidth = false;
    int mColumnCount = COLUMN_COUNT_NORMAL;
    float mColumnWidth = AUTO_VALUE;
    float mAvailableWidth = 0;
    float mColumnGap = COLUMN_GAP_NORMAL;

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

      if (prop == HEIGHT && getStyleHeight() == 0.0 && GetStyle(FLEX).empty()) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      } else if (prop == WIDTH && getStyleWidth() == 0.0 && GetStyle(FLEX).empty()) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }


    std::map<std::string, std::string> *GetDefaultAttr() {
      if (!mIsPreCalculateCellWidth) {
        return preCalculateCellWidth();
      }
      return nullptr;
    }


    std::map<std::string, std::string> *preCalculateCellWidth() {
        std::map<std::string, std::string> *attrs = new std::map<std::string, std::string>();
        if (Attributes() != nullptr) {
            mColumnCount = getColumnCount();
            LOGE("listen mColumnCount 1 = %d", mColumnCount);
            mColumnWidth = getColumnWidth();
            mColumnGap = getColumnGap();

            mAvailableWidth = getStyleWidth()- getPaddingLeft() - getPaddingRight();
            // mAvailableWidth = getWebPxByWidth(mAvailableWidth, GetRenderPage()->ViewPortWidth());

            if (AUTO_VALUE == mColumnCount && AUTO_VALUE == mColumnWidth) {
                mColumnCount = COLUMN_COUNT_NORMAL;
                mColumnWidth = (mAvailableWidth - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
                LOGE("listen mColumnCount 2 = %d", mColumnCount);
            } else if (AUTO_VALUE == mColumnWidth && AUTO_VALUE != mColumnCount) {
                mColumnWidth = (mAvailableWidth - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
                LOGE("listen mColumnCount 3 = %d", mColumnCount);
            } else if (AUTO_VALUE != mColumnWidth && AUTO_VALUE == mColumnCount) {
                mColumnCount = round((mAvailableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = mColumnCount > 0 ? mColumnCount :1;
                if (mColumnCount <= 0)
                    mColumnCount = COLUMN_COUNT_NORMAL;
                mColumnWidth =((mAvailableWidth + mColumnGap) / mColumnCount) - mColumnGap;

                LOGE("listen mColumnCount 4 = %d", mColumnCount);
            } else if(AUTO_VALUE != mColumnWidth && AUTO_VALUE != mColumnCount){
                int columnCount = round((mAvailableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = columnCount > mColumnCount ? mColumnCount :columnCount;
                if (mColumnCount <= 0)
                    mColumnCount = COLUMN_COUNT_NORMAL;
                mColumnWidth= ((mAvailableWidth + mColumnGap) / mColumnCount) - mColumnGap;

                LOGE("listen mColumnCount 5 = %d", mColumnCount);
            }

            mIsPreCalculateCellWidth = true;
            LOGE("listen 6 mColumnCount=%d mColumnWidth=%f mColumnGap=%f", mColumnCount, mColumnWidth, mColumnGap);
            attrs->insert(std::pair<std::string, std::string>(COLUMN_COUNT, std::to_string(mColumnCount)));
            attrs->insert(std::pair<std::string, std::string>(COLUMN_WIDTH, std::to_string(mColumnWidth)));
            attrs->insert(std::pair<std::string, std::string>(COLUMN_GAP, std::to_string(mColumnGap)));
        }
        return attrs;
    }

    float getStyleWidth() {
        float width =  getLayoutWidth();
        if (isnan(width) || width <= 0){
            if(getParent() != nullptr){
                width = getParent()->getLayoutWidth();
            }
            if (isnan(width) || width <= 0){
                width = RenderObject::getStyleWidth();
            }
        }
        if (isnan(width) || width <= 0){
            width = GetRenderPage()->ViewPortWidth();
        }
        return width;
    }

    void AddRenderObject(int index, RenderObject *child) {
      RenderObject::AddRenderObject(index, child);

      if (!mIsPreCalculateCellWidth) {
        preCalculateCellWidth();
      }

      if(mColumnWidth != 0 && !isnan(mColumnWidth)) {
        //LOGE("listen child->ApplyStyle %s %s", child->Ref().c_str(), std::to_string(mColumnWidth).c_str());
        AddRenderObjectWidth(child);
      }
    }

    void AddRenderObjectWidth(RenderObject *child) {
        if (Type() == kRenderWaterfall) {
            if(child->Type() == kRenderHeader || child->Type() == kRenderFooter) {
                child->ApplyStyle(WIDTH, std::to_string(mAvailableWidth));
            } else if (child->Type() == kRenderCell){
                child->ApplyStyle(WIDTH, std::to_string(mColumnWidth));
            } else if (child->getStypePositionType() == kSticky) {
                child->ApplyStyle(WIDTH, std::to_string(GetRenderPage()->ViewPortWidth()));
            }
        }
    }

    void UpdateAttr(const std::string &key, const std::string &value) {
      RenderObject::UpdateAttr(key, value);

      if(!GetAttr(COLUMN_COUNT).empty() || !GetAttr(COLUMN_GAP).empty() || !GetAttr(COLUMN_WIDTH).empty()){
        preCalculateCellWidth();

        if(mColumnWidth == 0 && isnan(mColumnWidth)) {
          return;
        }

        int count = getChildCount();
        for (Index i = 0; i < count; i++) {
          RenderObject *child = GetChild(i);
          //LOGE("listen child->UpdateAttr %s %s", child->Ref().c_str(), std::to_string(mColumnWidth).c_str());
          // ApplyStyle(WIDTH, std::to_string(mColumnWidth));
          AddRenderObjectWidth(this);
        }
      }
    }

    float getColumnCount() {
      std::string columnCount = GetAttr(COLUMN_COUNT);

      if (columnCount.empty() || columnCount == AUTO) {
        return AUTO_VALUE;
      }

      float columnCountValue = atof(columnCount.c_str());
      return columnCountValue > 0 ? columnCountValue : AUTO_VALUE;
    }

    float getColumnGap() {
      std::string columnGap = GetAttr(COLUMN_GAP);

      if (columnGap.empty() || columnGap == NORMAL) {
        return COLUMN_GAP_NORMAL;
      }

      float columnGapValue = atof(columnGap.c_str());
      return columnGapValue > 0 ? columnGapValue : AUTO_VALUE;
    }

    float getColumnWidth() {
      std::string columnWidth = GetAttr(COLUMN_WIDTH);

      if (columnWidth.empty() || columnWidth == AUTO) {
        return AUTO_VALUE;
      }

      float columnWidthValue = atof(columnWidth.c_str());
      return columnWidthValue > 0 ? columnWidthValue : 0;
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
