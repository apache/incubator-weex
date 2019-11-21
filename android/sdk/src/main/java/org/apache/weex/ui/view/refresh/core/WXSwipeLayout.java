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
package org.apache.weex.ui.view.refresh.core;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Color;
import android.os.Build;
import android.support.annotation.Nullable;
import android.support.v4.view.NestedScrollingChild;
import android.support.v4.view.NestedScrollingChildHelper;
import android.support.v4.view.NestedScrollingParent;
import android.support.v4.view.NestedScrollingParentHelper;
import android.support.v4.view.ViewCompat;
import android.support.v4.view.ViewParentCompat;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.AbsListView;
import android.widget.FrameLayout;

import java.util.LinkedList;
import java.util.List;

public class WXSwipeLayout extends FrameLayout implements NestedScrollingParent, NestedScrollingChild {

  private NestedScrollingParentHelper mNestedScrollingParentHelper;
  private NestedScrollingChildHelper mNestedScrollingChildHelper;
  private final int[] mParentScrollConsumed = new int[2];
  private final int[] mParentOffsetInWindow = new int[2];
  private boolean mNestedScrollInProgress;
  private WXOnRefreshListener onRefreshListener;
  private WXOnLoadingListener onLoadingListener;

  private ViewParent mNestedScrollAcceptedParent;

  private final List<OnRefreshOffsetChangedListener> mRefreshOffsetChangedListeners = new LinkedList<>();

  public interface OnRefreshOffsetChangedListener {
    void onOffsetChanged(int verticalOffset);
  }

  /**
   * On refresh Callback, call on start refresh
   */
  public interface WXOnRefreshListener {

    void onRefresh();

    void onPullingDown(float dy, int pullOutDistance, float viewHeight);
  }

  /**
   * On loadmore Callback, call on start loadmore
   */
  public interface WXOnLoadingListener {

    void onLoading();
    void onPullingUp(float dy, int pullOutDistance, float viewHeight);
  }

  static class WXRefreshAnimatorListener implements Animator.AnimatorListener {

    @Override
    public void onAnimationStart(Animator animation) {
    }

    @Override
    public void onAnimationEnd(Animator animation) {
    }

    @Override
    public void onAnimationCancel(Animator animation) {
    }

    @Override
    public void onAnimationRepeat(Animator animation) {
    }
  }

  private WXRefreshView headerView;
  private WXRefreshView footerView;
  private View mTargetView;

  private static final int INVALID = -1;
  private static final int PULL_REFRESH = 0;
  private static final int LOAD_MORE = 1;

  // Enable PullRefresh and Loadmore
  private boolean mPullRefreshEnable = false;
  private boolean mPullLoadEnable = false;

  // Is Refreshing
  volatile private boolean mRefreshing = false;

  // RefreshView Height
  private volatile float refreshViewHeight = 0;
  private volatile float loadingViewHeight = 0;

  // RefreshView Over Flow Height
  private volatile float refreshViewFlowHeight = 0;
  private volatile float loadingViewFlowHeight = 0;

  private static final float overFlow = 1.0f;

  private static final float DAMPING = 0.4f;

  // Drag Action
  private int mCurrentAction = -1;
  private boolean isConfirm = false;

  // RefreshView Attrs
  private int mRefreshViewBgColor;
  private int mProgressBgColor;
  private int mProgressColor;

  public WXSwipeLayout(Context context) {
    super(context);
    initAttrs(context, null);
  }

  public WXSwipeLayout(Context context, AttributeSet attrs) {
    super(context, attrs);
    initAttrs(context, attrs);
  }

  public WXSwipeLayout(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    initAttrs(context, attrs);
  }

  @TargetApi(Build.VERSION_CODES.LOLLIPOP)
  public WXSwipeLayout(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
    initAttrs(context, attrs);
  }

