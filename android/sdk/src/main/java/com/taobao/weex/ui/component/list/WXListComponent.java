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
import android.util.Log;
import android.util.Pair;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.component.WXBaseRefresh;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXLoading;
import com.taobao.weex.ui.component.WXRefresh;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.ui.view.listview.adapter.ListBaseViewHolder;
import com.taobao.weex.ui.view.refresh.wrapper.BounceRecyclerView;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;

import java.util.Map;

/**
 * Unlike other components, there is immutable bi-directional association between View and
 * ViewHolder, while only mutable and temporal uni-directional association between view and
 * components. The association only exist from {@link #onBindViewHolder(ListBaseViewHolder, int)} to
 * {@link #onViewRecycled(ListBaseViewHolder)}. In other situations, the association may not valid
 * or not even exist.
 */
@Component(lazyload = false)

public class WXListComponent extends BasicListComponent<BounceRecyclerView> {
  private String TAG = "WXListComponent";
//  private WXRecyclerDomObject mDomObject;
  private float mPaddingLeft;
  private float mPaddingRight;

  @Deprecated
  public WXListComponent(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }


  public WXListComponent(WXSDKInstance instance, WXVContainer parent, boolean lazy, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    // TODO
//    if (node != null && node instanceof WXRecyclerDomObject) {
//      mRecyclerDom = (WXRecyclerDomObject) node;
//      mRecyclerDom.preCalculateCellWidth();
//
//      if(WXBasicComponentType.WATERFALL.equals(node.getType())){
//        mLayoutType = WXRecyclerView.TYPE_STAGGERED_GRID_LAYOUT;
//      }else{
//        mLayoutType = getAttrs().getLayoutType();
//      }
//      updateRecyclerAttr();
//
//    }
  }

  @Override
  protected BounceRecyclerView generateListView(Context context, int orientation) {
    BounceRecyclerView bounceRecyclerView = new BounceRecyclerView(context,mLayoutType,mColumnCount,mColumnGap,orientation);
    if(bounceRecyclerView.getSwipeLayout()  != null){
      if(WXUtils.getBoolean(getAttrs().get(Constants.Name.NEST_SCROLLING_ENABLED), false)) {
        bounceRecyclerView.getSwipeLayout().setNestedScrollingEnabled(true);
      }
    }
    // TODO
//    if(mRecyclerDom != null && mRecyclerDom.getSpanOffsets() != null){
//      bounceRecyclerView.getInnerView().addItemDecoration(new GapItemDecoration(this));
//    }
    return  bounceRecyclerView;
  }

  @Override
  public void addChild(final WXComponent child, int index) {
    super.addChild(child, index);
    if (child == null || index < -1) {
      return;
    }

    if (child instanceof WXRefresh && getHostView() != null) {
      getHostView().setOnRefreshListener((WXRefresh) child);
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          getHostView().setHeaderView(child);
        }
      }), 100);
    }

    if (child instanceof WXLoading && getHostView() != null) {
      getHostView().setOnLoadingListener((WXLoading) child);
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          getHostView().setFooterView(child);
        }
      }), 100);
    }

    // Synchronize DomObject's attr to Component and Native View
    if(getHostView() != null && (mColumnWidth != getAttrs().getColumnWidth() ||
            mColumnCount != getAttrs().getColumnCount() ||
            mColumnGap != getAttrs().getColumnGap())) {
      updateRecyclerAttr();
      getHostView().getInnerView().initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  private void updateRecyclerAttr(){
    if(getAttrs() != null) {
      mColumnCount = getAttrs().getColumnCount();
      mColumnGap = getAttrs().getColumnGap();
      mColumnWidth = getAttrs().getColumnWidth();
      mPaddingLeft = getPadding().get(CSSShorthand.EDGE.LEFT);
      mPaddingRight = getPadding().get(CSSShorthand.EDGE.RIGHT);
      // TODO
//      mLeftGap = mRecyclerDom.getLeftGap();
//      mRightGap = mRecyclerDom.getRightGap();
//      mRecyclerDom.preCalculateCellWidth();
    }
  }

  // TODO
//  @WXComponentProp(name = Constants.Name.LEFT_GAP)
//  public void setLeftGap(float leftGap)  {
//    if(mRecyclerDom != null && mRecyclerDom.getLeftGap() != mLeftGap){
//      markComponentUsable();
//      mRecyclerDom.preCalculateCellWidth();
//      updateRecyclerAttr();
//      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
//      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
//    }
//  }
//
//  @WXComponentProp(name = Constants.Name.RIGHT_GAP)
//  public void setRightGap(float rightGap)  {
//    if(mRecyclerDom != null && mRecyclerDom.getRightGap() != mRightGap){
//      markComponentUsable();
//      mRecyclerDom.preCalculateCellWidth();
//      updateRecyclerAttr();
//      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
//      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
//    }
//  }

  @WXComponentProp(name = Constants.Name.COLUMN_WIDTH)
  public void setColumnWidth(int columnCount)  {
    if(getAttrs().getColumnWidth() != mColumnWidth){
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_COUNT)
  public void setColumnCount(int columnCount){
    if(getAttrs().getColumnCount() != mColumnCount){
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_GAP)
  public void setColumnGap(float columnGap) throws InterruptedException {
    if(getAttrs().getColumnGap() != mColumnGap) {
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType, mColumnCount, mColumnGap, getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.SCROLLABLE)
  public void setScrollable(boolean scrollable) {
    WXRecyclerView inner = getHostView().getInnerView();
    inner.setScrollable(scrollable);
  }

  @Override
  public void updateProperties(Map<String, Object> props) {
    super.updateProperties(props);
    if(props.containsKey(Constants.Name.PADDING)
            ||props.containsKey(Constants.Name.PADDING_LEFT)
            || props.containsKey(Constants.Name.PADDING_RIGHT)){

      if(getPadding() != null && (mPaddingLeft != getPadding().get(CSSShorthand.EDGE.LEFT)
              || mPaddingRight != getPadding().get(CSSShorthand.EDGE.RIGHT))) {

        markComponentUsable();
        updateRecyclerAttr();
        WXRecyclerView wxRecyclerView = getHostView().getInnerView();
        wxRecyclerView.initView(getContext(), mLayoutType, mColumnCount, mColumnGap, getOrientation());
      }
    }

  }

  @Override
  public void createChildViewAt(int index) {
    Pair<WXComponent, Integer> ret = rearrangeIndexAndGetChild(index);
    if(ret.first != null) {
      final WXComponent child = getChild(ret.second);
      if (child instanceof WXBaseRefresh) {
        child.createView();
        if (child instanceof WXRefresh) {
          getHostView().setOnRefreshListener((WXRefresh) child);
          getHostView().postDelayed(WXThread.secure(new Runnable() {
            @Override
            public void run() {
              getHostView().setHeaderView(child);
            }
          }), 100);
        } else if (child instanceof WXLoading) {
          getHostView().setOnLoadingListener((WXLoading) child);
          getHostView().postDelayed(WXThread.secure(new Runnable() {
            @Override
            public void run() {
              getHostView().setFooterView(child);
            }
          }), 100);
        }
      } else {
        super.createChildViewAt(ret.second);
      }
    }
  }

  // TODO
//  public WXRecyclerDomObject getRecyclerDom() {
//    return mRecyclerDom;
//  }

  public void remove(WXComponent child, boolean destroy) {
    super.remove(child, destroy);
    if (child instanceof WXLoading) {
      getHostView().removeFooterView(child);
    } else if (child instanceof WXRefresh) {
      getHostView().removeHeaderView(child);
    }
  }
}
