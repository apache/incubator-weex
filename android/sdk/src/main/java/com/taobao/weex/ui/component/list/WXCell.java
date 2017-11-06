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
import android.support.annotation.NonNull;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.ui.action.WXUIAction;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.view.WXFrameLayout;

/**
 * Root component for components in {@link WXListComponent}
 */
@Component(lazyload = false)

public class WXCell extends WXVContainer<WXFrameLayout> {

    private int mLastLocationY = 0;
    private ViewGroup mRealView;
    private View mTempStickyView;
    private View mHeadView;
    private boolean mLazy = false;

    /** used in list sticky detect **/
    private int mScrollPosition = -1;


    @Deprecated
    public WXCell(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, WXUIAction action) {
        this(instance,parent,isLazy,action);
    }

    public WXCell(WXSDKInstance instance, WXVContainer parent, boolean isLazy, WXUIAction action) {
        super(instance, parent,true ,action);
    }

    @Override
    public boolean isLazy() {
        return mLazy;
    }

    public void lazy(boolean lazy) {
        mLazy = lazy;
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
            return view;
        } else {
            WXFrameLayout view = new WXFrameLayout(context);
            mRealView = view;
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
        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams((int) getLayoutSize().getWidth(),
                (int) getLayoutSize().getHeight());
        getHostView().addView(mTempStickyView, lp);
        mHeadView.setTranslationX(headerViewOffsetX);
        mHeadView.setTranslationY(headerViewOffsetY);
    }

    public void recoverySticky() {
        getHostView().removeView(mTempStickyView);
        getHostView().addView(mHeadView);
        mHeadView.setTranslationX(0);
        mHeadView.setTranslationY(0);
    }
}
