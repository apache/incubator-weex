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

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.Interpolator;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.reflect.Field;

/**
 */
@SuppressLint("HandlerLeak")
public class WXCircleViewPager extends ViewPager implements WXGestureObservable {

  private final int SCROLL_TO_NEXT = 1;
  private WXGesture wxGesture;
  private boolean isAutoScroll;
  private long intervalTime = 3 * 1000;
  private WXSmoothScroller mScroller;
  private boolean needLoop = true;
  private boolean scrollable = true;
  private int mState = ViewPager.SCROLL_STATE_IDLE;
  private Handler mAutoScrollHandler = new Handler(Looper.getMainLooper()) {
    @Override
    public void handleMessage(Message msg) {
      if (msg.what == SCROLL_TO_NEXT) {
        WXLogUtils.d("[CircleViewPager] trigger auto play action");
        showNextItem();
        this.sendEmptyMessageDelayed(SCROLL_TO_NEXT, intervalTime);
        return;
      }
      super.handleMessage(msg);
    }
  };

  @SuppressLint("NewApi")
  public WXCircleViewPager(Context context) {
    super(context);
    init();
  }

  private void init() {
    setOverScrollMode(View.OVER_SCROLL_NEVER);

    addOnPageChangeListener(new OnPageChangeListener() {
      @Override
      public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

      }

      @Override
      public void onPageSelected(int position) {

      }

      @Override
      public void onPageScrollStateChanged(int state) {
        mState = state;
        WXCirclePageAdapter adapter = getCirclePageAdapter();
        int currentItemInternal = WXCircleViewPager.super.getCurrentItem();
        if (needLoop && state == ViewPager.SCROLL_STATE_IDLE && adapter.getCount() > 1) {
          if (currentItemInternal == adapter.getCount() - 1) {
            superSetCurrentItem(1, false);
          } else if (currentItemInternal == 0) {
            superSetCurrentItem(adapter.getCount() - 2, false);
          }
        }
      }
    });

    postInitViewPager();
  }

  /**
   * Override the Scroller instance with our own class so we can change the
   * duration
   */
  private void postInitViewPager() {
    if (isInEditMode()) {
      return;
    }
    try {
      Field scroller = ViewPager.class.getDeclaredField("mScroller");
      scroller.setAccessible(true);
      Field interpolator = ViewPager.class
          .getDeclaredField("sInterpolator");
      interpolator.setAccessible(true);

      mScroller = new WXSmoothScroller(getContext(),
          (Interpolator) interpolator.get(null));
      scroller.set(this, mScroller);
    } catch (Exception e) {
      WXLogUtils.e("[CircleViewPager] postInitViewPager: ", e);
    }
  }

  @SuppressLint("NewApi")
  public WXCircleViewPager(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  @Override
  public int getCurrentItem() {
    return getRealCurrentItem();
  }

  public int superGetCurrentItem() {
    return super.getCurrentItem();
  }

  @Override
  public boolean onTouchEvent(MotionEvent ev) {
    if(!scrollable) {
      return true;
    }
    return super.onTouchEvent(ev);
  }

  @Override
  public void scrollTo(int x, int y) {
    if(scrollable || mState != ViewPager.SCROLL_STATE_DRAGGING) {
      super.scrollTo(x, y);
    }
  }

  /**
   * Start auto scroll. Must be called after {@link #setAdapter(PagerAdapter)}
   */
  public void startAutoScroll() {
    isAutoScroll = true;
    mAutoScrollHandler.removeCallbacksAndMessages(null);
    mAutoScrollHandler.sendEmptyMessageDelayed(SCROLL_TO_NEXT, intervalTime);
  }

  public void pauseAutoScroll(){
    mAutoScrollHandler.removeCallbacksAndMessages(null);
  }

  /**
   * Stop auto scroll.
   */
  public void stopAutoScroll() {
    isAutoScroll = false;
    mAutoScrollHandler.removeCallbacksAndMessages(null);
  }

  public boolean isAutoScroll() {
    return isAutoScroll;
  }

  @Override
  public void setCurrentItem(int item) {
    setRealCurrentItem(item);
  }

  /**
   * @return the circlePageAdapter
   */
  public WXCirclePageAdapter getCirclePageAdapter() {
    return (WXCirclePageAdapter) getAdapter();
  }

  /**
   * @param circlePageAdapter the circlePageAdapter to set
   */
  public void setCirclePageAdapter(WXCirclePageAdapter circlePageAdapter) {
    this.setAdapter(circlePageAdapter);
  }

  /**
   * Get auto scroll interval. The time unit is micro second.
   * The default time interval is 3000 micro second
   * @return the intervalTime
   */
  public long getIntervalTime() {
    return intervalTime;
  }

  /**
   * Set auto scroll interval. The time unit is micro second.
   * The default time interval is 3000 micro second
   * @param intervalTime the intervalTime to set
   */
  public void setIntervalTime(long intervalTime) {
    this.intervalTime = intervalTime;
  }

  public void setCircle(boolean circle) {
    needLoop = circle;
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent ev) {
    switch (ev.getAction()) {
      case MotionEvent.ACTION_DOWN:
      case MotionEvent.ACTION_MOVE:
        mAutoScrollHandler.removeCallbacksAndMessages(null);
        break;
      case MotionEvent.ACTION_UP:
      case MotionEvent.ACTION_CANCEL:
        if (isAutoScroll()) {
          mAutoScrollHandler.sendEmptyMessageDelayed(SCROLL_TO_NEXT, intervalTime);
        }
        break;
    }
    boolean result = super.dispatchTouchEvent(ev);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, ev);
    }
    return result;
  }

  public void destory() {
    mAutoScrollHandler.removeCallbacksAndMessages(null);
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  public int getRealCurrentItem() {
    int i = super.getCurrentItem();
    return ((WXCirclePageAdapter) getAdapter()).getRealPosition(i);
  }

  private void setRealCurrentItem(int item) {
    superSetCurrentItem(((WXCirclePageAdapter) getAdapter()).getFirst() + item, false);
  }

  private void superSetCurrentItem(int item, boolean smooth) {
    try {
      super.setCurrentItem(item, smooth);
    } catch (IllegalStateException e) {
      WXLogUtils.e(e.toString());
      if (getAdapter() != null) {
        getAdapter().notifyDataSetChanged();
        super.setCurrentItem(item, smooth);
      }
    }
  }

  public int getRealCount() {
    return ((WXCirclePageAdapter) getAdapter()).getRealCount();
  }

  @Override
  protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    try {
      super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    } catch (IllegalStateException e) {
      WXLogUtils.e(e.toString());
      if (getAdapter() != null) {
        getAdapter().notifyDataSetChanged();
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
      }
    }
  }

  public boolean isScrollable() {
    return scrollable;
  }

  public void setScrollable(boolean scrollable) {
    this.scrollable = scrollable;
  }

  private void showNextItem() {
    if (!needLoop && superGetCurrentItem() == getRealCount() - 1) {
      return;
    }
    if (getRealCount() == 2 && superGetCurrentItem() == 1) {
      superSetCurrentItem(0, true);
    } else {
      superSetCurrentItem(superGetCurrentItem() + 1, true);
    }
  }
}
