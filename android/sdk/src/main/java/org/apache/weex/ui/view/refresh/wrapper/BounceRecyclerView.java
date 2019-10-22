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

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.common.Constants;
import com.taobao.weex.ui.component.list.ListComponentView;
import com.taobao.weex.ui.component.list.StickyHeaderHelper;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.ui.view.listview.adapter.RecyclerViewBaseAdapter;

import org.xmlpull.v1.XmlPullParser;

import java.lang.reflect.Method;

import io.dcloud.weex.WeexReID;

public class BounceRecyclerView extends BaseBounceView<WXRecyclerView> implements ListComponentView,WXGestureObservable {

  public static final int DEFAULT_COLUMN_COUNT = 1;
  public static final int DEFAULT_COLUMN_GAP = 1;
  private RecyclerViewBaseAdapter adapter = null;
//  private WXGesture mGesture;
  private int mLayoutType = WXRecyclerView.TYPE_LINEAR_LAYOUT;
  private int mColumnCount = DEFAULT_COLUMN_COUNT;
  private float mColumnGap = DEFAULT_COLUMN_GAP;
  private StickyHeaderHelper mStickyHeaderHelper;
  //DCloud modify
  private int mOrientation = 0;

  public BounceRecyclerView(Context context,int type,int columnCount,float columnGap,int orientation) {
    super(context, orientation);
    mLayoutType = type;
    mColumnCount = columnCount;
    mColumnGap = columnGap;
    mOrientation = orientation;
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
//    if (mGesture != null) {
//      result |= mGesture.onTouch(this, event);
//    }
    return result;
  }

  @Override
  public WXRecyclerView setInnerView(Context context) {
    //DCloud modify 让WXRecyclerView 显示滚动条
    int layoutId = WeexReID.getId(context, "layout", "weex_recycler_layout");
    WXRecyclerView wxRecyclerView = null;
    if(layoutId != 0) {
      @SuppressLint("ResourceType")
      XmlPullParser parser = getResources().getXml(layoutId);
      AttributeSet attributes = Xml.asAttributeSet(parser);
      int type;
      try{
        while ((type = parser.next()) != XmlPullParser.START_TAG &&
                type != XmlPullParser.END_DOCUMENT) {
          // Empty
        }
        if (type != XmlPullParser.START_TAG) {
          //Log.e("","the xml file is error!\n");
        }
      } catch (Exception e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
      }
      wxRecyclerView = new WXRecyclerView(context, attributes);
      wxRecyclerView.setScrollBarStyle(View.SCROLLBARS_OUTSIDE_OVERLAY);
      try {
        @SuppressLint("PrivateApi")
        Method method = View.class.getDeclaredMethod("initializeScrollbars", TypedArray.class);
        method.setAccessible(true);
        method.invoke(wxRecyclerView, (Object) null);
      } catch (Exception e){
        e.printStackTrace();
      }
      if(mOrientation == Constants.Orientation.VERTICAL) {
        wxRecyclerView.setVerticalScrollBarEnabled(true);
      } else {
        wxRecyclerView.setHorizontalScrollBarEnabled(true);
      }
    } else {
      wxRecyclerView = new WXRecyclerView(context);
    }
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
//    mGesture = wxGesture;
    getInnerView().registerGestureListener(wxGesture);
  }

  @Override
  public WXGesture getGestureListener() {
    return getInnerView().getGestureListener();
  }
}

