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
package org.apache.weex.ui.component;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.v4.view.ViewCompat;
import android.text.TextUtils;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.common.Constants;
import org.apache.weex.common.ICheckBindingScroller;
import org.apache.weex.common.OnWXScrollListener;
import org.apache.weex.common.WXThread;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.ui.ComponentCreator;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.component.helper.ScrollStartEndHelper;
import org.apache.weex.ui.component.helper.WXStickyHelper;
import org.apache.weex.ui.view.IWXScroller;
import org.apache.weex.ui.view.WXBaseRefreshLayout;
import org.apache.weex.ui.view.WXHorizontalScrollView;
import org.apache.weex.ui.view.WXScrollView;
import org.apache.weex.ui.view.WXScrollView.WXScrollViewListener;
import org.apache.weex.ui.view.refresh.wrapper.BaseBounceView;
import org.apache.weex.ui.view.refresh.wrapper.BounceScrollerView;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;
import org.apache.weex.utils.WXViewUtils;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Component for scroller. It also support features like
 * "appear", "disappear" and "sticky"
 */
@Component(lazyload = false)

public class WXScroller extends WXVContainer<ViewGroup> implements WXScrollViewListener,Scrollable {

  public static final String DIRECTION = "direction";
  protected int mOrientation = Constants.Orientation.VERTICAL;
  private List<WXComponent> mRefreshs=new ArrayList<>();
  /** Use for offset children layout */
  private int mChildrenLayoutOffset = 0;
  private boolean mForceLoadmoreNextTime = false;
  private int mOffsetAccuracy = 10;
  private Point mLastReport = new Point(-1, -1);
  private boolean mHasAddScrollEvent = false;
  private Boolean mIslastDirectionRTL;

  private static final int SWIPE_MIN_DISTANCE = 5;
  private static final int SWIPE_THRESHOLD_VELOCITY = 300;
  private int mActiveFeature = 0;
  /**
   * scroll start and scroll end event
   * */
  private ScrollStartEndHelper mScrollStartEndHelper;

  private GestureDetector mGestureDetector;

  private int pageSize = 0;
  private boolean pageEnable = false;
  private boolean mIsHostAttachedToWindow = false;
  private View.OnAttachStateChangeListener mOnAttachStateChangeListener;

  private boolean mlastDirectionRTL = false;

