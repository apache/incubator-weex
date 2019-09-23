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
package org.apache.weex.ui.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;
import android.support.v4.view.NestedScrollingChild;
import android.support.v4.view.NestedScrollingChildHelper;
import android.support.v4.view.ViewCompat;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ScrollView;

import org.apache.weex.common.WXThread;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXScroller;
import org.apache.weex.ui.view.gesture.WXGesture;
import org.apache.weex.ui.view.gesture.WXGestureObservable;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXReflectionUtils;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;


/**
 * Custom-defined scrollView
 */
public class WXScrollView extends ScrollView implements Callback, IWXScroller,
        WXGestureObservable,NestedScrollingChild {

  private NestedScrollingChildHelper childHelper;
  private float ox;
  private float oy;
  private int[] consumed = new int[2];
  private int[] offsetInWindow = new int[2];

  int mScrollX;
  int mScrollY;
  private WXGesture wxGesture;
  private List<WXScrollViewListener> mScrollViewListeners;
  private WXScroller mWAScroller;
  //sticky
  private View mCurrentStickyView;
  private boolean mRedirectTouchToStickyView;
  private int mStickyOffset;
  private boolean mHasNotDoneActionDown = true;
  @SuppressLint("HandlerLeak")
  private Handler mScrollerTask;
  private int mInitialPosition;
  private int mCheckTime = 100;
  /**
   * The location of mCurrentStickyView
   */
  private int[] mStickyP = new int[2];
  /**
   * Location of the scrollView
   */
  private Rect mScrollRect;
  private int[] stickyScrollerP = new int[2];
  private int[] stickyViewP = new int[2];
  private boolean scrollable = true;

  public WXScrollView(Context context) {
    super(context);
    mScrollViewListeners = new ArrayList<>();
    init();
    try {
      WXReflectionUtils.setValue(this, "mMinimumVelocity", 5);
    } catch (Exception e) {
      WXLogUtils.e("[WXScrollView] WXScrollView: ", e);
    }
  }

  private void init() {
    setWillNotDraw(false);
    startScrollerTask();
    setOverScrollMode(View.OVER_SCROLL_NEVER);
    childHelper = new NestedScrollingChildHelper(this);
    childHelper.setNestedScrollingEnabled(true);
  }

  public void startScrollerTask() {
    if (mScrollerTask == null) {
      mScrollerTask = new Handler(WXThread.secure(this));
    }
    mInitialPosition = getScrollY();
    mScrollerTask.sendEmptyMessageDelayed(0, mCheckTime);
  }

  public WXScrollView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  public WXScrollView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
    setOverScrollMode(View.OVER_SCROLL_NEVER);
  }

  /**
   * Add listener for scrollView.
   */
  public void addScrollViewListener(WXScrollViewListener scrollViewListener) {
    if (!mScrollViewListeners.contains(scrollViewListener)) {
      mScrollViewListeners.add(scrollViewListener);
    }
  }

  public void removeScrollViewListener(WXScrollViewListener scrollViewListener) {
    mScrollViewListeners.remove(scrollViewListener);
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent ev) {
    if (ev.getAction() == MotionEvent.ACTION_DOWN) {
      mRedirectTouchToStickyView = true;
    }

    if (mRedirectTouchToStickyView) {
      mRedirectTouchToStickyView = mCurrentStickyView != null;

      if (mRedirectTouchToStickyView) {
        mRedirectTouchToStickyView = ev.getY() <= mCurrentStickyView.getHeight()
                && ev.getX() >= mCurrentStickyView.getLeft()
                && ev.getX() <= mCurrentStickyView.getRight();
      }
    }

    if (mRedirectTouchToStickyView) {
      if (mScrollRect == null) {
        mScrollRect = new Rect();
        getGlobalVisibleRect(mScrollRect);
      }
      mCurrentStickyView.getLocationOnScreen(stickyViewP);
      ev.offsetLocation(0, stickyViewP[1] - mScrollRect.top);
    }
    boolean result = super.dispatchTouchEvent(ev);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, ev);
    }
    return result;
  }

  @Override
  protected void dispatchDraw(Canvas canvas) {
    super.dispatchDraw(canvas);
    if (mCurrentStickyView != null) {
      canvas.save();
      mCurrentStickyView.getLocationOnScreen(mStickyP);
      int realOffset = (mStickyOffset <= 0 ? mStickyOffset : 0);
      canvas.translate(mStickyP[0], getScrollY() + realOffset);
      canvas.clipRect(0, realOffset, mCurrentStickyView.getWidth(),
              mCurrentStickyView.getHeight());
      mCurrentStickyView.draw(canvas);
      canvas.restore();
    }
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  public boolean onTouchEvent(MotionEvent ev) {
    if(!scrollable) {
      return true; // when scrollable is set to false, then eat the touch event
    }
    if (mRedirectTouchToStickyView) {

      if (mScrollRect == null) {
        mScrollRect = new Rect();
        getGlobalVisibleRect(mScrollRect);
      }
      mCurrentStickyView.getLocationOnScreen(stickyViewP);
      ev.offsetLocation(0, -(stickyViewP[1] - mScrollRect.top));
    }

    if (ev.getAction() == MotionEvent.ACTION_DOWN) {
      mHasNotDoneActionDown = false;
    }

    if (mHasNotDoneActionDown) {
      MotionEvent down = MotionEvent.obtain(ev);
      down.setAction(MotionEvent.ACTION_DOWN);
      mHasNotDoneActionDown = false;
      down.recycle();
    }

    if (ev.getAction() == MotionEvent.ACTION_DOWN) {
      ox = ev.getX();
      oy = ev.getY();
      // Dispatch touch event to parent view
      startNestedScroll(ViewCompat.SCROLL_AXIS_HORIZONTAL | ViewCompat.SCROLL_AXIS_VERTICAL);
    }

    if (ev.getAction() == MotionEvent.ACTION_UP || ev.getAction() == MotionEvent.ACTION_CANCEL) {
      mHasNotDoneActionDown = true;
      // stop nested scrolling dispatch
      stopNestedScroll();
    }

    if (ev.getAction() == MotionEvent.ACTION_MOVE) {
      float clampedX = ev.getX();
      float clampedY = ev.getY();
      int dx = (int) (ox - clampedX);
      int dy = (int) (oy - clampedY);

      if (dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow)) {
        // sub dx/dy was consumed by parent view!!!
        ev.setLocation(clampedX+consumed[0],clampedY+consumed[1]);
      }
      ox = ev.getX();
      oy = ev.getY();
    }

    return super.onTouchEvent(ev);
  }

  @Override
  public void setNestedScrollingEnabled(boolean enabled) {
    childHelper.setNestedScrollingEnabled(enabled);
  }

  @Override
  public boolean isNestedScrollingEnabled() {
    return childHelper.isNestedScrollingEnabled();
  }

  @Override
  public boolean startNestedScroll(int axes) {
    return childHelper.startNestedScroll(axes);
  }

  @Override
  public void stopNestedScroll() {
    childHelper.stopNestedScroll();
  }

  @Override
  public boolean hasNestedScrollingParent() {
    return childHelper.hasNestedScrollingParent();
  }

  public boolean isScrollable() {
    return scrollable;
  }

  public void setScrollable(boolean scrollable) {
    this.scrollable = scrollable;
  }

  @Override
  public boolean dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int[] offsetInWindow) {
    return childHelper.dispatchNestedScroll(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed, offsetInWindow);
  }

  @Override
  public boolean dispatchNestedPreScroll(int dx, int dy, int[] consumed, int[] offsetInWindow) {
    return childHelper.dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow);
  }

  @Override
  public boolean dispatchNestedFling(float velocityX, float velocityY, boolean consumed) {
    return childHelper.dispatchNestedFling(velocityX, velocityY, consumed);
  }

  @Override
  public boolean dispatchNestedPreFling(float velocityX, float velocityY) {
    return childHelper.dispatchNestedPreFling(velocityX, velocityY);
  }

  @Override
  public boolean onNestedPreFling(View target, float velocityX,
                                  float velocityY) {
    return dispatchNestedPreFling(velocityX, velocityY);
  }

  @Override
  public boolean onNestedFling(View target, float velocityX, float velocityY,
                               boolean consumed) {
    return dispatchNestedFling(velocityX, velocityY, consumed);
  }

  @Override
  public void fling(int velocityY) {
    super.fling(velocityY);
    if (mScrollerTask != null) {
      mScrollerTask.removeMessages(0);
    }
    startScrollerTask();
  }

  @Override
  protected void onScrollChanged(int x, int y, int oldx, int oldy) {
    mScrollX = getScrollX();
    mScrollY = getScrollY();
    onScroll(WXScrollView.this, mScrollX, mScrollY);
    View view = getChildAt(getChildCount() - 1);
    if (view == null) {
      return;
    }
    int d = view.getBottom();
    d -= (getHeight() + mScrollY);
    if (d == 0) {
      onScrollToBottom(mScrollX, mScrollY);
    }
    int count = mScrollViewListeners == null ? 0 : mScrollViewListeners.size();
    for (int i = 0; i < count; ++i) {
      mScrollViewListeners.get(i).onScrollChanged(this, x, y, oldx, oldy);
    }

    showStickyView();
  }

  protected void onScroll(WXScrollView scrollView, int x, int y) {
    int count = mScrollViewListeners == null ? 0 : mScrollViewListeners.size();
    for (int i = 0; i < count; ++i) {
      mScrollViewListeners.get(i).onScroll(this, x, y);
    }
  }

  protected void onScrollToBottom(int x, int y) {
    int count = mScrollViewListeners == null ? 0 : mScrollViewListeners.size();
    for (int i = 0; i < count; ++i) {
      mScrollViewListeners.get(i).onScrollToBottom(this, x, y);
    }
  }

  private void showStickyView() {
    if(mWAScroller == null){
      return;
    }
    View curStickyView = procSticky(mWAScroller.getStickMap());

    if (curStickyView != null) {
      mCurrentStickyView = curStickyView;
    } else {
      mCurrentStickyView = null;
    }
  }

  private View procSticky(Map<String, Map<String, WXComponent>> mStickyMap) {
    if (mStickyMap == null) {
      return null;
    }
    Map<String, WXComponent> stickyMap = mStickyMap.get(mWAScroller.getRef());
    if (stickyMap == null) {
      return null;
    }

    Iterator<Entry<String, WXComponent>> iterator = stickyMap.entrySet().iterator();
    Entry<String, WXComponent> entry = null;
    WXComponent stickyData;
    while (iterator.hasNext()) {
      entry = iterator.next();
      stickyData = entry.getValue();

      getLocationOnScreen(stickyScrollerP);
      stickyData.getHostView().getLocationOnScreen(stickyViewP);
      int parentH = 0;
      if(stickyData.getParent()!=null && stickyData.getParent().getRealView()!=null){
        parentH=stickyData.getParent().getRealView().getHeight();
      }
      int stickyViewH = stickyData.getHostView().getHeight();
      int stickyShowPos = stickyScrollerP[1];
      int stickyStartHidePos = -parentH + stickyScrollerP[1] + stickyViewH;
      if (stickyViewP[1] <= stickyShowPos && stickyViewP[1] >= (stickyStartHidePos - stickyViewH)) {
        mStickyOffset = stickyViewP[1] - stickyStartHidePos;
        stickyData.setStickyOffset(stickyViewP[1]-stickyScrollerP[1]);
        return stickyData.getHostView();
      }else{
        stickyData.setStickyOffset(0);
      }
    }
    return null;
  }

  @Override
  public boolean handleMessage(Message msg) {
    switch (msg.what) {
      case 0:
        if (mScrollerTask != null) {
          mScrollerTask.removeMessages(0);
        }
        int newPosition = getScrollY();
        if (mInitialPosition - newPosition == 0) {//has stopped
          onScrollStopped(WXScrollView.this, getScrollX(), getScrollY());
        } else {
          onScroll(WXScrollView.this, getScrollX(), getScrollY());
          mInitialPosition = getScrollY();
          if (mScrollerTask != null) {
            mScrollerTask.sendEmptyMessageDelayed(0, mCheckTime);
          }
        }
        break;
      default:
        break;
    }
    return true;
  }

  protected void onScrollStopped(WXScrollView scrollView, int x, int y) {
    int count = mScrollViewListeners == null ? 0 : mScrollViewListeners.size();
    for (int i = 0; i < count; ++i) {
      mScrollViewListeners.get(i).onScrollStopped(this, x, y);
    }
  }

  @Override
  public void destroy() {
    if (mScrollerTask != null) {
      mScrollerTask.removeCallbacksAndMessages(null);
    }
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public WXGesture getGestureListener() {
    return wxGesture;
  }

  public Rect getContentFrame() {
    return new Rect(0, 0, computeHorizontalScrollRange(), computeVerticalScrollRange());
  }

  public interface WXScrollViewListener {

    void onScrollChanged(WXScrollView scrollView, int x, int y, int oldx, int oldy);

    void onScrollToBottom(WXScrollView scrollView, int x, int y);

    void onScrollStopped(WXScrollView scrollView, int x, int y);

    void onScroll(WXScrollView scrollView, int x, int y);
  }

  public void setWAScroller(WXScroller mWAScroller) {
    this.mWAScroller = mWAScroller;
  }
}
