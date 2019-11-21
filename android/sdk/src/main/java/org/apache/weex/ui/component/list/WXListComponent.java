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
package org.apache.weex.ui.component.list;

import android.content.Context;
import android.support.v4.util.ArrayMap;
import android.support.v7.widget.PagerSnapHelper;
import android.text.TextUtils;
import com.alibaba.fastjson.JSON;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.common.Constants;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXThread;
import org.apache.weex.ui.ComponentCreator;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.component.WXBaseRefresh;
import org.apache.weex.ui.component.WXBasicComponentType;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXComponentProp;
import org.apache.weex.ui.component.WXLoading;
import org.apache.weex.ui.component.WXRefresh;
import org.apache.weex.ui.component.WXVContainer;
import org.apache.weex.ui.view.listview.WXRecyclerView;
import org.apache.weex.ui.view.listview.adapter.ListBaseViewHolder;
import org.apache.weex.ui.view.refresh.wrapper.BounceRecyclerView;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
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
  private String mSpanOffsetsStr;
  private Float[] mSpanOffsets;
  private boolean hasSetGapItemDecoration = false;

  public static class Creator implements ComponentCreator {
    public WXComponent createInstance(WXSDKInstance instance,
                                      WXVContainer parent,
                                      BasicComponentData basicComponentData)
            throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXListComponent(instance, parent, true, basicComponentData);
    }
  }

  @Deprecated
  public WXListComponent(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }


  public WXListComponent(WXSDKInstance instance, WXVContainer parent, boolean lazy, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
  }

  @Override
  protected BounceRecyclerView generateListView(Context context, int orientation) {
    BounceRecyclerView bounceRecyclerView = new BounceRecyclerView(context, mLayoutType, mColumnCount, mColumnGap, orientation);
    if (bounceRecyclerView.getSwipeLayout() != null) {
      if (WXUtils.getBoolean(getAttrs().get(Constants.Name.NEST_SCROLLING_ENABLED), false)) {
        bounceRecyclerView.getSwipeLayout().setNestedScrollingEnabled(true);
      }
    }

    /**
     *  enable pagingEnabled attr
     */
    if(WXUtils.getBoolean(getAttrs().get(Constants.Name.PAGE_ENABLED),false)){
      PagerSnapHelper snapHelper = null;
      String pageSize = WXUtils.getString(getAttrs().get(Constants.Name.PAGE_SIZE), null);
      if(TextUtils.isEmpty(pageSize)) {
        snapHelper = new PagerSnapHelper();
      } else  {
        snapHelper = new WXPagerSnapHelper();
      }

      snapHelper.attachToRecyclerView(bounceRecyclerView.getInnerView());
    }

    return bounceRecyclerView;
  }

  @Override
  public void addChild(WXComponent child, int index) {
    super.addChild(child, index);
    if (child == null || index < -1) {
      return;
    }
    setRefreshOrLoading(child);
    // Synchronize DomObject's attr to Component and Native View
    if(getHostView() != null && hasColumnPros()) {
      updateRecyclerAttr();
      getHostView().getInnerView().initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  private boolean hasColumnPros() {
    return (getAttrs().containsKey(Constants.Name.COLUMN_WIDTH) && mColumnWidth != WXUtils.parseFloat(getAttrs().get(Constants.Name.COLUMN_WIDTH))) ||
            (getAttrs().containsKey(Constants.Name.COLUMN_COUNT) &&  mColumnCount != WXUtils.parseInt(getAttrs().get(Constants.Name.COLUMN_COUNT))) ||
            (getAttrs().containsKey(Constants.Name.COLUMN_GAP) && mColumnGap != WXUtils.parseFloat(getAttrs().get(Constants.Name.COLUMN_GAP)));
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
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          getHostView().setHeaderView(child);
        }
      }), 100);
      return true;
    }

    if (child instanceof WXLoading) {
      getHostView().setOnLoadingListener((WXLoading) child);
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          getHostView().setFooterView(child);
        }
      }), 100);
      return true;
    }
    return false;
  }

  private void updateRecyclerAttr() {
    mColumnCount = WXUtils.parseInt(getAttrs().get(Constants.Name.COLUMN_COUNT));
    if (mColumnCount <= 0 && mLayoutType != WXRecyclerView.TYPE_LINEAR_LAYOUT) {
      Map<String, String> ext = new ArrayMap<>();
      ext.put("componentType", getComponentType());
      ext.put("attribute", getAttrs().toString());
      ext.put("stackTrace", Arrays.toString(Thread.currentThread().getStackTrace()));
      WXExceptionUtils.commitCriticalExceptionRT(getInstanceId(),
          WXErrorCode.WX_RENDER_ERR_LIST_INVALID_COLUMN_COUNT, "columnCount",
          String.format(Locale.ENGLISH,
              "You are trying to set the list/recycler/vlist/waterfall's column to %d, which is illegal. The column count should be a positive integer",
              mColumnCount),
          ext);
      mColumnCount = Constants.Value.COLUMN_COUNT_NORMAL;
    }
    mColumnGap = WXUtils.parseFloat(getAttrs().get(Constants.Name.COLUMN_GAP));
    mColumnWidth = WXUtils.parseFloat(getAttrs().get(Constants.Name.COLUMN_WIDTH));
    mPaddingLeft = WXUtils.parseFloat(getAttrs().get(Constants.Name.PADDING_LEFT));
    mPaddingRight = WXUtils.parseFloat(getAttrs().get(Constants.Name.PADDING_RIGHT));
    mSpanOffsetsStr = (String)getAttrs().get(Constants.Name.SPAN_OFFSETS);

    try {
      if (!TextUtils.isEmpty(mSpanOffsetsStr)) {
        List<Float> list = JSON.parseArray(mSpanOffsetsStr, Float.class);
        final int size = list.size();
        if (null == mSpanOffsets || mSpanOffsets.length != size) {
          mSpanOffsets = new Float[size];
        }
        list.toArray(mSpanOffsets);
      } else {
        mSpanOffsets = null;
      }
    } catch (Throwable e) {
      WXLogUtils.w("Parser SpanOffsets error ", e);
    }

    if (!hasSetGapItemDecoration && null != getSpanOffsets() && null != getHostView()
        && null != getHostView().getInnerView()) {
      hasSetGapItemDecoration = true;
      getHostView().getInnerView().addItemDecoration(new GapItemDecoration(this));
    }
  }

  @WXComponentProp(name = Constants.Name.SPAN_OFFSETS)
  public void setSpanOffsets(String spanOffsets)  {
    if (!TextUtils.equals(spanOffsets, mSpanOffsetsStr)) {
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType, mColumnCount, mColumnGap, getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_WIDTH)
  public void setColumnWidth(float columnWidth)  {
    if(columnWidth != mColumnWidth){
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_COUNT)
  public void setColumnCount(int columnCount){
    if(columnCount != mColumnCount) {
      markComponentUsable();
      updateRecyclerAttr();
      WXRecyclerView wxRecyclerView = getHostView().getInnerView();
      wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
    }
  }

  @WXComponentProp(name = Constants.Name.COLUMN_GAP)
  public void setColumnGap(float columnGap) throws InterruptedException {
    if(columnGap != mColumnGap) {
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
    if (isRecycler(this)) {
      if(WXBasicComponentType.WATERFALL.equals(getComponentType())){
        mLayoutType = WXRecyclerView.TYPE_STAGGERED_GRID_LAYOUT;
      }else{
        mLayoutType = getAttrs().getLayoutType();
      }
    }

    if(props.containsKey(Constants.Name.PADDING)
            ||props.containsKey(Constants.Name.PADDING_LEFT)
            || props.containsKey(Constants.Name.PADDING_RIGHT)){
      if(mPaddingLeft != WXUtils.parseFloat(props.get(Constants.Name.PADDING_LEFT)) || mPaddingRight != WXUtils.parseFloat(props.get(Constants.Name.PADDING_RIGHT))) {
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

  private boolean isRecycler(WXComponent component) {
    return WXBasicComponentType.WATERFALL.equals(component.getComponentType())
            || WXBasicComponentType.RECYCLE_LIST.equals(component.getComponentType())
            || WXBasicComponentType.RECYCLER.equals(component.getComponentType());
  }

  public Float[] getSpanOffsets() {
    return mSpanOffsets;
  }
}
