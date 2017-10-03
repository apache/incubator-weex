/*
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
package com.taobao.weex.dom;

import android.support.v4.util.ArrayMap;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;


/**
 * Created by zhengshihan on 2017/2/21.
 */
public class WXRecyclerDomObject extends WXDomObject{


    private int mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
    private float mColumnWidth = Constants.Value.AUTO;
    private float mColumnGap = Constants.Value.COLUMN_GAP_NORMAL;
    private float mAvailableWidth = 0;
    private boolean mIsPreCalculateCellWidth =false;

    /**cell-slot not on the tree */
    private List<WXCellDomObject> cellList;


    public float getAvailableWidth() {
        return WXViewUtils.getRealPxByWidth(mAvailableWidth,getViewPortWidth());
    }

    public int getLayoutType(){
        return getAttrs().getLayoutType();
    }

    public float getColumnGap() {
        return WXViewUtils.getRealPxByWidth(mColumnGap,getViewPortWidth());
    }

    public int getColumnCount() {
        return mColumnCount;
    }

    public float getColumnWidth() {
        return WXViewUtils.getRealPxByWidth(mColumnWidth,getViewPortWidth());
    }
    @Override
    public void add(WXDomObject child, int index) {
        if(WXBasicComponentType.CELL_SLOT.equals(child.getType())
                && child instanceof  WXCellDomObject){
            if(cellList == null){
                cellList = Collections.synchronizedList(new ArrayList<WXCellDomObject>());
            }
            cellList.add((WXCellDomObject)child);
        }else{
            super.add(child, index);
        }

        if (WXBasicComponentType.CELL.equals(child.getType())
                || WXBasicComponentType.CELL_SLOT.equals(child.getType())) {
            if (!mIsPreCalculateCellWidth) {
                preCalculateCellWidth();
            }
            if(mColumnWidth!=0 && mColumnWidth!= Float.NaN) {
                child.getStyles().put(Constants.Name.WIDTH, mColumnWidth);
            }
        }
    }

    @Override
    public void remove(WXDomObject child) {
        if(cellList != null){
            cellList.remove(child);
        }
        super.remove(child);
    }

    @Override
    public void removeFromDom(WXDomObject child) {
        if(cellList != null){
            cellList.remove(child);
        }
        super.removeFromDom(child);
    }

    @Override
    public float getStyleWidth() {
        float width =  getLayoutWidth();
        if (Float.isNaN(width) || width <= 0){
            if(getParent() != null){
                width = getParent().getLayoutWidth();
            }
            if (Float.isNaN(width) || width <= 0){
                width = super.getStyleWidth();
            }
        }
        if (Float.isNaN(width) || width <= 0){
            width = getViewPortWidth();
        }
        return width;
    }

    public void preCalculateCellWidth(){

        if (getAttrs() != null) {
            mColumnCount = getAttrs().getColumnCount();
            mColumnWidth = getAttrs().getColumnWidth();
            mColumnGap =  getAttrs().getColumnGap();

            mAvailableWidth = getStyleWidth()-getPadding().get(Spacing.LEFT)-getPadding().get(Spacing.RIGHT);
            mAvailableWidth = WXViewUtils.getWebPxByWidth(mAvailableWidth,getViewPortWidth());

            if (Constants.Value.AUTO == mColumnCount && Constants.Value.AUTO == mColumnWidth) {
                mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
            } else if (Constants.Value.AUTO == mColumnWidth && Constants.Value.AUTO != mColumnCount) {
                mColumnWidth = (mAvailableWidth - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
            } else if (Constants.Value.AUTO != mColumnWidth && Constants.Value.AUTO == mColumnCount) {
                mColumnCount = Math.round((mAvailableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = mColumnCount > 0 ? mColumnCount :1;
                if (mColumnCount <= 0)
                    mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
                mColumnWidth =((mAvailableWidth + mColumnGap) / mColumnCount) - mColumnGap;
            } else if(Constants.Value.AUTO != mColumnWidth && Constants.Value.AUTO != mColumnCount){
                int columnCount = Math.round((mAvailableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = columnCount > mColumnCount ? mColumnCount :columnCount;
                if (mColumnCount <= 0)
                    mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
                mColumnWidth= ((mAvailableWidth + mColumnGap) / mColumnCount) - mColumnGap;
            }
            mIsPreCalculateCellWidth = true;
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d("preCalculateCellWidth mColumnGap :" + mColumnGap + " mColumnWidth:" + mColumnWidth + " mColumnCount:" + mColumnCount);
            }
        }
    }

    public boolean hasPreCalculateCellWidth(){
        return mIsPreCalculateCellWidth;
    }

    public void updateRecyclerAttr(){
        preCalculateCellWidth();
        if(mColumnWidth ==0 && mColumnWidth == Float.NaN){
            WXLogUtils.w("preCalculateCellWidth mColumnGap :" + mColumnGap + " mColumnWidth:" + mColumnWidth + " mColumnCount:" + mColumnCount);
            return;
        }
        int count = getChildCount();
        for(int i=0;i<count; i++){
            WXDomObject domObject = getChild(i);
            if(WXBasicComponentType.CELL.equals(domObject.getType())) {
                getChild(i).getStyles().put(Constants.Name.WIDTH, mColumnWidth);
            }
        }
    }

    @Override
    public void updateAttr(Map<String, Object> attrs) {
        super.updateAttr(attrs);
        if(attrs.containsKey(Constants.Name.COLUMN_COUNT)
                || attrs.containsKey(Constants.Name.COLUMN_GAP)
                || attrs.containsKey(Constants.Name.COLUMN_WIDTH)){
            updateRecyclerAttr();
        }
    }

    @Override
    protected Map<String, String> getDefaultStyle() {
        Map<String,String> map = new ArrayMap<>();

        boolean isVertical = true;
        if (parent != null) {
            if (parent.getType() != null) {
                if (parent.getType().equals(WXBasicComponentType.HLIST)) {
                    isVertical = false;
                }else{
                    if(getOrientation() == Constants.Orientation.HORIZONTAL){
                        isVertical = false;
                    }
                }
            }
        }

        String prop = isVertical ? Constants.Name.HEIGHT : Constants.Name.WIDTH;
        if (getStyles().get(prop) == null &&
                getStyles().get(Constants.Name.FLEX) == null) {
            map.put(Constants.Name.FLEX, "1");
        }

        return map;
    }


    public int getOrientation(){
        String direction = (String) getAttrs().get(Constants.Name.SCROLL_DIRECTION);
        if(Constants.Value.HORIZONTAL.equals(direction)){
            return Constants.Orientation.HORIZONTAL;
        }
        return  Constants.Orientation.VERTICAL;
    }

    @Override
    public WXDomObject clone() {
        if(isCloneThis()){
            return  this;
        }
        WXRecyclerDomObject domObject = (WXRecyclerDomObject) super.clone();
        domObject.cellList = cellList;
        return domObject;
    }

    public List<WXCellDomObject> getCellList() {
        return cellList;
    }
}
