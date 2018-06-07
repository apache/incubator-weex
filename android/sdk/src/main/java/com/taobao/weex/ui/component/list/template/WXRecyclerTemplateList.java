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
import android.os.Build;
import android.os.Looper;
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
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.ui.action.BasicComponentData;
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
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import static com.taobao.weex.common.Constants.Name.LOADMOREOFFSET;

/**
 * weex template list supported, high performance recycler-list
 * https://github.com/Hanks10100/weex-native-directive
 * Created by jianbai.gbj on 2017/8/17.
 */
@Component(lazyload = false)
public class WXRecyclerTemplateList extends WXVContainer<BounceRecyclerView> implements
        IRecyclerAdapterListener<TemplateViewHolder>, IOnLoadMoreListener, Scrollable {

    /**
     * trace log for template list
     * */
    public static final boolean ENABLE_TRACE_LOG = true;

    public static final String TAG = "WXRecyclerTemplateList";

    private static final String EMPTY_HOLDER_TEMPLATE_KEY  = "";
    private static final String NAME_HAS_FIXED_SIZE = "hasFixedSize";
    private static final String NAME_ITEM_VIEW_CACHE_SIZE = "itemViewCacheSize";
    private static final String NAME_TEMPLATE_CACHE_SIZE = "templateCacheSize";

    // TODO
//    private WXRecyclerDomObject mDomObject;
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


    private CellDataManager cellDataManager;
    private String listDataKey = Constants.Name.Recycler.LIST_DATA;
    private String listDataItemKey = null;
    private String listDataIndexKey = null;
    private ArrayMap<String, Integer> mTemplateViewTypes;


    private Map<String, WXCell> mTemplateSources;
    private String  listDataTemplateKey = Constants.Name.Recycler.SLOT_TEMPLATE_CASE;
    private Runnable listUpdateRunnable;
    private ConcurrentHashMap<String, TemplateCache> mTemplatesCache;
    private int templateCacheSize = 2;


    /**
     * case default cell and key
     * */
    private WXCell defaultTemplateCell;
    private String defaultTemplateKey = "@default_template_cell";

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

    private CellRenderContext cellRenderContext = new CellRenderContext();

    private Runnable mAppearChangeRunnable = null;
    private static final long APPEAR_CHANGE_RUNNABLE_DELAY = 50;

    /**
     * has append tree done
     * */
    private boolean hasAppendTreeDone = false;

    /**
     * has layout done
     * */
    private boolean hasLayoutDone = false;

    public WXRecyclerTemplateList(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
        super(instance, parent, basicComponentData);
        initRecyclerTemplateList(instance, basicComponentData, parent);
    }

    private void initRecyclerTemplateList(WXSDKInstance instance, BasicComponentData basicComponentData,
                                          WXVContainer parent){

        updateRecyclerAttr();

        mTemplateViewTypes = new ArrayMap<>();
        mTemplateViewTypes.put(EMPTY_HOLDER_TEMPLATE_KEY, 0); //empty view, when template was not sended
        mTemplateSources = new HashMap<>();
        mTemplatesCache = new ConcurrentHashMap<>();
        mStickyHelper = new TemplateStickyHelper(this);
        orientation =  basicComponentData.getAttrs().getOrientation();
        listDataTemplateKey = WXUtils.getString(getAttrs().get(Constants.Name.Recycler.LIST_DATA_TEMPLATE_SWITCH_KEY), Constants.Name.Recycler.SLOT_TEMPLATE_CASE);
        listDataItemKey = WXUtils.getString(getAttrs().get(Constants.Name.Recycler.LIST_DATA_ITEM), listDataItemKey);
        listDataIndexKey = WXUtils.getString(getAttrs().get(Constants.Name.Recycler.LIST_DATA_ITEM_INDEX), listDataIndexKey);
        cellDataManager = new CellDataManager(this);
        cellDataManager.listData = parseListDataToJSONArray(getAttrs().get(Constants.Name.Recycler.LIST_DATA));
        /**
         * we have separate cell with list, post add cell in dom thread ensure
         * list has layout and can archive better user experience and better load time,
         * which reduce waste cell layout when list layout changes.
         * */
        // TODO
//        if(mDomObject != null
//                && mDomObject.getCellList() != null
//                && mDomObject.getCellList().size() > 0){
//            Runnable runnable =  new Runnable() {
//                // @Override
//                public void run() {
//                    if(isDestoryed()){
//                        return;
//                    }
//                    long start = System.currentTimeMillis();
//                    if(mDomObject != null && mDomObject.getCellList() != null){
//                        for(int i=0; i<mDomObject.getCellList().size(); i++){
//                            addChild(ComponentUtils.buildTree(mDomObject.getCellList().get(i),  WXRecyclerTemplateList.this));
//                        }
//                    }
//                    if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG){
//                        WXLogUtils.d(TAG, "TemplateList BuildDomTree Used " + (System.currentTimeMillis() - start));
//                    }
//                }
//            };
//            WXSDKManager.getInstance().getWXDomManager().post(runnable);
//        }
    }

    @Override
    protected BounceRecyclerView initComponentHostView(@NonNull Context context) {
        final BounceRecyclerView bounceRecyclerView = new BounceRecyclerView(context,mLayoutType,mColumnCount,mColumnGap, getOrientation());
        WXAttr attrs = getAttrs();
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
            itemViewCacheSize = WXUtils.getNumberInt(getAttrs().get(NAME_ITEM_VIEW_CACHE_SIZE), itemViewCacheSize);
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
            if(WXUtils.getBoolean(getAttrs().get("nestedScrollingEnabled"), false)) {
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
                    if(mStickyHelper.getStickyTypes().size() > 0){
                        mStickyHelper.getStickyPositions().clear();
                        if(cellDataManager.listData != null){
                            for(int i = 0; i< cellDataManager.listData.size(); i++){
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
                }
                if(getHostView() != null && getHostView().getRecyclerViewBaseAdapter() != null){
                    getHostView().getRecyclerViewBaseAdapter().notifyDataSetChanged();
                }
                if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG){
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
        WXComponent template = findParentType(component, WXCell.class);
        if(template == null){
            return;
        }
        if(mStickyHelper == null){
            return;
        }
        if(!mStickyHelper.getStickyTypes().contains(template.getRef())){
            mStickyHelper.getStickyTypes().add(template.getRef());
            notifyUpdateList();
        }
    }

    @Override
    public void unbindStickStyle(WXComponent component) {
        WXComponent template = findParentType(component, WXCell.class);
        if(template == null
                || mStickyHelper == null){
            return;
        }
        if(mStickyHelper.getStickyTypes().contains(template.getRef())){
            mStickyHelper.getStickyTypes().remove(template.getRef());
            notifyUpdateList();
        }
    }

    private @Nullable
    WXCell findCell(WXComponent component) {
        if(component instanceof WXCell){
            return (WXCell) component;
        }
        WXComponent parent;
        if (component == null || (parent = component.getParent()) == null) {
            return null;
        }
        return findCell(parent);
    }

    private void setAppearanceWatch(WXComponent component, int event, boolean enable) {
        if(cellDataManager.listData == null
                || mAppearHelpers == null
                || TextUtils.isEmpty(component.getRef())){
            return;
        }
        WXCell cell = findCell(component);
        int type = getCellTemplateItemType(cell);
        if(type < 0){
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
        if(mAppearChangeRunnable == null){
            mAppearChangeRunnable =  new Runnable() {
                @Override
                public void run() {
                    if(mAppearChangeRunnable != null) {
                        notifyAppearStateChange(0, 0, 0, 0);
                    }
                }
            };
        }
        if (getHostView() != null) {
            getHostView().removeCallbacks(mAppearChangeRunnable);
            getHostView().postDelayed(mAppearChangeRunnable, APPEAR_CHANGE_RUNNABLE_DELAY);
        }
    }

    @Override
    public void bindDisappearEvent(WXComponent component) {
        setAppearanceWatch(component, AppearanceHelper.DISAPPEAR, true);
        if(mAppearChangeRunnable == null){
            mAppearChangeRunnable =  new Runnable() {
                @Override
                public void run() {
                    if(mAppearChangeRunnable != null) {
                        notifyAppearStateChange(0, 0, 0, 0);
                    }
                }
            };
        }
        if (getHostView() != null) {
            getHostView().removeCallbacks(mAppearChangeRunnable);
            getHostView().postDelayed(mAppearChangeRunnable, APPEAR_CHANGE_RUNNABLE_DELAY);
        }
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
            if(cellDataManager.listData != null && component.getRef() != null){
                int typePosition = 0;
                for(int i = 0; i< cellDataManager.listData.size(); i++){
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
                    position = cellDataManager.listData.size() - 1;
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
        /**
         *  dom object in component is not tree, build tree
         * */
        if(!(child instanceof WXCell)) {
            super.addChild(child, index);
        }
        if(child instanceof WXBaseRefresh){
            return;
        }
        if(child instanceof WXCell){
            if(child.getAttrs() != null){
                Object templateId = child.getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE);
                String key = WXUtils.getString(templateId, null);
                if(getAttrs().containsKey(Constants.Name.Recycler.LIST_DATA_TEMPLATE_SWITCH_KEY)){
                    if(defaultTemplateCell == null){
                        defaultTemplateCell = (WXCell) child;
                        if(!TextUtils.isEmpty(key)){
                            defaultTemplateKey = key;
                        }else{
                            key = defaultTemplateKey;
                            child.getAttrs().put(Constants.Name.Recycler.SLOT_TEMPLATE_CASE, key);
                        }
                    }
                }else{
                    if(defaultTemplateCell == null
                            || child.getAttrs().containsKey(Constants.Name.Recycler.SLOT_TEMPLATE_DEFAULT)){
                        defaultTemplateCell = (WXCell) child;
                        if(!TextUtils.isEmpty(key)){
                            defaultTemplateKey = key;
                        }else{
                            key = defaultTemplateKey;
                            child.getAttrs().put(Constants.Name.Recycler.SLOT_TEMPLATE_CASE, key);
                        }
                    }
                }
                if(key != null){
                    // TODO
//                    if(child.getDomObject() instanceof  WXCellDomObject
//                            && getDomObject() instanceof  WXRecyclerDomObject){
//                        WXCellDomObject domObject = (WXCellDomObject) child.getDomObject();
//                        domObject.setRecyclerDomObject((WXRecyclerDomObject) getDomObject());
//                    }
                    mTemplateSources.put(key, (WXCell) child);
                    if(mTemplateViewTypes.get(key) == null){
                        mTemplateViewTypes.put(key, mTemplateViewTypes.size());
                    }
                }
            }

            ((WXCell) child).setCellAppendTreeListener(new WXCell.CellAppendTreeListener() {
                @Override
                public void onAppendTreeDone() {
                    checkAppendDone(false);
                }
            });
        }
    }


    /**
     * check all the cell has append tree done, then show list
     * */
    private void  checkAppendDone(boolean listDone){
        if(mTemplateSources.size() == 0){
            return;
        }
        Set<Map.Entry<String,WXCell>> cells = mTemplateSources.entrySet();
        for(Map.Entry<String,WXCell> entry : cells){
            if(!entry.getValue().isAppendTreeDone()){
                return;
            }
        }
        hasAppendTreeDone = true;
        if(hasLayoutDone) {
            notifyUpdateList();
        }
    }

    @Override
    protected void setHostLayoutParams(BounceRecyclerView host, int width, int height, int left, int right, int top, int bottom) {
        super.setHostLayoutParams(host, width, height, left, right, top, bottom);
        if(!hasLayoutDone){
            hasLayoutDone = true;
            hasAppendTreeDone = true;
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
            case Constants.Name.Recycler.LIST_DATA_TEMPLATE_SWITCH_KEY:
            case Constants.Name.Recycler.SLOT_TEMPLATE_CASE:
                listDataTemplateKey = WXUtils.getString(param, Constants.Name.Recycler.SLOT_TEMPLATE_CASE);
                return true;
            case LOADMOREOFFSET:
                return true;
            case Constants.Name.SCROLLABLE:
                boolean scrollable = WXUtils.getBoolean(param, true);
                setScrollable(scrollable);
                return true;
            case Constants.Name.SCROLL_DIRECTION:
                if(param != null) {
                    setScrollDirection(param.toString());
                }
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
        mLayoutType = getAttrs().getLayoutType();
        mColumnCount = getAttrs().getColumnCount();
        mColumnGap = getAttrs().getColumnGap();
        mColumnWidth = getAttrs().getColumnWidth();
        mPaddingLeft = getPadding().get(CSSShorthand.EDGE.LEFT);
        mPaddingRight = getPadding().get(CSSShorthand.EDGE.RIGHT);

    }


    @WXComponentProp(name = Constants.Name.SCROLL_DIRECTION)
    public void setScrollDirection(String direction){
        if(orientation != getAttrs().getOrientation()) {
            orientation = getAttrs().getOrientation();
            updateRecyclerAttr();
            WXRecyclerView wxRecyclerView = getHostView().getInnerView();
            wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap, getOrientation());

        }
    }

    @WXComponentProp(name = Constants.Name.COLUMN_WIDTH)
    public void setColumnWidth(int columnCount)  {
        if(getAttrs().getColumnWidth() != mColumnWidth){
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
        if(getAttrs().getColumnCount() != mColumnCount){
            updateRecyclerAttr();
            WXRecyclerView wxRecyclerView = getHostView().getInnerView();
            wxRecyclerView.initView(getContext(), mLayoutType,mColumnCount,mColumnGap,getOrientation());
        }
    }

    @WXComponentProp(name = Constants.Name.COLUMN_GAP)
    public void setColumnGap(float columnGap) throws InterruptedException {
        if(getAttrs().getColumnGap() != mColumnGap) {
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
        param = parseListDataToJSONArray(param);
        boolean update = cellDataManager.listData != param;
        if(param instanceof  JSONArray){
            if(update){
                cellDataManager.setListData((JSONArray) param);
                notifyUpdateList();
            }
        }
    }

    @JSMethod
    public void  appendData(JSONArray data){
        if(data == null || data.size() == 0){
            return;
        }
        if(cellDataManager.listData == null){
            cellDataManager.listData = new JSONArray();
        }
        int position = cellDataManager.listData.size();
        if(position < 0){
            position = 0;
        }
        if(data instanceof  JSONArray){
            cellDataManager.listData.addAll(data);
        }
        getHostView().getRecyclerViewBaseAdapter().notifyItemRangeInserted(position, data.size());
    }

    @JSMethod
    public void  insertData(int index, Object data){
        if(data == null){
            return;
        }

        if(cellDataManager.listData == null || index > cellDataManager.listData.size()){
            return;
        }
        boolean renderStateChanged = cellDataManager.insertData(index, data);
        if(renderStateChanged){
            notifyUpdateList();
        }else{
            getHostView().getRecyclerViewBaseAdapter().notifyItemInserted(index);
        }
    }

    @JSMethod
    public void  appendRange(int index, JSONArray data){
         insertRange(index, data);
    }


    /**
     * when update data, list maybe contains sticky, may use position, when position changed should be rendered
     * so use notifyUpdateList is better
     * */

    @JSMethod
    public void  insertRange(int index, JSONArray data){
        if(data == null || data.size() == 0){
            return;
        }
        if(cellDataManager.listData == null || index > cellDataManager.listData.size()){
            return;
        }
        boolean renderStateChange = cellDataManager.insertRange(index, data);
        if(renderStateChange){
            notifyUpdateList();
        }else{
            getHostView().getRecyclerViewBaseAdapter().notifyItemRangeInserted(index, data.size());
        }

    }

    @JSMethod
    public void  updateData(int index, Object data){
        if(data == null){
            return;
        }
        if(cellDataManager.listData == null || index >= cellDataManager.listData.size()){
            return;
        }
        boolean onlyDataChange = cellDataManager.updateData(data, index);
        if(onlyDataChange) {
            getHostView().getRecyclerViewBaseAdapter().notifyItemChanged(index, data);
        }else{
            notifyUpdateList();
        }
    }

    @JSMethod
    public void  removeData(int index, int count){
        if(cellDataManager.listData == null
                || index >= cellDataManager.listData.size()){
            return;
        }
        if(count <= 0){
            count = 1;
        }
        int removeCount = 0;
        while (count > 0 && index < cellDataManager.listData.size()){
            cellDataManager.removeData(index);
            count--;
            removeCount++;
        }
        if(removeCount > 0) {
            notifyUpdateList();
        }
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

            if(mPaddingLeft != getPadding().get(CSSShorthand.EDGE.LEFT)
                    || mPaddingRight != getPadding().get(CSSShorthand.EDGE.RIGHT)) {
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
                    if(!getEvents().contains(Constants.Event.SCROLL)){
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
            if(mAppearChangeRunnable != null) {
                getHostView().removeCallbacks(mAppearChangeRunnable);
                mAppearChangeRunnable = null;
            }
            getHostView().removeCallbacks(listUpdateRunnable);
            if(getHostView().getInnerView() != null){
                getHostView().getInnerView().setAdapter(null);
            }
        }
        if(cellDataManager.listData != null){
            cellDataManager.setListData(null);
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
        templateViewHolder.setHolderPosition(position);
        Object data = cellDataManager.listData.get(position);
        CellRenderState cellRenderState = cellDataManager.getRenderState(position);
        if((component.getRenderData() == data && (cellRenderState == null || !cellRenderState.isDirty()))){
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG){
                WXLogUtils.d(TAG,  position + " position "+ getTemplateKey(position) + " onBindViewHolder none data update "
                        + " component " + component.hashCode());
            }
            return;  //none update just return
        }else{
            List<WXComponent> updates = doRenderTemplate(component, position);
            Statements.doInitCompontent(updates);
            component.setRenderData(data);
            Layouts.doLayoutAsync(templateViewHolder, true);
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG){
                WXLogUtils.d(TAG,  position + " position "+ getTemplateKey(position) + " onBindViewHolder used " + (System.currentTimeMillis() - start)
                  + " component " + component.hashCode());
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
            return new TemplateViewHolder(this, view, viewType);
        }
        WXCell component =  getCellTemplateFromCache(template);
        boolean cacheHit = true;
        if(component == null){
            cacheHit = false;
            if(!source.isSourceUsed()){
                source.setSourceUsed(true);
                renderTemplateCellWithData(source);
                component = source;
                if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG) {
                    WXLogUtils.d(TAG, template + " onCreateViewHolder source");
                }
            }
        }
        if(component == null) {
            long start = System.currentTimeMillis();
            component = (WXCell) copyComponentFromSourceCell(source);
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder copy used " + (System.currentTimeMillis() - start));
            }
        }
        if(component.isLazy() || component.getHostView() == null) {
            doCreateCellViewBindData(component, template, false);
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder  cache hit " + cacheHit   + " view not idle init");
            }
        }else{
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG) {
                WXLogUtils.d(TAG, template + " onCreateViewHolder  cache hit " + cacheHit + " view idle init " + component.hashCode()
                 + "  " + source.hashCode());
            }
        }
        TemplateViewHolder templateViewHolder = new TemplateViewHolder(this, component, viewType);
        return  templateViewHolder;
    }



    /**
     * @param position
     * when template not send, return an invalid id, use empty view holder.
     * when template has sended, use real template id to refresh view, use real view holder.
     * */
    @Override
    public int getItemViewType(int position) {
        String template = getTemplateKey(position);
        int type =  mTemplateViewTypes.indexOfKey(template);
        if(type < 0){
            type = mTemplateViewTypes.indexOfKey(EMPTY_HOLDER_TEMPLATE_KEY);
        }
        return type;
    }


    /**
     * create code context for render component and do render
     * */
    private List<WXComponent> doRenderTemplate(WXCell cell, int position){
        this.cellRenderContext.clear();
        Object item = cellDataManager.listData.get(position);
        CellRenderState cellRenderState = cellDataManager.getRenderState(position);
        cellRenderContext.renderState = cellRenderState;
        cellRenderContext.templateList = this;
        cellRenderContext.position = position;

        ArrayStack stack = cellRenderContext.stack;
        Map map = cellRenderContext.map;
        if(cellDataManager.listData != null){
            stack.push(map);
            map.put(listDataKey, cellDataManager.listData);
            if(!TextUtils.isEmpty(listDataIndexKey)) {
                map.put(listDataIndexKey, new PositionRef(cellRenderState));
            }
            if(!TextUtils.isEmpty(listDataItemKey)) {
                map.put(listDataItemKey, item);
            }else{
                stack.push(item);
            }
        }
        if(cellRenderState.itemId <= 0){
            getItemId(position);
        }
        List<WXComponent> updates = Statements.doRender(cell, this.cellRenderContext);
        if(cellRenderState.isDirty()){
            cellRenderState.resetDirty();
        }
        return  updates;
    }

    public ArrayStack copyStack(CellRenderContext context, ArrayStack stack){
        ArrayStack onceStack = new ArrayStack();
        for(int index=0;  index <  stack.size(); index++) {
            Object value = stack.get(index);
            if(value instanceof  Map){
                value  = new HashMap((Map) value);
            }
            onceStack.push(value);
        }
        return onceStack;
    }


    /**
     * return tepmlate key for position
     * */
    public String getTemplateKey(int position){
        Object data =  safeGetListData(position);
        return getTemplateKey(data);
    }

    /**
     * return template key for position never be null
     * */
    public String getTemplateKey(Object data){
        String template = null;
        if(data instanceof  JSONObject) {
            template = ((JSONObject)data).getString(listDataTemplateKey);
        }
        if(TextUtils.isEmpty(template)){
            if(defaultTemplateCell != null){
                template  = defaultTemplateKey;
            }else {
                template = "";
            }
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
     * get template key from cell; -1 for  cann't find  type
     * */
    private int getCellTemplateItemType(WXCell cell){
        if(cell == null){
            return  -1;
        }
        if(cell.getAttrs() != null){
            Object templateId = cell.getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE);
            String template = WXUtils.getString(templateId, null);
            if(cell == defaultTemplateCell){
                template = defaultTemplateKey;
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
        if(!hasLayoutDone){
            return 0;
        }
        if(!hasAppendTreeDone){
            return 0;
        }
        if(cellDataManager.listData == null){
            return  0;
        }
        if(mTemplateViewTypes == null || mTemplateViewTypes.size() <= 1){
            return 0;
        }
        if(mTemplateSources == null || mTemplateSources.size() == 0){
            return  0;
        }
        return cellDataManager.listData.size();
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
        CellRenderState renderState = cellDataManager.getRenderState(position);
        if(renderState.itemId <=  0){
            String template = getTemplateKey(position);
            if(TextUtils.isEmpty(template)){
                return RecyclerView.NO_ID;
            }
            Object data = safeGetListData(position);
            if(data instanceof  JSONObject && ((JSONObject)data).containsKey("keyItemId")){
                renderState.itemId =  ((JSONObject)data).getLongValue("keyItemId");
            }else{
                long id = Math.abs(data.hashCode());
                long itemId = (id<< 24) + position;
                renderState.itemId = itemId;
            }
        }
        return renderState.itemId;
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
            String offset = getAttrs().getLoadMoreOffset();

            if (TextUtils.isEmpty(offset)) {
                offset = "0";
            }
            float offsetParsed = WXViewUtils.getRealPxByWidth(Integer.parseInt(offset),getInstance().getInstanceViewPortWidth());

            if (offScreenY <= offsetParsed && cellDataManager.listData != null) {
                if (mListCellCount != cellDataManager.listData.size()
                        || mForceLoadmoreNextTime) {
                    fireEvent(Constants.Event.LOADMORE);
                    mListCellCount = cellDataManager.listData.size();
                    mForceLoadmoreNextTime = false;
                }
            }
        } catch (Exception e) {
            WXLogUtils.d(TAG + " onLoadMore : ", e);
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
        if(mAppearChangeRunnable != null) {
            getHostView().removeCallbacks(mAppearChangeRunnable);
            mAppearChangeRunnable = null;
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
                            if(childLisener.getEvents() != null
                                    && childLisener.getEvents().getEventBindingArgsValues() != null
                                    && childLisener.getEvents().getEventBindingArgsValues().get(Constants.Event.DISAPPEAR) != null){
                                eventArgs = childLisener.getEvents().getEventBindingArgsValues().get(Constants.Event.DISAPPEAR);
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
                WXEvent events = component.getEvents();
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


    private Object safeGetListData(int position){
        try{
            return cellDataManager.listData.get(position);
        }catch (Exception e){return  JSONObject.parseObject("{}");}
    }

    public void  notifyUpdateList(){
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
        if(cellDataManager.listData == null){
            return totalHeight;
        }
        for (int i = 0; i < cellDataManager.listData.size(); i++) {
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
    public WXComponent findParentType(WXComponent component, Class type){
        if(type.isAssignableFrom(component.getClass())){
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
        if(container != null && (!(container instanceof WXRecyclerTemplateList))){
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent element = container.getChild(i);
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
     * @param  template  template name
     * get cell template component from cache, if cell component not load
     * start load cell template
     * */
    private WXCell getCellTemplateFromCache(final String template){
        TemplateCache cache = mTemplatesCache.get(template);
        WXCell component =  null;
        if(cache != null && cache.cells != null && cache.cells.size() > 0){
            component = cache.cells.poll();
        }
        if(cache == null ||  !cache.isLoadIng){
            if(cache == null){
                cache = new TemplateCache();
                mTemplatesCache.put(template, cache);
            }
            cache.isLoadIng = true;
            WXCell source = mTemplateSources.get(template);
            if(source != null){
                boolean allowPreload = WXUtils.getBoolean(source.getAttrs().get("preload"), true);
                if(allowPreload) {
                    AsyncCellLoadTask asyncCellLoadTask = new AsyncCellLoadTask(template, source, this);
                    asyncCellLoadTask.startTask();
                }
            }
        }
        return  component;
    }

    /**
     * copy cell component from source, init render data, and return source
     * if none data, return null
     * */
    public WXComponent copyComponentFromSourceCell(WXCell cell){
        renderTemplateCellWithData(cell);
        WXCell component = (WXCell) Statements.copyComponentTree(cell);
        return component;
    }

    /**
     *  render  init with  cell with one data,
     *  if template has already render with data, done nothing
     *  @param  cell
     * */
    private synchronized void renderTemplateCellWithData(WXCell cell){
        if(cell.getRenderData() == null){
            if(cellDataManager.listData != null && cellDataManager.listData.size() > 0){
                synchronized (this){
                    if(cell.getRenderData() == null){
                        Statements.parseStatementsToken(cell);
                        for(int i = 0; i< cellDataManager.listData.size(); i++){
                            if(cell == getSourceTemplate(i)){
                                Object data = cellDataManager.listData.get(i);
                                doRenderTemplate(cell, i);
                                Layouts.doLayoutSync(cell, getLayoutWidth(), getLayoutHeight());
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
     * create view for lazy cell and bind data
     * */
    public static void doCreateCellViewBindData(WXCell component, String template, boolean inPreload){
        if(component.isLazy() || component.getHostView() == null){
            long start = System.currentTimeMillis();
            Statements.initLazyComponent(component, null);
            if(WXEnvironment.isOpenDebugLog() && ENABLE_TRACE_LOG) {
                WXLogUtils.d(TAG, " doCreateCellViewBindData " + template + " in preload "+ inPreload + " used " + (System.currentTimeMillis() - start));
            }
        }
    }

    public ScrollStartEndHelper getScrollStartEndHelper() {
        if(mScrollStartEndHelper == null){
            mScrollStartEndHelper = new ScrollStartEndHelper(this);
        }
        return mScrollStartEndHelper;
    }

    public int getTemplateCacheSize() {
        return templateCacheSize;
    }

    public ConcurrentHashMap<String, TemplateCache> getTemplatesCache() {
        if(mTemplatesCache == null){
            mTemplatesCache = new ConcurrentHashMap<>();
        }
        return mTemplatesCache;
    }


    public CellDataManager getCellDataManager() {
        return cellDataManager;
    }


    private JSONArray parseListDataToJSONArray(Object value){
        try{
            if(value instanceof  JSONArray){
                return (JSONArray) value;
            }
            if(value instanceof String){
                JSONArray array = JSONArray.parseArray(getAttrs().get(Constants.Name.Recycler.LIST_DATA).toString());
                return array;
            }
        }catch (Exception e){
            WXLogUtils.e(TAG, "parseListDataException" + e.getMessage());
        }
        return new JSONArray();
    }
}
