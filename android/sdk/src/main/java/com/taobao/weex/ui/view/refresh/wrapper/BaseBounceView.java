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
package com.taobao.weex.ui.view.refresh.wrapper;

import android.content.Context;
import android.graphics.Color;
import android.support.v7.widget.OrientationHelper;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.ImmutableDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.view.WXLoadingLayout;
import com.taobao.weex.ui.view.WXRefreshLayout;
import com.taobao.weex.ui.view.refresh.core.WXRefreshView;
import com.taobao.weex.ui.view.refresh.core.WXSwipeLayout;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;

/**
 * BounceView(SwipeLayout) contains Scroller/List and refresh/loading view
 * @param <T> InnerView
 */
public abstract class BaseBounceView<T extends View> extends FrameLayout {

    private int mOrientation = OrientationHelper.VERTICAL;
    protected WXSwipeLayout swipeLayout;
    private T mInnerView;

    public BaseBounceView(Context context,int orientation) {
        this(context, null,orientation);
    }

    public BaseBounceView(Context context, AttributeSet attrs,int orientation) {
        super(context, attrs);
        mOrientation = orientation;
    }

    public int getOrientation(){
        return mOrientation;
    }

    public void init(Context context) {
        createBounceView(context);
    }

    boolean isVertical(){
        return mOrientation==OrientationHelper.VERTICAL;
    }

    public void setOnRefreshListener(WXSwipeLayout.WXOnRefreshListener onRefreshListener) {
        if (swipeLayout != null)
            swipeLayout.setOnRefreshListener(onRefreshListener);
    }

    public void setOnLoadingListener(WXSwipeLayout.WXOnLoadingListener onLoadingListener) {
        if (swipeLayout != null)
            swipeLayout.setOnLoadingListener(onLoadingListener);
    }

    public void finishPullRefresh() {
        if (swipeLayout != null)
            swipeLayout.finishPullRefresh();
    }

    public void finishPullLoad() {
        if (swipeLayout != null)
            swipeLayout.finishPullLoad();
    }

    /**
     * Init wipelayout
     */
    private WXSwipeLayout createBounceView(Context context) {
        swipeLayout = new WXSwipeLayout(context);
        swipeLayout.setLayoutParams(new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
        mInnerView = setInnerView(context);
        if (mInnerView == null)
            return null;
        swipeLayout.addView(mInnerView, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
        addView(swipeLayout, LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        return swipeLayout;
    }

    /**
     * @return the child of swipelayout : recyclerview or scrollview
     */
    public T getInnerView() {
        return mInnerView;
    }

    public abstract T setInnerView(Context context);

    /**
     *
     * @param refresh should be {@link WXRefreshView}
     */
    public void setHeaderView(WXComponent refresh) {
        setRefreshEnable(true);
        if (swipeLayout != null) {
            WXRefreshView refreshView = swipeLayout.getHeaderView();
            if (refreshView != null) {
                ImmutableDomObject immutableDomObject = refresh.getDomObject();
                if (immutableDomObject != null) {
                    int refreshHeight = (int) immutableDomObject.getLayoutHeight();
                    swipeLayout.setRefreshHeight(refreshHeight);
                    String colorStr = (String) immutableDomObject.getStyles().get(Constants.Name.BACKGROUND_COLOR);
                    String bgColor = WXUtils.getString(colorStr, null);
                    if (bgColor != null) {
                        if (!TextUtils.isEmpty(bgColor)) {
                            int colorInt = WXResourceUtils.getColor(bgColor);
                            if (!(colorInt == Color.TRANSPARENT)) {
                                swipeLayout.setRefreshBgColor(colorInt);
                            }
                        }
                    }
                    refreshView.setRefreshView(refresh.getHostView());
                }
            }
        }
    }

    /**
     *
     * @param loading should be {@link WXRefreshView}
     */
    public void setFooterView(WXComponent loading) {
        setLoadmoreEnable(true);
        if (swipeLayout != null) {
            WXRefreshView refreshView = swipeLayout.getFooterView();
            if (refreshView != null) {
                ImmutableDomObject immutableDomObject = loading.getDomObject();
                if (immutableDomObject != null) {
                    int loadingHeight = (int) immutableDomObject.getLayoutHeight();
                    swipeLayout.setLoadingHeight(loadingHeight);
                    String colorStr = (String) immutableDomObject.getStyles().get(Constants.Name.BACKGROUND_COLOR);
                    String bgColor = WXUtils.getString(colorStr, null);
                    if (bgColor != null) {
                        if (!TextUtils.isEmpty(bgColor)) {
                            int colorInt = WXResourceUtils.getColor(bgColor);
                            if (!(colorInt == Color.TRANSPARENT)) {
                                swipeLayout.setLoadingBgColor(colorInt);
                            }
                        }
                    }
                    refreshView.setRefreshView(loading.getHostView());
                }
            }
        }
    }

    public void removeFooterView(WXComponent loading){
        setLoadmoreEnable(false);
        if(swipeLayout!=null){
            if(swipeLayout.getFooterView()!=null){
                swipeLayout.setLoadingHeight(0);
                swipeLayout.getFooterView().removeView(loading.getHostView());
                swipeLayout.finishPullLoad();
            }
        }
    }
    //TODO There are bugs, will be more than a rolling height
    public void removeHeaderView(WXComponent refresh){
        setRefreshEnable(false);
        if(swipeLayout!=null){
            if(swipeLayout.getHeaderView()!=null){
                swipeLayout.setRefreshHeight(0);
                swipeLayout.getHeaderView().removeView(refresh.getHostView());
                swipeLayout.finishPullRefresh();
            }
        }
    }

    public void setRefreshEnable(boolean enable) {
        if (swipeLayout != null)
            swipeLayout.setPullRefreshEnable(enable);
    }

    public void setLoadmoreEnable(boolean enable) {
        if (swipeLayout != null)
            swipeLayout.setPullLoadEnable(enable);
    }

  @Override
  public void removeView(View view) {
    if (view instanceof WXLoadingLayout) {
      finishPullLoad();
      setLoadmoreEnable(false);
      if (swipeLayout != null) {
        swipeLayout.removeView(swipeLayout.getFooterView());
      }
    } else if (view instanceof WXRefreshLayout) {
      finishPullRefresh();
      setRefreshEnable(false);
      if (swipeLayout != null) {
        swipeLayout.removeView(swipeLayout.getHeaderView());
      }
    } else {
      super.removeView(view);
    }
  }

    public WXSwipeLayout getSwipeLayout() {
        return swipeLayout;
    }

    public abstract void onRefreshingComplete();

    public abstract void onLoadmoreComplete();
}
