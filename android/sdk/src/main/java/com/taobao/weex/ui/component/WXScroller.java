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
package com.taobao.weex.ui.component;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.ICheckBindingScroller;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.component.helper.WXStickyHelper;
import com.taobao.weex.ui.view.IWXScroller;
import com.taobao.weex.ui.view.WXBaseRefreshLayout;
import com.taobao.weex.ui.view.WXHorizontalScrollView;
import com.taobao.weex.ui.view.WXScrollView;
import com.taobao.weex.ui.view.WXScrollView.WXScrollViewListener;
import com.taobao.weex.ui.view.refresh.wrapper.BaseBounceView;
import com.taobao.weex.ui.view.refresh.wrapper.BounceScrollerView;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

//import com.taobao.weex.ui.WXRecycleImageManager;

/**
 * Component for scroller. It also support features like
 * "appear", "disappear" and "sticky"
 */
@Component(lazyload = false)

public class WXScroller extends WXVContainer<ViewGroup> implements WXScrollViewListener,Scrollable {

  public static final String DIRECTION = "direction";
  protected int mOrientation = Constants.Orientation.VERTICAL;
  private List<WXComponent> mRefreshs=new ArrayList<>();
  private int mChildrenLayoutOffset = 0;//Use for offset children layout
  private boolean mForceLoadmoreNextTime = false;
  private int mOffsetAccuracy = 10;
  private Point mLastReport = new Point(-1, -1);

  public static class Creator implements ComponentCreator {
    public WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXScroller(instance,node,parent);
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

  private int mContentHeight = 0;

  private WXStickyHelper stickyHelper;
  private Handler handler=new Handler();

  private boolean isScrollable = true;

  @Deprecated
  public WXScroller(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance,dom,parent);
  }

  public WXScroller(WXSDKInstance instance, WXDomObject node,
                    WXVContainer parent) {
    super(instance, node, parent);
    stickyHelper = new WXStickyHelper(this);
  }

  /**
   * @return FrameLayout inner ScrollView
   */
  @Override
  public ViewGroup getRealView() {
    return mRealView;
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
    if(getHostView() == null)
      return null;
    if (getHostView() instanceof BounceScrollerView) {
      return ((BounceScrollerView) getHostView()).getInnerView();
    } else {
      return getHostView();
    }
  }

  @Override
  public void addEvent(String type) {
    super.addEvent(type);
    if (Constants.Event.SCROLL.equals(type) && getInnerView() != null) {
      if (getInnerView() instanceof WXScrollView) {
        ((WXScrollView) getInnerView()).addScrollViewListener(new WXScrollViewListener() {
          @Override
          public void onScrollChanged(WXScrollView scrollView, int x, int y, int oldx, int oldy) {
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
            if (shouldReport(x, y)) {
              fireScrollEvent(scrollView.getContentFrame(), x, y, oldx, oldy);
            }
          }
        });
      }
    }
  }

  private void fireScrollEvent(Rect contentFrame, int x, int y, int oldx, int oldy) {
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

    fireEvent(Constants.Event.SCROLL, event);
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
  protected void addSubView(View child, int index) {
    if (child == null || getRealView() == null) {
      return;
    }

    if (child instanceof WXBaseRefreshLayout) {
      return;
    }

    int count = getRealView().getChildCount();
    index = index >= count ? -1 : index;
    if (index == -1) {
      getRealView().addView(child);
    } else {
      getRealView().addView(child, index);
    }
  }

  @Override
  protected int getChildrenLayoutTopOffset() {
    return mChildrenLayoutOffset;
  }

  /**
   * Intercept refresh view and loading view
   */
  @Override
  public void addChild(WXComponent child, int index) {
    mChildrenLayoutOffset += child.getLayoutTopOffsetForSibling();
    if (child instanceof WXBaseRefresh) {
      if (!checkRefreshOrLoading(child)) {
        mRefreshs.add(child);
      }
    }

    super.addChild(child,index);
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
    if (getInnerView() != null && getInnerView() instanceof IWXScroller) {
      ((IWXScroller) getInnerView()).destroy();
    }
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

  @Override
  protected ViewGroup initComponentHostView(@NonNull Context context) {
    String scroll;
    if (getDomObject() == null || getDomObject().getAttrs().isEmpty()) {
      scroll = "vertical";
    } else {
      scroll = getDomObject().getAttrs().getScrollDirection();
    }

    ViewGroup host;
    if(("horizontal").equals(scroll)){
      mOrientation = Constants.Orientation.HORIZONTAL;
      WXHorizontalScrollView scrollView = new WXHorizontalScrollView(context);
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

      host = scrollView;
    }else{
      mOrientation = Constants.Orientation.VERTICAL;
      BounceScrollerView scrollerView = new BounceScrollerView(context, mOrientation, this);
      mRealView = new FrameLayout(context);
      WXScrollView innerView = scrollerView.getInnerView();
      innerView.addScrollViewListener(this);
      FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
      innerView.addView(mRealView, layoutParams);
      innerView.setVerticalScrollBarEnabled(true);
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
        if (result != null)
          setShowScrollbar(result);
        return true;
      case Constants.Name.SCROLLABLE:
        boolean scrollable = WXUtils.getBoolean(param, true);
        setScrollable(scrollable);
        return true;
      case Constants.Name.OFFSET_ACCURACY:
        int accuracy = WXUtils.getInteger(param, 10);
        setOffsetAccuracy(accuracy);
        return true;
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


  // TODO Need constrain, each container can only have one sticky child
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

    procAppear(0,0,0,0);//check current components appearance status.
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

    int viewYInScroller=component.getAbsoluteY() - getAbsoluteY();
    int viewXInScroller=component.getAbsoluteX() - getAbsoluteX();

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

  /**
   * Process event like appear and disappear
   */
  private void procAppear(int x, int y, int oldx,
                          int oldy) {
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
      boolean visible = helper.isViewVisible(false);

      int result = helper.setAppearStatus(visible);
      if (result != AppearanceHelper.RESULT_NO_CHANGE) {
        helper.getAwareChild().notifyAppearStateChange(result == AppearanceHelper.RESULT_APPEAR ? Constants.Event.APPEAR : Constants.Event.DISAPPEAR, direction);
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
      String offset = getDomObject().getAttrs().getLoadMoreOffset();
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
}
