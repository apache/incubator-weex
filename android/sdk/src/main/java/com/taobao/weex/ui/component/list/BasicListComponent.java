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

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.PointF;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.support.v4.view.MotionEventCompat;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.text.TextUtils;
import android.util.SparseArray;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.ICheckBindingScroller;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.dom.ImmutableDomObject;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.AppearanceHelper;
import com.taobao.weex.ui.component.Scrollable;
import com.taobao.weex.ui.component.WXBaseRefresh;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXLoading;
import com.taobao.weex.ui.component.WXRefresh;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.helper.WXStickyHelper;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.ui.view.listview.adapter.IOnLoadMoreListener;
import com.taobao.weex.ui.view.listview.adapter.IRecyclerAdapterListener;
import com.taobao.weex.ui.view.listview.adapter.ListBaseViewHolder;
import com.taobao.weex.ui.view.listview.adapter.RecyclerViewBaseAdapter;
import com.taobao.weex.ui.view.listview.adapter.TransformItemDecoration;
import com.taobao.weex.ui.view.listview.adapter.WXRecyclerViewOnScrollListener;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by sospartan on 13/12/2016.
 */

public abstract class BasicListComponent<T extends ViewGroup & ListComponentView> extends WXVContainer<T> implements
    IRecyclerAdapterListener<ListBaseViewHolder>, IOnLoadMoreListener, Scrollable {
  public static final String TRANSFORM = "transform";
  public static final String LOADMOREOFFSET = "loadmoreoffset";
  private String TAG = "BasicListComponent";
  private int mListCellCount = 0;
  private boolean mForceLoadmoreNextTime = false;
  private ArrayList<ListBaseViewHolder> recycleViewList = new ArrayList<>();
  private static int visibleCellCount = 6;
  private static final Pattern transformPattern = Pattern.compile("([a-z]+)\\(([0-9\\.]+),?([0-9\\.]+)?\\)");

  private Map<String, AppearanceHelper> mAppearComponents = new HashMap<>();

  private boolean isScrollable = true;
  private ArrayMap<String, Long> mRefToViewType;
  private SparseArray<ArrayList<WXComponent>> mViewTypes;
  private WXRecyclerViewOnScrollListener mViewOnScrollListener = new WXRecyclerViewOnScrollListener(this);

  private static final int MAX_VIEWTYPE_ALLOW_CACHE = 9;
  private static boolean mAllowCacheViewHolder = true;
  private static boolean mDownForBidCacheViewHolder = false;


  protected int mLayoutType = WXRecyclerView.TYPE_LINEAR_LAYOUT;
  protected int mColumnCount = 1;
  protected float mColumnGap = 0;
  protected float mColumnWidth = 0;

  private int mOffsetAccuracy = 10;
  private Point mLastReport = new Point(-1, -1);

  private RecyclerView.ItemAnimator mItemAnimator;

  private DragHelper mDragHelper;

  /**
   * exclude cell when dragging(attributes for cell)
   */
  private static final String EXCLUDED = "dragExcluded";

  /**
   * the type to trigger drag-drop
   */
  private static final String DRAG_TRIGGER_TYPE = "dragTriggerType";

  private static final String DEFAULT_TRIGGER_TYPE = DragTriggerType.LONG_PRESS;
  private static final boolean DEFAULT_EXCLUDED = false;

  private static final String DRAG_ANCHOR = "dragAnchor";

  /**
   * gesture type which can trigger drag&drop
   */
  interface DragTriggerType {
    String PAN = "pan";
    String LONG_PRESS = "longpress";
  }

  private String mTriggerType;

  /**
   * Map for storing component that is sticky.
   **/
  private Map<String, Map<String, WXComponent>> mStickyMap = new HashMap<>();
  private WXStickyHelper stickyHelper;


  public BasicListComponent(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
    stickyHelper = new WXStickyHelper(this);
  }

  @Override
  protected void onHostViewInitialized(T host) {
    super.onHostViewInitialized(host);

    WXRecyclerView recyclerView = host.getInnerView();
    if (recyclerView == null || recyclerView.getAdapter() == null) {
      WXLogUtils.e(TAG, "RecyclerView is not found or Adapter is not bound");
      return;
    }

    if (mChildren == null) {
      WXLogUtils.e(TAG, "children is null");
      return;
    }

    mDragHelper = new DefaultDragHelper(mChildren, recyclerView, new EventTrigger() {
      @Override
      public void triggerEvent(String type, Map<String, Object> args) {
        fireEvent(type, args);
      }
    });

    mTriggerType = getTriggerType(getDomObject());
  }

  /**
   * Measure the size of the recyclerView.
   *
   * @param width  the expected width
   * @param height the expected height
   * @return the result of measurement
   */
  @Override
  protected MeasureOutput measure(int width, int height) {
    int screenH = WXViewUtils.getScreenHeight(WXEnvironment.sApplication);
    int weexH = WXViewUtils.getWeexHeight(getInstanceId());
    int outHeight = height > (weexH >= screenH ? screenH : weexH) ? weexH - getAbsoluteY() : height;
    return super.measure((int)(width+mColumnGap), outHeight);
  }

  public int getOrientation() {
    return Constants.Orientation.VERTICAL;
  }

  @Override
  public void destroy() {
    super.destroy();
    if (mStickyMap != null)
      mStickyMap.clear();
    if (mViewTypes != null)
      mViewTypes.clear();
    if (mRefToViewType != null)
      mRefToViewType.clear();
  }

  @Override
  public ViewGroup.LayoutParams getChildLayoutParams(WXComponent child, View hostView, int width, int height, int left, int right, int top, int bottom) {
    ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) hostView.getLayoutParams();
    if (child instanceof WXBaseRefresh && params == null) {
      params = new LinearLayout.LayoutParams(width, height);
    } else if (params == null) {
      params = new RecyclerView.LayoutParams(width, height);
    } else {
      params.width = width;
      params.height = height;
      params.setMargins(left, 0, right, 0);
    }
    return params;
  }

  /**
   * These transform functions are supported:
   * - `scale(x,y)`: scale item, x and y should be a positive float number.
   * - `translate(x,y)`: translate item, `x` and `y` shoule be integer numbers.
   * - `opacity(n)`: change the transparency of item, `n` must in `[0,1.0]`.
   * - `rotate(n)`: rotate item, n is integer number.
   *
   * @param raw
   * @return
   */
  private RecyclerView.ItemDecoration parseTransforms(String raw) {
    if (raw == null) {
      return null;
    }
    float scaleX = 0f, scaleY = 0f;
    int translateX = 0, translateY = 0;
    float opacity = 0f;
    int rotate = 0;
    //public TransformItemDecoration(boolean isVertical,float alpha,int translateX,int translateY,int rotation,float scale)
    Matcher matcher = transformPattern.matcher(raw);
    while (matcher.find()) {
      String match = matcher.group();
      String name = matcher.group(1);
      try {
        switch (name) {
          case "scale":
            scaleX = Float.parseFloat(matcher.group(2));
            scaleY = Float.parseFloat(matcher.group(3));
            break;
          case "translate":
            translateX = Integer.parseInt(matcher.group(2));
            translateY = Integer.parseInt(matcher.group(3));
            break;
          case "opacity":
            opacity = Float.parseFloat(matcher.group(2));
            break;
          case "rotate":
            rotate = Integer.parseInt(matcher.group(2));
            break;
          default:
            WXLogUtils.e(TAG, "Invaild transform expression:" + match);
            break;
        }
      } catch (NumberFormatException e) {
        WXLogUtils.e("", e);
        WXLogUtils.e(TAG, "Invaild transform expression:" + match);
      }
    }
    return new TransformItemDecoration(getOrientation() == Constants.Orientation.VERTICAL, opacity, translateX, translateY, rotate, scaleX, scaleY);
  }

  abstract T generateListView(Context context, int orientation);

  @Override
  protected T initComponentHostView(@NonNull Context context) {
    T bounceRecyclerView = generateListView(context, getOrientation());

    String transforms = (String) getDomObject().getAttrs().get(TRANSFORM);
    if (transforms != null) {
      bounceRecyclerView.getInnerView().addItemDecoration(parseTransforms(transforms));
    }

    mItemAnimator=bounceRecyclerView.getInnerView().getItemAnimator();

    RecyclerViewBaseAdapter recyclerViewBaseAdapter = new RecyclerViewBaseAdapter<>(this);
    recyclerViewBaseAdapter.setHasStableIds(true);
    bounceRecyclerView.setRecyclerViewBaseAdapter(recyclerViewBaseAdapter);
    bounceRecyclerView.setOverScrollMode(View.OVER_SCROLL_NEVER);
    bounceRecyclerView.getInnerView().clearOnScrollListeners();
    bounceRecyclerView.getInnerView().addOnScrollListener(mViewOnScrollListener);
    bounceRecyclerView.getInnerView().addOnScrollListener(new RecyclerView.OnScrollListener() {
      @Override
      public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
        super.onScrollStateChanged(recyclerView, newState);

        if (newState == RecyclerView.SCROLL_STATE_IDLE)
          recycleViewHolderList();

        List<OnWXScrollListener> listeners = getInstance().getWXScrollListeners();
        if (listeners != null && listeners.size() > 0) {
          for (OnWXScrollListener listener : listeners) {
            if (listener != null) {
              View topView = recyclerView.getChildAt(0);
              if (topView != null) {
                int y = topView.getTop();
                listener.onScrollStateChanged(recyclerView, 0, y, newState);
              }
            }
          }
        }
      }

      @Override
      public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
        super.onScrolled(recyclerView, dx, dy);
        List<OnWXScrollListener> listeners = getInstance().getWXScrollListeners();
        if (listeners != null && listeners.size() > 0) {
          try {
            for (OnWXScrollListener listener : listeners) {
              if (listener != null) {
                if (listener instanceof ICheckBindingScroller) {
                  if (((ICheckBindingScroller) listener).isNeedScroller(getRef(), null)) {
                    listener.onScrolled(recyclerView, dx, dy);
                  }
                } else {
                  listener.onScrolled(recyclerView, dx, dy);
                }
              }
            }
          } catch (Exception e) {
            e.printStackTrace();
          }
        }
      }
    });
    bounceRecyclerView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
      @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
      @Override
      public void onGlobalLayout() {
        T view;
        if ((view = getHostView()) == null)
          return;
        mViewOnScrollListener.onScrolled(view.getInnerView(), 0, 0);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
          view.getViewTreeObserver().removeOnGlobalLayoutListener(this);
        } else {
          view.getViewTreeObserver().removeGlobalOnLayoutListener(this);
        }
      }
    });
    return bounceRecyclerView;
  }

  private void recycleViewHolderList() {
    for (ListBaseViewHolder holder : recycleViewList) {
      if (holder != null
              && holder.getComponent() != null
              && !holder.getComponent().isUsing()) {
        holder.recycled();
      }
    }
    recycleViewList.clear();
  }

  @Override
  public void bindStickStyle(WXComponent component) {
    stickyHelper.bindStickStyle(component, mStickyMap);
  }

  @Override
  public void unbindStickStyle(WXComponent component) {
    stickyHelper.unbindStickStyle(component, mStickyMap);
  }

  private
  @Nullable
  WXComponent findDirectListChild(WXComponent comp) {
    WXComponent parent;
    if (comp == null || (parent = comp.getParent()) == null) {
      return null;
    }

    if (parent instanceof BasicListComponent) {
      return comp;
    }

    return findDirectListChild(parent);
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case LOADMOREOFFSET:
        return true;
      case Constants.Name.SCROLLABLE:
        boolean scrollable = WXUtils.getBoolean(param, true);
        setScrollable(scrollable);
        return true;
      case Constants.Name.OFFSET_ACCURACY:
        int accuracy = WXUtils.getInteger(param, 10);
        setOffsetAccuracy(accuracy);
        return true;
      case Constants.Name.DRAGGABLE:
        boolean draggable = WXUtils.getBoolean(param,false);
        setDraggable(draggable);
        return true;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.SCROLLABLE)
  public void setScrollable(boolean scrollable) {
    this.isScrollable = scrollable;
    WXRecyclerView inner = getHostView().getInnerView();
    if(inner != null) {
      inner.setScrollable(scrollable);
    }
  }

  @WXComponentProp(name = Constants.Name.OFFSET_ACCURACY)
  public void setOffsetAccuracy(int accuracy) {
    float real = WXViewUtils.getRealPxByWidth(accuracy, getInstance().getInstanceViewPortWidth());
    this.mOffsetAccuracy = (int) real;
  }

  @WXComponentProp(name = Constants.Name.DRAGGABLE)
  public void setDraggable(boolean isDraggable) {
    if (mDragHelper != null) {
      mDragHelper.setDraggable(isDraggable);
    }
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("set draggable : " + isDraggable);
    }
  }

  @Override
  public boolean isScrollable() {
    return isScrollable;
  }


  private void setAppearanceWatch(WXComponent component, int event, boolean enable) {
    AppearanceHelper item = mAppearComponents.get(component.getRef());
    if (item != null) {
      item.setWatchEvent(event, enable);
    } else if (!enable) {
      //Do nothing if disable target not exist.
    } else {
      WXComponent dChild = findDirectListChild(component);
      int index = mChildren.indexOf(dChild);
      if (index != -1) {
        item = new AppearanceHelper(component, index);
        item.setWatchEvent(event, true);
        mAppearComponents.put(component.getRef(), item);
      }
    }
  }

  @Override
  public void bindAppearEvent(WXComponent component) {
    setAppearanceWatch(component, AppearanceHelper.APPEAR, true);
  }

  @Override
  public void bindDisappearEvent(WXComponent component) {
    setAppearanceWatch(component, AppearanceHelper.DISAPPEAR, true);
  }

  @Override
  public void unbindAppearEvent(WXComponent component) {
    setAppearanceWatch(component, AppearanceHelper.APPEAR, false);
  }

  @Override
  public void unbindDisappearEvent(WXComponent component) {
    setAppearanceWatch(component, AppearanceHelper.DISAPPEAR, false);
  }

  @Override
  public void scrollTo(WXComponent component, Map<String, Object> options) {
    float offsetFloat = 0;
    boolean smooth = true;

    if (options != null) {
      String offsetStr = options.get(Constants.Name.OFFSET) == null ? "0" : options.get(Constants.Name.OFFSET).toString();
      smooth = WXUtils.getBoolean(options.get(Constants.Name.ANIMATED), true);
      if (offsetStr != null) {
        try {
          offsetFloat = WXViewUtils.getRealPxByWidth(Float.parseFloat(offsetStr), getInstance().getInstanceViewPortWidth());
        }catch (Exception e ){
          WXLogUtils.e("Float parseFloat error :"+e.getMessage());
        }
      }
    }

    final int offset = (int) offsetFloat;

    T bounceRecyclerView = getHostView();
    if (bounceRecyclerView == null) {
      return;
    }

    WXComponent parent = component;
    WXCell cell = null;
    while (parent != null) {
      if (parent instanceof WXCell) {
        cell = (WXCell) parent;
        break;
      }
      parent = parent.getParent();
    }

    if (cell != null) {
      final int pos = mChildren.indexOf(cell);
      if (pos == -1) {
        //Invalid position
        return;
      }

      final WXRecyclerView view = bounceRecyclerView.getInnerView();

      if (!smooth) {
        RecyclerView.LayoutManager layoutManager = view.getLayoutManager();
        if (layoutManager instanceof LinearLayoutManager) {
          //GridLayoutManager is also instance of LinearLayoutManager
          ((LinearLayoutManager) layoutManager).scrollToPositionWithOffset(pos, -offset);
        } else if (layoutManager instanceof StaggeredGridLayoutManager) {
          ((StaggeredGridLayoutManager) layoutManager).scrollToPositionWithOffset(pos, -offset);
        }
        //Any else?
      } else {
        view.smoothScrollToPosition(pos);
        if (offset != 0) {
          view.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
              if (newState == RecyclerView.SCROLL_STATE_IDLE) {
                if (getOrientation() == Constants.Orientation.VERTICAL) {
                  recyclerView.smoothScrollBy(0, offset);
                } else {
                  recyclerView.smoothScrollBy(offset, 0);
                }
                recyclerView.removeOnScrollListener(this);
              }
            }
          });
        }
      }
    }
  }

  @Override
  public void onBeforeScroll(int dx, int dy) {
    T bounceRecyclerView = getHostView();
    if (mStickyMap == null || bounceRecyclerView == null) {
      return;
    }
    Map<String, WXComponent> stickyMap = mStickyMap.get(getRef());
    if (stickyMap == null) {
      return;
    }
    Iterator<Map.Entry<String, WXComponent>> iterator = stickyMap.entrySet().iterator();
    Map.Entry<String, WXComponent> entry;
    WXComponent stickyComponent;
    int currentStickyPos = -1;
    while (iterator.hasNext()) {
      entry = iterator.next();
      stickyComponent = entry.getValue();

      if (stickyComponent != null && stickyComponent.getDomObject() != null
          && stickyComponent instanceof WXCell) {

        WXCell cell = (WXCell) stickyComponent;
        if (cell.getHostView() == null) {
          return;
        }

          RecyclerView.LayoutManager layoutManager;
          boolean beforeFirstVisibleItem = false;
          layoutManager = getHostView().getInnerView().getLayoutManager();
          if (layoutManager instanceof LinearLayoutManager || layoutManager instanceof GridLayoutManager) {
            int fVisible = ((LinearLayoutManager) layoutManager).findFirstVisibleItemPosition();
            int pos = mChildren.indexOf(cell);
            cell.setScrollPositon(pos);

            if (pos <= fVisible) {
              beforeFirstVisibleItem = true;
              if(pos > currentStickyPos) {
                currentStickyPos = pos;
              }
            }
          } else if(layoutManager instanceof StaggeredGridLayoutManager){
            int [] firstItems= new int[3];
            int fVisible = ((StaggeredGridLayoutManager) layoutManager).findFirstVisibleItemPositions(firstItems)[0];
            int pos = mChildren.indexOf(cell);

            if (pos <= fVisible) {
              beforeFirstVisibleItem = true;
            }
          }

          int[] location = new int[2];
          stickyComponent.getHostView().getLocationOnScreen(location);
          int[] parentLocation = new int[2];
          stickyComponent.getParentScroller().getView().getLocationOnScreen(parentLocation);

          int top = location[1] - parentLocation[1];

          boolean showSticky = beforeFirstVisibleItem && cell.getLocationFromStart() >= 0 && top <= 0 && dy >= 0;
          boolean removeSticky = cell.getLocationFromStart() <= 0 && top > 0 && dy <= 0;
          if (showSticky) {
            bounceRecyclerView.notifyStickyShow(cell);
          } else if (removeSticky) {
            bounceRecyclerView.notifyStickyRemove(cell);
          }
          cell.setLocationFromStart(top);
        }
    }

    if(currentStickyPos>=0){
      bounceRecyclerView.updateStickyView(currentStickyPos);
    }
  }

  @Override
  public int getScrollY() {
    T bounceRecyclerView = getHostView();
    return bounceRecyclerView == null ? 0 : bounceRecyclerView.getInnerView().getScrollY();
  }

  @Override
  public int getScrollX() {
    T bounceRecyclerView = getHostView();
    return bounceRecyclerView == null ? 0 : bounceRecyclerView.getInnerView().getScrollX();
  }

  /**
   * Append a child component to the end of list. This will not refresh the underlying
   * view immediately. The message of index of the inserted child is given to the adapter, and the
   * adapter will determine when to refresh. The default implementation of adapter will push the
   * message into a message and refresh the view in a period of time.
   *
   * @param child the inserted child
   */
  @Override
  public void addChild(WXComponent child) {
    addChild(child, -1);
  }

  @Override
  protected int getChildrenLayoutTopOffset() {
    return 0;
  }

  /**
   * @param child the inserted child
   * @param index the index of the child to be inserted.
   * @see #addChild(WXComponent)
   */
  @Override
  public void addChild(WXComponent child, int index) {
    super.addChild(child, index);

    if (child == null || index < -1) {
      return;
    }
    int count = mChildren.size();
    index = index >= count ? -1 : index;
    bindViewType(child);

    int adapterPosition = index == -1 ? mChildren.size() - 1 : index;
    T view = getHostView();
    if (view != null) {
      boolean isAddAnimation = false;
      ImmutableDomObject domObject = child.getDomObject();
      if (domObject != null) {
        Object attr = domObject.getAttrs().get(Constants.Name.INSERT_CELL_ANIMATION);
        if (Constants.Value.DEFAULT.equals(attr)) {
          isAddAnimation = true;
        }
      }
      if (isAddAnimation) {
        view.getInnerView().setItemAnimator(mItemAnimator);
      } else {
        view.getInnerView().setItemAnimator(null);
      }
      boolean isKeepScrollPosition =  false;
      if (child.getDomObject() != null) {
        Object attr = child.getDomObject().getAttrs().get(Constants.Name.KEEP_SCROLL_POSITION);
        if (WXUtils.getBoolean(attr, false) && index <= getChildCount() && index>-1) {
            isKeepScrollPosition = true;
        }
      }
      if (isKeepScrollPosition) {
        int last=((LinearLayoutManager)view.getInnerView().getLayoutManager()).findLastVisibleItemPosition();
        view.getInnerView().getLayoutManager().scrollToPosition(last);
        view.getRecyclerViewBaseAdapter().notifyItemInserted(adapterPosition);
      } else {
        view.getRecyclerViewBaseAdapter().notifyItemChanged(adapterPosition);
      }
    }
    relocateAppearanceHelper();
  }



  private void relocateAppearanceHelper() {
    Iterator<Map.Entry<String, AppearanceHelper>> iterator = mAppearComponents.entrySet().iterator();
    while (iterator.hasNext()) {
      Map.Entry<String, AppearanceHelper> item = iterator.next();
      AppearanceHelper value = item.getValue();
      WXComponent dChild = findDirectListChild(value.getAwareChild());
      int index = mChildren.indexOf(dChild);
      value.setCellPosition(index);
    }
  }


  /**
   * RecyclerView manage its children in a way that different from {@link WXVContainer}. Therefore,
   * {@link WXVContainer#addSubView(View, int)} is an empty implementation in {@link
   * com.taobao.weex.ui.view.listview.WXRecyclerView}
   */
  @Override
  protected void addSubView(View child, int index) {

  }

  /**
   * Remove the child from list. This method will use {@link
   * java.util.List#indexOf(Object)} to retrieve the component to be deleted. Like {@link
   * #addChild(WXComponent)}, this method will not refresh the view immediately, the adapter will
   * decide when to refresh.
   *
   * @param child the child to be removed
   */
  @Override
  public void remove(WXComponent child, boolean destroy) {
    int index = mChildren.indexOf(child);
    if (destroy) {
      child.detachViewAndClearPreInfo();
    }
    unBindViewType(child);

    T view = getHostView();
    if (view == null) {
      return;
    }

    boolean isRemoveAnimation = false;
    ImmutableDomObject domObject = child.getDomObject();
    if (domObject != null) {
      Object attr = domObject.getAttrs().get(Constants.Name.DELETE_CELL_ANIMATION);
      if (Constants.Value.DEFAULT.equals(attr)) {
        isRemoveAnimation = true;
      }
    }
    if (isRemoveAnimation) {
      view.getInnerView().setItemAnimator(mItemAnimator);
    } else {
      view.getInnerView().setItemAnimator(null);
    }

    view.getRecyclerViewBaseAdapter().notifyItemRemoved(index);
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(TAG, "removeChild child at " + index);
    }
    super.remove(child, destroy);
  }




  @Override
  public void computeVisiblePointInViewCoordinate(PointF pointF) {
    RecyclerView view = getHostView().getInnerView();
    pointF.set(view.computeHorizontalScrollOffset(), view.computeVerticalScrollOffset());
  }

  /**
   * Recycle viewHolder and its underlying view. This may because the view is removed or reused.
   * Either case, this method will be called.
   *
   * @param holder The view holder to be recycled.
   */
  @Override
  public void onViewRecycled(ListBaseViewHolder holder) {
    long begin = System.currentTimeMillis();

    holder.setComponentUsing(false);
    if(holder.canRecycled()) {
      recycleViewList.add(holder);

      // recycleViewList allowed max size
      int threshold = visibleCellCount >= 6 ? (visibleCellCount * 6) : (6*6);

      /**
       * Recycle cache{@link recycleViewList} when recycleViewList.size() > list max child count or threshold
       */
      if (recycleViewList.size() > getChildCount() + 1 || recycleViewList.size() >= threshold) {
        WXLogUtils.d(TAG, "Recycle holder list recycled : cache size is " + recycleViewList.size() +
                ", visibleCellCount is " + visibleCellCount + ", threshold is " + threshold +
                ", child count is " + getChildCount());
        recycleViewHolderList();
      }
    } else {
      WXLogUtils.w(TAG, "this holder can not be allowed to  recycled" );
    }
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(TAG, "Recycle holder " + (System.currentTimeMillis() - begin) + "  Thread:" + Thread.currentThread().getName());
    }
  }

  /**
   * Bind the component of the position to the holder. Then flush the view.
   *
   * @param holder   viewHolder, which holds reference to the view
   * @param position position of component in list
   */
  @Override
  public void onBindViewHolder(final ListBaseViewHolder holder, int position) {
    if (holder == null) return;
    holder.setComponentUsing(true);
    WXComponent component = getChild(position);
    if (component == null
        || (component instanceof WXRefresh)
        || (component instanceof WXLoading)
        || (component.getDomObject() != null && component.getDomObject().isFixed())
        ) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d(TAG, "Bind WXRefresh & WXLoading " + holder);
      }
      return;
    }

    if (holder.getComponent() != null && holder.getComponent() instanceof WXCell) {
      if(holder.isRecycled()) {
        holder.bindData(component);
        component.onRenderFinish(STATE_UI_FINISH);
      }
      if (mDragHelper == null || !mDragHelper.isDraggable()) {
        return;
      }
      mTriggerType = (mTriggerType == null) ? DEFAULT_TRIGGER_TYPE : mTriggerType;

      WXCell cell = (WXCell) holder.getComponent();
      boolean isExcluded = DEFAULT_EXCLUDED;
      if(cell.getDomObject() != null){
         WXAttr cellAttrs = cell.getDomObject().getAttrs();
         isExcluded = WXUtils.getBoolean(cellAttrs.get(EXCLUDED), DEFAULT_EXCLUDED);
      }

      mDragHelper.setDragExcluded(holder, isExcluded);

      //NOTICE: event maybe consumed by other views
      if (DragTriggerType.PAN.equals(mTriggerType)) {
        mDragHelper.setLongPressDragEnabled(false);

        WXComponent anchorComponent = findComponentByAnchorName(cell, DRAG_ANCHOR);

        if (anchorComponent != null && anchorComponent.getHostView() != null && !isExcluded) {
          View anchor = anchorComponent.getHostView();
          anchor.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
              if (MotionEventCompat.getActionMasked(event) == MotionEvent.ACTION_DOWN) {
                mDragHelper.startDrag(holder);
              }
              return true;
            }
          });
        } else {
          if (WXEnvironment.isApkDebugable()) {
            if(!isExcluded) {
              WXLogUtils.e(TAG, "[error] onBindViewHolder: the anchor component or view is not found");
            } else {
              WXLogUtils.d(TAG, "onBindViewHolder: position "+ position + " is drag excluded");
            }
          }
        }

      } else if (DragTriggerType.LONG_PRESS.equals(mTriggerType)) {
        mDragHelper.setLongPressDragEnabled(true);
      }
    }

  }

  protected void markComponentUsable(){
    for (WXComponent component : mChildren){
      component.setUsing(false);
    }
  }
  /**
   * Create an instance of {@link ListBaseViewHolder} for the given viewType (not for the given
   * index). This  markComponentUsable();method will look up for the first component that fits the viewType requirement and
   * doesn't be used. Then create the certain type of view, detach the view f[rom the component.
   *
   * @param parent   the ViewGroup into which the new view will be inserted
   * @param viewType the type of the new view
   * @return the created view holder.
   */
  @Override
  public ListBaseViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    if (mChildren != null) {
      if (mViewTypes == null)
        return createVHForFakeComponent(viewType);
      ArrayList<WXComponent> mTypes = mViewTypes.get(viewType);
      checkRecycledViewPool(viewType);
      if (mTypes == null)
        return createVHForFakeComponent(viewType);

      for (int i = 0; i < mTypes.size(); i++) {
        WXComponent component = mTypes.get(i);
        if (component == null
            || component.isUsing()) {
          continue;
        }
        if (component.getDomObject() != null && component.getDomObject().isFixed()) {
          return createVHForFakeComponent(viewType);
        } else {
          if (component instanceof WXCell) {
            if (component.getRealView() != null) {
              return new ListBaseViewHolder(component, viewType);
            } else {
              ((WXCell) component).lazy(false);
              component.createView();
              component.applyLayoutAndEvent(component);
              return new ListBaseViewHolder(component, viewType);
            }
          } else if (component instanceof WXBaseRefresh) {
            return createVHForRefreshComponent(viewType);
          } else {
            WXLogUtils.e(TAG, "List cannot include element except cell、header、fixed、refresh and loading");
            return createVHForFakeComponent(viewType);
          }
        }
      }
    }
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.e(TAG, "Cannot find request viewType: " + viewType);
    }
    return createVHForFakeComponent(viewType);
  }

  /**
   * Forbid ViewHolder cache if viewType > MAX_VIEWTYPE_ALLOW_CACHE
   *
   * @param viewType
   */
  private void checkRecycledViewPool(int viewType) {
    try {
      if (mViewTypes.size() > MAX_VIEWTYPE_ALLOW_CACHE)
        mAllowCacheViewHolder = false;

      if (mDownForBidCacheViewHolder)
        if (getHostView() != null && getHostView().getInnerView() != null)
          getHostView().getInnerView().getRecycledViewPool().setMaxRecycledViews(viewType, 0);

      if (!mDownForBidCacheViewHolder) {
        if (!mAllowCacheViewHolder) {
          if (getHostView() != null && getHostView().getInnerView() != null) {
            for (int i = 0; i < mViewTypes.size(); i++) {
              getHostView().getInnerView().getRecycledViewPool().setMaxRecycledViews(mViewTypes.keyAt(i), 0);
            }
            mDownForBidCacheViewHolder = true;
          }
        }
      }
    } catch (Exception e) {
      WXLogUtils.e(TAG, "Clear recycledViewPool error!");
    }
  }

  /**
   * Return the child component type. The type is defined by scopeValue in .we file.
   *
   * @param position the position of the child component.
   * @return the type of certain component.
   */
  @Override
  public int getItemViewType(int position) {
    return generateViewType(getChild(position));
  }

  @Nullable
  private WXComponent findComponentByAnchorName(@NonNull WXComponent root, @NonNull String anchorName) {
    long start = 0;
    if (WXEnvironment.isApkDebugable()) {
      start = System.currentTimeMillis();
    }

    Deque<WXComponent> deque = new ArrayDeque<>();
    deque.add(root);
    while (!deque.isEmpty()) {
      WXComponent curComponent = deque.removeFirst();
      ImmutableDomObject object = curComponent.getDomObject();
      if (object != null) {
        String isAnchorSet = WXUtils.getString(object.getAttrs().get(anchorName), null);

        //hit
        if (isAnchorSet != null && isAnchorSet.equals("true")) {
          if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d("dragPerf", "findComponentByAnchorName time: " + (System.currentTimeMillis() - start) + "ms");
          }
          return curComponent;
        }
      }
      if (curComponent instanceof WXVContainer) {
        WXVContainer container = (WXVContainer) curComponent;
        for (int i = 0, len = container.childCount(); i < len; i++) {
          WXComponent child = container.getChild(i);
          deque.add(child);
        }
      }
    }

    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("dragPerf", "findComponentByAnchorName elapsed time: " + (System.currentTimeMillis() - start) + "ms");
    }
    return null;

  }

  private String getTriggerType(@Nullable ImmutableDomObject domObject) {
    String triggerType = DEFAULT_TRIGGER_TYPE;
    if (domObject == null) {
      return triggerType;
    }
    triggerType = WXUtils.getString(domObject.getAttrs().get(DRAG_TRIGGER_TYPE), DEFAULT_TRIGGER_TYPE);
    if (!DragTriggerType.LONG_PRESS.equals(triggerType) && !DragTriggerType.PAN.equals(triggerType)) {
      triggerType = DEFAULT_TRIGGER_TYPE;
    }

    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(TAG, "trigger type is " + triggerType);
    }

    return triggerType;
  }


  /**
   * ViewType will be classified into {HashMap<Integer,ArrayList<Integer>> mViewTypes}
   *
   * @param component
   */
  private void bindViewType(WXComponent component) {
    int id = generateViewType(component);

    if (mViewTypes == null) {
      mViewTypes = new SparseArray<>();
    }

    ArrayList<WXComponent> mTypes = mViewTypes.get(id);

    if (mTypes == null) {
      mTypes = new ArrayList<>();
      mViewTypes.put(id, mTypes);
    }
    mTypes.add(component);
  }

  private void unBindViewType(WXComponent component) {
    int id = generateViewType(component);

    if (mViewTypes == null)
      return;
    ArrayList<WXComponent> mTypes = mViewTypes.get(id);
    if (mTypes == null)
      return;

    mTypes.remove(component);
  }

  /**
   * generate viewtype by component
   *
   * @param component
   * @return
   */
  private int generateViewType(WXComponent component) {
    long id;
    try {
      id = Integer.parseInt(component.getDomObject().getRef());
      String type = component.getDomObject().getAttrs().getScope();

      if (!TextUtils.isEmpty(type)) {
        if (mRefToViewType == null) {
          mRefToViewType = new ArrayMap<>();
        }
        if (!mRefToViewType.containsKey(type)) {
          mRefToViewType.put(type, id);
        }
        id = mRefToViewType.get(type);

      }
    } catch (RuntimeException e) {
      WXLogUtils.eTag(TAG, e);
      id = RecyclerView.NO_ID;
      WXLogUtils.e(TAG, "getItemViewType: NO ID, this will crash the whole render system of WXListRecyclerView");
    }
    return (int) id;
  }

  /**
   * Get child component num.
   *
   * @return return the size of {@link #mChildren} if mChildren is not empty, otherwise, return 0;
   */
  @Override
  public int getItemCount() {
    return getChildCount();
  }

  @Override
  public boolean onFailedToRecycleView(ListBaseViewHolder holder) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(TAG, "Failed to recycle " + holder);
    }
    return false;
  }

  @Override
  public long getItemId(int position) {
    long id;
    try {
      id = Long.parseLong(getChild(position).getDomObject().getRef());
    } catch (RuntimeException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
      id = RecyclerView.NO_ID;
    }
    return id;
  }

  @Override
  public void onLoadMore(int offScreenY) {
    try {
      String offset = getDomObject().getAttrs().getLoadMoreOffset();

      if (TextUtils.isEmpty(offset)) {
        offset = "0";
      }
      float offsetParsed = WXViewUtils.getRealPxByWidth(Integer.parseInt(offset),getInstance().getInstanceViewPortWidth());

      if (offScreenY < offsetParsed) {

        if (mListCellCount != mChildren.size()
            || mForceLoadmoreNextTime) {
          fireEvent(Constants.Event.LOADMORE);
          mListCellCount = mChildren.size();
          mForceLoadmoreNextTime = false;
        }
      }
    } catch (Exception e) {
      WXLogUtils.d(TAG + "onLoadMore :", e);
    }
  }

  @Override
  public void notifyAppearStateChange(int firstVisible, int lastVisible, int directionX, int directionY) {
    //notify appear state
    Iterator<AppearanceHelper> it = mAppearComponents.values().iterator();
    String direction = directionY > 0 ? Constants.Value.DIRECTION_UP :
        directionY < 0 ? Constants.Value.DIRECTION_DOWN : null;
    if (getOrientation() == Constants.Orientation.HORIZONTAL && directionX != 0) {
      direction = directionX > 0 ? Constants.Value.DIRECTION_LEFT : Constants.Value.DIRECTION_RIGHT;
    }

    if (mColumnCount > 0)
      visibleCellCount = (lastVisible - firstVisible) * mColumnCount;

    while (it.hasNext()) {
      AppearanceHelper item = it.next();
      WXComponent component = item.getAwareChild();

      if (!item.isWatch()) {
        continue;
      }

      boolean outOfVisibleRange = item.getCellPositionINScollable() < firstVisible || item.getCellPositionINScollable() > lastVisible;

      View view = component.getHostView();
      if (view == null) {
        continue;
      }

      boolean visible = (!outOfVisibleRange) && item.isViewVisible();

      int result = item.setAppearStatus(visible);
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("appear", "item " + item.getCellPositionINScollable() + " result " + result);
      }
      if (result == AppearanceHelper.RESULT_NO_CHANGE) {
        continue;
      }
      component.notifyAppearStateChange(result == AppearanceHelper.RESULT_APPEAR ? Constants.Event.APPEAR : Constants.Event.DISAPPEAR, direction);
    }
  }

  @NonNull
  private ListBaseViewHolder createVHForFakeComponent(int viewType) {
    FrameLayout view = new FrameLayout(getContext());
    view.setBackgroundColor(Color.WHITE);
    view.setLayoutParams(new FrameLayout.LayoutParams(0, 0));
    return new ListBaseViewHolder(view, viewType);
  }


  private ListBaseViewHolder createVHForRefreshComponent(int viewType) {
    FrameLayout view = new FrameLayout(getContext());
    view.setBackgroundColor(Color.WHITE);
    view.setLayoutParams(new FrameLayout.LayoutParams(1, 1));
    view.setVisibility(View.GONE);
    return new ListBaseViewHolder(view, viewType);
  }

  @JSMethod
  public void resetLoadmore() {
    mForceLoadmoreNextTime = true;
    mListCellCount = 0;
  }

  @Override
  public void addEvent(String type) {
    super.addEvent(type);
    if (Constants.Event.SCROLL.equals(type) && getHostView() != null && getHostView().getInnerView() != null) {
      WXRecyclerView innerView = getHostView().getInnerView();
      innerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
        private int offsetXCorrection, offsetYCorrection;
        private boolean mFirstEvent = true;

        @Override
        public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
          super.onScrolled(recyclerView, dx, dy);
//          WXLogUtils.e("SCROLL", dx + ", " + dy + ", " + recyclerView.computeHorizontalScrollRange()
//          + ", " + recyclerView.computeVerticalScrollRange()
//          + ", " + recyclerView.computeHorizontalScrollOffset()
//          + ", " + recyclerView.computeVerticalScrollOffset());

          int offsetX = recyclerView.computeHorizontalScrollOffset();
          int offsetY = recyclerView.computeVerticalScrollOffset();

          if (dx == 0 && dy == 0) {
            offsetXCorrection = offsetX;
            offsetYCorrection = offsetY;
            offsetX = 0;
            offsetY = 0;
          } else {
            offsetX = offsetX - offsetXCorrection;
            offsetY = offsetY - offsetYCorrection;
          }

          if (mFirstEvent) {
            //skip first event
            mFirstEvent = false;
            return;
          }

          RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
          if (!layoutManager.canScrollVertically()) {
            return;
          }

          if (shouldReport(offsetX, offsetY)) {
            fireScrollEvent(recyclerView, offsetX, offsetY);
          }
        }
      });
    }
  }

  private void fireScrollEvent(RecyclerView recyclerView, int offsetX, int offsetY) {
    offsetY = - calcContentOffset(recyclerView);
    int contentWidth = recyclerView.getMeasuredWidth() + recyclerView.computeHorizontalScrollRange();
    int contentHeight = 0;
    for (int i = 0; i < getChildCount(); i++) {
      WXComponent child = getChild(i);
      if (child != null) {
        contentHeight += child.getLayoutHeight();
      }
    }

    Map<String, Object> event = new HashMap<>(2);
    Map<String, Object> contentSize = new HashMap<>(2);
    Map<String, Object> contentOffset = new HashMap<>(2);

    contentSize.put(Constants.Name.WIDTH, WXViewUtils.getWebPxByWidth(contentWidth, getInstance().getInstanceViewPortWidth()));
    contentSize.put(Constants.Name.HEIGHT, WXViewUtils.getWebPxByWidth(contentHeight, getInstance().getInstanceViewPortWidth()));

    contentOffset.put(Constants.Name.X, - WXViewUtils.getWebPxByWidth(offsetX, getInstance().getInstanceViewPortWidth()));
    contentOffset.put(Constants.Name.Y, - WXViewUtils.getWebPxByWidth(offsetY, getInstance().getInstanceViewPortWidth()));
    event.put(Constants.Name.CONTENT_SIZE, contentSize);
    event.put(Constants.Name.CONTENT_OFFSET, contentOffset);

    fireEvent(Constants.Event.SCROLL, event);
  }

  private boolean shouldReport(int offsetX, int offsetY) {
    if (mLastReport.x == -1 && mLastReport.y == -1) {
      mLastReport.x = offsetX;
      mLastReport.y = offsetY;
      return true;
    }

    int gapX = Math.abs(mLastReport.x - offsetX);
    int gapY = Math.abs(mLastReport.y - offsetY);

    if (gapX >= mOffsetAccuracy || gapY >= mOffsetAccuracy) {
      mLastReport.x = offsetX;
      mLastReport.y = offsetY;
      return true;
    }

    return false;
  }



  private int calcContentOffset(RecyclerView recyclerView) {
    RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
    if (layoutManager instanceof LinearLayoutManager) {
      int firstVisibleItemPosition = ((LinearLayoutManager) layoutManager).findFirstVisibleItemPosition();
      View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);
      int offset = 0;
      for (int i=0;i<firstVisibleItemPosition;i++) {
        WXComponent child = getChild(i);
        if (child != null) {
          offset -= child.getLayoutHeight();
        }
      }

      if (layoutManager instanceof GridLayoutManager) {
        int spanCount = ((GridLayoutManager) layoutManager).getSpanCount();
        offset = offset / spanCount;
      }

      offset += firstVisibleView.getTop();
      return offset;
    } else if (layoutManager instanceof StaggeredGridLayoutManager) {
      int spanCount = ((StaggeredGridLayoutManager) layoutManager).getSpanCount();
      int firstVisibleItemPosition = ((StaggeredGridLayoutManager) layoutManager).findFirstVisibleItemPositions(null)[0];
      View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);

      int offset = 0;
      for (int i=0;i<firstVisibleItemPosition;i++) {
        WXComponent child = getChild(i);
        if (child != null) {
          offset -= child.getLayoutHeight();
        }
      }

      offset = offset / spanCount;
      offset += firstVisibleView.getTop();
      return offset;
    }
    return -1;
  }
}
