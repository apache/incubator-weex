/**
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
package com.taobao.weex.ui.component.list.template;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Point;
import android.graphics.PointF;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Looper;
import android.os.MessageQueue;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.ICheckBindingScroller;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXCellDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXRecyclerDomObject;
import com.taobao.weex.dom.flex.CSSLayoutContext;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.ui.component.AppearanceHelper;
import com.taobao.weex.ui.component.Scrollable;
import com.taobao.weex.ui.component.WXBaseRefresh;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXLoading;
import com.taobao.weex.ui.component.WXRefresh;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.binding.Layouts;
import com.taobao.weex.ui.component.binding.Statements;
import com.taobao.weex.ui.component.helper.ScrollStartEndHelper;
import com.taobao.weex.ui.component.list.RecyclerTransform;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.ui.view.listview.adapter.IOnLoadMoreListener;
import com.taobao.weex.ui.view.listview.adapter.IRecyclerAdapterListener;
import com.taobao.weex.ui.view.listview.adapter.RecyclerViewBaseAdapter;
import com.taobao.weex.ui.view.listview.adapter.WXRecyclerViewOnScrollListener;
import com.taobao.weex.ui.view.refresh.wrapper.BounceRecyclerView;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

import static com.taobao.weex.common.Constants.Name.LOADMOREOFFSET;

/**
 * weex template list supported, high performance recycler-list
 * https://github.com/Hanks10100/weex-native-directive
 * Created by jianbai.gbj on 2017/8/17.
 */
