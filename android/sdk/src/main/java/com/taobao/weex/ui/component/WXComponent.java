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
import android.content.Intent;
import android.content.res.ColorStateList;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.RippleDrawable;
import android.os.Build;
import android.os.Message;
import android.support.annotation.CallSuper;
import android.support.annotation.CheckResult;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.v4.view.AccessibilityDelegateCompat;
import android.support.v4.view.ViewCompat;
import android.support.v4.view.accessibility.AccessibilityNodeInfoCompat;
import android.text.TextUtils;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.ViewOverlay;
import android.widget.FrameLayout;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.ComponentObserver;
import com.taobao.weex.IWXActivityStateListener;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXAccessibilityRoleAdapter;
import com.taobao.weex.bridge.EventResult;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.IWXObject;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.dom.ImmutableDomObject;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXDomTask;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.IFComponentHolder;
import com.taobao.weex.ui.animation.WXAnimationModule;
import com.taobao.weex.ui.component.binding.Statements;
import com.taobao.weex.ui.component.pesudo.OnActivePseudoListener;
import com.taobao.weex.ui.component.pesudo.PesudoStatus;
import com.taobao.weex.ui.component.pesudo.TouchActivePseudoListener;
import com.taobao.weex.ui.flat.FlatComponent;
import com.taobao.weex.ui.flat.FlatGUIContext;
import com.taobao.weex.ui.flat.WidgetContainer;
import com.taobao.weex.ui.flat.widget.AndroidViewWidget;
import com.taobao.weex.ui.flat.widget.Widget;
import com.taobao.weex.ui.view.border.BorderDrawable;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.ui.view.gesture.WXGestureType;
import com.taobao.weex.utils.BoxShadowUtil;
import com.taobao.weex.utils.WXDataStructureUtil;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXReflectionUtils;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * abstract component
 *
 */
public abstract class  WXComponent<T extends View> implements IWXObject, IWXActivityStateListener,OnActivePseudoListener {

  public static final String PROP_FIXED_SIZE = "fixedSize";
  public static final String PROP_FS_MATCH_PARENT = "m";
  public static final String PROP_FS_WRAP_CONTENT = "w";

  private int mFixedProp = 0;
  public static volatile int mComponentNum = 0;
  /** package **/ T mHost;

  private volatile WXVContainer mParent;
  private volatile ImmutableDomObject mDomObj;
  private WXSDKInstance mInstance;
  private Context mContext;

  private int mAbsoluteY = 0;
  private int mAbsoluteX = 0;
  private Set<String> mGestureType;

  private BorderDrawable mBackgroundDrawable;
  private Drawable mRippleBackground;
  private int mPreRealWidth = 0;
  private int mPreRealHeight = 0;
  private int mPreRealLeft = 0;
  private int mPreRealTop = 0;
  private int mStickyOffset = 0;
  protected WXGesture mGesture;
  private IFComponentHolder mHolder;
  private boolean isUsing = false;
  private List<OnClickListener> mHostClickListeners;
  private List<OnFocusChangeListener> mFocusChangeListeners;
  private String mCurrentRef;
  private Set<String> mAppendEvents = new HashSet<>();
  private WXAnimationModule.AnimationHolder mAnimationHolder;
  private PesudoStatus mPesudoStatus = new PesudoStatus();
  private boolean mIsDestroyed = false;
  private boolean mIsDisabled = false;
  private int mType = TYPE_COMMON;
  private boolean mNeedLayoutOnAnimation = false;
  private String mLastBoxShadowId;


  public WXTracing.TraceInfo mTraceInfo = new WXTracing.TraceInfo();

  public static final int TYPE_COMMON = 0;
  public static final int TYPE_VIRTUAL = 1;

  private boolean waste = false;


  //Holding the animation bean when component is uninitialized
  public void postAnimation(WXAnimationModule.AnimationHolder holder) {
    this.mAnimationHolder = holder;
  }

  //This method will be removed once flatGUI is completed.
  @RestrictTo(Scope.LIBRARY)
  public boolean isFlatUIEnabled(){
    return mParent != null && mParent.isFlatUIEnabled();
  }

  private OnClickListener mClickEventListener = new OnClickListener() {
    @Override
    public void onHostViewClick() {
      Map<String, Object> param= WXDataStructureUtil.newHashMapWithExpectedSize(1);
      Map<String, Object> position = WXDataStructureUtil.newHashMapWithExpectedSize(4);
      int[] location = new int[2];
      mHost.getLocationOnScreen(location);
      position.put("x", WXViewUtils.getWebPxByWidth(location[0],mInstance.getInstanceViewPortWidth()));
      position.put("y", WXViewUtils.getWebPxByWidth(location[1],mInstance.getInstanceViewPortWidth()));
      position.put("width", WXViewUtils.getWebPxByWidth(mDomObj.getLayoutWidth(),mInstance.getInstanceViewPortWidth()));
      position.put("height", WXViewUtils.getWebPxByWidth(mDomObj.getLayoutHeight(),mInstance.getInstanceViewPortWidth()));
      param.put(Constants.Name.POSITION, position);
      fireEvent(Constants.Event.CLICK,param);
    }
  };

  public String getInstanceId() {
    return mInstance.getInstanceId();
  }

  public Rect getComponentSize() {
    Rect size = new Rect();
    if (mHost != null) {
      int[] location = new int[2];
      int[] anchor = new int[2];
      mHost.getLocationOnScreen(location);
      mInstance.getContainerView().getLocationOnScreen(anchor);

      int left = location[0] - anchor[0];
      int top = (location[1] - mStickyOffset) - anchor[1];
      int width = (int) mDomObj.getLayoutWidth();
      int height = (int) mDomObj.getLayoutHeight();
      size.set(left, top, left + width, top + height);
    }
    return size;
  }

  public final void invoke(String method, JSONArray args) {
    final Invoker invoker = mHolder.getMethodInvoker(method);
    if (invoker != null) {
      try {
        getInstance()
            .getNativeInvokeHelper()
            .invoke(this,invoker,args);

      } catch (Exception e) {
        WXLogUtils.e("[WXComponent] updateProperties :" + "class:" + getClass() + "method:" + invoker.toString() + " function " + WXLogUtils.getStackTrace(e));
      }
    }else{
      onInvokeUnknownMethod(method,args);
    }
  }

  /**
   * Will be invoked when request method not found.
   * Subclass should override this method, If you return hard-code method list in {@link IFComponentHolder#getMethods()}
   * @param method
   * @param args
   */
  protected void onInvokeUnknownMethod(String method, JSONArray args){

  }

  public interface OnClickListener{
    void onHostViewClick();
  }

  public interface OnFocusChangeListener{
    void onFocusChange(boolean hasFocus);
  }