  private void initAttrs(Context context, AttributeSet attrs) {

    if (getChildCount() > 1) {
      throw new RuntimeException("WXSwipeLayout should not have more than one child");
    }

    mNestedScrollingParentHelper = new NestedScrollingParentHelper(this);
    mNestedScrollingChildHelper = new NestedScrollingChildHelper(this);
    setNestedScrollingEnabled(false);

    if (isInEditMode() && attrs == null) {
      return;
    }

    mRefreshViewBgColor = Color.TRANSPARENT;
    mProgressBgColor = Color.TRANSPARENT;
    mProgressColor = Color.RED;
  }

  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    if(mTargetView == null && getChildCount() > 0){
      mTargetView = getChildAt(0);
    }
    if(mTargetView != null){
      if(headerView == null || footerView == null){
        setRefreshView();
      }
    }
  }


  public void addTargetView(View mInnerView){
    this.addView(mInnerView, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
    setRefreshView();
  }
  /**
   * Init refresh view or loading view
   */
  private void setRefreshView() {
    // SetUp HeaderView
    FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, 0);
    headerView = new WXRefreshView(getContext());
    headerView.setStartEndTrim(0, 0.75f);
    headerView.setBackgroundColor(mRefreshViewBgColor);
    headerView.setProgressBgColor(mProgressBgColor);
    headerView.setProgressColor(mProgressColor);
    headerView.setContentGravity(Gravity.BOTTOM);
    addView(headerView, lp);

    // SetUp FooterView
    lp = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, 0);
    lp.gravity = Gravity.BOTTOM;
    footerView = new WXRefreshView(getContext());
    footerView.setStartEndTrim(0.5f, 1.25f);
    footerView.setBackgroundColor(mRefreshViewBgColor);
    footerView.setProgressBgColor(mProgressBgColor);
    footerView.setProgressColor(mProgressColor);
    footerView.setContentGravity(Gravity.TOP);
    addView(footerView, lp);
  }

  @Override
  public boolean onInterceptTouchEvent(MotionEvent ev) {
    if ((!mPullRefreshEnable && !mPullLoadEnable)) {
      return false;
    }
    if (!isEnabled() || canChildScrollUp()
            || mRefreshing || mNestedScrollInProgress) {
      // Fail fast if we're not in a state where a swipe is possible
      return false;
    }

    return super.onInterceptTouchEvent(ev);
  }

  // NestedScrollingChild

  @Override
  public void setNestedScrollingEnabled(boolean enabled) {
    mNestedScrollingChildHelper.setNestedScrollingEnabled(enabled);
  }

  @Override
  public boolean isNestedScrollingEnabled() {
    return mNestedScrollingChildHelper.isNestedScrollingEnabled();
  }

  @Override
  public boolean startNestedScroll(int axes) {
    boolean result = mNestedScrollingChildHelper.startNestedScroll(axes);
    if(result){
      if(mNestedScrollAcceptedParent == null){
        ViewParent parent  = this.getParent();
        View child = this;
        while (parent != null) {
          if (ViewParentCompat.onStartNestedScroll(parent, child, this, axes)){
            mNestedScrollAcceptedParent = parent;
            break;
          }
          if(parent instanceof  View){
            child = (View) parent;
          }
          parent = parent.getParent();
        }
      }
    }
    return result;
  }

  @Override
  public void stopNestedScroll() {
    mNestedScrollingChildHelper.stopNestedScroll();
    if(mNestedScrollAcceptedParent != null){
      mNestedScrollAcceptedParent = null;
    }
  }

  @Override
  public boolean hasNestedScrollingParent() {
    return mNestedScrollingChildHelper.hasNestedScrollingParent();
  }

  @Override
  public boolean dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
                                      int dyUnconsumed, int[] offsetInWindow) {
    return mNestedScrollingChildHelper.dispatchNestedScroll(dxConsumed, dyConsumed,
            dxUnconsumed, dyUnconsumed, offsetInWindow);
  }

  @Override
  public boolean dispatchNestedPreScroll(int dx, int dy, int[] consumed, int[] offsetInWindow) {
    return mNestedScrollingChildHelper.dispatchNestedPreScroll(
            dx, dy, consumed, offsetInWindow);
  }



  @Override
  public boolean dispatchNestedFling(float velocityX, float velocityY, boolean consumed) {
    return mNestedScrollingChildHelper.dispatchNestedFling(velocityX, velocityY, consumed);
  }

  @Override
  public boolean dispatchNestedPreFling(float velocityX, float velocityY) {
    return mNestedScrollingChildHelper.dispatchNestedPreFling(velocityX, velocityY);
  }

  /*********************************** NestedScrollParent *************************************/


  @Override
  public boolean onNestedPreFling(View target, float velocityX,
                                  float velocityY) {
    if(isNestedScrollingEnabled()) {
      return dispatchNestedPreFling(velocityX, velocityY);
    }
    return  false;
  }

  @Override
  public boolean onNestedFling(View target, float velocityX, float velocityY,
                               boolean consumed) {
    if(isNestedScrollingEnabled()) {
      return dispatchNestedFling(velocityX, velocityY, consumed);
    }
    return  false;
  }

  @Override
  public boolean onStartNestedScroll(View child, View target, int nestedScrollAxes) {
    boolean result =  isEnabled()  && !mRefreshing
            && (nestedScrollAxes & ViewCompat.SCROLL_AXIS_VERTICAL) != 0;

    return  result;
  }

  @Override
  public void onNestedScrollAccepted(View child, View target, int axes) {
    mNestedScrollingParentHelper.onNestedScrollAccepted(child, target, axes);
    if(isNestedScrollingEnabled()){
      startNestedScroll(axes & ViewCompat.SCROLL_AXIS_VERTICAL);
      mNestedScrollInProgress = true;
    }
  }


  /**
   * With child view to processing move events
   * @param target the child view
   * @param dx move x
   * @param dy move y
   * @param consumed parent consumed move distance
   */
  @Override
  public void onNestedPreScroll(View target, int dx, int dy, int[] consumed) {
    // Now let our nested parent consume the leftovers
    final int[] parentConsumed = mParentScrollConsumed;
    if(isNestedScrollingEnabled()){
      if (dispatchNestedPreScroll(dx - consumed[0], dy - consumed[1], parentConsumed, null)) {
        consumed[0] += parentConsumed[0];
        consumed[1] += parentConsumed[1];
        return;
      }
    }
    if ((!mPullRefreshEnable && !mPullLoadEnable)) {
      return;
    }

    /**
     * when in nest-scroll, list canChildScrollUp() false,
     * maybe parent scroll can scroll up
     * */
    if(!canChildScrollUp() && isNestedScrollingEnabled()){
      if(mNestedScrollAcceptedParent != null && mNestedScrollAcceptedParent != mTargetView){
        ViewGroup group = (ViewGroup) mNestedScrollAcceptedParent;
        if(group.getChildCount() > 0){
          int count = group.getChildCount();
          for(int i=0; i<count; i++){
            View view  = group.getChildAt(i);
            if(view.getVisibility() != View.GONE && view.getMeasuredHeight() > 0){
              if(view.getTop() < 0){
                return;
              }else{
                break;
              }
            }
          }
        }
      }
    }



    int spinnerDy = (int) calculateDistanceY(target, dy);

    mRefreshing = false;

    if (!isConfirm) {
      if (spinnerDy < 0 && !canChildScrollUp()) {
        mCurrentAction = PULL_REFRESH;
        isConfirm = true;
      } else if (spinnerDy > 0 && !canChildScrollDown() && (!mRefreshing)) {
        mCurrentAction = LOAD_MORE;
        isConfirm = true;
      }
    }

    if (moveSpinner(-spinnerDy)) {
      if (!canChildScrollUp() && mPullRefreshEnable
              && mTargetView.getTranslationY() > 0
              && dy > 0) {
        consumed[1] += dy;
      }else if (!canChildScrollDown() && mPullLoadEnable
              && mTargetView.getTranslationY() < 0
              && dy < 0){
        consumed[1] += dy;
      }else{
        consumed[1] += spinnerDy;
      }
    }
  }

  @Override
  public int getNestedScrollAxes() {
    return mNestedScrollingParentHelper.getNestedScrollAxes();
  }


  /**
   * Callback on TouchEvent.ACTION_CANCLE or TouchEvent.ACTION_UP
   * handler : refresh or loading
   * @param child : child view of SwipeLayout,RecyclerView or Scroller
   */
  @Override
  public void onStopNestedScroll(View child) {
    mNestedScrollingParentHelper.onStopNestedScroll(child);
    handlerAction();
    if(isNestedScrollingEnabled()) {
      mNestedScrollInProgress = true;
      stopNestedScroll();
    }
  }


  @Override
  public void onNestedScroll(View target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed) {
    if(isNestedScrollingEnabled()) {
      dispatchNestedScroll(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed, mParentOffsetInWindow);
    }
  }





  private double calculateDistanceY(View target, int dy) {
    int viewHeight = target.getMeasuredHeight();
    double ratio = (viewHeight - Math.abs(target.getY())) / 1.0d / viewHeight * DAMPING;
    if (ratio <= 0.01d) {
      //Filter tiny scrolling action
      ratio = 0.01d;
    }
    return ratio * dy;
  }

  /**
   * Adjust the refresh or loading view according to the size of the gesture
   *
   * @param distanceY move distance of Y
   */
  private boolean moveSpinner(float distanceY) {
    if (mRefreshing) {
      return false;
    }

    if (!canChildScrollUp() && mPullRefreshEnable && mCurrentAction == PULL_REFRESH) {
      // Pull Refresh
      LayoutParams lp = (LayoutParams) headerView.getLayoutParams();
      lp.height += distanceY;
      if (lp.height < 0) {
        lp.height = 0;
      }

      if (lp.height == 0) {
        isConfirm = false;
        mCurrentAction = INVALID;
      }
      headerView.setLayoutParams(lp);
      onRefreshListener.onPullingDown(distanceY, lp.height, refreshViewFlowHeight);
      notifyOnRefreshOffsetChangedListener(lp.height);
      headerView.setProgressRotation(lp.height / refreshViewFlowHeight);
      moveTargetView(lp.height);
      return true;
    } else if (!canChildScrollDown() && mPullLoadEnable && mCurrentAction == LOAD_MORE) {
      // Load more
      LayoutParams lp = (LayoutParams) footerView.getLayoutParams();
      lp.height -= distanceY;
      if (lp.height < 0) {
        lp.height = 0;
      }

      if (lp.height == 0) {
        isConfirm = false;
        mCurrentAction = INVALID;
      }
      footerView.setLayoutParams(lp);
      onLoadingListener.onPullingUp(distanceY, lp.height, loadingViewFlowHeight);
      footerView.setProgressRotation(lp.height / loadingViewFlowHeight);
      moveTargetView(-lp.height);
      return true;
    }
    return false;
  }

  /**
   * Adjust contentView(Scroller or List) at refresh or loading time
   * @param h Height of refresh view or loading view
   */
  private void moveTargetView(float h) {
    mTargetView.setTranslationY(h);
  }

  /**
   * Decide on the action refresh or loadmore
   */
  private void handlerAction() {

    if (isRefreshing()) {
      return;
    }
    isConfirm = false;

    LayoutParams lp;
    if (mPullRefreshEnable && mCurrentAction == PULL_REFRESH) {
      lp = (LayoutParams) headerView.getLayoutParams();
      if (lp.height >= refreshViewHeight) {
        startRefresh(lp.height);
      } else if (lp.height > 0) {
        resetHeaderView(lp.height);
      } else {
        resetRefreshState();
      }
    }

    if (mPullLoadEnable && mCurrentAction == LOAD_MORE) {
      lp = (LayoutParams) footerView.getLayoutParams();
      if (lp.height >= loadingViewHeight) {
        startLoadmore(lp.height);
      } else if (lp.height > 0) {
        resetFootView(lp.height);
      } else {
        resetLoadmoreState();
      }
    }
  }

  /**
   * Start Refresh
   * @param headerViewHeight
   */
  private void startRefresh(int headerViewHeight) {
    mRefreshing = true;
    ValueAnimator animator = ValueAnimator.ofFloat(headerViewHeight, refreshViewHeight);
    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
      @Override
      public void onAnimationUpdate(ValueAnimator animation) {
        LayoutParams lp = (LayoutParams) headerView.getLayoutParams();
        lp.height = (int) ((Float) animation.getAnimatedValue()).floatValue();
        notifyOnRefreshOffsetChangedListener(lp.height);
        headerView.setLayoutParams(lp);
        moveTargetView(lp.height);
      }
    });
    animator.addListener(new WXRefreshAnimatorListener() {
      @Override
      public void onAnimationEnd(Animator animation) {
        headerView.startAnimation();
        //TODO updateLoadText
        if (onRefreshListener != null) {
          onRefreshListener.onRefresh();
        }
      }
    });
    animator.setDuration(300);
    animator.start();
  }

  /**
   * Reset refresh state
   * @param headerViewHeight
   */
  private void resetHeaderView(int headerViewHeight) {
    headerView.stopAnimation();
    headerView.setStartEndTrim(0, 0.75f);
    ValueAnimator animator = ValueAnimator.ofFloat(headerViewHeight, 0);
    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
      @Override
      public void onAnimationUpdate(ValueAnimator animation) {
        LayoutParams lp = (LayoutParams) headerView.getLayoutParams();
        lp.height = (int) ((Float) animation.getAnimatedValue()).floatValue();
        notifyOnRefreshOffsetChangedListener(lp.height);
        headerView.setLayoutParams(lp);
        moveTargetView(lp.height);
      }
    });
    animator.addListener(new WXRefreshAnimatorListener() {
      @Override
      public void onAnimationEnd(Animator animation) {
        resetRefreshState();

      }
    });
    animator.setDuration(300);
    animator.start();
  }

  private void resetRefreshState() {
    mRefreshing = false;
    isConfirm = false;
    mCurrentAction = -1;
    //TODO updateLoadText
  }

  /**
   * Start loadmore
   * @param headerViewHeight
   */
  private void startLoadmore(int headerViewHeight) {
    mRefreshing = true;
    ValueAnimator animator = ValueAnimator.ofFloat(headerViewHeight, loadingViewHeight);
    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
      @Override
      public void onAnimationUpdate(ValueAnimator animation) {
        LayoutParams lp = (LayoutParams) footerView.getLayoutParams();
        lp.height = (int) ((Float) animation.getAnimatedValue()).floatValue();
        footerView.setLayoutParams(lp);
        moveTargetView(-lp.height);
      }
    });
    animator.addListener(new WXRefreshAnimatorListener() {
      @Override
      public void onAnimationEnd(Animator animation) {
        footerView.startAnimation();
        //TODO updateLoadText
        if (onLoadingListener != null) {
          onLoadingListener.onLoading();
        }
      }
    });
    animator.setDuration(300);
    animator.start();
  }

  /**
   * Reset loadmore state
   * @param headerViewHeight
   */
  private void resetFootView(int headerViewHeight) {
    footerView.stopAnimation();
    footerView.setStartEndTrim(0.5f, 1.25f);
    ValueAnimator animator = ValueAnimator.ofFloat(headerViewHeight, 0);
    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
      @Override
      public void onAnimationUpdate(ValueAnimator animation) {
        LayoutParams lp = (LayoutParams) footerView.getLayoutParams();
        lp.height = (int) ((Float) animation.getAnimatedValue()).floatValue();
        footerView.setLayoutParams(lp);
        moveTargetView(-lp.height);
      }
    });
    animator.addListener(new WXRefreshAnimatorListener() {
      @Override
      public void onAnimationEnd(Animator animation) {
        resetLoadmoreState();

      }
    });
    animator.setDuration(300);
    animator.start();
  }

  private void resetLoadmoreState() {
    mRefreshing = false;
    isConfirm = false;
    mCurrentAction = -1;
    //TODO updateLoadText
  }

  /**
   * Whether child view can scroll up
   * @return
   */
  @SuppressLint("ObsoleteSdkInt")
  public boolean canChildScrollUp() {
    if (mTargetView == null) {
      return false;
    }
    if (Build.VERSION.SDK_INT < 14) {
      //The minimum version of Android platform that Weex supports is API 14,
      //so this block should never enter.
      if (mTargetView instanceof AbsListView) {
        final AbsListView absListView = (AbsListView) mTargetView;
        return absListView.getChildCount() > 0
                && (absListView.getFirstVisiblePosition() > 0 || absListView.getChildAt(0)
                .getTop() < absListView.getPaddingTop());
      } else {
        return ViewCompat.canScrollVertically(mTargetView, -1) || mTargetView.getScrollY() > 0;
      }
    } else {
      return ViewCompat.canScrollVertically(mTargetView, -1);
    }
  }

  /**
   * Whether child view can scroll down
   * @return
   */
  @SuppressLint("ObsoleteSdkInt")
  public boolean canChildScrollDown() {
    if (mTargetView == null) {
      return false;
    }
    if (Build.VERSION.SDK_INT < 14) {
      //The minimum version of Android platform that Weex supports is API 14,
      //so this block should never enter.
      if (mTargetView instanceof AbsListView) {
        final AbsListView absListView = (AbsListView) mTargetView;
        if (absListView.getChildCount() > 0) {
          int lastChildBottom = absListView.getChildAt(absListView.getChildCount() - 1)
                  .getBottom();
          return absListView.getLastVisiblePosition() == absListView.getAdapter().getCount() - 1
                  && lastChildBottom <= absListView.getMeasuredHeight();
        } else {
          return false;
        }

      } else {
        return ViewCompat.canScrollVertically(mTargetView, 1) || mTargetView.getScrollY() > 0;
      }
    } else {
      return ViewCompat.canScrollVertically(mTargetView, 1);
    }
  }

  public float dipToPx(Context context, float value) {
    DisplayMetrics metrics = context.getResources().getDisplayMetrics();
    return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, metrics);
  }

  public void setOnLoadingListener(WXOnLoadingListener onLoadingListener) {
    this.onLoadingListener = onLoadingListener;
  }

  public void setOnRefreshListener(WXOnRefreshListener onRefreshListener) {
    this.onRefreshListener = onRefreshListener;
  }

  @SuppressWarnings("unused")
  public void addOnRefreshOffsetChangedListener(@Nullable OnRefreshOffsetChangedListener listener) {
    if(listener != null && !mRefreshOffsetChangedListeners.contains(listener)) {
      mRefreshOffsetChangedListeners.add(listener);
    }
  }

  @SuppressWarnings("unused")
  public boolean removeOnRefreshOffsetChangedListener(@Nullable OnRefreshOffsetChangedListener listener) {
    if(listener != null) {
      return mRefreshOffsetChangedListeners.remove(listener);
    }
    return false;
  }

  private void notifyOnRefreshOffsetChangedListener(int verticalOffset) {
    int size = mRefreshOffsetChangedListeners.size();
    OnRefreshOffsetChangedListener listener;
    for (int i=0; i<size; i++) {
      if(i >= mRefreshOffsetChangedListeners.size()){
        break;
      }
      listener = mRefreshOffsetChangedListeners.get(i);

      if (listener != null) {
        listener.onOffsetChanged(verticalOffset);
      }
    }
  }

  /**
   * Callback on refresh finish
   */
  public void finishPullRefresh() {
    if (mCurrentAction == PULL_REFRESH) {
      resetHeaderView(headerView == null ? 0 : headerView.getMeasuredHeight());
    }
  }

  /**
   * Callback on loadmore finish
   */
  public void finishPullLoad() {
    if (mCurrentAction == LOAD_MORE) {
      resetFootView(footerView == null ? 0 : footerView.getMeasuredHeight());
    }
  }

  public WXRefreshView getHeaderView() {
    return headerView;
  }

  public WXRefreshView getFooterView() {
    return footerView;
  }

  public boolean isPullLoadEnable() {
    return mPullLoadEnable;
  }

  public void setPullLoadEnable(boolean mPullLoadEnable) {
    this.mPullLoadEnable = mPullLoadEnable;
  }

  public boolean isPullRefreshEnable() {
    return mPullRefreshEnable;
  }

  public void setPullRefreshEnable(boolean mPullRefreshEnable) {
    this.mPullRefreshEnable = mPullRefreshEnable;
  }

  public boolean isRefreshing() {
    return mRefreshing;
  }

  public void setRefreshHeight(int height) {
    refreshViewHeight = height;
    refreshViewFlowHeight = refreshViewHeight * overFlow;
  }

  public void setLoadingHeight(int height) {
    loadingViewHeight = height;
    loadingViewFlowHeight = loadingViewHeight * overFlow;
  }

  public void setRefreshBgColor(int color) {
    headerView.setBackgroundColor(color);
  }

  public void setLoadingBgColor(int color) {
    footerView.setBackgroundColor(color);
  }
}
