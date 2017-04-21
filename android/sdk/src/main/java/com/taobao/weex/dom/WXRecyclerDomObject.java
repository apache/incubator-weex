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

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.Map;

/**
 * Created by zhengshihan on 2017/2/21.
 */

public class WXRecyclerDomObject extends WXDomObject{


    private int mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
    private float mColumnWidth = Constants.Value.AUTO;
    private float mColumnGap = Constants.Value.COLUMN_GAP_NORMAL;
    private boolean mIsPreCalculateCellWidth =false;

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
        super.add(child, index);

        if (WXBasicComponentType.CELL.equals(child.getType())) {
            if (!mIsPreCalculateCellWidth) {
                preCalculateCellWidth();
            }
            child.getStyles().put(Constants.Name.WIDTH, mColumnWidth);
        }
    }

    public void preCalculateCellWidth(){

        if (getAttrs() != null) {
            mColumnCount = getAttrs().getColumnCount();
            mColumnWidth = getAttrs().getColumnWidth();
            mColumnGap =  getAttrs().getColumnGap();

            float availableWidth = getStyleWidth()-getPadding().get(Spacing.LEFT)-getPadding().get(Spacing.RIGHT);
            availableWidth = WXViewUtils.getWebPxByWidth(availableWidth,getViewPortWidth());

            if (Constants.Value.AUTO == mColumnCount && Constants.Value.AUTO == mColumnWidth) {
                mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
            } else if (Constants.Value.AUTO == mColumnWidth && Constants.Value.AUTO != mColumnCount) {
                mColumnWidth = (availableWidth - ((mColumnCount - 1) * mColumnGap)) / mColumnCount;
                mColumnWidth = mColumnWidth > 0 ? mColumnWidth :0;
            } else if (Constants.Value.AUTO != mColumnWidth && Constants.Value.AUTO == mColumnCount) {
                mColumnCount = Math.round((availableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = mColumnCount > 0 ? mColumnCount :1;
                mColumnWidth =((availableWidth + mColumnGap) / mColumnCount) - mColumnGap;
            } else if(Constants.Value.AUTO != mColumnWidth && Constants.Value.AUTO != mColumnCount){
                int columnCount = Math.round((availableWidth + mColumnGap) / (mColumnWidth + mColumnGap)-0.5f);
                mColumnCount = columnCount > mColumnCount ? mColumnCount :columnCount;
                mColumnWidth= ((availableWidth + mColumnGap) / mColumnCount) - mColumnGap;
            }
            mIsPreCalculateCellWidth = true;
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d("preCalculateCellWidth mColumnGap :" + mColumnGap + " mColumnWidth:" + mColumnWidth + " mColumnCount:" + mColumnCount);
            }

        }
    }

    public void updateRecyclerAttr(){
        preCalculateCellWidth();
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
    public void updateStyle(Map<String, Object> styles, boolean byPesudo) {
        super.updateStyle(styles, byPesudo);
        if(styles.containsKey(Constants.Name.PADDING)
                ||styles.containsKey(Constants.Name.PADDING_LEFT)
                || styles.containsKey(Constants.Name.PADDING_RIGHT)){
            preCalculateCellWidth();
        }
    }
}
