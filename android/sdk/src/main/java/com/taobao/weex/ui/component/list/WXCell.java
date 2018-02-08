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
import android.os.Build.VERSION_CODES;
import android.support.annotation.NonNull;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants.Name;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.flat.WidgetContainer;
import com.taobao.weex.ui.view.WXFrameLayout;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

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
    private int mScrollPositon = -1;
    private boolean mFlatUIEnabled = false;


    private Object  renderData;

    private boolean isSourceUsed = false;


    @Deprecated
    public WXCell(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
        super(instance, dom, parent);
        lazy(true);
    }

    public WXCell(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, boolean isLazy) {
        super(instance, dom, parent);
        lazy(true);
        if(Build.VERSION.SDK_INT< VERSION_CODES.LOLLIPOP) {
            try {
                //TODO a WTF is necessary if anyone try to change the flat flag during update attrs.
                WXAttr attr = getDomObject().getAttrs();
                if (attr.containsKey(Name.FLAT)) {
                    mFlatUIEnabled = WXUtils.getBoolean(attr.get(Name.FLAT), false);
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
    @RestrictTo(Scope.LIBRARY)
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

    public void setScrollPositon(int pos){
        mScrollPositon = pos;
    }

    public int getScrollPositon() {
        return mScrollPositon;
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
            FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams((int) getDomObject().getLayoutWidth(),
                    (int) getDomObject().getLayoutHeight());
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
            if(getHostView() != null) {
                getHostView().removeView(mTempStickyView);
                getHostView().addView(mHeadView);
            }
            mHeadView.setTranslationX(0);
            mHeadView.setTranslationY(0);
        }
    }

    @Override
    protected void mountFlatGUI() {
      if(getHostView()!=null) {
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
        if(getDomObject() == null){
            return  0;
        }
        WXDomObject domObject = (WXDomObject) getDomObject();
        if(domObject.getAttrs().get(STICKY_OFFSET) == null){
            return 0;
        }
        float  offset = WXUtils.getFloat(domObject.getAttrs().get(STICKY_OFFSET));
        return (int)(WXViewUtils.getRealPxByWidth(offset,domObject.getViewPortWidth()));
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

}