  public static class Creator implements ComponentCreator {
    @Override
    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      // For performance message collection
      instance.setUseScroller(true);
      return new WXScroller(instance, parent, basicComponentData);
    }
  }
  /**
   * Map for storing appear information
   **/
  private Map<String,AppearanceHelper> mAppearanceComponents = new HashMap<>();

  /**
   * Map for storing component that is sticky.
   **/
  private Map<String, Map<String, WXComponent>> mStickyMap = new HashMap<>();
  private FrameLayout mRealView;
  private FrameLayout mScrollerView;

  private int mContentHeight = 0;

  private WXStickyHelper stickyHelper;
  private Handler handler=new Handler(Looper.getMainLooper());

  private boolean isScrollable = true;

  @Deprecated
  public WXScroller(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXScroller(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    stickyHelper = new WXStickyHelper(this);
    instance.getApmForInstance().updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_SCROLLER_NUM,1);
  }

  /**
   * @return FrameLayout inner ScrollView
   */
  @Override
  public ViewGroup getRealView() {
    return mScrollerView;
  }


  @Override
  public void createViewImpl() {
    super.createViewImpl();
    for (int i = 0; i < mRefreshs.size(); i++) {
      WXComponent component = mRefreshs.get(i);
      component.createViewImpl();
      checkRefreshOrLoading(component);
    }
  }

  /**
   * @return ScrollView
   */
  public ViewGroup getInnerView() {
    if(getHostView() == null) {
      return null;
    }
    if (getHostView() instanceof BounceScrollerView) {
      return ((BounceScrollerView) getHostView()).getInnerView();
    } else {
      return getHostView();
    }
  }

  @Override
  public void addEvent(String type) {
    super.addEvent(type);
    if (ScrollStartEndHelper.isScrollEvent(type)
            && getInnerView() != null && !mHasAddScrollEvent) {
      mHasAddScrollEvent = true;
      if (getInnerView() instanceof WXScrollView) {
        ((WXScrollView) getInnerView()).addScrollViewListener(new WXScrollViewListener() {
          @Override
          public void onScrollChanged(WXScrollView scrollView, int x, int y, int oldx, int oldy) {
            getScrollStartEndHelper().onScrolled(x, y);
            if(!getEvents().contains(Constants.Event.SCROLL)){
              return;
            }
            if (shouldReport(x, y)) {
              fireScrollEvent(scrollView.getContentFrame(), x, y, oldx, oldy);
            }
          }

          @Override
          public void onScrollToBottom(WXScrollView scrollView, int x, int y) {
            //ignore
          }

          @Override
          public void onScrollStopped(WXScrollView scrollView, int x, int y) {
            //ignore
          }

          @Override
          public void onScroll(WXScrollView scrollView, int x, int y) {
            //ignore
          }
        });
      } else if (getInnerView() instanceof WXHorizontalScrollView) {
        ((WXHorizontalScrollView) getInnerView()).addScrollViewListener(new WXHorizontalScrollView.ScrollViewListener() {
          @Override
          public void onScrollChanged(WXHorizontalScrollView scrollView, int x, int y, int oldx, int oldy) {
            getScrollStartEndHelper().onScrolled(x, y);
            if(!getEvents().contains(Constants.Event.SCROLL)){
              return;
            }
            if (shouldReport(x, y)) {
              fireScrollEvent(scrollView.getContentFrame(), x, y, oldx, oldy);
            }
          }
        });
      }
    }
  }

  private void fireScrollEvent(Rect contentFrame, int x, int y, int oldx, int oldy) {
    fireEvent(Constants.Event.SCROLL, getScrollEvent(x, y));
  }

  public Map<String, Object> getScrollEvent(int x, int y){
    Rect contentFrame =  new Rect();
    if (getInnerView() instanceof WXScrollView) {
      contentFrame = ((WXScrollView) getInnerView()).getContentFrame();
    }else if (getInnerView() instanceof WXHorizontalScrollView) {
      contentFrame = ((WXHorizontalScrollView) getInnerView()).getContentFrame();
    }
    Map<String, Object> event = new HashMap<>(2);
    Map<String, Object> contentSize = new HashMap<>(2);
    Map<String, Object> contentOffset = new HashMap<>(2);

    int viewport = getInstance().getInstanceViewPortWidth();

    contentSize.put(Constants.Name.WIDTH, WXViewUtils.getWebPxByWidth(contentFrame.width(), viewport));
    contentSize.put(Constants.Name.HEIGHT, WXViewUtils.getWebPxByWidth(contentFrame.height(), viewport));

    contentOffset.put(Constants.Name.X, -WXViewUtils.getWebPxByWidth(x, viewport));
    contentOffset.put(Constants.Name.Y, -WXViewUtils.getWebPxByWidth(y, viewport));

    event.put(Constants.Name.CONTENT_SIZE, contentSize);
    event.put(Constants.Name.CONTENT_OFFSET, contentOffset);
    return  event;
  }

  private boolean shouldReport(int x, int y) {
    if (mLastReport.x == -1 && mLastReport.y == -1) {
      mLastReport.x = x;
      mLastReport.y = y;
      return true;
    }

    if (mOrientation == Constants.Orientation.HORIZONTAL
            && Math.abs(x - mLastReport.x) >= mOffsetAccuracy) {
      mLastReport.x = x;
      mLastReport.y = y;
      return true;
    }

    if (mOrientation == Constants.Orientation.VERTICAL
            && Math.abs(y - mLastReport.y) >= mOffsetAccuracy) {
      mLastReport.x = x;
      mLastReport.y = y;
      return true;
    }

    return false;
  }

  /**
   * Intercept refresh view and loading view
   */
  @Override
  public void addSubView(View child, int index) {
    if (child == null || mRealView == null) {
      return;
    }

    if (child instanceof WXBaseRefreshLayout) {
      return;
    }

    int count = mRealView.getChildCount();
    index = index >= count ? -1 : index;
    if (index == -1) {
      mRealView.addView(child);
    } else {
      mRealView.addView(child, index);
    }
  }

  @Override
  protected int getChildrenLayoutTopOffset() {
    if (mChildrenLayoutOffset == 0) {
      // Child LayoutSize data set after call Layout. So init mChildrenLayoutOffset here
      final int listSize = mRefreshs.size();
      if (listSize > 0) {
        for (int i = 0; i < listSize; i++) {
          WXComponent child = mRefreshs.get(i);
          mChildrenLayoutOffset += child.getLayoutTopOffsetForSibling();
        }
      }
    }
    return mChildrenLayoutOffset;
  }

  /**
   * Intercept refresh view and loading view
   */
  @Override
  public void addChild(WXComponent child, int index) {
    if (child instanceof WXBaseRefresh) {
      if (checkRefreshOrLoading(child)) {
        mRefreshs.add(child);
      }
    }
    super.addChild(child, index);
  }

  /**
   * Setting refresh view and loading view
   * @param child the refresh_view or loading_view
   */

  private boolean checkRefreshOrLoading(final WXComponent child) {
    boolean result = false;
    if (child instanceof WXRefresh && getHostView() != null) {
      ((BaseBounceView) getHostView()).setOnRefreshListener((WXRefresh) child);
      Runnable runnable = WXThread.secure(new Runnable(){
        @Override
        public void run() {
          ((BaseBounceView) getHostView()).setHeaderView(child);
        }
      });
      handler.postDelayed(runnable,100);
      result = true;
    }

    if (child instanceof WXLoading && getHostView() !=null) {
      ((BaseBounceView) getHostView()).setOnLoadingListener((WXLoading)child);
      Runnable runnable= WXThread.secure(new Runnable(){
        @Override
        public void run() {
          ((BaseBounceView) getHostView()).setFooterView(child);
        }
      });
      handler.postDelayed(runnable, 100);
      result = true;
    }
    return result;
  }

  @Override
  public void remove(WXComponent child,boolean destory) {
    super.remove(child,destory);
    if(child instanceof WXLoading){
      ((BaseBounceView)getHostView()).removeFooterView(child);
    }else if(child instanceof WXRefresh){
      ((BaseBounceView)getHostView()).removeHeaderView(child);
    }
  }

  @Override
  public void destroy() {
    super.destroy();
    if (mAppearanceComponents != null) {
      mAppearanceComponents.clear();
    }
    if (mStickyMap != null) {
      mStickyMap.clear();
    }
    if (mOnAttachStateChangeListener != null && getInnerView() != null) {
      getInnerView().removeOnAttachStateChangeListener(mOnAttachStateChangeListener);
    }
    if (getInnerView() != null && getInnerView() instanceof IWXScroller) {
      ((IWXScroller) getInnerView()).destroy();
    }
  }

  @SuppressLint("RtlHardcoded")
  @Override
  public void setMarginsSupportRTL(ViewGroup.MarginLayoutParams lp, int left, int top, int right, int bottom) {
    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR1) {
      lp.setMargins(left, top, right, bottom);
      lp.setMarginStart(left);
      lp.setMarginEnd(right);
    } else {
      if (lp instanceof FrameLayout.LayoutParams) {
        FrameLayout.LayoutParams lp_frameLayout = (FrameLayout.LayoutParams) lp;
        if (isLayoutRTL()) {
          lp_frameLayout.gravity = Gravity.RIGHT | Gravity.TOP;
          lp.setMargins(right, top, left, bottom);
        } else {
          lp_frameLayout.gravity = Gravity.LEFT | Gravity.TOP;
          lp.setMargins(left, top, right, bottom);
        }
      } else {
        lp.setMargins(left, top, right, bottom);
      }
    }
  }

  @Override
  public void setLayout(WXComponent component) {
    if (TextUtils.isEmpty(component.getComponentType())
            || TextUtils.isEmpty(component.getRef()) || component.getLayoutPosition() == null
            || component.getLayoutSize() == null) {
      return;
    }
    if (component.getHostView() != null) {
      int layoutDirection = component.isLayoutRTL() ? ViewCompat.LAYOUT_DIRECTION_RTL : ViewCompat.LAYOUT_DIRECTION_LTR;
      ViewCompat.setLayoutDirection(component.getHostView(), layoutDirection);
    }
    super.setLayout(component);
  }

  @Override
  protected MeasureOutput measure(int width, int height) {
    MeasureOutput measureOutput = new MeasureOutput();
    if (this.mOrientation == Constants.Orientation.HORIZONTAL) {
      int screenW = WXViewUtils.getScreenWidth(WXEnvironment.sApplication);
      int weexW = WXViewUtils.getWeexWidth(getInstanceId());
      measureOutput.width = width > (weexW >= screenW ? screenW : weexW) ? FrameLayout.LayoutParams.MATCH_PARENT
              : width;
      measureOutput.height = height;
    } else {
      int screenH = WXViewUtils.getScreenHeight(WXEnvironment.sApplication);
      int weexH = WXViewUtils.getWeexHeight(getInstanceId());
      measureOutput.height = height > (weexH >= screenH ? screenH : weexH) ? FrameLayout.LayoutParams.MATCH_PARENT
              : height;
      measureOutput.width = width;
    }
    return measureOutput;
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  protected ViewGroup initComponentHostView(@NonNull Context context) {
    String scroll;
    if (getAttrs().isEmpty()) {
      scroll = "vertical";
    } else {
      scroll = getAttrs().getScrollDirection();

      Object o = getAttrs().get(Constants.Name.PAGE_ENABLED);

      pageEnable = o != null && Boolean.parseBoolean(o.toString());

      Object pageSize = getAttrs().get(Constants.Name.PAGE_SIZE);
      if (pageSize != null) {
        float aFloat = WXUtils.getFloat(pageSize);


        float realPxByWidth = WXViewUtils.getRealPxByWidth(aFloat, getInstance().getInstanceViewPortWidth());
        if (realPxByWidth != 0) {
          this.pageSize = (int) realPxByWidth;
        }
      }

    }

    ViewGroup host;
    if(("horizontal").equals(scroll)){
      mOrientation = Constants.Orientation.HORIZONTAL;
      final WXHorizontalScrollView scrollView = new WXHorizontalScrollView(context);
      mRealView = new FrameLayout(context);
      scrollView.setScrollViewListener(new WXHorizontalScrollView.ScrollViewListener() {
        @Override
        public void onScrollChanged(WXHorizontalScrollView scrollView, int x, int y, int oldx, int oldy) {
          procAppear(x,y,oldx,oldy);
        }
      });
      FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
              LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
      scrollView.addView(mRealView, layoutParams);
      scrollView.setHorizontalScrollBarEnabled(false);
      mScrollerView = scrollView;
      final WXScroller component = this;
      final View.OnLayoutChangeListener listener = new View.OnLayoutChangeListener() {
        @Override
        public void onLayoutChange(View view, final int left, int top, final int right, int bottom, final int oldLeft, int oldTop, final int oldRight, int oldBottom) {
          final View frameLayout = view;
          scrollView.post(new Runnable() {
            @Override
            public void run() {
              if (mIslastDirectionRTL != null && isLayoutRTL() != mIslastDirectionRTL) {
                // when layout direction changed we need convert x to RTL x for scroll to the same item
                int currentX = getScrollX();
                int totalWidth = getInnerView().getChildAt(0).getWidth();
                int displayWidth = getInnerView().getMeasuredWidth();
                scrollView.scrollTo(totalWidth - currentX - displayWidth, component.getScrollY());
              } else if (isLayoutRTL()) {
                // if layout direction not changed, but width changede, we need keep RTL offset
                int oldWidth = oldRight - oldLeft;
                int width = right - left;
                int changedWidth = width - oldWidth;
                if (changedWidth != 0) {
                  scrollView.scrollBy(changedWidth, component.getScrollY());
                }
              }
              mIslastDirectionRTL = isLayoutRTL();
            }
          });
        }
      };
      mRealView.addOnAttachStateChangeListener(new View.OnAttachStateChangeListener() {
        @Override
        public void onViewAttachedToWindow(View view) {
          view.addOnLayoutChangeListener(listener);
        }

        @Override
        public void onViewDetachedFromWindow(View view) {
          view.removeOnLayoutChangeListener(listener);
        }
      });


      if(pageEnable) {
        mGestureDetector = new GestureDetector(new MyGestureDetector(scrollView));
        scrollView.setOnTouchListener(new View.OnTouchListener() {
          @Override
          public boolean onTouch(View v, MotionEvent event) {
            if (pageSize == 0)  {
              pageSize = v.getMeasuredWidth();
            }

            if (mGestureDetector.onTouchEvent(event)) {
              return true;
            }
            else if(event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL ){
              int scrollX = getScrollX();
              int featureWidth = pageSize;
              mActiveFeature = ((scrollX + (featureWidth/2))/featureWidth);
              int scrollTo = mActiveFeature*featureWidth;
              scrollView.smoothScrollTo(scrollTo, 0);
              return true;
            }
            else{
              return false;
            }
          }
        });
      }


      host = scrollView;
    }else{
      mOrientation = Constants.Orientation.VERTICAL;
      BounceScrollerView scrollerView = new BounceScrollerView(context, mOrientation, this);
      mRealView = new FrameLayout(context);
      WXScrollView innerView = scrollerView.getInnerView();
      innerView.addScrollViewListener(this);
      FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
              LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
      mScrollerView = innerView;
      innerView.addView(mRealView, layoutParams);
      innerView.setVerticalScrollBarEnabled(true);
      innerView.setNestedScrollingEnabled(WXUtils.getBoolean(getAttrs().get(Constants.Name.NEST_SCROLLING_ENABLED), true));
      innerView.addScrollViewListener(new WXScrollViewListener() {
        @Override
        public void onScrollChanged(WXScrollView scrollView, int x, int y, int oldx, int oldy) {

        }

        @Override
        public void onScrollToBottom(WXScrollView scrollView, int x, int y) {

        }

        @Override
        public void onScrollStopped(WXScrollView scrollView, int x, int y) {
          List<OnWXScrollListener> listeners = getInstance().getWXScrollListeners();
          if(listeners!=null && listeners.size()>0){
            for (OnWXScrollListener listener : listeners) {
              if (listener != null) {
                listener.onScrollStateChanged(scrollView,x,y,OnWXScrollListener.IDLE);
              }
            }
          }
          getScrollStartEndHelper().onScrollStateChanged(OnWXScrollListener.IDLE);
        }

        @Override
        public void onScroll(WXScrollView scrollView, int x, int y) {
          List<OnWXScrollListener> listeners = getInstance().getWXScrollListeners();
          if(listeners!=null && listeners.size()>0){
            for (OnWXScrollListener listener : listeners) {
              if (listener != null) {
                if(listener instanceof ICheckBindingScroller){
                  if(((ICheckBindingScroller) listener).isNeedScroller(getRef(),null)){
                    listener.onScrolled(scrollView, x, y);
                  }
                }else {
                  listener.onScrolled(scrollView, x, y);
                }
              }
            }
          }
        }
      });
      host = scrollerView;
    }

    host.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
      @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
      @Override
      public void onGlobalLayout() {
        procAppear(0,0,0,0);
        View view;
        if( (view = getHostView()) == null){
          return;
        }
        if(Build.VERSION.SDK_INT >=  Build.VERSION_CODES.JELLY_BEAN) {
          view.getViewTreeObserver().removeOnGlobalLayoutListener(this);
        }else{
          view.getViewTreeObserver().removeGlobalOnLayoutListener(this);
        }
      }
    });
    mOnAttachStateChangeListener = new View.OnAttachStateChangeListener() {
      @Override
      public void onViewAttachedToWindow(View v) {
        mIsHostAttachedToWindow = true;
        procAppear(getScrollX(), getScrollY(), getScrollX(), getScrollY());
      }

      @Override
      public void onViewDetachedFromWindow(View v) {
        mIsHostAttachedToWindow = false;
        dispatchDisappearEvent();
      }
    };
    host.addOnAttachStateChangeListener(mOnAttachStateChangeListener);
    return host;
  }

  @Override
  public int getScrollY() {
    return getInnerView() == null ? 0 : getInnerView().getScrollY();
  }

  @Override
  public int getScrollX() {
    return getInnerView() == null ? 0 : getInnerView().getScrollX();
  }

  @Override
  public int getOrientation() {
    return mOrientation;
  }

  public Map<String, Map<String, WXComponent>> getStickMap() {
    return mStickyMap;
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.SHOW_SCROLLBAR:
        Boolean result = WXUtils.getBoolean(param,null);
        if (result != null) {
          setShowScrollbar(result);
        }
        return true;
      case Constants.Name.SCROLLABLE:
        boolean scrollable = WXUtils.getBoolean(param, true);
        setScrollable(scrollable);
        return true;
      case Constants.Name.OFFSET_ACCURACY:
        int accuracy = WXUtils.getInteger(param, 10);
        setOffsetAccuracy(accuracy);
        return true;
        default:
          break;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.SHOW_SCROLLBAR)
  public void setShowScrollbar(boolean show) {
    if(getInnerView()==null){
      return;
    }
    if (mOrientation == Constants.Orientation.VERTICAL) {
      getInnerView().setVerticalScrollBarEnabled(show);
    } else {
      getInnerView().setHorizontalScrollBarEnabled(show);
    }
  }

  @WXComponentProp(name = Constants.Name.SCROLLABLE)
  public void setScrollable(boolean scrollable) {
    this.isScrollable = scrollable;
    View hostView = getInnerView();
    if(hostView instanceof WXHorizontalScrollView) {
      ((WXHorizontalScrollView)hostView).setScrollable(scrollable);
    }else if(hostView instanceof WXScrollView) {
      ((WXScrollView)hostView).setScrollable(scrollable);
    }
  }

  @WXComponentProp(name = Constants.Name.OFFSET_ACCURACY)
  public void setOffsetAccuracy(int accuracy) {
    float realPx = WXViewUtils.getRealPxByWidth(accuracy, getInstance().getInstanceViewPortWidth());
    this.mOffsetAccuracy = (int) realPx;
  }

  @Override
  public boolean isScrollable() {
    return isScrollable;
  }


  @Override
  public void bindStickStyle(WXComponent component) {
    stickyHelper.bindStickStyle(component,mStickyMap);
  }

  @Override
  public void unbindStickStyle(WXComponent component) {
    stickyHelper.unbindStickStyle(component,mStickyMap);
  }

  /**
   * Bind appear event
   */
  @Override
  public void bindAppearEvent(WXComponent component) {
    setWatch(AppearanceHelper.APPEAR,component,true);
  }

  private void setWatch(int event,WXComponent component,boolean isWatch){
    AppearanceHelper item = mAppearanceComponents.get(component.getRef());
    if (item == null) {
      item = new AppearanceHelper(component);
      mAppearanceComponents.put(component.getRef(),item);
    }

    item.setWatchEvent(event,isWatch);

    //check current components appearance status.
    procAppear(0,0,0,0);
  }

  /**
   * Bind disappear event
   */
  @Override
  public void bindDisappearEvent(WXComponent component) {
    setWatch(AppearanceHelper.DISAPPEAR,component,true);
  }

  /**
   * Remove appear event
   */
  @Override
  public void unbindAppearEvent(WXComponent component) {
    setWatch(AppearanceHelper.APPEAR,component,false);
  }

  /**
   * Remove disappear event
   */
  @Override
  public void unbindDisappearEvent(WXComponent component) {
    setWatch(AppearanceHelper.DISAPPEAR,component,false);
  }

  @Override
  public void scrollTo(WXComponent component, Map<String, Object> options) {
    float offsetFloat = 0;
    boolean smooth = true;

    if (options != null) {
      String offset = options.get(Constants.Name.OFFSET) == null ? "0" : options.get(Constants.Name.OFFSET).toString();
      smooth = WXUtils.getBoolean(options.get(Constants.Name.ANIMATED), true);
      if (offset != null) {
        try {
          offsetFloat = WXViewUtils.getRealPxByWidth(Float.parseFloat(offset), getInstance().getInstanceViewPortWidth());
        }catch (Exception e ){
          WXLogUtils.e("Float parseFloat error :"+e.getMessage());
        }
      }
    }

    if(pageEnable) {
      mActiveFeature = mChildren.indexOf(component);
    }

    int viewYInScroller = component.getAbsoluteY() - getAbsoluteY();
    int viewXInScroller = 0;
    if (this.isLayoutRTL()) {
      // if layout direction is rtl, we need calculate rtl scroll x;
      if (component.getParent() != null && component.getParent() == this) {
        if (getInnerView().getChildCount() > 0) {
          int totalWidth = getInnerView().getChildAt(0).getWidth();
          int displayWidth = getInnerView().getMeasuredWidth();
          viewXInScroller = totalWidth - (component.getAbsoluteX() - getAbsoluteX()) - displayWidth;
        } else {
          viewXInScroller = component.getAbsoluteX() - getAbsoluteX();
        }
      } else {
        int displayWidth = getInnerView().getMeasuredWidth();
        viewXInScroller = component.getAbsoluteX() - getAbsoluteX() - displayWidth + (int)component.getLayoutWidth();
      }
      offsetFloat = -offsetFloat;
    } else {
      viewXInScroller = component.getAbsoluteX() - getAbsoluteX();
    }
    scrollBy(viewXInScroller - getScrollX() + (int) offsetFloat, viewYInScroller - getScrollY() + (int) offsetFloat, smooth);

  }

  /**
   * Scroll by specified distance. Horizontal scroll is not supported now.
   * @param x horizontal distance, not support
   * @param y vertical distance. Negative for scroll to top
   */
  public void scrollBy(final int x, final int y) {
    scrollBy(x, y, false);
  }

  public void scrollBy(final int x, final int y, final boolean smooth) {
    if (getInnerView() == null) {
      return;
    }

    getInnerView().postDelayed(new Runnable() {
      @Override
      public void run() {
        if (mOrientation == Constants.Orientation.VERTICAL) {
          if (smooth) {
            ((WXScrollView) getInnerView()).smoothScrollBy(0, y);
          } else {
            ((WXScrollView) getInnerView()).scrollBy(0, y);
          }
        } else {
          if (smooth) {
            ((WXHorizontalScrollView) getInnerView()).smoothScrollBy(x, 0);
          } else {
            ((WXHorizontalScrollView) getInnerView()).scrollBy(x, 0);
          }
        }
        getInnerView().invalidate();
      }
    }, 16);
  }

  @Override
  public void onScrollChanged(WXScrollView scrollView, int x, int y,
                              int oldx, int oldy) {
    procAppear( x, y, oldx, oldy);
  }

  @Override
  public void notifyAppearStateChange(String wxEventType, String direction) {
    if (containsEvent(Constants.Event.APPEAR) || containsEvent(Constants.Event.DISAPPEAR)) {
      Map<String, Object> params = new HashMap<>();
      params.put("direction", direction);
      fireEvent(wxEventType, params);
    }
    // No-op. The moment to notify children is decided by the time when scroller is attached
    // or detached to window. Do not call super as scrollview has different disposal.
  }

  /**
   * Process event like appear and disappear
   *
   * This method will be invoked in several situation below.
   *    1. bind or unbind event
   *    2. host view is attached to window
   *    3. when scrollview is scrolling
   */
  private void procAppear(int x, int y, int oldx,
                          int oldy) {
    if (!mIsHostAttachedToWindow) return;
    int moveY = y - oldy;
    int moveX = x - oldx;
    String direction = moveY > 0 ? Constants.Value.DIRECTION_UP :
            moveY < 0 ? Constants.Value.DIRECTION_DOWN : null;
    if (mOrientation == Constants.Orientation.HORIZONTAL && moveX != 0) {
      direction = moveX > 0 ? Constants.Value.DIRECTION_RIGHT : Constants.Value.DIRECTION_LEFT;
    }

    for (Entry<String, AppearanceHelper> item : mAppearanceComponents.entrySet()) {
      AppearanceHelper helper = item.getValue();

      if (!helper.isWatch()) {
        continue;
      }
      boolean visible = checkItemVisibleInScroller(helper.getAwareChild());

      int result = helper.setAppearStatus(visible);
      if (result != AppearanceHelper.RESULT_NO_CHANGE) {
        helper.getAwareChild().notifyAppearStateChange(result == AppearanceHelper.RESULT_APPEAR ? Constants.Event.APPEAR : Constants.Event.DISAPPEAR, direction);
      }
    }
  }

  /**
   * Check the view of given component is visible in scrollview.
   *
   * @param component ready to be check
   * @return item is visible
   */
  private boolean checkItemVisibleInScroller(WXComponent component) {
    boolean visible = false;
    while (component != null && !(component instanceof WXScroller)) {
      if (component.getParent() instanceof WXScroller) {
        if (mOrientation == Constants.Orientation.HORIZONTAL) {
          int offsetLeft = (int) component.getLayoutPosition().getLeft() - getScrollX();
          visible = (offsetLeft > 0 - component.getLayoutWidth() && offsetLeft < getLayoutWidth());
        } else {
          int offsetTop = (int) component.getLayoutPosition().getTop() - getScrollY();
          visible = (offsetTop > 0 - component.getLayoutHeight() && offsetTop < getLayoutHeight());
        }
      }
      component = component.getParent();
    }
    return visible;
  }

  /**
   * Dispatch disappear event to the child components in need.
   */
  private void dispatchDisappearEvent() {
    for (Entry<String, AppearanceHelper> item : mAppearanceComponents.entrySet()) {
      AppearanceHelper helper = item.getValue();
      if (!helper.isWatch()) {
        continue;
      }
      int result = helper.setAppearStatus(false);
      if (result != AppearanceHelper.RESULT_NO_CHANGE) {
        helper.getAwareChild().notifyAppearStateChange(result == AppearanceHelper.RESULT_APPEAR ?
                Constants.Event.APPEAR : Constants.Event.DISAPPEAR, "");
      }
    }
  }

  @Override
  public void onScrollToBottom(WXScrollView scrollView, int x, int y) {

  }

  @Override
  public void onScrollStopped(WXScrollView scrollView, int x, int y) {
  }

  @Override
  public void onScroll(WXScrollView scrollView, int x, int y) {

    this.onLoadMore(scrollView, x, y);
  }

  /**
   * Handle loadMore Event.when Scroller has bind loadMore Event and set the attr of loadMoreOffset
   * it will tell the JS to handle the event of onLoadMore;
   * @param scrollView  the WXScrollView
   * @param x the X direction
   * @param y the Y direction
   */
  protected void onLoadMore(WXScrollView scrollView, int x, int y) {
    try {
      String offset = getAttrs().getLoadMoreOffset();
      if (TextUtils.isEmpty(offset)) {
        return;
      }
      int offsetInt = (int)WXViewUtils.getRealPxByWidth(Float.parseFloat(offset), getInstance().getInstanceViewPortWidth());

      int contentH = scrollView.getChildAt(0).getHeight();
      int scrollerH = scrollView.getHeight();
      int offScreenY = contentH - y - scrollerH;
      if (offScreenY < offsetInt) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("[WXScroller-onScroll] offScreenY :" + offScreenY);
        }
        if (mContentHeight != contentH || mForceLoadmoreNextTime) {
          fireEvent(Constants.Event.LOADMORE);
          mContentHeight = contentH;
          mForceLoadmoreNextTime = false;
        }
      }
    } catch (Exception e) {
      WXLogUtils.d("[WXScroller-onScroll] ", e);
    }

  }

  @JSMethod
  public void resetLoadmore() {
    mForceLoadmoreNextTime = true;
  }

  public ScrollStartEndHelper getScrollStartEndHelper() {
    if(mScrollStartEndHelper == null){
      mScrollStartEndHelper = new ScrollStartEndHelper(this);
    }
    return mScrollStartEndHelper;
  }


  class MyGestureDetector extends GestureDetector.SimpleOnGestureListener {
    public WXHorizontalScrollView getScrollView() {
      return scrollView;
    }

    private final WXHorizontalScrollView scrollView;

    MyGestureDetector(WXHorizontalScrollView horizontalScrollView) {
      scrollView = horizontalScrollView;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
      int mItems = mChildren.size();
      try {
        //right to left
        if(e1.getX() - e2.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
          int featureWidth = pageSize;
          mActiveFeature = (mActiveFeature < (mItems - 1))? mActiveFeature + 1:mItems -1;
          scrollView.smoothScrollTo(mActiveFeature*featureWidth, 0);
          return true;
        }
        //left to right
        else if (e2.getX() - e1.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
          int featureWidth = pageSize;
          mActiveFeature = (mActiveFeature > 0)? mActiveFeature - 1:0;
          scrollView.smoothScrollTo(mActiveFeature*featureWidth, 0);
          return true;
        }
      } catch (Exception e) {
        WXLogUtils.e("There was an error processing the Fling event:" + e.getMessage());
      }
      return false;
    }
  }
}
