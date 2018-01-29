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
import android.support.annotation.Nullable;
import android.view.MotionEvent;

import com.taobao.weex.ui.component.list.ListComponentView;
import com.taobao.weex.ui.component.list.StickyHeaderHelper;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.ui.view.listview.adapter.RecyclerViewBaseAdapter;

public class BounceRecyclerView extends BaseBounceView<WXRecyclerView> implements ListComponentView,WXGestureObservable {

  public static final int DEFAULT_COLUMN_COUNT = 1;
  public static final int DEFAULT_COLUMN_GAP = 1;
  private RecyclerViewBaseAdapter adapter = null;
  private WXGesture mGesture;
  private int mLayoutType = WXRecyclerView.TYPE_LINEAR_LAYOUT;
  private int mColumnCount = DEFAULT_COLUMN_COUNT;
  private float mColumnGap = DEFAULT_COLUMN_GAP;
  private StickyHeaderHelper mStickyHeaderHelper;

  public BounceRecyclerView(Context context,int type,int columnCount,float columnGap,int orientation) {
    super(context, orientation);
    mLayoutType = type;
    mColumnCount = columnCount;
    mColumnGap = columnGap;
    init(context);
    mStickyHeaderHelper = new StickyHeaderHelper(this);
  }

  public BounceRecyclerView(Context context,int type,int orientation) {
    this(context,type, DEFAULT_COLUMN_COUNT, DEFAULT_COLUMN_GAP,orientation);
  }

  public void setRecyclerViewBaseAdapter(RecyclerViewBaseAdapter adapter) {
    this.adapter = adapter;
    if (getInnerView() != null) {
      getInnerView().setAdapter(adapter);
    }
  }

  public RecyclerViewBaseAdapter getRecyclerViewBaseAdapter() {
    return adapter;
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent event) {
    boolean result = super.dispatchTouchEvent(event);
    if (mGesture != null) {
      result |= mGesture.onTouch(this, event);
    }
    return result;
  }

  @Override
  public WXRecyclerView setInnerView(Context context) {
    WXRecyclerView wxRecyclerView = new WXRecyclerView(context);
    wxRecyclerView.initView(context, mLayoutType,mColumnCount,mColumnGap,getOrientation());
    return wxRecyclerView;
  }

  @Override
  public void onRefreshingComplete() {
    if (adapter != null) {
      adapter.notifyDataSetChanged();
    }
  }

  @Override
  public void onLoadmoreComplete() {
    if (adapter != null) {
      adapter.notifyDataSetChanged();
    }
  }

  /**
   * @param component
   */
  public void notifyStickyShow(WXCell component) {
    mStickyHeaderHelper.notifyStickyShow(component);
  }

  @Override
  public void updateStickyView(int currentStickyPos) {
    mStickyHeaderHelper.updateStickyView(currentStickyPos);
  }

  /**
   *
   * @param compToRemove
   */
  @Override
  public void notifyStickyRemove(WXCell compToRemove) {
    mStickyHeaderHelper.notifyStickyRemove(compToRemove);
  }

  public StickyHeaderHelper getStickyHeaderHelper() {
    return mStickyHeaderHelper;
  }

  @Override
  public void registerGestureListener(@Nullable WXGesture wxGesture) {
    mGesture = wxGesture;
    getInnerView().registerGestureListener(wxGesture);
  }
}
