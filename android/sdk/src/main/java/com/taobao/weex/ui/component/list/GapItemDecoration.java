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
package com.taobao.weex.ui.component.list;

import android.graphics.Rect;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.view.View;

import com.taobao.weex.dom.WXRecyclerDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXViewUtils;

/**
 * Created by furture on 2018/2/12.
 * recyclerview's height is layout width + column gap,
 * so the the offset should include, leftGap and rightGap is via this
 */
public class GapItemDecoration extends RecyclerView.ItemDecoration {

    private WXListComponent listComponent;

    public GapItemDecoration(WXListComponent listComponent) {
        this.listComponent = listComponent;
    }

    @Override
    public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
        int position = parent.getChildAdapterPosition(view);
        if(position < 0){
            return;
        }
        if(view.getLayoutParams() instanceof StaggeredGridLayoutManager.LayoutParams){
            StaggeredGridLayoutManager.LayoutParams params = (StaggeredGridLayoutManager.LayoutParams) view.getLayoutParams();
            if(params.isFullSpan()){
                return;
            }
            WXComponent component = listComponent.getChild(position);
            if(component instanceof  WXCell){
                WXCell cell = (WXCell) component;
                if(cell.isFixed() || cell.isSticky()){
                    return;
                }
                WXRecyclerDomObject recyclerDomObject = listComponent.getRecyclerDom();
                if(recyclerDomObject.getSpanOffsets() == null){
                    return;
                }
                float spanOffset = recyclerDomObject.getSpanOffsets()[params.getSpanIndex()];
                int   spanOffsetPx =  Math.round(WXViewUtils.getRealPxByWidth(spanOffset, recyclerDomObject.getViewPortWidth()));
                outRect.left =  spanOffsetPx;
                outRect.right = -spanOffsetPx;
            }
        }
    }
}
