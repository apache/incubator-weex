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
package com.taobao.weex.ui.component.list;

import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.RestrictTo;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.flat.WidgetContainer;
import com.taobao.weex.ui.view.WXFrameLayout;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.LinkedList;

import static com.taobao.weex.common.Constants.Name.STICKY_OFFSET;

/**
 * Root component for components in {@link WXListComponent}
 */
@Component(lazyload = false)

public class WXCell extends WidgetContainer<WXFrameLayout> {

    private int mLastLocationY = 0;
    private ViewGroup mRealView;
    private View mTempStickyView;
    private View mHeadView;

    /** used in list sticky detect **/
    private int mScrollPosition = -1;
    private boolean mFlatUIEnabled = false;


    private Object  renderData;

    private boolean isSourceUsed = false;

    private boolean isAppendTreeDone;

    private CellAppendTreeListener cellAppendTreeListener;

    @Deprecated
    public WXCell(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
        super(instance, parent, basicComponentData);
    }

    public WXCell(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
        super(instance, parent, basicComponentData);
        lazy(true);
        if(Build.VERSION.SDK_INT< Build.VERSION_CODES.LOLLIPOP) {
            try {
                //TODO a WTF is necessary if anyone try to change the flat flag during update attrs.
                WXAttr attr = getAttrs();
                if (attr.containsKey(Constants.Name.FLAT)) {
                    mFlatUIEnabled = WXUtils.getBoolean(attr.get(Constants.Name.FLAT), false);
                }
            } catch (NullPointerException e) {
                WXLogUtils.e("Cell", WXLogUtils.getStackTrace(e));
            }
        }
    }

    @Override
    public boolean isLazy() {
        return super.isLazy() && !isFixed();
    }

    @Override
    @RestrictTo(RestrictTo.Scope.LIBRARY)
    public boolean isFlatUIEnabled() {
        return mFlatUIEnabled;
    }

    /**
     * If Cell is Sticky, need wraped FrameLayout
     */
    @Override
    protected WXFrameLayout initComponentHostView(@NonNull Context context) {
        if (isSticky()) {
            WXFrameLayout view = new WXFrameLayout(context);
            mRealView = new WXFrameLayout(context);
            view.addView(mRealView);
            //TODO Maybe there is a better solution for hardware-acceleration view's display list.
            if (isFlatUIEnabled()) {
                view.setLayerType(View.LAYER_TYPE_HARDWARE, null);
            }
            return view;
        } else {
            WXFrameLayout view = new WXFrameLayout(context);
            mRealView = view;
            if (isFlatUIEnabled()) {
                view.setLayerType(View.LAYER_TYPE_HARDWARE, null);
            }
            return view;
        }
    }

    public int getLocationFromStart(){
        return mLastLocationY;
    }

    public void setLocationFromStart(int l){
        mLastLocationY = l;
    }

    void setScrollPositon(int pos){
        mScrollPosition = pos;
    }

    public int getScrollPositon() {
        return mScrollPosition;
    }

    @Override
    public ViewGroup getRealView() {
        return mRealView;
    }

    public void removeSticky() {
        if(getHostView().getChildCount() > 0) {
            mHeadView = getHostView().getChildAt(0);
            int[] location = new int[2];
            int[] parentLocation = new int[2];
            getHostView().getLocationOnScreen(location);
            getParentScroller().getView().getLocationOnScreen(parentLocation);
            int headerViewOffsetX = location[0] - parentLocation[0];
            int headerViewOffsetY = getParent().getHostView().getTop();
            getHostView().removeView(mHeadView);
            mRealView = (ViewGroup) mHeadView;
            mTempStickyView = new FrameLayout(getContext());
            FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams((int) getLayoutWidth(),
                    (int) getLayoutHeight());
            getHostView().addView(mTempStickyView, lp);
            mHeadView.setTranslationX(headerViewOffsetX);
            mHeadView.setTranslationY(headerViewOffsetY);
        }
    }

    public void recoverySticky() {
        if(mHeadView != null){
            if(mHeadView.getLayoutParams() != null){
                ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) mHeadView.getLayoutParams();
                if(params.topMargin > 0){
                    params.topMargin = 0;
                }
            }
            if(mHeadView.getVisibility() != View.VISIBLE){
                mHeadView.setVisibility(View.VISIBLE);
            }
            if(mHeadView.getParent() != null){
                ((ViewGroup)mHeadView.getParent()).removeView(mHeadView);
            }
            getHostView().removeView(mTempStickyView);
            getHostView().addView(mHeadView);
            mHeadView.setTranslationX(0);
            mHeadView.setTranslationY(0);
        }
    }

    @Override
    protected void mountFlatGUI() {
        if(getHostView()!=null) {
            if(widgets == null){
                widgets = new LinkedList<>();
            }
            getHostView().mountFlatGUI(widgets);
        }
    }

    @Override
    public void unmountFlatGUI() {
        if (getHostView() != null) {
            getHostView().unmountFlatGUI();
        }
    }

    @Override
    public boolean intendToBeFlatContainer() {
        return getInstance().getFlatUIContext().isFlatUIEnabled(this) && WXCell.class.equals(getClass()) && !isSticky();
    }

    public int getStickyOffset(){
        if(getAttrs().get(STICKY_OFFSET) == null){
            return 0;
        }
        float  offset = WXUtils.getFloat(getAttrs().get(STICKY_OFFSET));
        return (int)(WXViewUtils.getRealPxByWidth(offset, getViewPortWidth()));
    }

    public Object getRenderData() {
        return renderData;
    }

    public void setRenderData(Object renderData) {
        this.renderData = renderData;
    }

    public boolean isSourceUsed() {
        return isSourceUsed;
    }

    public void setSourceUsed(boolean sourceUsed) {
        isSourceUsed = sourceUsed;
    }


    public boolean isAppendTreeDone(){
        return isAppendTreeDone;
    }

    @Override
    public void appendTreeCreateFinish() {
        super.appendTreeCreateFinish();
        isAppendTreeDone = true;
        if(cellAppendTreeListener != null){
            cellAppendTreeListener.onAppendTreeDone();
        }
    }

    public void setCellAppendTreeListener(CellAppendTreeListener cellAppendTreeListener) {
        this.cellAppendTreeListener = cellAppendTreeListener;
        if(isAppendTreeDone){
            cellAppendTreeListener.onAppendTreeDone();
        }
    }

    public interface CellAppendTreeListener{
        public void onAppendTreeDone();
    }
}
