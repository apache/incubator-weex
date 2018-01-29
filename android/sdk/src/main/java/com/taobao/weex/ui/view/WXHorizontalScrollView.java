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
package com.taobao.weex.ui.view;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.HorizontalScrollView;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;

import java.util.ArrayList;
import java.util.List;

public class WXHorizontalScrollView extends HorizontalScrollView implements IWXScroller, WXGestureObservable {

  private WXGesture wxGesture;
  private ScrollViewListener mScrollViewListener;
  private List<ScrollViewListener> mScrollViewListeners;
  private boolean scrollable = true;

  public WXHorizontalScrollView(Context context) {
    super(context);
    init();
  }

  private void init() {
    setWillNotDraw(false);
    setOverScrollMode(View.OVER_SCROLL_NEVER);
  }

  public WXHorizontalScrollView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  @Override
  protected void onScrollChanged(int l, int t, int oldl, int oldt) {
    super.onScrollChanged(l, t, oldl, oldt);
    if (mScrollViewListener != null) {
      mScrollViewListener.onScrollChanged(this, l, t, oldl, oldt);
    }
    if (mScrollViewListeners != null) {
      for (ScrollViewListener listener : mScrollViewListeners) {
        listener.onScrollChanged(this, l, t, oldl, oldt);
      }
    }
  }

  public void setScrollViewListener(ScrollViewListener scrollViewListener) {
    this.mScrollViewListener = scrollViewListener;
  }

  @Override
  public void destroy() {

  }

  public void addScrollViewListener(ScrollViewListener scrollViewListener) {
    if (mScrollViewListeners == null) {
      mScrollViewListeners = new ArrayList<>();
    }
    if (!mScrollViewListeners.contains(scrollViewListener)) {
      mScrollViewListeners.add(scrollViewListener);
    }
  }

  public void removeScrollViewListener(ScrollViewListener scrollViewListener) {
    mScrollViewListeners.remove(scrollViewListener);
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent event) {
    boolean result = super.dispatchTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
  }

  @Override
  public boolean onTouchEvent(MotionEvent ev) {
    if(!scrollable) {
      return true; // when scrollable is set to false, then eat the touch event
    }
    return super.onTouchEvent(ev);
  }

  public interface ScrollViewListener {

    void onScrollChanged(WXHorizontalScrollView scrollView, int x, int y, int oldx, int oldy);
  }

  public boolean isScrollable() {
    return scrollable;
  }

  public void setScrollable(boolean scrollable) {
    this.scrollable = scrollable;
  }

  public Rect getContentFrame() {
    return new Rect(0, 0, computeHorizontalScrollRange(), computeVerticalScrollRange());
  }
}