@Component(lazyload = false)
public class WXRecyclerTemplateList extends WXVContainer<BounceRecyclerView> implements
        IRecyclerAdapterListener<TemplateViewHolder>, IOnLoadMoreListener, Scrollable {
    public static final String TAG = "WXRecyclerTemplateList";

    private static final String NAME_HAS_FIXED_SIZE = "hasFixedSize";
    private static final String NAME_ITEM_VIEW_CACHE_SIZE = "itemViewCacheSize";
    private static final String NAME_TEMPLATE_CACHE_SIZE = "templateCacheSize";


    private WXRecyclerDomObject mDomObject;
    protected int mLayoutType = WXRecyclerView.TYPE_LINEAR_LAYOUT;
    protected int mColumnCount = 1;
    protected float mColumnGap = 0;
    protected float mColumnWidth = 0;
    private float mPaddingLeft;
    private float mPaddingRight;

    private WXRecyclerViewOnScrollListener mViewOnScrollListener = new WXRecyclerViewOnScrollListener(this);
    private int mListCellCount = 0;
    private boolean mForceLoadmoreNextTime = false;
    private RecyclerView.ItemAnimator mItemAnimator;

    /**
     * default orientation
     * */
    private  int orientation = Constants.Orientation.VERTICAL;

    /**
     * offset reported
     * */
    private boolean isScrollable = true;
    private int mOffsetAccuracy = 10;
    private Point mLastReport = new Point(-1, -1);
    private boolean mHasAddScrollEvent = false;

    private JSONArray listData;
    private String listDataKey = Constants.Name.Recycler.LIST_DATA;
    private String listDataItemKey = null;
    private String listDataIndexKey = null;
    private ArrayMap<String, Integer> mTemplateViewTypes;



    private Map<String, WXCell> mTemplateSources;
    private String  listDataTemplateKey = Constants.Name.Recycler.SLOT_TEMPLATE_TYPE;
    private Runnable listUpdateRunnable;
    private ConcurrentHashMap<String, TemplateCache> mTemplatesCache;
    private int templateCacheSize = 2;


    /**
     * scroll start and scroll end event
     * */
    private ScrollStartEndHelper mScrollStartEndHelper;



    /**
     * sticky helper
     * */
    private TemplateStickyHelper mStickyHelper;


    /**
     * appear and disappear event managaer
     * */
    private ArrayMap<Integer, List<AppearanceHelper>> mAppearHelpers = new ArrayMap<>();

    /**
     * disappear event will be fire,
     * fist layer map key position,
     * send layer map key String ref
     * three layer map key view hash code, value is event arguments
     * */
    private ArrayMap<Integer, Map<String,Map<Integer, List<Object>>>> mDisAppearWatchList = new ArrayMap<>();

    private ArrayStack bindIngStackContext = new ArrayStack();
    private Map bindIngMapContext = new HashMap();

    public WXRecyclerTemplateList(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
        super(instance, node, parent);
        initRecyclerTemplateList(instance, node, parent);
    }

    private void initRecyclerTemplateList(WXSDKInstance instance, WXDomObject node, WXVContainer parent){
        if (node != null && node instanceof WXRecyclerDomObject) {
            mDomObject = (WXRecyclerDomObject) node;
            mDomObject.preCalculateCellWidth();
            mLayoutType = mDomObject.getLayoutType();
            updateRecyclerAttr();
        }
        mTemplateViewTypes = new ArrayMap<>();
        mTemplateViewTypes.put("", 0); //empty view, when template was not sended
        mTemplateSources = new HashMap<>();
        mTemplatesCache = new ConcurrentHashMap<>();
        mStickyHelper = new TemplateStickyHelper(this);
        orientation = mDomObject.getOrientation();
        listDataTemplateKey = WXUtils.getString(getDomObject().getAttrs().get(Constants.Name.Recycler.LIST_DATA_TEMPLATE_KEY), Constants.Name.Recycler.SLOT_TEMPLATE_TYPE);
        listDataItemKey = WXUtils.getString(getDomObject().getAttrs().get(Constants.Name.Recycler.LIST_DATA_ITEM), listDataItemKey);
        listDataIndexKey = WXUtils.getString(getDomObject().getAttrs().get(Constants.Name.Recycler.LIST_DATA_ITEM_INDEX), listDataIndexKey);
        if( getDomObject().getAttrs().get(Constants.Name.Recycler.LIST_DATA) instanceof  JSONArray) {
            JSONArray array = (JSONArray)getDomObject().getAttrs().get(Constants.Name.Recycler.LIST_DATA);
            if(array.size() > 0) {
                listData = array;
            }
        }
        long start = System.currentTimeMillis();
        if(mDomObject != null && mDomObject.getCellList() != null){
            for(int i=0; i<mDomObject.getCellList().size(); i++){
                addChild(DomTreeBuilder.buildTree(mDomObject.getCellList().get(i),  this));
            }
        }
        if(WXEnvironment.isApkDebugable()){
            WXLogUtils.d(TAG, "TemplateList BuildDomTree Used " + (System.currentTimeMillis() - start));
        }
    }

    @Override
    protected BounceRecyclerView initComponentHostView(@NonNull Context context) {
        final BounceRecyclerView bounceRecyclerView = new BounceRecyclerView(context,mLayoutType,mColumnCount,mColumnGap, getOrientation());
        WXAttr attrs = getDomObject().getAttrs();
        String transforms = (String) attrs.get(Constants.Name.TRANSFORM);
        if (transforms != null) {
            bounceRecyclerView.getInnerView().addItemDecoration(RecyclerTransform.parseTransforms(getOrientation(), transforms));
        }
        mItemAnimator = bounceRecyclerView.getInnerView().getItemAnimator();

        if(attrs.get(NAME_TEMPLATE_CACHE_SIZE) != null){
            templateCacheSize =  WXUtils.getInteger(attrs.get(NAME_TEMPLATE_CACHE_SIZE), templateCacheSize);
        }

        boolean hasFixedSize = false;
        int itemViewCacheSize = 2;
        if(attrs.get(NAME_ITEM_VIEW_CACHE_SIZE) != null){
            itemViewCacheSize = WXUtils.getNumberInt(getDomObject().getAttrs().get(NAME_ITEM_VIEW_CACHE_SIZE), itemViewCacheSize);
        }
        if(attrs.get(NAME_HAS_FIXED_SIZE) != null){
            hasFixedSize =  WXUtils.getBoolean(attrs.get(NAME_HAS_FIXED_SIZE), hasFixedSize);
        }
        RecyclerViewBaseAdapter recyclerViewBaseAdapter = new RecyclerViewBaseAdapter<>(this);
        recyclerViewBaseAdapter.setHasStableIds(true);
        bounceRecyclerView.getInnerView().setItemAnimator(null);
        if(itemViewCacheSize != 2) {
            bounceRecyclerView.getInnerView().setItemViewCacheSize(itemViewCacheSize);
        }
        if(bounceRecyclerView.getSwipeLayout()  != null){
            if(WXUtils.getBoolean(getDomObject().getAttrs().get("nestedScrollingEnabled"), false)) {
                bounceRecyclerView.getSwipeLayout().setNestedScrollingEnabled(true);
            }
        }
        bounceRecyclerView.getInnerView().setHasFixedSize(hasFixedSize);
        bounceRecyclerView.setRecyclerViewBaseAdapter(recyclerViewBaseAdapter);
        bounceRecyclerView.setOverScrollMode(View.OVER_SCROLL_NEVER);
        bounceRecyclerView.getInnerView().clearOnScrollListeners();
        bounceRecyclerView.getInnerView().addOnScrollListener(mViewOnScrollListener);
        bounceRecyclerView.getInnerView().addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(recyclerView, newState);
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
                BounceRecyclerView view;
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
        listUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                /**
                 * compute sticky position
                 * */
                if(mStickyHelper != null){
                    mStickyHelper.getStickyPositions().clear();
                    if(listData != null){
                        for(int i=0; i<listData.size(); i++){
                            WXCell cell = getSourceTemplate(i);
                            if(cell == null){
                                continue;
                            }
                            if(cell.isSticky()){
                                mStickyHelper.getStickyPositions().add(i);
                            }
                        }
                    }
                }
                if(getHostView() != null && getHostView().getRecyclerViewBaseAdapter() != null){
                    getHostView().getRecyclerViewBaseAdapter().notifyDataSetChanged();
                }
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.d(TAG, "WXTemplateList notifyDataSetChanged");
                }
            }
        };
        return bounceRecyclerView;
    }




    @Override
    protected void onHostViewInitialized(BounceRecyclerView host) {
        super.onHostViewInitialized(host);
        WXRecyclerView recyclerView = host.getInnerView();
        if (recyclerView == null || recyclerView.getAdapter() == null) {
            WXLogUtils.e(TAG, "RecyclerView is not found or Adapter is not bound");
            return;
        }
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


    @Override
    public void bindStickStyle(WXComponent component) {
        WXComponent  template = findParentType(component, WXCell.class);
        if(template == null){
            return;
        }
        if(listData == null || mStickyHelper == null){
            return;
        }
        if(!mStickyHelper.getStickyTypes().contains(template.getRef())){
            mStickyHelper.getStickyTypes().add(template.getRef());
            notifyUpdateList();
        }
    }

    @Override
    public void unbindStickStyle(WXComponent component) {
        WXComponent  template = findParentType(component, WXCell.class);
        if(template == null
                || listData == null
                || mStickyHelper == null){
            return;
        }
        if(mStickyHelper.getStickyTypes().contains(template.getRef())){
            mStickyHelper.getStickyTypes().remove(template.getRef());
            notifyUpdateList();
        }
    }

    private @Nullable WXCell findCell(WXComponent component) {
        if(component instanceof  WXCell){
            return (WXCell) component;
        }
        WXComponent parent;
        if (component == null || (parent = component.getParent()) == null) {
            return null;
        }
        return findCell(parent);
    }

    private void setAppearanceWatch(WXComponent component, int event, boolean enable) {
        if(listData == null
                || mAppearHelpers == null
                || TextUtils.isEmpty(component.getRef())){
            return;
        }
        WXComponent cell = findCell(component);
        int type = getCellItemType(cell);
        if(type <= 0){
            return;
        }
        List<AppearanceHelper>  mAppearListeners = mAppearHelpers.get(type);
        if(mAppearListeners == null){
            mAppearListeners = new ArrayList<>();
            mAppearHelpers.put(type, mAppearListeners);
        }
        AppearanceHelper item = null;
        for(AppearanceHelper mAppearListener : mAppearListeners){
            if(component.getRef().equals(mAppearListener.getAwareChild().getRef())){
                item = mAppearListener;
                break;
            }
        }
        if (item != null) {
            item.setWatchEvent(event, enable);
        }else {
            item = new AppearanceHelper(component,  type);
            item.setWatchEvent(event, enable);
            mAppearListeners.add(item);
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


    @JSMethod
    public void scrollTo(int position, Map<String, Object> options){
        if (position >= 0) {
            boolean smooth = true;
            if(options != null) {
                smooth = WXUtils.getBoolean(options.get(Constants.Name.ANIMATED), true);
            }

            final int pos = position;
            BounceRecyclerView bounceRecyclerView = getHostView();
            if (bounceRecyclerView == null) {
                return;
            }
            final WXRecyclerView view = bounceRecyclerView.getInnerView();
            view.scrollTo(smooth, pos, 0, getOrientation());
        }
    }


    @Override
    public void scrollTo(WXComponent component, Map<String, Object> options) {
        float offsetFloat = 0;
        boolean smooth = true;
        int position = -1;
        int typeIndex = -1;
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
            position = WXUtils.getNumberInt(options.get(Constants.Name.Recycler.CELL_INDEX), -1);
            typeIndex = WXUtils.getNumberInt(options.get(Constants.Name.Recycler.TYPE_INDEX), -1);
        }
        WXCell cell = findCell(component);
        if(typeIndex >= 0){
            if(listData != null && component.getRef() != null){
                int typePosition = 0;
                for(int i=0; i<listData.size(); i++){
                    WXCell template = getSourceTemplate(i);
                    if(template == null){
                        continue;
                    }
                    if(cell.getRef().equals(template.getRef())){
                        typePosition ++;
                    }
                    if(typePosition > typeIndex){
                        position = i;
                        break;
                    }
                }
                if(position < 0){
                    position = listData.size() - 1;
                }
            }
        }

        final int offset = (int) offsetFloat;
        BounceRecyclerView bounceRecyclerView = getHostView();
        if (bounceRecyclerView == null) {
            return;
        }
        if (position >= 0) {
            final int pos = position;
            final WXRecyclerView view = bounceRecyclerView.getInnerView();
            view.scrollTo(smooth, pos, offset, getOrientation());
        }
    }

    @Override
    public int getScrollY() {
        BounceRecyclerView bounceRecyclerView = getHostView();
        return bounceRecyclerView == null ? 0 : bounceRecyclerView.getInnerView().getScrollY();
    }

    @Override
    public int getScrollX() {
        BounceRecyclerView bounceRecyclerView = getHostView();
        return bounceRecyclerView == null ? 0 : bounceRecyclerView.getInnerView().getScrollX();
    }

    public int getOrientation() {
        return orientation;
    }

    @Override
    public boolean isScrollable() {
        return isScrollable;
    }



    @Override
    public void addChild(WXComponent child) {
        this.addChild(child, -1);
    }

    @Override
    protected int getChildrenLayoutTopOffset() {
        return 0;
    }

    @Override
    public void addChild(WXComponent child, int index) {
        if(!(child instanceof  WXCell)) {
            super.addChild(child, index);
        }
        if(child instanceof  WXBaseRefresh){
            return;
        }
        if(child instanceof WXCell){
            if(child.getDomObject() != null && child.getDomObject().getAttrs() != null){
                Object templateId = child.getDomObject().getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_TYPE);
                String key = WXUtils.getString(templateId, null);
                if(key != null){
                    if(child.getDomObject() instanceof  WXCellDomObject
                            && getDomObject() instanceof  WXRecyclerDomObject){
                        WXCellDomObject domObject = (WXCellDomObject) child.getDomObject();
                        domObject.setRecyclerDomObject((WXRecyclerDomObject) getDomObject());
                    }
                    mTemplateSources.put(key, (WXCell) child);
                    ensureSourceCellRenderWithData((WXCell)child);
                    if(mTemplateViewTypes.get(key) == null){
                        mTemplateViewTypes.put(key, mTemplateViewTypes.size());
                    }
                }
            }
            notifyUpdateList();
        }
    }



    /**
     * RecyclerView manage its children in a way that different from {@link WXVContainer}. Therefore,
     * {@link WXVContainer#addSubView(View, int)} is an empty implementation in {@link
     * com.taobao.weex.ui.view.listview.WXRecyclerView}
     */
    @Override
    public void addSubView(View child, int index) {

    }


    /**
     * all child is template, none need onCreate child except loading and refresh.
     * */
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
            setRefreshOrLoading(child);
        }
    }

    @Override
    public void remove(WXComponent child, boolean destroy) {
        removeFooterOrHeader(child);
        super.remove(child, destroy);
    }



    @Override
    public void computeVisiblePointInViewCoordinate(PointF pointF) {
        RecyclerView view = getHostView().getInnerView();
        pointF.set(view.computeHorizontalScrollOffset(), view.computeVerticalScrollOffset());
    }

    @Override
    protected boolean setProperty(String key, Object param) {
        switch (key) {
            case Constants.Name.Recycler.LIST_DATA:{
                     setListData(param);
                }
                return true;
            case Constants.Name.Recycler.LIST_DATA_ITEM:
                listDataItemKey = WXUtils.getString(param, listDataItemKey);
                return true;
            case Constants.Name.Recycler.LIST_DATA_ITEM_INDEX:
                listDataIndexKey = WXUtils.getString(param, listDataIndexKey);
                return true;
            case Constants.Name.Recycler.LIST_DATA_TEMPLATE_KEY:
            case Constants.Name.Recycler.SLOT_TEMPLATE_TYPE:
                listDataTemplateKey = WXUtils.getString(param, Constants.Name.Recycler.SLOT_TEMPLATE_TYPE);
                return true;
            case LOADMOREOFFSET:
                return true;
            case Constants.Name.SCROLLABLE:
                boolean scrollable = WXUtils.getBoolean(param, true);
                setScrollable(scrollable);
                return true;
            case Constants.Name.SHOW_SCROLLBAR:
                Boolean result = WXUtils.getBoolean(param,null);
                if (result != null)
                    setShowScrollbar(result);
                return true;
            case NAME_ITEM_VIEW_CACHE_SIZE:
                return true;
            case NAME_HAS_FIXED_SIZE:
                return true;
            case Constants.Name.OFFSET_ACCURACY:
                int accuracy = WXUtils.getInteger(param, 10);
                setOffsetAccuracy(accuracy);
                return true;
        }
        return super.setProperty(key, param);
    }


    @WXComponentProp(name = Constants.Name.OFFSET_ACCURACY)
    public void setOffsetAccuracy(int accuracy) {
        float real = WXViewUtils.getRealPxByWidth(accuracy, getInstance().getInstanceViewPortWidth());
        this.mOffsetAccuracy = (int) real;
    }


    private void updateRecyclerAttr(){
        mColumnCount = mDomObject.getColumnCount();
        mColumnGap = mDomObject.getColumnGap();
        mColumnWidth = mDomObject.getColumnWidth();
        mPaddingLeft =mDomObject.getPadding().get(Spacing.LEFT);
        mPaddingRight =mDomObject.getPadding().get(Spacing.RIGHT);
    }

    @WXComponentProp(name = Constants.Name.COLUMN_WIDTH)
    public void setColumnWidth(int columnCount)  {
        if(mDomObject.getColumnWidth() != mColumnWidth){
            updateRecyclerAttr();
            WXRecyclerView wxRecyclerView = getHostView().getInnerView();
            wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap, getOrientation());
        }
    }

    @WXComponentProp(name = Constants.Name.SHOW_SCROLLBAR)
    public void setShowScrollbar(boolean show) {
        if(getHostView() == null || getHostView().getInnerView() == null){
            return;
        }
        if (getOrientation() == Constants.Orientation.VERTICAL) {
            getHostView().getInnerView().setVerticalScrollBarEnabled(show);
        } else {
            getHostView().getInnerView().setHorizontalScrollBarEnabled(show);
        }
    }

    @WXComponentProp(name = Constants.Name.COLUMN_COUNT)
    public void setColumnCount(int columnCount){
        if(mDomObject.getColumnCount() != mColumnCount){
            updateRecyclerAttr();
            WXRecyclerView wxRecyclerView = getHostView().getInnerView();
            wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
        }
    }

    @WXComponentProp(name = Constants.Name.COLUMN_GAP)
    public void setColumnGap(float columnGap) throws InterruptedException {
        if(mDomObject.getColumnGap() != mColumnGap) {
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

    @JSMethod
    public void setListData(Object param){
        boolean update = listData != null &&  listData != param;
        if(param instanceof  JSONArray){
            listData = (JSONArray) param;
        }
        if(update){
            notifyUpdateList();
        }
    }

    @JSMethod
    public void  appendData(JSONArray arrayObject){
        if(listData == null){
            listData = new JSONArray();
        }
        if(arrayObject instanceof  JSONArray){
            listData.addAll(arrayObject);
        }
        notifyUpdateList();
    }

    @JSMethod
    public void  insertData(JSONObject data, int index){
        if(data == null){
            return;
        }
        if(listData == null || index >= listData.size()){
            return;
        }
        listData.add(index, data);
        notifyUpdateList();
    }

    @JSMethod
    public void  updateData(JSONObject data, int index){
        if(data == null){
            return;
        }
        if(listData == null || index >= listData.size()){
            return;
        }
        listData.set(index, data);
        notifyUpdateList();
    }

    @JSMethod
    public void  removeData(List<Integer> array){
        if(array == null || array.size() == 0){
            return;
        }
        int offset = 0;
        for(Integer index : array){
            if(listData == null
                    || index == null){
                return;
            }
            index -= offset;
            if(index < listData.size()){
                listData.remove((int)index);
                offset++;
            }
        }
        notifyUpdateList();
    }


    @JSMethod
    public void resetLoadmore() {
        mForceLoadmoreNextTime = true;
        mListCellCount = 0;
    }


    @Override
    public void updateProperties(Map<String, Object> props) {
        super.updateProperties(props);
        if(props.containsKey(Constants.Name.PADDING)
                || props.containsKey(Constants.Name.PADDING_LEFT)
                || props.containsKey(Constants.Name.PADDING_RIGHT)){

            if(mPaddingLeft !=mDomObject.getPadding().get(Spacing.LEFT)
                    || mPaddingRight !=mDomObject.getPadding().get(Spacing.RIGHT)) {
                updateRecyclerAttr();
                WXRecyclerView wxRecyclerView = getHostView().getInnerView();
                wxRecyclerView.initView(getContext(), mLayoutType, mColumnCount, mColumnGap, getOrientation());
            }
        }
    }



    @Override
    public void addEvent(String type) {
        super.addEvent(type);
        if (ScrollStartEndHelper.isScrollEvent(type)
                && getHostView() != null
                && getHostView().getInnerView() != null
                && !mHasAddScrollEvent) {
            mHasAddScrollEvent = true;
            WXRecyclerView innerView = getHostView().getInnerView();
            innerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
                private int offsetXCorrection, offsetYCorrection;
                private boolean mFirstEvent = true;

                @Override
                public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
                    super.onScrolled(recyclerView, dx, dy);
                    RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
                    if (!layoutManager.canScrollVertically()) {
                        return;
                    }
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
                    getScrollStartEndHelper().onScrolled(offsetX, offsetY);
                    if(!getDomObject().getEvents().contains(Constants.Event.SCROLL)){
                        return;
                    }
                    if (mFirstEvent) {
                        //skip first event
                        mFirstEvent = false;
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
        fireEvent(Constants.Event.SCROLL, getScrollEvent(recyclerView, offsetX, offsetY));
    }

    public Map<String, Object> getScrollEvent(RecyclerView recyclerView, int offsetX, int offsetY){
        offsetY = -calcContentOffset(recyclerView);
        int contentWidth = recyclerView.getMeasuredWidth() + recyclerView.computeHorizontalScrollRange();
        int contentHeight = calcContentSize();

        Map<String, Object> event = new HashMap<>(2);
        Map<String, Object> contentSize = new HashMap<>(2);
        Map<String, Object> contentOffset = new HashMap<>(2);

        contentSize.put(Constants.Name.WIDTH, WXViewUtils.getWebPxByWidth(contentWidth, getInstance().getInstanceViewPortWidth()));
        contentSize.put(Constants.Name.HEIGHT, WXViewUtils.getWebPxByWidth(contentHeight, getInstance().getInstanceViewPortWidth()));

        contentOffset.put(Constants.Name.X, - WXViewUtils.getWebPxByWidth(offsetX, getInstance().getInstanceViewPortWidth()));
        contentOffset.put(Constants.Name.Y, - WXViewUtils.getWebPxByWidth(offsetY, getInstance().getInstanceViewPortWidth()));
        event.put(Constants.Name.CONTENT_SIZE, contentSize);
        event.put(Constants.Name.CONTENT_OFFSET, contentOffset);
        return event;
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


    /**
     * Setting refresh view and loading view
     *
     * @param child the refresh_view or loading_view
     */
    private boolean setRefreshOrLoading(final WXComponent child) {
        if (child instanceof WXRefresh && getHostView() != null) {
            getHostView().setOnRefreshListener((WXRefresh) child);
            getHostView().postDelayed(WXThread.secure(new Runnable() {
                @Override
                public void run() {
                    getHostView().setHeaderView(child);
                }
            }), 100);
            return true;
        }

        if (child instanceof WXLoading && getHostView() != null) {
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


    private void removeFooterOrHeader(WXComponent child) {
        if (child instanceof WXLoading) {
            getHostView().removeFooterView(child);
        } else if (child instanceof WXRefresh) {
            getHostView().removeHeaderView(child);
        }
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


    @Override
    public void destroy() {
        if(getHostView() != null){
            getHostView().removeCallbacks(listUpdateRunnable);
            if(getHostView().getInnerView() != null){
                getHostView().getInnerView().setAdapter(null);
            }
        }
        if(listData != null){
            listData = null;
        }
        if(mStickyHelper != null){
            mStickyHelper = null;
        }
        if(mTemplateViewTypes != null){
            mTemplateViewTypes.clear();
        }
        if(mTemplateSources != null){
            mTemplateSources.clear();
        }
        if(mAppearHelpers != null){
            mAppearHelpers.clear();
        }
        if(mDisAppearWatchList != null){
            mDisAppearWatchList.clear();
        }
        super.destroy();
    }



    @Override
    public void onViewRecycled(TemplateViewHolder holder) {}

    @Override
    public void onBindViewHolder(final TemplateViewHolder templateViewHolder, int position) {
        if(templateViewHolder == null){
            return;
        }
        WXCell component = templateViewHolder.getTemplate();
        if(component == null){
            return;
        }
        long start = System.currentTimeMillis();
        boolean resuse = templateViewHolder.getHolderPosition() >= 0;
        templateViewHolder.setHolderPosition(position);
        Object data = listData.get(position);
        if(component.getRenderData() == data){
            component.setHasLayout(true);
        }else{
            List<WXComponent> updates = Statements.doRender(component, getStackContextForPosition(position, data));
            Statements.doInitCompontent(updates);
            component.setRenderData(data);
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.d(TAG, position + getTemplateKey(position) + " onBindViewHolder render used " + (System.currentTimeMillis() - start));
            }
            if(component.isHasLayout()){
                resuse = true;
            }
            Layouts.doLayoutAsync(templateViewHolder, true);
            component.setHasLayout(true);
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.d(TAG,  position + getTemplateKey(position) + " onBindViewHolder layout used " + (System.currentTimeMillis() - start) + resuse);
            }
        }
    }

    @Override
    public TemplateViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        String template = mTemplateViewTypes.keyAt(viewType);
        WXCell source = mTemplateSources.get(template);
        if(source == null){
            FrameLayout view = new FrameLayout(getContext());
            view.setLayoutParams(new FrameLayout.LayoutParams(0, 0));
            return new TemplateViewHolder(view, viewType);
        }
        TemplateCache cache = mTemplatesCache.get(template);
        WXCell component =  null;
        boolean cacheHit = true;
        if(cache != null && cache.cells != null && cache.cells.size() > 0){
            component = cache.cells.poll();
        }
        if(cache == null ||  !cache.isLoadIng){
            asyncLoadTemplateCache(template);
        }
        if(component == null){
            cacheHit = false;
            if(!source.isSourceUsed()){
                source.setSourceUsed(true);
                ensureSourceCellRenderWithData(source);
                component = source;
                if(WXEnvironment.isApkDebugable()) {
                    WXLogUtils.d(TAG, template + " onCreateViewHolder source");
                }
            }
        }
        if(component == null) {
            long start = System.currentTimeMillis();
            component = (WXCell) copyCell(source);
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder copy used " + (System.currentTimeMillis() - start));
            }
        }
        if(component.isLazy()) {
            doInitLazyCell(component, template, false);
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder  cache hit " + cacheHit   + " idle init false ");
            }
        }else{
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder  cache hit " + cacheHit + " idle init true");
            }
        }
        TemplateViewHolder templateViewHolder = new TemplateViewHolder(component, viewType);
        return  templateViewHolder;
    }


    /**
     * copy cell component from source, init render data, and return source
     * if none data, return null
     * */
    private WXComponent copyCell(WXCell cell){
        ensureSourceCellRenderWithData(cell);
        WXCell component = (WXCell) Statements.copyComponentTree(cell);
        if(component.getDomObject() instanceof  WXCellDomObject
                && getDomObject() instanceof  WXRecyclerDomObject){
            WXCellDomObject domObject = (WXCellDomObject) component.getDomObject();
            domObject.setRecyclerDomObject((WXRecyclerDomObject) getDomObject());
        }
        component.setRenderData(cell.getRenderData());
        return component;
    }

    private void ensureSourceCellRenderWithData(WXCell cell){
        if(cell.getRenderData() == null){
            if(listData != null && listData.size() > 0){
                synchronized (this){
                    if(cell.getRenderData() == null){
                        for(int i=0; i<listData.size(); i++){
                            if(cell == getSourceTemplate(i)){
                                Object data = listData.get(i);
                                Statements.doRender(cell, getStackContextForPosition(i, data));
                                Layouts.doSafeLayout(cell, new CSSLayoutContext());
                                cell.setRenderData(data);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @param position
     * when template not send, return an invalid id, use empty view holder.
     * when template has sended, use real template id to refresh view, use real view holder.
     * */
    @Override
    public int getItemViewType(int position) {
        JSONObject data = safeGetListData(position);
        String template = data.getString(listDataTemplateKey);
        if(TextUtils.isEmpty(template)){
            template = "";
        }
        int type =  mTemplateViewTypes.indexOfKey(template);
        if(type < 0){
            type = 0;
        }
        return type;
    }


    /**
     * return code context for render component
     * */
    private ArrayStack getStackContextForPosition(int position,  Object item){
        if(!bindIngStackContext.isEmpty()){
            bindIngStackContext.getList().clear();
        }
        if(!bindIngMapContext.isEmpty()){
            bindIngMapContext.clear();
        }
        ArrayStack stack = bindIngStackContext;
        Map map = bindIngMapContext;
        if(listData != null){
            stack.push(listData);
            stack.push(map);
            map.put(listDataKey, listData);
            if(!TextUtils.isEmpty(listDataIndexKey)) {
                map.put(listDataIndexKey, position);
            }
            if(!TextUtils.isEmpty(listDataItemKey)) {
                map.put(listDataItemKey, item);
            }else{
                stack.push(item);
            }
        }
        return stack;
    }

    /**
     * return tepmlate key for position
     * */
    public String getTemplateKey(int position){
        JSONObject data =  safeGetListData(position);
        String template = data.getString(listDataTemplateKey);
        if(TextUtils.isEmpty(template)){
            template = "";
        }
        return  template;
    }

    /**
     * get source template
     * */
    public WXCell getSourceTemplate(int position){
        String template = getTemplateKey(position);
        return mTemplateSources.get(template);
    }



    /**
     * get template key from cell; 0  for default type
     * */
    private int getCellItemType(WXComponent cell){
        if(cell == null){
            return  -1;
        }
        if(cell.getDomObject() != null && cell.getDomObject().getAttrs() != null){
            Object templateId = cell.getDomObject().getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_TYPE);
            String template = WXUtils.getString(templateId, null);
            if(template == null){
                return  0;
            }
            int type =  mTemplateViewTypes.indexOfKey(template);
            if(type < 0){
                return -1;
            }
            return  type;
        }
        return  0;
    }

    @Override
    public int getItemCount() {
        if(listData == null){
            return  0;
        }
        if(mTemplateViewTypes == null || mTemplateViewTypes.size() <= 1){
            return 0;
        }
        if(mTemplateSources == null || mTemplateSources.size() == 0){
            return  0;
        }
        return listData.size();
    }

    @Override
    public boolean onFailedToRecycleView(TemplateViewHolder holder) {
        return false;
    }


    /**
     * @param position
     * when template not send by javascript, return an invalid id, force  use empty view holder.
     * when template has sended by javascript, use real template id to refresh view, use real view holder.
     * */
    @Override
    public long getItemId(int position) {
        if(getItemViewType(position) <= 0){
            return RecyclerView.NO_ID;
        }
        JSONObject data = safeGetListData(position);
        if(data.containsKey(Constants.Name.Recycler.LIST_DATA_ITEM_ID)) {
            String itemKey = data.getString(Constants.Name.Recycler.LIST_DATA_ITEM_ID);
            if(TextUtils.isEmpty(itemKey)){
                return  position;
            }
            return  itemKey.hashCode();
        }
        return position;
    }

    @Override
    public void onBeforeScroll(int dx, int dy) {
        if(mStickyHelper != null){
            mStickyHelper.onBeforeScroll(dx, dy);
        }
    }

    @Override
    public void onLoadMore(int offScreenY) {
        try {
            String offset = getDomObject().getAttrs().getLoadMoreOffset();

            if (TextUtils.isEmpty(offset)) {
                offset = "0";
            }
            float offsetParsed = WXViewUtils.getRealPxByWidth(Integer.parseInt(offset),getInstance().getInstanceViewPortWidth());

            if (offScreenY <= offsetParsed && listData != null) {
                if (mListCellCount != listData.size()
                        || mForceLoadmoreNextTime) {
                    fireEvent(Constants.Event.LOADMORE);
                    mListCellCount = listData.size();
                    mForceLoadmoreNextTime = false;
                }
            }
        } catch (Exception e) {
            WXLogUtils.d(TAG + "onLoadMore :", e);
        }
    }

    /**
     *
     * first fire appear event.
     * */
    @Override
    public void notifyAppearStateChange(int firstVisible, int lastVisible, int directionX, int directionY) {
        if(mAppearHelpers == null
                || mAppearHelpers.size() <= 0){
            return;
        }
        String direction = directionY > 0 ? Constants.Value.DIRECTION_UP :
                directionY < 0 ? Constants.Value.DIRECTION_DOWN : null;
        if (getOrientation() == Constants.Orientation.HORIZONTAL && directionX != 0) {
            direction = directionX > 0 ? Constants.Value.DIRECTION_LEFT : Constants.Value.DIRECTION_RIGHT;
        }
        RecyclerView recyclerView = getHostView().getInnerView();
        for(int position=firstVisible; position<=lastVisible; position++){
            int type = getItemViewType(position);
            List<AppearanceHelper> helpers = mAppearHelpers.get(type);
            if(helpers == null){
                continue;
            }
            for(AppearanceHelper helper : helpers){
                if(!helper.isWatch()){
                    continue;
                }
                TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(position);
                if(itemHolder == null || itemHolder.getComponent() == null){
                    break;
                }
                List<WXComponent> childListeners = findChildListByRef(itemHolder.getComponent(), helper.getAwareChild().getRef());
                if(childListeners == null || childListeners.size() == 0){
                    break;
                }

                Map<String, Map<Integer, List<Object>>> disAppearList = mDisAppearWatchList.get(position);
                if(disAppearList == null){
                    disAppearList = new ArrayMap<>();
                    mDisAppearWatchList.put(position, disAppearList);
                }

                Map<Integer, List<Object>> componentDisAppearList = disAppearList.get(helper.getAwareChild().getRef());
                if(componentDisAppearList == null){
                    componentDisAppearList = new ArrayMap<>();
                    disAppearList.put(helper.getAwareChild().getRef(), componentDisAppearList);
                }

                for(int m=0; m<childListeners.size(); m++){
                    WXComponent childLisener = childListeners.get(m);
                    if(childLisener.getHostView() == null){
                        continue;
                    }
                    boolean appear = helper.isViewVisible(childLisener.getHostView());
                    int key = childLisener.getHostView().hashCode();
                    if(appear){
                        if(!componentDisAppearList.containsKey(key)){
                            childLisener.notifyAppearStateChange(Constants.Event.APPEAR, direction);
                            List<Object> eventArgs = null;
                            if(childLisener.getDomObject().getEvents() != null
                                    && childLisener.getDomObject().getEvents().getEventBindingArgsValues() != null
                                    && childLisener.getDomObject().getEvents().getEventBindingArgsValues().get(Constants.Event.DISAPPEAR) != null){
                                eventArgs = childLisener.getDomObject().getEvents().getEventBindingArgsValues().get(Constants.Event.DISAPPEAR);
                            }
                            componentDisAppearList.put(key, eventArgs);
                        }
                    }else{
                        if(componentDisAppearList.containsKey(key)){
                            childLisener.notifyAppearStateChange(Constants.Event.DISAPPEAR, direction);
                            componentDisAppearList.remove(key);
                        }
                    }
                }
            }
        }

        //handle disappear event, out of position
        int count = getItemCount();
        for (int position=0; position<count; position++){
            if(position >= firstVisible && position <= lastVisible){
                position = lastVisible + 1;
                continue;
            }
            Map<String, Map<Integer, List<Object>>> map = mDisAppearWatchList.get(position);
            if(map == null){
                continue;
            }
            WXCell template = mTemplateSources.get(getTemplateKey(position));
            if(template == null){
                return;
            }
            Set<Map.Entry<String, Map<Integer, List<Object>>>> cellWatcherEntries = map.entrySet();
            for(Map.Entry<String,Map<Integer, List<Object>>> cellWatcherEntry : cellWatcherEntries){
                String ref = cellWatcherEntry.getKey();
                WXComponent component = findChildByRef(template, ref);
                if(component == null){
                    continue;
                }
                Map<Integer, List<Object>> eventWatchers = cellWatcherEntry.getValue();
                if(eventWatchers == null || eventWatchers.size() == 0){
                    continue;
                }
                WXEvent events = component.getDomObject().getEvents();
                Set<Map.Entry<Integer, List<Object>>> eventWatcherEntries = eventWatchers.entrySet();
                for(Map.Entry<Integer, List<Object>> eventWatcherEntry : eventWatcherEntries){
                    events.putEventBindingArgsValue(Constants.Event.DISAPPEAR, eventWatcherEntry.getValue());
                    component.notifyAppearStateChange(Constants.Event.DISAPPEAR, direction);
                }
                eventWatchers.clear();
            }
            mDisAppearWatchList.remove(position);
        }
    }


    private JSONObject safeGetListData(int position){
        try{
            return listData.getJSONObject(position);
        }catch (Exception e){return  JSONObject.parseObject("{}");}
    }

    private void  notifyUpdateList(){
        if(getHostView() == null
                || getHostView().getInnerView() == null
                || listUpdateRunnable == null){
            return;
        }
        if(Looper.getMainLooper().getThread().getId() != Thread.currentThread().getId()){
            getHostView().removeCallbacks(listUpdateRunnable);
            getHostView().post(listUpdateRunnable);
        }else{
            listUpdateRunnable.run();
        }
    }

    private int calcContentSize() {
        int totalHeight = 0;
        if(listData == null){
            return totalHeight;
        }
        for (int i = 0; i < listData.size(); i++) {
            WXCell child = getSourceTemplate(i);
            if (child != null) {
                totalHeight += child.getLayoutHeight();
            }
        }
        return totalHeight;
    }

    public int calcContentOffset(RecyclerView recyclerView) {
        RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
        if (layoutManager instanceof LinearLayoutManager) {
            int firstVisibleItemPosition = ((LinearLayoutManager) layoutManager).findFirstVisibleItemPosition();
            int offset = 0;
            for (int i=0;i<firstVisibleItemPosition;i++) {
                WXCell cell = getSourceTemplate(i);
                if(cell == null){
                    continue;
                }
                offset -= cell.getLayoutHeight();
            }

            if (layoutManager instanceof GridLayoutManager) {
                int spanCount = ((GridLayoutManager) layoutManager).getSpanCount();
                offset = offset / spanCount;
            }
            View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);
            if(firstVisibleView != null) {
                offset += firstVisibleView.getTop();
            }
            return offset;
        } else if (layoutManager instanceof StaggeredGridLayoutManager) {
            int spanCount = ((StaggeredGridLayoutManager) layoutManager).getSpanCount();
            int firstVisibleItemPosition = ((StaggeredGridLayoutManager) layoutManager).findFirstVisibleItemPositions(null)[0];
            int offset = 0;
            for (int i=0;i<firstVisibleItemPosition;i++) {
                WXCell cell = getSourceTemplate(i);
                if(cell == null){
                    continue;
                }
                offset -= cell.getLayoutHeight();
            }
            offset = offset / spanCount;

            View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);
            if(firstVisibleView != null) {
                offset += firstVisibleView.getTop();
            }
            return offset;
        }
        return -1;
    }

    /**
     * find certain class type parent
     * */
    public  WXComponent findParentType(WXComponent component, Class type){
        if(component.getClass() == type){
            return component;
        }
        if(component.getParent() != null) {
            findTypeParent(component.getParent(), type);
        }
        return  null;
    }


    /**
     * find child by ref
     * */
    public WXComponent findChildByRef(WXComponent component, String ref){
        if(ref.equals(component.getRef())){
            return component;
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent child = findChildByRef(container.getChild(i), ref);
                if(child != null){
                    return  child;
                }
            }
        }
        return  null;
    }

    /**
     * find child list, has same ref
     * */
    public List<WXComponent> findChildListByRef(WXComponent component, String ref){
        WXComponent child = findChildByRef(component, ref);
        if(child == null){
            return  null;
        }
        List<WXComponent> componentList = new ArrayList<>();
        WXVContainer container = child.getParent();
        if(container != null && (!(container instanceof  WXRecyclerTemplateList))){
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent   element = container.getChild(i);
                if(ref.equals(element.getRef())){
                    componentList.add(element);
                }
            }
        }else{
            componentList.add(child);
        }
        return  componentList;
    }


    /**
     * copy cell async and save to cache
     * */
    private void asyncLoadTemplateCache(final String template) {
        if(Thread.currentThread() != Looper.getMainLooper().getThread()){
            if(listData == null || listData.size() == 0){
                return;
            }
            boolean firstScreenContains = false;
            for(int i=0; i<listData.size(); i++){
                if(template.equals(getTemplateKey(i))){
                    firstScreenContains = true;
                    break;
                }
            }
            if(!firstScreenContains){
                return;
            }
        }
        final WXCell source = mTemplateSources.get(template);
        if(source == null){
            return;
        }
        TemplateCache cellCache = mTemplatesCache.get(template);
        if(cellCache == null){
            cellCache = new TemplateCache();
            mTemplatesCache.put(template, cellCache);
        }
        if(cellCache.cells.size() > 0){
            cellCache.isLoadIng = false;
            return;
        }
        if(cellCache.isLoadIng){
            return;
        }
        cellCache.isLoadIng = true;
        AsyncTask<Void,Void, Void> preloadTask = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                TemplateCache cellCache = mTemplatesCache.get(template);
                if(cellCache == null || cellCache.cells == null){
                    return null;
                }
                while (cellCache.cells.size() < templateCacheSize){
                    WXCell component = (WXCell) copyCell(source);
                    if(component == null){
                        return null;
                    }
                    if(source.getInstance() == null || source.getInstance().isDestroy()){
                        return null;
                    }
                    cellCache.cells.add(component);
                }
                return null;
            }
            @Override
            protected void onPostExecute(Void aVoid) {
                if(source.getInstance() == null || source.getInstance().isDestroy()){
                    return;
                }
                final TemplateCache cellCache = mTemplatesCache.get(template);
                if(cellCache == null){
                    return;
                }
                if(cellCache.cells == null
                        || cellCache.cells.size() == 0){
                    cellCache.isLoadIng = false;
                    return;
                }
                Looper.myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
                    @Override
                    public boolean queueIdle() {
                        if(source.getInstance() == null || source.getInstance().isDestroy()){
                            return false;
                        }
                        ConcurrentLinkedQueue<WXCell> queue =  cellCache.cells;
                        Iterator<WXCell> iterator =  queue.iterator();
                        while (iterator.hasNext()){
                            WXCell  component =  iterator.next();
                            if(component.isLazy()){
                                doInitLazyCell(component, template, true);
                                return iterator.hasNext();
                            }
                        }
                        return false;
                    }
                });
                cellCache.isLoadIng = false;
            }
        };
        preloadTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private static void  doInitLazyCell(WXCell component, String template, boolean inPreload){
        if(component.isLazy()){
            long start = System.currentTimeMillis();
            component.lazy(false);
            component.createView();
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG,  "doInitLazyCell idle" + inPreload + template +  " createView used " + (System.currentTimeMillis() - start));
            }
            component.applyLayoutAndEvent(component);
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG,  "doInitLazyCell idle" + inPreload  + template +  " apply layout used " + (System.currentTimeMillis() - start));
            }
            component.bindData(component);
            if(WXEnvironment.isApkDebugable()) {
                WXLogUtils.d(TAG, "doInitLazyCell idle" + inPreload + template + " bindData used " + (System.currentTimeMillis() - start));
            }
        }
    }

    public ScrollStartEndHelper getScrollStartEndHelper() {
        if(mScrollStartEndHelper == null){
            mScrollStartEndHelper = new ScrollStartEndHelper(this);
        }
        return mScrollStartEndHelper;
    }
}
