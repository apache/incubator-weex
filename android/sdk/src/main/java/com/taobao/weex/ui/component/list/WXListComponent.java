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

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXRecyclerDomObject;
import com.taobao.weex.dom.flex.Spacing;
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
  private WXRecyclerDomObject mDomObject;
  private float mPaddingLeft;
  private float mPaddingRight;

  @Deprecated
  public WXListComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance, dom, parent, isLazy);
  }


  public WXListComponent(WXSDKInstance instance, WXDomObject node, WXVContainer parent, boolean lazy) {
    super(instance, node, parent);
    if (node != null && node instanceof WXRecyclerDomObject) {
      mDomObject = (WXRecyclerDomObject) node;
      mDomObject.preCalculateCellWidth();

      if(WXBasicComponentType.WATERFALL.equals(node.getType())){
        mLayoutType = WXRecyclerView.TYPE_STAGGERED_GRID_LAYOUT;
      }else{
        mLayoutType = mDomObject.getLayoutType();
      }
      updateRecyclerAttr();

    }
  }

  @Override
  protected BounceRecyclerView generateListView(Context context, int orientation) {

    return new BounceRecyclerView(context,mLayoutType,mColumnCount,mColumnGap,orientation);
  }

  @Override
  public void addChild(WXComponent child, int index) {
    super.addChild(child, index);
    if (child == null || index < -1) {
      return;
    }
    setRefreshOrLoading(child);

    // Synchronize DomObject's attr to Component and Native View
    if(mDomObject != null && getHostView() != null && (mColumnWidth != mDomObject.getColumnWidth() ||
            mColumnCount != mDomObject.getColumnCount() ||
            mColumnGap != mDomObject.getColumnGap())) {
      updateRecyclerAttr();
      getHostView().getInnerView().initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  /**
   * Setting refresh view and loading view
   *
   * @param child the refresh_view or loading_view
   */
  private boolean setRefreshOrLoading(final WXComponent child) {

    if (getHostView() == null) {
      WXLogUtils.e(TAG, "setRefreshOrLoading: HostView == null !!!!!! check list attr has append =tree");
      return true;
    }
    if (child instanceof WXRefresh) {
      getHostView().setOnRefreshListener((WXRefresh) child);
      getHostView().postDelayed(new Runnable() {
        @Override
        public void run() {
          getHostView().setHeaderView(child);
        }
      }, 100);
      return true;
    }

    if (child instanceof WXLoading) {
      getHostView().setOnLoadingListener((WXLoading) child);
      getHostView().postDelayed(new Runnable() {
        @Override
        public void run() {
          getHostView().setFooterView(child);
        }
      }, 100);
      return true;
    }
    return false;
  }

  private void updateRecyclerAttr(){
    mColumnCount = mDomObject.getColumnCount();
    mColumnGap = mDomObject.getColumnGap();
    mColumnWidth = mDomObject.getColumnWidth();
    mPaddingLeft =mDomObject.getPadding().get(Spacing.LEFT);
    mPaddingRight =mDomObject.getPadding().get(Spacing.RIGHT);
  }

  @WXComponentProp(name = Constants.Name.COLUMN_WIDTH)
  public void setColumnWidth(int columnCount)  {
    if(mDomObject.getColumnWidth() != mColumnWidth){
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_COUNT)
  public void setColumnCount(int columnCount){
    if(mDomObject.getColumnCount() != mColumnCount){
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_GAP)
  public void setColumnGap(float columnGap) throws InterruptedException {
    if(mDomObject.getColumnGap() != mColumnGap) {
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

      if(mPaddingLeft !=mDomObject.getPadding().get(Spacing.LEFT)
              || mPaddingRight !=mDomObject.getPadding().get(Spacing.RIGHT)) {

        markComponentUsable();
        updateRecyclerAttr();
        WXRecyclerView wxRecyclerView = getHostView().getInnerView();
        wxRecyclerView.initView(getContext(), mLayoutType, mColumnCount, mColumnGap, getOrientation());
      }
    }

  }

  @Override
  public void createChildViewAt(int index) {
    int indexToCreate = index;
    if (indexToCreate < 0) {
      indexToCreate = childCount() - 1;
      if (indexToCreate < 0) {
        return;
      }
    }
    final WXComponent child = getChild(indexToCreate);
    if (child instanceof WXBaseRefresh) {
      child.createView();
      if (child instanceof WXRefresh) {
        getHostView().setOnRefreshListener((WXRefresh) child);
        getHostView().postDelayed(new Runnable() {
          @Override
          public void run() {
            getHostView().setHeaderView(child);
          }
        }, 100);
      } else if (child instanceof WXLoading) {
        getHostView().setOnLoadingListener((WXLoading) child);
        getHostView().postDelayed(new Runnable() {
          @Override
          public void run() {
            getHostView().setFooterView(child);
          }
        }, 100);
      }
    } else {
      super.createChildViewAt(indexToCreate);
    }
  }

  public void remove(WXComponent child, boolean destroy) {
    super.remove(child, destroy);
    removeFooterOrHeader(child);
  }

  private void removeFooterOrHeader(WXComponent child) {
    if (child instanceof WXLoading) {
      getHostView().removeFooterView(child);
    } else if (child instanceof WXRefresh) {
      getHostView().removeHeaderView(child);
    }
  }
}