  @Deprecated
  public WXComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance,dom,parent,isLazy);
  }

  @Deprecated
  public WXComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, boolean isLazy) {
    this(instance,dom,parent);
  }

  public WXComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent) {
    this(instance, dom, parent, TYPE_COMMON);
  }

  public WXComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, int type) {
    mInstance = instance;
    mContext = mInstance.getContext();
    mParent = parent;
    mType = type;
    mDomObj = dom.clone();
    mCurrentRef = mDomObj.getRef();
    mGestureType = new HashSet<>();
    ++mComponentNum;
    onCreate();
    ComponentObserver observer;
    if ((observer = getInstance().getComponentObserver()) != null) {
      observer.onCreate(this);
    }
  }

  protected void onCreate(){

  }

  public void bindHolder(IFComponentHolder holder){
    mHolder = holder;
  }


  public WXSDKInstance getInstance(){
    return mInstance;
  }

  public Context getContext(){
    return mContext;
  }

  /**
   * Find component by component reference.
   * @param ref
   * @return
   */
  protected final WXComponent findComponent(String ref){
    if(mInstance != null && ref != null){
      return WXSDKManager.getInstance()
          .getWXRenderManager()
          .getWXComponent(mInstance.getInstanceId(), ref);
    }
    return null;
  }

  public final void fireEvent(String type){
    fireEvent(type,null);
  }

  public final void fireEvent(String type, Map<String, Object> params){
    if(WXUtils.getBoolean(getDomObject().getAttrs().get("fireEventSyn"), false)){
        fireEventWait(type, params);
    }else{
        fireEvent(type, params,null, null);
    }
  }

  public final EventResult fireEventWait(String type, Map<String, Object> params){
    final CountDownLatch waitLatch = new CountDownLatch(1);
    EventResult callback = new EventResult(){
      @Override
      public void onCallback(Object result) {
        super.onCallback(result);
        waitLatch.countDown();
      }
    };
    try{
      fireEvent(type, params, null, callback);
      waitLatch.await(10, TimeUnit.MILLISECONDS);
      return  callback;
    }catch (Exception e){
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e("fireEventWait", e);
      }
      return  callback;
    }
  }

  protected final void fireEvent(String type, Map<String, Object> params,Map<String, Object> domChanges){
      fireEvent(type, params, domChanges, null);
  }


    private final void fireEvent(String type, Map<String, Object> params,Map<String, Object> domChanges, EventResult callback){
    if(mInstance != null && mDomObj != null) {
        List<Object> eventArgsValues = null;
        if(mDomObj.getEvents() != null && mDomObj.getEvents().getEventBindingArgsValues() != null){
             eventArgsValues = mDomObj.getEvents().getEventBindingArgsValues().get(type);
        }
        if(params != null){
          String componentId = Statements.getComponentId(this);
          if(componentId != null) {
            params.put("componentId", componentId);
          }
        }
        mInstance.fireEvent(mCurrentRef, type, params,domChanges, eventArgsValues, callback);
    }
  }


  /**
   * find certain class type parent
   * */
  public  Object findTypeParent(WXComponent component, Class type){
    if(component.getClass() == type){
      return component;
    }
    if(component.getParent() != null) {
        findTypeParent(component.getParent(), type);
    }
    return  null;
  }

  /**
   * The view is created as needed
   * @return true for lazy
   */
  public boolean isLazy() {
    if(mLazy){
      return true;
    }
    return mParent != null && mParent.isLazy();
  }

  public void applyLayoutAndEvent(WXComponent component) {
    long startNanos = System.nanoTime();
    if(!isLazy()) {
      if (component == null) {
        component = this;
      }
      setLayout(component.getDomObject());
      setPadding(component.getDomObject().getPadding(), component.getDomObject().getBorder());
      addEvents();
    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  protected final void addFocusChangeListener(OnFocusChangeListener l){
    View view;
    if(l != null && (view = getRealView()) != null) {
      if( mFocusChangeListeners == null){
        mFocusChangeListeners = new ArrayList<>();
        view.setFocusable(true);
        view.setOnFocusChangeListener(new View.OnFocusChangeListener() {
          @Override
          public void onFocusChange(View v, boolean hasFocus) {
            for (OnFocusChangeListener listener : mFocusChangeListeners){
              if(listener != null){
                listener.onFocusChange(hasFocus);
              }
            }
          }
        });
      }
      mFocusChangeListeners.add(l);
    }
  }

  protected final void addClickListener(OnClickListener l){
    View view;
    if(l != null && (view = getRealView()) != null) {
      if(mHostClickListeners == null){
        mHostClickListeners = new ArrayList<>();
        view.setOnClickListener(new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            if(mGesture != null && mGesture.isTouchEventConsumedByAdvancedGesture()){
              //event is already consumed by gesture
              return;
            }
            for (OnClickListener listener : mHostClickListeners){
              if(listener != null) {
                listener.onHostViewClick();
              }
            }
          }
        });
      }
      mHostClickListeners.add(l);

    }
  }

  protected final void removeClickListener(OnClickListener l){
    mHostClickListeners.remove(l);
  }

  public void bindData(WXComponent component){
    long startNanos = System.nanoTime();
    if(!isLazy()) {
      if (component == null) {
        component = this;
      }
      mCurrentRef = component.getDomObject().getRef();
      updateStyle(component);
      updateAttrs(component);
      updateExtra(component.getDomObject().getExtra());
    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  public void updateStyle(WXComponent component){
    ImmutableDomObject domObject = component.getDomObject();
    if(domObject !=null){
      updateProperties(domObject.getStyles());
    }
  }

  public void updateAttrs(WXComponent component){
    ImmutableDomObject domObject = component.getDomObject();
    if(domObject !=null){
      updateProperties(domObject.getAttrs());
    }
  }

  public void refreshData(WXComponent component){

  }

  protected BorderDrawable getOrCreateBorder() {
    if (mBackgroundDrawable == null) {
      mBackgroundDrawable = new BorderDrawable();
      if (mHost != null) {
        WXViewUtils.setBackGround(mHost, null);
        if (mRippleBackground == null) {
          WXViewUtils.setBackGround(mHost, mBackgroundDrawable);
        } else {
          //TODO Not strictly clip according to background-clip:border-box
          WXViewUtils.setBackGround(mHost, new LayerDrawable(new Drawable[]{
              mRippleBackground, mBackgroundDrawable}));
        }
      }
    }
    return mBackgroundDrawable;
  }

  /**
   * layout view
   */
  public final void setLayout(ImmutableDomObject domObject) {
    if ( domObject == null || TextUtils.isEmpty(mCurrentRef)) {
      return;
    }

    boolean nullParent = mParent == null;//parent is nullable
    mDomObj = domObject;

    //offset by sibling
    int siblingOffset = nullParent?0:mParent.getChildrenLayoutTopOffset();

    Spacing parentPadding = (nullParent?new Spacing():mParent.getDomObject().getPadding());
    Spacing parentBorder = (nullParent?new Spacing():mParent.getDomObject().getBorder());
    Spacing margin = mDomObj.getMargin();
    int realWidth = (int) mDomObj.getLayoutWidth();
    int realHeight = (int) mDomObj.getLayoutHeight();
    int realLeft = (int) (mDomObj.getLayoutX() - parentPadding.get(Spacing.LEFT) -
                          parentBorder.get(Spacing.LEFT));
    int realTop = (int) (mDomObj.getLayoutY() - parentPadding.get(Spacing.TOP) -
                         parentBorder.get(Spacing.TOP)) + siblingOffset;
    int realRight = (int) margin.get(Spacing.RIGHT);
    int realBottom = (int) margin.get(Spacing.BOTTOM);
    Point rawOffset = new Point(
        (int) mDomObj.getCSSLayoutLeft(),
        (int) mDomObj.getCSSLayoutTop());

    if (mPreRealWidth == realWidth && mPreRealHeight == realHeight && mPreRealLeft == realLeft && mPreRealTop == realTop) {
      return;
    }

    mAbsoluteY = (int) (nullParent?0:mParent.getAbsoluteY() + mDomObj.getLayoutY());
    mAbsoluteX = (int) (nullParent?0:mParent.getAbsoluteX() + mDomObj.getLayoutX());

    //calculate first screen time
    if (!mInstance.mEnd &&!(mHost instanceof ViewGroup) && mAbsoluteY+realHeight > mInstance.getWeexHeight()+1) {
      mInstance.firstScreenRenderFinished();
    }


    MeasureOutput measureOutput = measure(realWidth, realHeight);
    realWidth = measureOutput.width;
    realHeight = measureOutput.height;

    setComponentLayoutParams(realWidth, realHeight, realLeft, realTop, realRight, realBottom, rawOffset);
  }

  private void setComponentLayoutParams(int realWidth, int realHeight, int realLeft, int realTop,
      int realRight, int realBottom, Point rawOffset) {
    FlatGUIContext UIImp = getInstance().getFlatUIContext();
    WidgetContainer ancestor;
    Widget widget;
    if ((ancestor = UIImp.getFlatComponentAncestor(this)) != null) {
      if (this instanceof FlatComponent && !((FlatComponent) this).promoteToView(true)) {
        widget = ((FlatComponent) this).getOrCreateFlatWidget();
      } else {
        widget = UIImp.getAndroidViewWidget(this);
      }
      setWidgetParams(widget, UIImp, rawOffset, realWidth, realHeight, realLeft, realRight, realTop,
          realBottom);
    } else if (mHost != null) {
      // clear box shadow before host's size changed
      clearBoxShadow();
      if (mDomObj.isFixed()) {
        setFixedHostLayoutParams(mHost, realWidth, realHeight, realLeft, realRight, realTop,
            realBottom);
      } else {
        setHostLayoutParams(mHost, realWidth, realHeight, realLeft, realRight, realTop, realBottom);
      }
      mPreRealWidth = realWidth;
      mPreRealHeight = realHeight;
      mPreRealLeft = realLeft;
      mPreRealTop = realTop;
      onFinishLayout();
      // restore box shadow
      updateBoxShadow();
    }
  }

  private void setWidgetParams(Widget widget, FlatGUIContext UIImp, Point rawoffset,
      int width, int height, int left, int right, int top, int bottom) {
    Point childOffset = new Point();
    if (mParent != null) {
      if (mParent instanceof FlatComponent &&
          UIImp.getFlatComponentAncestor(mParent) != null &&
          UIImp.getAndroidViewWidget(mParent) == null) {
        childOffset.set(rawoffset.x, rawoffset.y);
      }
      else{
        childOffset.set(left, top);
      }

      if (mParent instanceof FlatComponent &&
          UIImp.getFlatComponentAncestor(mParent) != null &&
          UIImp.getAndroidViewWidget(mParent) == null) {
        Point parentLayoutOffset = ((FlatComponent) mParent).getOrCreateFlatWidget().getLocInFlatContainer();
        childOffset.offset(parentLayoutOffset.x, parentLayoutOffset.y);
      }
      ViewGroup.LayoutParams lp = mParent
          .getChildLayoutParams(this, mHost, width, height, left, right, top, bottom);
      if (lp instanceof MarginLayoutParams) {
        width = lp.width;
        height = lp.height;
        left = ((MarginLayoutParams) lp).leftMargin;
        right = ((MarginLayoutParams) lp).rightMargin;
        top = ((MarginLayoutParams) lp).topMargin;
        bottom = ((MarginLayoutParams) lp).bottomMargin;
      }
    }
    widget.setLayout(width, height, left, right, top, bottom, childOffset);

    if (widget instanceof AndroidViewWidget && ((AndroidViewWidget) widget).getView()!=null) {
      //TODO generic method if ever possible
      setHostLayoutParams((T) ((AndroidViewWidget) widget).getView(),
          width, height, childOffset.x, right, childOffset.y, bottom);
    }
  }

  protected void setHostLayoutParams(T host, int width, int height, int left, int right, int top, int bottom){
    ViewGroup.LayoutParams lp;
    if(mParent == null){
      FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(width,height);
      params.setMargins(left, top, right, bottom);
      lp = params;
    }else{
      lp = mParent.getChildLayoutParams(this,host,width, height, left,right,top,bottom);
    }
    if(lp != null) {
      mHost.setLayoutParams(lp);
    }
  }

  private void setFixedHostLayoutParams(T host, int width, int height, int left, int right, int top, int bottom){
    FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);

    params.width = width;
    params.height = height;
    params.setMargins(left, top, right, bottom);
    host.setLayoutParams(params);
    mInstance.moveFixedView(host);

    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("Weex_Fixed_Style", "WXComponent:setLayout :" + left + " " + top + " " + width + " " + height);
      WXLogUtils.d("Weex_Fixed_Style", "WXComponent:setLayout Left:" + mDomObj.getStyles().getLeft() + " " + (int) mDomObj.getStyles().getTop());
    }
  }

  /**
   * After component's layout result is apply to view. May be invoke multiple times since
   * DOM can be changed in js runtime.
   */
  protected void onFinishLayout() {
    Object param = mDomObj.getStyles() != null ? mDomObj.getStyles().get(Constants.Name.BACKGROUND_IMAGE) : null;
    if (param != null) {
      setBackgroundImage(param.toString());
    }
  }

  public int getLayoutTopOffsetForSibling(){
    return 0;
  }

  public float getLayoutWidth(){
    float w = 0f;
    if (mDomObj != null) {
      w = mDomObj.getLayoutWidth();
      w = Float.isNaN(w) ? 0f : w;
    }
    return w;
  }

  public float getLayoutHeight(){
    float h = 0f;
    if (mDomObj != null) {
      h = mDomObj.getLayoutHeight();
      h = Float.isNaN(h) ? 0f : h;
    }
    return h;
  }

  public void setPadding(Spacing padding, Spacing border) {
    int left = (int) (padding.get(Spacing.LEFT) + border.get(Spacing.LEFT));
    int top = (int) (padding.get(Spacing.TOP) + border.get(Spacing.TOP));
    int right = (int) (padding.get(Spacing.RIGHT) + border.get(Spacing.RIGHT));
    int bottom = (int) (padding.get(Spacing.BOTTOM) + border.get(Spacing.BOTTOM));

    if (this instanceof FlatComponent && !((FlatComponent) this).promoteToView(true)) {
      ((FlatComponent) this).getOrCreateFlatWidget().setContentBox(left, top, right, bottom);
    } else if (mHost != null) {
      mHost.setPadding(left, top, right, bottom);
    }
  }

  private void addEvents() {
    int count = mDomObj.getEvents().size();
    for (int i = 0; i < count; ++i) {
      addEvent(mDomObj.getEvents().get(i));
    }
    setActiveTouchListener();
  }

  public void updateExtra(Object extra) {

  }

  public ImmutableDomObject getDomObject() {
    return mDomObj;
  }

  /**
   * measure
   */
  protected MeasureOutput measure(int width, int height) {
    MeasureOutput measureOutput = new MeasureOutput();

    if(mFixedProp != 0){
      measureOutput.width = mFixedProp;
      measureOutput.height = mFixedProp;
    }else {
      measureOutput.width = width;
      measureOutput.height = height;
    }
    return measureOutput;
  }


  @Deprecated
  public void updateProperties(Map<String, Object> props) {
    if (props == null || (mHost == null && !isVirtualComponent())) {
      return;
    }

    for (Map.Entry<String, Object> entry : props.entrySet()) {
      String key = entry.getKey();
      Object param = entry.getValue();
      String value = WXUtils.getString(param, null);
      if (TextUtils.isEmpty(value)) {
        param = convertEmptyProperty(key, value);
      }
      if (!setProperty(key, param)) {
        if (mHolder == null) {
          return;
        }
        Invoker invoker = mHolder.getPropertyInvoker(key);
        if (invoker != null) {
          try {
            Type[] paramClazzs = invoker.getParameterTypes();
            if (paramClazzs.length != 1) {
              WXLogUtils.e("[WXComponent] setX method only one parameter：" + invoker);
              return;
            }
            param = WXReflectionUtils.parseArgument(paramClazzs[0], param);
            invoker.invoke(this, param);
          } catch (Exception e) {
            WXLogUtils.e("[WXComponent] updateProperties :" + "class:" + getClass() + "method:" + invoker.toString() + " function " + WXLogUtils.getStackTrace(e));
          }
        }
      }
    }
    readyToRender();
    if (this instanceof FlatComponent && mBackgroundDrawable != null) {
      FlatComponent flatComponent = (FlatComponent) this;
      if (!flatComponent.promoteToView(true) && !(flatComponent
          .getOrCreateFlatWidget() instanceof AndroidViewWidget)) {
        flatComponent.getOrCreateFlatWidget().setBackgroundAndBorder(mBackgroundDrawable);
      }
    }
  }

  /**
   * Apply styles and attributes.
   * @param key name of argument
   * @param param value of argument
   * @return true means that the property is consumed
     */
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.PREVENT_MOVE_EVENT:
        if(mGesture != null){
          mGesture.setPreventMoveEvent(WXUtils.getBoolean(param,false));
        }
        return true;
      case Constants.Name.DISABLED:
        Boolean disabled = WXUtils.getBoolean(param,null);
        if (disabled != null) {
          setDisabled(disabled);
          setPseudoClassStatus(Constants.PSEUDO.DISABLED, disabled);
        }
        return true;
      case Constants.Name.POSITION:
        String position = WXUtils.getString(param,null);
        if (position != null)
          setSticky(position);
        return true;
      case Constants.Name.BACKGROUND_COLOR:
        String bgColor = WXUtils.getString(param,null);
        if (bgColor != null)
          setBackgroundColor(bgColor);
        return true;
      case Constants.Name.BACKGROUND_IMAGE:
        String bgImage = WXUtils.getString(param, null);
        if (bgImage != null) {
          setBackgroundImage(bgImage);
        }
        return true;
      case Constants.Name.OPACITY:
        Float opacity = WXUtils.getFloat(param,null);
        if (opacity != null)
          setOpacity(opacity);
        return true;
      case Constants.Name.BORDER_RADIUS:
      case Constants.Name.BORDER_TOP_LEFT_RADIUS:
      case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
      case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
      case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
        Float radius = WXUtils.getFloat(param,null);
        if (radius != null) {
          setBorderRadius(key, radius);
        }
        return true;
      case Constants.Name.BORDER_WIDTH:
      case Constants.Name.BORDER_TOP_WIDTH:
      case Constants.Name.BORDER_RIGHT_WIDTH:
      case Constants.Name.BORDER_BOTTOM_WIDTH:
      case Constants.Name.BORDER_LEFT_WIDTH:
        Float width = WXUtils.getFloat(param,null);
        if (width != null)
          setBorderWidth(key,width);
        return true;
      case Constants.Name.BORDER_STYLE:
      case Constants.Name.BORDER_RIGHT_STYLE:
      case Constants.Name.BORDER_BOTTOM_STYLE:
      case Constants.Name.BORDER_LEFT_STYLE:
      case Constants.Name.BORDER_TOP_STYLE:
        String border_style = WXUtils.getString(param,null);
        if (border_style != null)
          setBorderStyle(key, border_style);
        return true;
      case Constants.Name.BORDER_COLOR:
      case Constants.Name.BORDER_TOP_COLOR:
      case Constants.Name.BORDER_RIGHT_COLOR:
      case Constants.Name.BORDER_BOTTOM_COLOR:
      case Constants.Name.BORDER_LEFT_COLOR:
        String border_color = WXUtils.getString(param,null);
        if (border_color != null)
          setBorderColor(key, border_color);
        return true;
      case Constants.Name.VISIBILITY:
        String visibility = WXUtils.getString(param,null);
        if (visibility != null)
          setVisibility(visibility);
        return true;
      case Constants.Name.ELEVATION:
        if(param!=null) {
          updateElevation();
        }
        return true;
      case PROP_FIXED_SIZE:
        String fixedSize = WXUtils.getString(param, PROP_FS_MATCH_PARENT);
        setFixedSize(fixedSize);
        return true;
      case Constants.Name.ARIA_LABEL:
        String label = WXUtils.getString(param,"");
        setAriaLabel(label);
        return true;
      case Constants.Name.ARIA_HIDDEN:
        boolean isHidden = WXUtils.getBoolean(param,false);
        setAriaHidden(isHidden);
        return true;
      case Constants.Name.WIDTH:
      case Constants.Name.MIN_WIDTH:
      case Constants.Name.MAX_WIDTH:
      case Constants.Name.HEIGHT:
      case Constants.Name.MIN_HEIGHT:
      case Constants.Name.MAX_HEIGHT:
      case Constants.Name.ALIGN_ITEMS:
      case Constants.Name.ALIGN_SELF:
      case Constants.Name.FLEX:
      case Constants.Name.FLEX_DIRECTION:
      case Constants.Name.JUSTIFY_CONTENT:
      case Constants.Name.FLEX_WRAP:
      case Constants.Name.MARGIN:
      case Constants.Name.MARGIN_TOP:
      case Constants.Name.MARGIN_LEFT:
      case Constants.Name.MARGIN_RIGHT:
      case Constants.Name.MARGIN_BOTTOM:
      case Constants.Name.PADDING:
      case Constants.Name.PADDING_TOP:
      case Constants.Name.PADDING_LEFT:
      case Constants.Name.PADDING_RIGHT:
      case Constants.Name.PADDING_BOTTOM:
      case Constants.Name.LEFT:
      case Constants.Name.TOP:
      case Constants.Name.RIGHT:
      case Constants.Name.BOTTOM:
        return true;
      case Constants.Name.BOX_SHADOW:
        try {
          updateBoxShadow();
        } catch (Throwable t) {
          t.printStackTrace();
        }
        return true;
      case Constants.Name.ROLE:
        setRole(WXUtils.getString(param, ""));
        return true;
      default:
        return false;
    }
  }

  protected void updateBoxShadow() {
    if (!BoxShadowUtil.isBoxShadowEnabled()) {
      WXLogUtils.w("BoxShadow", "box-shadow disabled");
      return;
    }

    if (getDomObject() != null && getDomObject().getStyles() != null) {
      Object boxShadow = getDomObject().getStyles().get(Constants.Name.BOX_SHADOW);
      Object shadowQuality = getDomObject().getAttrs().get(Constants.Name.SHADOW_QUALITY);
      if (boxShadow == null) {
        return;
      }

      View target = mHost;
      if (this instanceof WXVContainer) {
        target = ((WXVContainer) this).getBoxShadowHost(false);
      }

      if (target == null) {
        return;
      }

      float quality = WXUtils.getFloat(shadowQuality, 0.5f);
      int viewPort = getInstance().getInstanceViewPortWidth();
      String token = new StringBuilder(boxShadow.toString()).append(" / [")
          .append(target.getMeasuredWidth()).append(",")
          .append(target.getMeasuredHeight()).append("] / ")
          .append(quality).toString();

      if (mLastBoxShadowId != null && mLastBoxShadowId.equals(token)) {
        WXLogUtils.d("BoxShadow", "box-shadow style was not modified. " + token);
        return;
      }

      float[] radii = new float[] {0, 0, 0, 0, 0, 0, 0, 0};
      WXStyle style = getDomObject().getStyles();
      if (style != null) {
        float tl = WXUtils.getFloat(style.get(Constants.Name.BORDER_TOP_LEFT_RADIUS), 0f);
        radii[0] = tl;
        radii[1] = tl;

        float tr = WXUtils.getFloat(style.get(Constants.Name.BORDER_TOP_RIGHT_RADIUS), 0f);
        radii[2] = tr;
        radii[3] = tr;

        float br = WXUtils.getFloat(style.get(Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS), 0f);
        radii[4] = br;
        radii[5] = br;

        float bl = WXUtils.getFloat(style.get(Constants.Name.BORDER_BOTTOM_LEFT_RADIUS), 0f);
        radii[6] = bl;
        radii[7] = bl;

        if (style.containsKey(Constants.Name.BORDER_RADIUS)) {
          float radius = WXUtils.getFloat(style.get(Constants.Name.BORDER_RADIUS), 0f);
          for (int i = 0; i < radii.length; i++) {
            radii[i] = radius;
          }
        }
      }

      BoxShadowUtil.setBoxShadow(target, boxShadow.toString(), radii, viewPort, quality);
      mLastBoxShadowId = token;
    } else {
      WXLogUtils.w("Can not resolve styles");
    }
  }

  protected void clearBoxShadow() {
    if (!BoxShadowUtil.isBoxShadowEnabled()) {
      WXLogUtils.w("BoxShadow", "box-shadow disabled");
      return;
    }

    if (getDomObject() != null && getDomObject().getStyles() != null) {
      Object obj = getDomObject().getStyles().get(Constants.Name.BOX_SHADOW);
      if (obj == null) {
        return;
      }
    }

    View target = mHost;
    if (this instanceof WXVContainer) {
      target = ((WXVContainer) this).getBoxShadowHost(true);
    }

    if (target != null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      ViewOverlay overlay = target.getOverlay();
      if (overlay != null) {
        overlay.clear();
      }
    }
    mLastBoxShadowId = null;
  }

  @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
  protected void setAriaHidden(boolean isHidden) {
    View host = getHostView();
    if(host != null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN){
      host.setImportantForAccessibility(isHidden?View.IMPORTANT_FOR_ACCESSIBILITY_NO:View.IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
  }

  protected void setAriaLabel(String label) {
    View host = getHostView();
    if(host != null){
      host.setContentDescription(label);
    }
  }

  protected void setRole(String roleKey) {
    View host = getHostView();
    String role = roleKey;
    if (host != null && !TextUtils.isEmpty(roleKey)) {
      IWXAccessibilityRoleAdapter roleAdapter = WXSDKManager.getInstance().getAccessibilityRoleAdapter();
      if (roleAdapter != null) {
        role = roleAdapter.getRole(roleKey);
      }
      final String finalRole = role;
      AccessibilityDelegateCompat delegate = new AccessibilityDelegateCompat() {
        @Override
        public void onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfoCompat info) {
          super.onInitializeAccessibilityNodeInfo(host, info);
          info.setRoleDescription(finalRole);
        }
      };
      ViewCompat.setAccessibilityDelegate(host, delegate);
    }
  }

  /**
   * Avoid large size view fail in GPU-Animation.
   * @param fixedSize
   */
  private void setFixedSize(String fixedSize) {
    if(PROP_FS_MATCH_PARENT.equals(fixedSize)){
      mFixedProp = ViewGroup.LayoutParams.MATCH_PARENT;
    }else if(PROP_FS_WRAP_CONTENT.equals(fixedSize)){
      mFixedProp = ViewGroup.LayoutParams.WRAP_CONTENT;
    }else{
      mFixedProp = 0;
      return;
    }
    if(mHost != null){
      ViewGroup.LayoutParams layoutParams = mHost.getLayoutParams();
      if(layoutParams != null){
        layoutParams.height = mFixedProp;
        layoutParams.width = mFixedProp;
        mHost.setLayoutParams(layoutParams);
      }
    }
  }

  /**
   * Add new event to component,this will post a task to DOM thread to add event.
   * @param type
   */
  protected void appendEventToDOM(String type){
    WXSDKManager.getInstance().getWXDomManager().postAction(getInstanceId(), Actions.getAddEvent(getRef(),type),false);
  }


  public void addEvent(Object type) {
    if(type instanceof  CharSequence){
       addEvent(type.toString());
    }else if(type instanceof JSONObject){
       JSONObject bindings = (JSONObject) type;
       String eventName = bindings.getString(WXEvent.EVENT_KEY_TYPE);
       if(eventName != null){
         addEvent(eventName);
       }
    }
  }

  /**
   * Do not use this method to add event, this only apply event already add to DomObject.
   * @param type
   */
  public void addEvent(String type) {
    if (TextUtils.isEmpty(type)
            || mAppendEvents.contains(type)
            || getRealView() == null) {
      return;
    }
    mAppendEvents.add(type);

    View view = getRealView();
    if (type.equals(Constants.Event.CLICK) && view != null) {
      addClickListener(mClickEventListener);
    } else if ((type.equals( Constants.Event.FOCUS) || type.equals( Constants.Event.BLUR)) ) {
      addFocusChangeListener(new OnFocusChangeListener() {
        public void onFocusChange(boolean hasFocus) {
          Map<String, Object> params = new HashMap<>();
          params.put("timeStamp", System.currentTimeMillis());
          fireEvent(hasFocus ? Constants.Event.FOCUS : Constants.Event.BLUR, params);
        }
      });
    } else if (view != null &&
               needGestureDetector(type)) {
      if (view instanceof WXGestureObservable) {
        if (mGesture == null) {
          mGesture = new WXGesture(this, mContext);
          boolean isPreventMove = WXUtils.getBoolean(getDomObject().getAttrs().get(Constants.Name.PREVENT_MOVE_EVENT),false);
          mGesture.setPreventMoveEvent(isPreventMove);
        }
        mGestureType.add(type);
        ((WXGestureObservable) view).registerGestureListener(mGesture);
      } else {
        WXLogUtils.e(view.getClass().getSimpleName() + " don't implement " +
                     "WXGestureObservable, so no gesture is supported.");
      }
    } else {
      Scrollable scroller = getParentScroller();
      if (type.equals(Constants.Event.APPEAR) && scroller != null) {
        scroller.bindAppearEvent(this);
      }
      if (type.equals(Constants.Event.DISAPPEAR) && scroller != null) {
        scroller.bindDisappearEvent(this);
      }
    }
  }

  public View getRealView() {
    return mHost;
  }

  /**
   * Judge whether need to set an onTouchListener.<br>
   * As there is only one onTouchListener in each view, so all the gesture that use onTouchListener should put there.
   *
   * @param type eventType {@link com.taobao.weex.common.Constants.Event}
   * @return true for set an onTouchListener, otherwise false
   */
  private boolean needGestureDetector(String type) {
    if (mHost != null) {
      for (WXGestureType gesture : WXGestureType.LowLevelGesture.values()) {
        if (type.equals(gesture.toString())) {
          return true;
        }
      }
      for (WXGestureType gesture : WXGestureType.HighLevelGesture.values()) {
        if (type.equals(gesture.toString())) {
          return true;
        }
      }
    }
    if(Constants.Event.STOP_PROPAGATION.equals(type)){
      return  true;
    }
    return false;
  }

  /**
   * get Scroller components
   */
  public Scrollable getParentScroller() {
    WXComponent component = this;
    WXVContainer container;
    Scrollable scroller;
    for (; ; ) {
      container = component.getParent();
      if (container == null) {
        return null;
      }
      if (container instanceof Scrollable) {
        scroller = (Scrollable) container;
        return scroller;
      }
      if (container.getRef().equals(WXDomObject.ROOT)) {
        return null;
      }
      component = container;
    }
  }

  /**
   * get Scroller components
   */
  @Nullable
  public Scrollable getFirstScroller() {
   if(this instanceof Scrollable){
     return (Scrollable)this;
   }
   return null;
  }

  public WXVContainer getParent() {
    return mParent;
  }

  public String getRef() {
    if (mDomObj == null) {
      return null;
    }
    return mCurrentRef;
  }

  /**
   * create view
   *
   */
  public final void createView() {
    long startNanos = System.nanoTime();
    if(!isLazy()) {
      createViewImpl();
    }
    mTraceInfo.uiThreadStart = System.currentTimeMillis();
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  protected void createViewImpl() {
    if (mContext != null) {
      mHost = initComponentHostView(mContext);
      if (mHost == null && !isVirtualComponent()) {
        //compatible
        initView();
      }
      if(mHost != null){
        mHost.setId(WXViewUtils.generateViewId());
        ComponentObserver observer;
        if ((observer = getInstance().getComponentObserver()) != null) {
          observer.onViewCreated(this, mHost);
        }
      }
      onHostViewInitialized(mHost);
    }else{
      WXLogUtils.e("createViewImpl","Context is null");
    }
  }

  /**
   * Use {@link #initComponentHostView(Context context)} instead.
   */
  @Deprecated
  protected void initView() {
    if (mContext != null)
      mHost = initComponentHostView(mContext);
  }


  /**
   * Create corresponding view for this component.
   * @param context
   * @return
   */
  protected T initComponentHostView(@NonNull Context context){
    /**
     * compatible old initView
     * TODO: change to abstract method in next V1.0 .
     */
    return null;
  }

  /**
   * Called after host view init. <br>
   * Any overriding methods should invoke this method at the right time, to ensure the cached animation can be triggered correctly.
   * (the animation will be cached when {@link #isLazy()} is true)
   * @param host the host view
   */
  @CallSuper
  protected void onHostViewInitialized(T host){
    if (mAnimationHolder != null) {
      //Performs cached animation
      mAnimationHolder.execute(mInstance, this);
    }
    setActiveTouchListener();
  }

  public T getHostView() {
    return mHost;
  }

  /**
   * use {@link #getHostView()} instead
   * @return
   */
  @Deprecated
  public View getView(){
    return mHost;
  }

  public int getAbsoluteY() {
    return mAbsoluteY;
  }

  public int getAbsoluteX() {
    return mAbsoluteX;
  }

  public void updateDom(WXDomObject dom) {
    if (dom == null) {
      return;
    }
    mDomObj = dom.clone();
  }

  public final void removeEvent(String type) {
    if (TextUtils.isEmpty(type)) {
      return;
    }
    mAppendEvents.remove(type);//only clean append events, not dom's events.
    mGestureType.remove(type);
    removeEventFromView(type);
  }

  protected void removeEventFromView(String type) {
    if (type.equals(Constants.Event.CLICK) && getRealView() != null && mHostClickListeners != null) {
      mHostClickListeners.remove(mClickEventListener);
      //click event only remove from listener array
    }
    Scrollable scroller = getParentScroller();
    if (type.equals(Constants.Event.APPEAR) && scroller != null) {
      scroller.unbindAppearEvent(this);
    }
    if (type.equals(Constants.Event.DISAPPEAR) && scroller != null) {
      scroller.unbindDisappearEvent(this);
    }
  }

  public final void removeAllEvent() {
    if (mDomObj == null || mDomObj.getEvents().size() < 1) {
      return;
    }
    for (String event : mDomObj.getEvents()) {
      if(event == null){
        continue;
      }
      removeEventFromView(event);
    }
    mAppendEvents.clear();//only clean append events, not dom's events.
    mGestureType.clear();
    mGesture = null;
    if (getRealView() != null &&
        getRealView() instanceof WXGestureObservable) {
      ((WXGestureObservable) getRealView()).registerGestureListener(null);
    }
    if(mHost != null) {
      mHost.setOnFocusChangeListener(null);
      if (mHostClickListeners != null && mHostClickListeners.size() > 0) {
        mHostClickListeners.clear();
        mHost.setOnClickListener(null);
      }
    }
  }

  public final void removeStickyStyle() {
    if (mDomObj == null ) {
      return;
    }

    if (isSticky()) {
      Scrollable scroller = getParentScroller();
      if (scroller != null) {
        scroller.unbindStickStyle(this);
      }
    }
  }

  public boolean isSticky() {
    return mDomObj.getStyles().isSticky();
  }

  public boolean isFixed() {
    return mDomObj.getStyles().isFixed();
  }

  public void setDisabled(boolean disabled) {
    mIsDisabled = disabled;
    if (mHost == null) {
      return;
    }
    mHost.setEnabled(!disabled);
  }

  public boolean isDisabled(){
    return mIsDisabled;
  }

  public void setSticky(String sticky) {
    if (!TextUtils.isEmpty(sticky) && sticky.equals(Constants.Value.STICKY)) {
      Scrollable waScroller = getParentScroller();
      if (waScroller != null) {
        waScroller.bindStickStyle(this);
      }
    }
  }

  public void setBackgroundColor(String color) {
    if (!TextUtils.isEmpty(color)) {
      int colorInt = WXResourceUtils.getColor(color);
      if (isRippleEnabled() && Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        mRippleBackground = prepareBackgroundRipple();
        if (mRippleBackground != null) {
          if (mBackgroundDrawable == null) {
            WXViewUtils.setBackGround(mHost, mRippleBackground);
          } else {
            LayerDrawable layerDrawable = new LayerDrawable(new Drawable[]{mRippleBackground, mBackgroundDrawable});
            WXViewUtils.setBackGround(mHost, layerDrawable);
          }
          return;
        }
      }
      if (!(colorInt == Color.TRANSPARENT && mBackgroundDrawable == null)) {
        getOrCreateBorder().setColor(colorInt);
      }
    }
  }

  private Drawable prepareBackgroundRipple() {
    try {
      if (getDomObject().getStyles() != null && getDomObject().getStyles().getPesudoResetStyles() != null) {
        Map<String, Object> resetStyles = getDomObject().getStyles().getPesudoResetStyles();

        Object bgColor = resetStyles.get(Constants.Name.BACKGROUND_COLOR);
        int colorInt = Color.TRANSPARENT;
        if (bgColor != null) {
          colorInt = WXResourceUtils.getColor(bgColor.toString(), Color.TRANSPARENT);
          if (colorInt == Color.TRANSPARENT) {
            return null;
          }
        }

        Object bg = resetStyles.get(Constants.Name.BACKGROUND_COLOR + Constants.PSEUDO.ACTIVE);
        if (bg == null) {
          return null;
        }
        int rippleColor = WXResourceUtils.getColor(bg.toString(), colorInt);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
          ColorStateList colorStateList = new ColorStateList(
              new int[][]{new int[]{}},
              new int[]{rippleColor});
          return new RippleDrawable(colorStateList, new ColorDrawable(colorInt), null) {
            @Override
            public void draw(@NonNull Canvas canvas) {
              if (mBackgroundDrawable != null) {
                Path border = mBackgroundDrawable.getContentPath(new RectF(0, 0, canvas.getWidth(), canvas.getHeight()));
                canvas.clipPath(border);
              }
              super.draw(canvas);
            }
          };
        }
      }
    } catch (Throwable t) {
      WXLogUtils.w("Exception on create ripple: ", t);
    }
    return null;
  }

  public void setBackgroundImage(@NonNull String bgImage) {
    if ("".equals(bgImage.trim())) {
      getOrCreateBorder().setImage(null);
    } else {
      Shader shader = WXResourceUtils.getShader(bgImage, mDomObj.getLayoutWidth(), mDomObj.getLayoutHeight());
      getOrCreateBorder().setImage(shader);
    }
  }

  public void setOpacity(float opacity) {
    if (opacity >= 0 && opacity <= 1 && mHost.getAlpha() != opacity) {
      if (isLayerTypeEnabled()) {
        mHost.setLayerType(View.LAYER_TYPE_HARDWARE, null);
      }
      mHost.setAlpha(opacity);
    }
  }

  public void setBorderRadius(String key, float borderRadius) {
    if (borderRadius >= 0) {
      switch (key) {
        case Constants.Name.BORDER_RADIUS:
          getOrCreateBorder().setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, WXViewUtils.getRealSubPxByWidth(borderRadius,mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_TOP_LEFT_RADIUS:
          getOrCreateBorder().setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, WXViewUtils.getRealSubPxByWidth(borderRadius,mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
          getOrCreateBorder().setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, WXViewUtils.getRealSubPxByWidth(borderRadius,mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
          getOrCreateBorder().setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, WXViewUtils.getRealSubPxByWidth(borderRadius,mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
          getOrCreateBorder().setBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS, WXViewUtils.getRealSubPxByWidth(borderRadius,mInstance.getInstanceViewPortWidth()));
          break;
      }
    }
  }

  public void setBorderWidth(String key, float borderWidth) {
    if (borderWidth >= 0) {
      switch (key) {
        case Constants.Name.BORDER_WIDTH:
          getOrCreateBorder().setBorderWidth(Spacing.ALL, WXViewUtils.getRealSubPxByWidth(borderWidth,getInstance().getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_TOP_WIDTH:
          getOrCreateBorder().setBorderWidth(Spacing.TOP, WXViewUtils.getRealSubPxByWidth(borderWidth,getInstance().getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_RIGHT_WIDTH:
          getOrCreateBorder().setBorderWidth(Spacing.RIGHT, WXViewUtils.getRealSubPxByWidth(borderWidth,getInstance().getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_BOTTOM_WIDTH:
          getOrCreateBorder().setBorderWidth(Spacing.BOTTOM, WXViewUtils.getRealSubPxByWidth(borderWidth,getInstance().getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_LEFT_WIDTH:
          getOrCreateBorder().setBorderWidth(Spacing.LEFT, WXViewUtils.getRealSubPxByWidth(borderWidth,getInstance().getInstanceViewPortWidth()));
          break;
      }
    }
  }

  public void setBorderStyle(String key, String borderStyle) {
    if(!TextUtils.isEmpty(borderStyle)){
      switch (key){
        case Constants.Name.BORDER_STYLE:
          getOrCreateBorder().setBorderStyle(Spacing.ALL,borderStyle);
          break;
        case Constants.Name.BORDER_RIGHT_STYLE:
          getOrCreateBorder().setBorderStyle(Spacing.RIGHT,borderStyle);
          break;
        case Constants.Name.BORDER_BOTTOM_STYLE:
          getOrCreateBorder().setBorderStyle(Spacing.BOTTOM,borderStyle);
          break;
        case Constants.Name.BORDER_LEFT_STYLE:
          getOrCreateBorder().setBorderStyle(Spacing.LEFT,borderStyle);
          break;
        case Constants.Name.BORDER_TOP_STYLE:
          getOrCreateBorder().setBorderStyle(Spacing.TOP,borderStyle);
          break;
      }
    }
  }

  public void setBorderColor(String key, String borderColor) {
    if (!TextUtils.isEmpty(borderColor)) {
      int colorInt = WXResourceUtils.getColor(borderColor);
      if (colorInt != Integer.MIN_VALUE) {
        switch (key) {
          case Constants.Name.BORDER_COLOR:
            getOrCreateBorder().setBorderColor(Spacing.ALL, colorInt);
            break;
          case Constants.Name.BORDER_TOP_COLOR:
            getOrCreateBorder().setBorderColor(Spacing.TOP, colorInt);
            break;
          case Constants.Name.BORDER_RIGHT_COLOR:
            getOrCreateBorder().setBorderColor(Spacing.RIGHT, colorInt);
            break;
          case Constants.Name.BORDER_BOTTOM_COLOR:
            getOrCreateBorder().setBorderColor(Spacing.BOTTOM, colorInt);
            break;
          case Constants.Name.BORDER_LEFT_COLOR:
            getOrCreateBorder().setBorderColor(Spacing.LEFT, colorInt);
            break;
        }
      }
    }
  }

  public
  @Nullable
  String getVisibility() {
    try {
      return (String) getDomObject().getStyles().get(Constants.Name.VISIBILITY);
    } catch (Exception e) {
      return Constants.Value.VISIBLE;
    }
  }

  public void setVisibility(String visibility) {
    View view;
    if ((view = getRealView()) != null) {
      if (TextUtils.equals(visibility, Constants.Value.VISIBLE)) {
        view.setVisibility(View.VISIBLE);
      } else if (TextUtils.equals(visibility, Constants.Value.HIDDEN)) {
        view.setVisibility(View.GONE);
      }
    }
  }

  /**
   * This is an experimental feature for elevation of material design.
   */
  private void updateElevation() {
    float elevation = getDomObject().getAttrs().getElevation(getInstance().getInstanceViewPortWidth());
    if (!Float.isNaN(elevation)) {
      ViewCompat.setElevation(getHostView(), elevation);
    }
  }

  @Deprecated
  public void registerActivityStateListener() {

  }


  /********************************
   *  begin hook Activity life cycle callback
   ********************************************************/
  public void onActivityCreate() {

  }

  public void onActivityStart() {

  }

  public void onActivityPause() {

  }

  public void onActivityResume() {

  }

  public void onActivityStop() {

  }

  public void onActivityDestroy() {

  }

  public boolean onActivityBack() {
    return false;
  }

  public void onActivityResult(int requestCode, int resultCode, Intent data){

  }

  public boolean onCreateOptionsMenu(Menu menu){return false;}

  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {

  }

  /********************************
   *  end hook Activity life cycle callback
   ********************************************************/
  @CallSuper
  public void recycled() {
    if(mDomObj.isFixed())
      return;

    clearBoxShadow();
  }

  public void destroy() {
    ComponentObserver observer;
    if ((observer = getInstance().getComponentObserver()) != null) {
      observer.onPreDestory(this);
    }

    if (WXEnvironment.isApkDebugable() && !WXUtils.isUiThread()) {
      throw new WXRuntimeException("[WXComponent] destroy can only be called in main thread");
    }
    if(mHost!= null && mHost.getLayerType()==View.LAYER_TYPE_HARDWARE && isLayerTypeEnabled()) {
      mHost.setLayerType(View.LAYER_TYPE_NONE, null);
    }
    removeAllEvent();
    removeStickyStyle();

    View view;
    if(mDomObj.isFixed() && (view = getHostView()) != null){
      getInstance().removeFixedView(view);
    }

    mDomObj = ImmutableDomObject.DESTROYED;
    mIsDestroyed = true;
  }

  public boolean isDestoryed() {
    return mIsDestroyed;
  }

  /**
   * Detach view from its component. Components,
   * which have difference between getHostView and getRealView or have temp calculation results,
   * must<strong> override</strong>  this method with their own implementation.
   *
   * @return the original View
   */
  public View detachViewAndClearPreInfo() {
    View original = mHost;
    mPreRealLeft = 0;
    mPreRealWidth = 0;
    mPreRealHeight = 0;
    mPreRealTop = 0;
//    mHost = null;
    return original;
  }

  public void  clearPreLayout(){
    mPreRealLeft = 0;
    mPreRealWidth = 0;
    mPreRealHeight = 0;
    mPreRealTop = 0;
  }

  /**
   * This method computes user visible left-top point in view's coordinate.
   * The default implementation uses the scrollX and scrollY of the view as the result,
   * and put the value in the parameter pointer.
   * Components with different computation algorithm
   * <strong> should override </strong> this method.
   *
   * @param pointF the user visible left-top point in view's coordinate.
   */
  public void computeVisiblePointInViewCoordinate(PointF pointF) {
    View view = getRealView();
    pointF.set(view.getScrollX(), view.getScrollY());
  }

  public boolean containsGesture(WXGestureType WXGestureType) {
    return mGestureType != null && mGestureType.contains(WXGestureType.toString());
  }

  public boolean containsEvent(String event){
    return mDomObj.getEvents().contains(event) || mAppendEvents.contains(event);
  }

  public void notifyAppearStateChange(String wxEventType,String direction){
    if(containsEvent(wxEventType)) {
       Map<String, Object> params = new HashMap<>();
       params.put("direction", direction);
       fireEvent(wxEventType, params);
    }
  }


  public boolean isUsing() {
    return isUsing;
  }

  public void setUsing(boolean using) {
    isUsing = using;
  }

  public void readyToRender() {
    if (mParent != null && getInstance().isTrackComponent()) {
      mParent.readyToRender();
    }
  }

  public static class MeasureOutput {

    public int width;
    public int height;
  }

  /**
   * Determine whether the current component needs to be placed in the real View tree
   * @return false component add subview
   */
  public boolean isVirtualComponent(){
    return mType == TYPE_VIRTUAL;
  }
  public void removeVirtualComponent() {
  }

  public void setType(int type) {
    mType = type;
  }

  public boolean hasScrollParent(WXComponent component) {
    if (component.getParent() == null) {
      return true;
    } else if (component.getParent() instanceof WXScroller) {
      return false;
    } else {
      return hasScrollParent(component.getParent());
    }
  }

  /**
   * Called when property has empty value
   * @param propName
     */
  @CheckResult
  protected Object convertEmptyProperty(String propName, Object originalValue) {
    switch (propName) {
      case Constants.Name.BACKGROUND_COLOR:
        return "transparent";
      case Constants.Name.BORDER_RADIUS:
      case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
      case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
      case Constants.Name.BORDER_TOP_LEFT_RADIUS:
      case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
        return 0;
      case Constants.Name.BORDER_WIDTH:
      case Constants.Name.BORDER_TOP_WIDTH:
      case Constants.Name.BORDER_LEFT_WIDTH:
      case Constants.Name.BORDER_RIGHT_WIDTH:
      case Constants.Name.BORDER_BOTTOM_WIDTH:
        return 0;
      case Constants.Name.BORDER_COLOR:
      case Constants.Name.BORDER_TOP_COLOR:
      case Constants.Name.BORDER_LEFT_COLOR:
      case Constants.Name.BORDER_RIGHT_COLOR:
      case Constants.Name.BORDER_BOTTOM_COLOR:
        return "black";
    }
    return originalValue;
  }

  private void setActiveTouchListener(){
    boolean hasActivePesudo = mDomObj.getStyles().getPesudoStyles().containsKey(Constants.PSEUDO.ACTIVE);
    View view;
    if(hasActivePesudo && (view = getRealView()) != null) {
      boolean hasTouchConsumer = isConsumeTouch();
      view.setOnTouchListener(new TouchActivePseudoListener(this,!hasTouchConsumer));
    }
  }

  protected boolean isConsumeTouch(){
    return (mHostClickListeners != null && mHostClickListeners.size() > 0) || mGesture != null;
  }

  @Override
  public void updateActivePseudo(boolean isSet) {
    setPseudoClassStatus(Constants.PSEUDO.ACTIVE,isSet);
  }

  /**
   *
   * @param clzName like ':active' or ':active:enabled'
   * @param status
   */
  protected void setPseudoClassStatus(String clzName,boolean status){
    WXStyle styles = getDomObject().getStyles();
    Map<String, Map<String,Object>> pesudoStyles = styles.getPesudoStyles();

    if(pesudoStyles == null || pesudoStyles.size() == 0){
      return;
    }
    Map<String,Object> resultStyles = mPesudoStatus.updateStatusAndGetUpdateStyles(
        clzName,
        status,
        pesudoStyles,
        styles.getPesudoResetStyles());

    if (resultStyles != null && isRippleEnabled()) {
      resultStyles.remove(Constants.Name.BACKGROUND_COLOR);
      if (resultStyles.isEmpty()) {
        WXLogUtils.d("PseudoClass", "skip empty pseudo styles");
        return;
      }
    }

    updateStyleByPesudo(resultStyles);
  }

  private void updateStyleByPesudo(Map<String,Object> styles){
    Message message = Message.obtain();
    WXDomTask task = new WXDomTask();
    task.instanceId = getInstanceId();
    task.args = new ArrayList<>();

    JSONObject styleJson = new JSONObject(styles);
    task.args.add(getRef());
    task.args.add(styleJson);
    task.args.add(true);//flag pesudo
    message.obj = task;
    message.what = WXDomHandler.MsgType.WX_DOM_UPDATE_STYLE;
    WXSDKManager.getInstance().getWXDomManager().sendMessage(message);
  }

  public int getStickyOffset() {
    return mStickyOffset;
  }

  public boolean canRecycled(){
    return (!isFixed() || !isSticky()) && mDomObj.getAttrs().canRecycled();
  }
  
  /**
   * Sets the offset for the sticky
   * @param stickyOffset child[y]-parent[y]
   */
  public void setStickyOffset(int stickyOffset) {
    mStickyOffset = stickyOffset;
  }

  /**
   * For now, this method respect the result of {@link WXSDKInstance#isLayerTypeEnabled()}
   * @return Refer {@link WXSDKInstance#isLayerTypeEnabled()}
   */
  public boolean isLayerTypeEnabled() {
    return getInstance().isLayerTypeEnabled();
  }

  /**
   * Sets whether or not to relayout page during animation, default is false
   */
  public void setNeedLayoutOnAnimation(boolean need) {
    this.mNeedLayoutOnAnimation = need;
  }

  /**
   * Trigger a updateStyle invoke to relayout current page
   */
  public void notifyNativeSizeChanged(int w, int h) {
    if (!mNeedLayoutOnAnimation) {
      return;
    }

    Message message = Message.obtain();
    WXDomTask task = new WXDomTask();
    task.instanceId = getInstanceId();
    if (task.args == null) {
      task.args = new ArrayList<>();
    }

    JSONObject style = new JSONObject(2);
    float webW = WXViewUtils.getWebPxByWidth(w);
    float webH = WXViewUtils.getWebPxByWidth(h);

    style.put("width", webW);
    style.put("height", webH);

    task.args.add(getRef());
    task.args.add(style);
    message.obj = task;
    message.what = WXDomHandler.MsgType.WX_DOM_UPDATE_STYLE;
    WXSDKManager.getInstance().getWXDomManager().sendMessage(message);
  }

  public static final int STATE_DOM_FINISH = 0;
  public static final int STATE_UI_FINISH = 1;
  public static final int STATE_ALL_FINISH = 2;
  @IntDef({STATE_DOM_FINISH, STATE_UI_FINISH, STATE_ALL_FINISH})
  @Retention(RetentionPolicy.SOURCE)
  @Target(ElementType.PARAMETER)
  public @interface RenderState {}

  @CallSuper
  public void onRenderFinish(@RenderState int state) {
    if (WXTracing.isAvailable()) {
      double domTime = Stopwatch.nanosToMillis(((WXDomObject) mDomObj).mDomThreadNanos + mTraceInfo.domThreadNanos);
      double uiTime = Stopwatch.nanosToMillis(mTraceInfo.uiThreadNanos);
      if (state == STATE_ALL_FINISH || state == STATE_DOM_FINISH) {
        WXTracing.TraceEvent domEvent = WXTracing.newEvent("DomExecute", getInstanceId(), mTraceInfo.rootEventId);
        domEvent.ph = "X";
        domEvent.duration = domTime;
        domEvent.ts = mTraceInfo.domThreadStart;
        domEvent.tname = "DOMThread";
        domEvent.name = getDomObject().getType();
        domEvent.classname = getClass().getSimpleName();
        if (getParent() != null) {
          domEvent.parentRef = getParent().getRef();
        }
        domEvent.submit();
      }

      if (state == STATE_ALL_FINISH || state == STATE_UI_FINISH) {
        if (mTraceInfo.uiThreadStart != -1) {
          WXTracing.TraceEvent uiEvent = WXTracing.newEvent("UIExecute", getInstanceId(), mTraceInfo.rootEventId);
          uiEvent.ph = "X";
          uiEvent.duration = uiTime;
          uiEvent.ts = mTraceInfo.uiThreadStart;
          uiEvent.name = getDomObject().getType();
          uiEvent.classname = getClass().getSimpleName();
          if (getParent() != null) {
            uiEvent.parentRef = getParent().getRef();
          }
          uiEvent.submit();
        } else {
          if(WXEnvironment.isApkDebugable() && !isLazy()) {
            WXLogUtils.w("onRenderFinish", "createView() not called");
          }
        }
      }
    }
  }

  protected boolean isRippleEnabled() {
    try {
      Object obj = getDomObject().getAttrs().get(Constants.Name.RIPPLE_ENABLED);
      return WXUtils.getBoolean(obj, false);
    } catch (Throwable t) {
      //ignore
    }
    return false;
  }



  public boolean isWaste() {
    return waste;
  }

  /**
   * mark node waste,
   * if node is waster should hidden, and dom tree should allow not show
   * */
  public void setWaste(boolean waste) {
    if(this.waste != waste){
      this.waste = waste;
      WXDomObject domObject = (WXDomObject) getDomObject();
      if(waste){
          //update dom not show, and put style to hidden
         domObject.setVisible(false);
         domObject.getStyles().put(Constants.Name.VISIBILITY, Constants.Value.HIDDEN);
         //if component not init, mark lazy init when use, reduce view count
         if(getHostView() == null){
           if(!mLazy){
              lazy(true);
           }
         }else{
           getHostView().setVisibility(View.GONE);
         }
      }else{
         domObject.setVisible(true);
         domObject.getStyles().put(Constants.Name.VISIBILITY, Constants.Value.VISIBLE);
         if(getHostView() == null){
             if(mLazy) { // when parent is lazy just mark node lazy false
               if(mParent != null && mParent.isLazy()){
                  lazy(false);
               }else{
                  ComponentUtils.initLazyComponent(this, mParent);
               }
             }
         }else{
           getHostView().setVisibility(View.VISIBLE);
         }
      }
    }
  }


  /** component key id in native,
   *  differ with ref, ref + position
   *  */
  public  String getViewTreeKey(){
     if(mViewTreeKey == null){
        if(getParent() == null){
          mViewTreeKey = getRef();
        }else{
          mViewTreeKey = getRef() + "_" + getParent().indexOf(this);
        }
     }
     return mViewTreeKey;
  }

  private String mViewTreeKey;


  /**
   * node is lazy
   * */
  private boolean mLazy = false;

  /***/
  public void lazy(boolean lazy) {
    mLazy = lazy;
  }
}