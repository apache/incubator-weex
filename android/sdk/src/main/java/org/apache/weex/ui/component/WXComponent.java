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
import android.support.annotation.CallSuper;
import android.support.annotation.CheckResult;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.v4.view.AccessibilityDelegateCompat;
import android.support.v4.view.ViewCompat;
import android.support.v4.view.accessibility.AccessibilityNodeInfoCompat;
import android.text.TextUtils;
import android.util.Pair;
import android.view.Gravity;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewOverlay;
import android.widget.FrameLayout;
import com.alibaba.fastjson.JSONArray;
import org.apache.weex.ComponentObserver;
import org.apache.weex.IWXActivityStateListener;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.IWXAccessibilityRoleAdapter;
import org.apache.weex.adapter.IWXConfigAdapter;
import org.apache.weex.bridge.EventResult;
import org.apache.weex.bridge.Invoker;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.common.Constants;
import org.apache.weex.common.IWXObject;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXPerformance;
import org.apache.weex.common.WXRuntimeException;
import org.apache.weex.dom.CSSShorthand;
import org.apache.weex.dom.CSSShorthand.CORNER;
import org.apache.weex.dom.WXEvent;
import org.apache.weex.dom.WXStyle;
import org.apache.weex.dom.transition.WXTransition;
import org.apache.weex.layout.ContentBoxMeasurement;
import org.apache.weex.performance.WXAnalyzerDataTransfer;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.tracing.Stopwatch;
import org.apache.weex.tracing.WXTracing;
import org.apache.weex.ui.IFComponentHolder;
import org.apache.weex.ui.WXRenderManager;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.action.GraphicActionAnimation;
import org.apache.weex.ui.action.GraphicActionUpdateStyle;
import org.apache.weex.ui.action.GraphicPosition;
import org.apache.weex.ui.action.GraphicSize;
import org.apache.weex.ui.animation.WXAnimationBean;
import org.apache.weex.ui.component.basic.WXBasicComponent;
import org.apache.weex.ui.component.binding.Statements;
import org.apache.weex.ui.component.list.WXCell;
import org.apache.weex.ui.component.list.template.jni.NativeRenderObjectUtils;
import org.apache.weex.ui.component.pesudo.OnActivePseudoListner;
import org.apache.weex.ui.component.pesudo.PesudoStatus;
import org.apache.weex.ui.component.pesudo.TouchActivePseudoListener;
import org.apache.weex.ui.flat.FlatComponent;
import org.apache.weex.ui.flat.FlatGUIContext;
import org.apache.weex.ui.flat.WidgetContainer;
import org.apache.weex.ui.flat.widget.AndroidViewWidget;
import org.apache.weex.ui.flat.widget.Widget;
import org.apache.weex.ui.view.border.BorderDrawable;
import org.apache.weex.ui.view.gesture.WXGesture;
import org.apache.weex.ui.view.gesture.WXGestureObservable;
import org.apache.weex.ui.view.gesture.WXGestureType;
import org.apache.weex.utils.BoxShadowUtil;
import org.apache.weex.utils.WXDataStructureUtil;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXReflectionUtils;
import org.apache.weex.utils.WXResourceUtils;
import org.apache.weex.utils.WXUtils;
import org.apache.weex.utils.WXViewUtils;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.apache.weex.common.Constants.Event;
import org.apache.weex.tracing.WXTracing.TraceInfo;
import org.apache.weex.ui.animation.WXAnimationModule.AnimationHolder;

/**
 * abstract component
 */
public abstract class WXComponent<T extends View> extends WXBasicComponent implements IWXObject,
    IWXActivityStateListener, OnActivePseudoListner {

  public static final String PROP_FIXED_SIZE = "fixedSize";
  public static final String PROP_FS_MATCH_PARENT = "m";
  public static final String PROP_FS_WRAP_CONTENT = "w";
  public static final String TYPE = "type";
  public static final String ROOT = "_root";

  private int mFixedProp = 0;
  /** package **/ T mHost;

  private volatile WXVContainer mParent;
  private WXSDKInstance mInstance;
  private Context mContext;

  private int mAbsoluteY = 0;
  private int mAbsoluteX = 0;
  private boolean isLastLayoutDirectionRTL = false;
  @Nullable
  private Set<String> mGestureType;

  private BorderDrawable mBackgroundDrawable;
  private Drawable mRippleBackground;
  private int mPreRealWidth = 0;
  private int mPreRealHeight = 0;
  private int mPreRealLeft = 0;
  private int mPreRealRight = 0;
  private int mPreRealTop = 0;
  private int mStickyOffset = 0;
  protected WXGesture mGesture;
  private IFComponentHolder mHolder;
  private boolean isUsing = false;
  private List<OnClickListener> mHostClickListeners;
  private List<OnFocusChangeListener> mFocusChangeListeners;
  private Set<String> mAppendEvents;
  private AnimationHolder mAnimationHolder;
  private PesudoStatus mPesudoStatus;
  private boolean mIsDestroyed = false;
  private boolean mIsDisabled = false;
  private int mType = TYPE_COMMON;
  private boolean mNeedLayoutOnAnimation = false;
  private String mLastBoxShadowId;
  public int mDeepInComponentTree = 0;
  public boolean mIsAddElementToTree = false;
  //for fix element case
  public int interactionAbsoluteX=0,interactionAbsoluteY=0;
  //for fix slider case :cssLeft is not real left base parent;
  protected int mChildrensWidth = 0;
  private boolean mHasAddFocusListener = false;

  public TraceInfo mTraceInfo = new TraceInfo();

  public static final int TYPE_COMMON = 0;
  public static final int TYPE_VIRTUAL = 1;

  private boolean waste = false;
  public boolean isIgnoreInteraction = false;

  protected ContentBoxMeasurement contentBoxMeasurement;
  private WXTransition mTransition;
  private GraphicSize mPseudoResetGraphicSize;
  @Nullable
  private ConcurrentLinkedQueue<Pair<String, Map<String, Object>>> animations;

  @Deprecated
  public WXComponent(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }

  @Deprecated
  public WXComponent(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXComponent(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    this(instance, parent, TYPE_COMMON, basicComponentData);
  }

  public WXComponent(WXSDKInstance instance, WXVContainer parent, int type, BasicComponentData basicComponentData) {
    super(basicComponentData);
    mInstance = instance;
    mContext = mInstance.getContext();
    mParent = parent;
    mType = type;

    if (instance != null)
      setViewPortWidth(instance.getInstanceViewPortWidth());

    onCreate();
    ComponentObserver observer;
    if ((observer = getInstance().getComponentObserver()) != null) {
      observer.onCreate(this);
    }
  }



  @Override
  protected final void bindComponent(WXComponent component) {
    super.bindComponent(component);
    if (getInstance() != null) {
      setViewPortWidth(getInstance().getInstanceViewPortWidth());
    }
    mParent = component.getParent();
    mType = component.getType();
  }

  protected final void setContentBoxMeasurement(final ContentBoxMeasurement contentBoxMeasurement) {
    this.contentBoxMeasurement = contentBoxMeasurement;
    mInstance.addContentBoxMeasurement(getRenderObjectPtr(), contentBoxMeasurement);
    WXBridgeManager.getInstance().bindMeasurementToRenderObject(getRenderObjectPtr());
  }


  @SuppressLint("RtlHardcoded")
  public void setMarginsSupportRTL(ViewGroup.MarginLayoutParams lp, int left, int top, int right, int bottom) {
      lp.setMargins(left, top, right, bottom);
      if (lp instanceof FrameLayout.LayoutParams) {
          FrameLayout.LayoutParams lp_frameLayout = (FrameLayout.LayoutParams) lp;
          lp_frameLayout.gravity = Gravity.LEFT | Gravity.TOP;
      }
  }

  public void updateStyles(WXComponent component) {
    if (component != null) {
      updateProperties(component.getStyles());
      applyBorder(component);
    }
  }

  public void updateStyles(Map<String, Object> styles) {
    if (styles != null) {
      updateProperties(styles);
      applyBorder(this);
    }
  }

  public void updateAttrs(WXComponent component) {
    if (component != null) {
      updateProperties(component.getAttrs());
    }
  }

  public void updateAttrs(Map<String, Object> attrs) {
    if (attrs != null) {
      updateProperties(attrs);
    }
  }

  private void applyBorder(WXComponent component) {
    CSSShorthand border = component.getBorder();
    float left = border.get(CSSShorthand.EDGE.LEFT);
    float top = border.get(CSSShorthand.EDGE.TOP);
    float right = border.get(CSSShorthand.EDGE.RIGHT);
    float bottom = border.get(CSSShorthand.EDGE.BOTTOM);

    if (mHost == null) {
      return;
    }

    setBorderWidth(Constants.Name.BORDER_LEFT_WIDTH, left);
    setBorderWidth(Constants.Name.BORDER_TOP_WIDTH, top);
    setBorderWidth(Constants.Name.BORDER_RIGHT_WIDTH, right);
    setBorderWidth(Constants.Name.BORDER_BOTTOM_WIDTH, bottom);
  }

  public void setPadding(CSSShorthand padding, CSSShorthand border) {
    int left = (int) (padding.get(CSSShorthand.EDGE.LEFT) + border.get(CSSShorthand.EDGE.LEFT));
    int top = (int) (padding.get(CSSShorthand.EDGE.TOP) + border.get(CSSShorthand.EDGE.TOP));
    int right = (int) (padding.get(CSSShorthand.EDGE.RIGHT) + border.get(CSSShorthand.EDGE.RIGHT));
    int bottom = (int) (padding.get(CSSShorthand.EDGE.BOTTOM) + border.get(CSSShorthand.EDGE.BOTTOM));

    if (this instanceof FlatComponent && !((FlatComponent) this).promoteToView(true)) {
      ((FlatComponent) this).getOrCreateFlatWidget().setContentBox(left, top, right, bottom);
    } else if (mHost != null) {
      mHost.setPadding(left, top, right, bottom);
    }
  }


  public void applyComponentEvents(){
       applyEvents();
  }

  private void applyEvents() {
    if (getEvents() == null || getEvents().isEmpty())
      return;
    WXEvent event = getEvents();
    int size = event.size();
    for (int i=0; i<size; i++) {
      if(i >= event.size()){
        break;
      }
      String type = event.get(i);
      addEvent(type);
    }
    setActiveTouchListener();
  }

  /**
   * Do not use this method to add event, this only apply event already add to DomObject.
   *
   * @param type
   */
  public void addEvent(final String type) {
    if (mAppendEvents == null) {
      mAppendEvents = new HashSet<>();
    }
    if (TextUtils.isEmpty(type) || mAppendEvents.contains(type)) {
      return;
    }
    final View view = getRealView();

    if (type.equals(Constants.Event.LAYEROVERFLOW))
      addLayerOverFlowListener(getRef());

    if (type.equals(Constants.Event.CLICK)) {
      if (view == null) {
        // wait next time to add.
        return;
      }
      if(mClickEventListener == null){
        mClickEventListener = new OnClickListenerImp();
      }
      addClickListener(mClickEventListener);
    } else if ((type.equals(Constants.Event.FOCUS) || type.equals(Constants.Event.BLUR))) {
      if (!mHasAddFocusListener){
        mHasAddFocusListener = true;
        addFocusChangeListener(new OnFocusChangeListener() {
        @Override
        public void onFocusChange(boolean hasFocus) {
          Map<String, Object> params = new HashMap<>();
          params.put("timeStamp", System.currentTimeMillis());
          fireEvent(hasFocus ? Constants.Event.FOCUS : Constants.Event.BLUR, params);
        }
      });
      }
    } else if (needGestureDetector(type)) {
      if (null == view) {
        // wait next time to add.
        return;
      }
      if (view instanceof WXGestureObservable) {
        if (mGesture == null) {
          mGesture = new WXGesture(this, mContext);
          boolean isPreventMove = WXUtils.getBoolean(getAttrs().get(Constants.Name.PREVENT_MOVE_EVENT), false);
          mGesture.setPreventMoveEvent(isPreventMove);
        }
        if (mGestureType == null) {
          mGestureType = new HashSet<>();
        }
        mGestureType.add(type);
        ((WXGestureObservable)view).registerGestureListener(mGesture);
      } else {
        WXLogUtils.e(view.getClass().getSimpleName() + " don't implement " +
            "WXGestureObservable, so no gesture is supported.");
      }
    } else {
      final Scrollable scroller = getParentScroller();
      if (scroller == null) {
        // wait next time to add.
        return;
      }
      if (type.equals(Constants.Event.APPEAR)) {
        scroller.bindAppearEvent(this);
      } else if (type.equals(Constants.Event.DISAPPEAR)) {
        scroller.bindDisappearEvent(this);
      }
    }
    // Final add to mAppendEvents.
    mAppendEvents.add(type);
  }

  protected void onCreate() {

  }

  public void bindHolder(IFComponentHolder holder) {
    mHolder = holder;
  }


  public WXSDKInstance getInstance() {
    return mInstance;
  }

  public Context getContext() {
    return mContext;
  }

  /**
   * Find component by component reference.
   *
   * @param ref
   * @return
   */
  protected final WXComponent findComponent(String ref) {
    if (mInstance != null && ref != null) {
      return WXSDKManager.getInstance()
              .getWXRenderManager()
              .getWXComponent(mInstance.getInstanceId(), ref);
    }
    return null;
  }

  public String getAttrByKey(String key) {
    return "default";
  }

  //Holding the animation bean when component is uninitialized
  public void postAnimation(AnimationHolder holder) {
    this.mAnimationHolder = holder;
  }

  //This method will be removed once flatGUI is completed.
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public boolean isFlatUIEnabled() {
    return mParent != null && mParent.isFlatUIEnabled();
  }

  private class OnClickListenerImp implements OnClickListener{
    @Override
    public void onHostViewClick() {
      Map<String, Object> param = WXDataStructureUtil.newHashMapWithExpectedSize(1);
      Map<String, Object> position = WXDataStructureUtil.newHashMapWithExpectedSize(4);
      int[] location = new int[2];
      mHost.getLocationOnScreen(location);
      position.put("x", WXViewUtils.getWebPxByWidth(location[0], mInstance.getInstanceViewPortWidth()));
      position.put("y", WXViewUtils.getWebPxByWidth(location[1], mInstance.getInstanceViewPortWidth()));
      position.put("width", WXViewUtils.getWebPxByWidth(getLayoutWidth(), mInstance.getInstanceViewPortWidth()));
      position.put("height", WXViewUtils.getWebPxByWidth(getLayoutHeight(), mInstance.getInstanceViewPortWidth()));
      param.put(Constants.Name.POSITION, position);
      fireEvent(Constants.Event.CLICK, param);
    }
  };
  private OnClickListenerImp mClickEventListener;

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
      int width = (int) getLayoutWidth();
      int height = (int) getLayoutHeight();
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
                .invoke(this, invoker, args);

      } catch (Exception e) {
        WXLogUtils.e("[WXComponent] updateProperties :" + "class:" + getClass() + "method:" + invoker.toString() + " function " + WXLogUtils.getStackTrace(e));
      }
    } else {
      onInvokeUnknownMethod(method, args);
    }
  }

  /**
   * Will be invoked when request method not found.
   * Subclass should override this method, If you return hard-code method list in {@link IFComponentHolder#getMethods()}
   *
   * @param method
   * @param args
   */
  protected void onInvokeUnknownMethod(String method, JSONArray args) {

  }

  public interface OnClickListener {
    void onHostViewClick();
  }

  public interface OnFocusChangeListener{
    void onFocusChange(boolean hasFocus);
  }

  public final void fireEvent(String type){
    fireEvent(type,null);
  }

  public final void fireEvent(String type, Map<String, Object> params){
    if(WXUtils.getBoolean(getAttrs().get("fireEventSyn"), false)){
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
      waitLatch.await(50, TimeUnit.MILLISECONDS);
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
    if(mInstance != null) {
      List<Object> eventArgsValues = null;
      if(getEvents() != null && getEvents().getEventBindingArgsValues() != null){
        eventArgsValues = getEvents().getEventBindingArgsValues().get(type);
      }
      if(params != null){
        String componentId = Statements.getComponentId(this);
        if(componentId != null) {
          params.put("componentId", componentId);
        }
      }
      mInstance.fireEvent(getRef(), type, params,domChanges, eventArgsValues, callback);
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

  protected final void removeClickListener(OnClickListener l) {
    mHostClickListeners.remove(l);
  }

  public void bindData(WXComponent component) {
    if (!isLazy()) {
      if (component == null) {
        component = this;
      }
      bindComponent(component);
      updateStyles(component);
      updateAttrs(component);
      updateExtra(component.getExtra());
    }
  }

  public void applyLayoutAndEvent(WXComponent component) {
    if (!isLazy()) {
      if (component == null) {
        component = this;
      }
      bindComponent(component);
      setSafeLayout(component);
      setPadding(component.getPadding(), component.getBorder());
      applyEvents();
    }
  }

  public void setDemission(GraphicSize size, GraphicPosition position) {
    setLayoutPosition(position);
    setLayoutSize(size);
  }

  public void updateDemission(float top, float bottom, float left, float right, float height, float width) {
    getLayoutPosition().update(top, bottom, left, right);
    getLayoutSize().update(width, height);
  }


  public void applyLayoutOnly(){
    if(!isLazy()) {
      setSafeLayout(this);
      setPadding(this.getPadding(), this.getBorder());
    }
  }


  public void refreshData(WXComponent component) {

  }

  @Deprecated
  public void updateProperties(Map<String, Object> props) {
    if (props == null || (mHost == null && !isVirtualComponent())) {
      return;
    }

    for (Map.Entry<String, Object> entry : props.entrySet()) {
      Object key_obj = entry.getKey();
      String key = WXUtils.getString(key_obj, null);
      if ((key != null) && !(key_obj instanceof String)) {
        Map<String, String> map = new HashMap<>();
        map.put("componentType", getComponentType());
        map.put("actual key", key == null ? "" : key);
        WXExceptionUtils.commitCriticalExceptionRT(getInstanceId(),
            WXErrorCode.WX_RENDER_ERR_COMPONENT_ATTR_KEY,
            "WXComponent.updateProperties",
            WXErrorCode.WX_RENDER_ERR_COMPONENT_ATTR_KEY.getErrorMsg(),
            map);
      }

      Object param = entry.getValue();
      String value = WXUtils.getString(param, null);

      if (key == null) {
        WXExceptionUtils.commitCriticalExceptionRT(getInstanceId(),
                WXErrorCode.WX_RENDER_ERR_NULL_KEY, "updateProperties",
                WXErrorCode.WX_RENDER_ERR_NULL_KEY.getErrorMsg(), null);
      } else {
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
   *
   * @param key   name of argument
   * @param param value of argument
   * @return true means that the property is consumed
   */
  protected boolean setProperty(String key, Object param) {
    if(key == null){
      return true;
    }
    switch (key) {
      case Constants.Name.PREVENT_MOVE_EVENT:
        if (mGesture != null) {
          mGesture.setPreventMoveEvent(WXUtils.getBoolean(param, false));
        }
        return true;
      case Constants.Name.DISABLED:
        Boolean disabled = WXUtils.getBoolean(param, null);
        if (disabled != null) {
          setDisabled(disabled);
          setPseudoClassStatus(Constants.PSEUDO.DISABLED, disabled);
        }
        return true;
      case Constants.Name.POSITION:
        String position = WXUtils.getString(param, null);
        if (position != null)
          setSticky(position);
        return true;
      case Constants.Name.BACKGROUND_COLOR:
        String bgColor = WXUtils.getString(param, null);
        if (bgColor != null)
          setBackgroundColor(bgColor);
        return true;
      case Constants.Name.BACKGROUND_IMAGE:
        String bgImage = WXUtils.getString(param, null);
        if (bgImage != null && mHost != null) {
          setBackgroundImage(bgImage);
        }
        return true;
      case Constants.Name.OPACITY:
        Float opacity = WXUtils.getFloat(param, null);
        if (opacity != null)
          setOpacity(opacity);
        return true;
      case Constants.Name.BORDER_RADIUS:
      case Constants.Name.BORDER_TOP_LEFT_RADIUS:
      case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
      case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
      case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
        Float radius = WXUtils.getFloat(param, null);
        if (radius != null)
          setBorderRadius(key, radius);
        return true;
      case Constants.Name.BORDER_STYLE:
      case Constants.Name.BORDER_RIGHT_STYLE:
      case Constants.Name.BORDER_BOTTOM_STYLE:
      case Constants.Name.BORDER_LEFT_STYLE:
      case Constants.Name.BORDER_TOP_STYLE:
        String border_style = WXUtils.getString(param, null);
        if (border_style != null)
          setBorderStyle(key, border_style);
        return true;
      case Constants.Name.BORDER_COLOR:
      case Constants.Name.BORDER_TOP_COLOR:
      case Constants.Name.BORDER_RIGHT_COLOR:
      case Constants.Name.BORDER_BOTTOM_COLOR:
      case Constants.Name.BORDER_LEFT_COLOR:
        String border_color = WXUtils.getString(param, null);
        if (border_color != null)
          setBorderColor(key, border_color);
        return true;
      case Constants.Name.VISIBILITY:
        String visibility = WXUtils.getString(param, null);
        if (visibility != null)
          setVisibility(visibility);
        return true;
      case Constants.Name.ELEVATION:
        if (param != null) {
          updateElevation();
        }
        return true;
      case PROP_FIXED_SIZE:
        String fixedSize = WXUtils.getString(param, PROP_FS_MATCH_PARENT);
        setFixedSize(fixedSize);
        return true;
      case Constants.Name.ARIA_LABEL:
        String label = WXUtils.getString(param, "");
        setAriaLabel(label);
        return true;
      case Constants.Name.ARIA_HIDDEN:
        boolean isHidden = WXUtils.getBoolean(param, false);
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
      case Constants.Name.BORDER_WIDTH:
      case Constants.Name.BORDER_TOP_WIDTH:
      case Constants.Name.BORDER_RIGHT_WIDTH:
      case Constants.Name.BORDER_BOTTOM_WIDTH:
      case Constants.Name.BORDER_LEFT_WIDTH:
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

  protected BorderDrawable getOrCreateBorder() {
    if (mBackgroundDrawable == null) {
      mBackgroundDrawable = new BorderDrawable();
      if (mHost != null) {
        WXViewUtils.setBackGround(mHost, null, this);
        if (mRippleBackground == null) {
          WXViewUtils.setBackGround(mHost, mBackgroundDrawable, this);
        } else {
          //TODO Not strictly clip according to background-clip:border-box
          WXViewUtils.setBackGround(mHost, new LayerDrawable(new Drawable[]{
                  mRippleBackground, mBackgroundDrawable}), this);
        }
      }
    }
    return mBackgroundDrawable;
  }

  /**
   * layout view
   */
  public void setSafeLayout(WXComponent component) {
    if (TextUtils.isEmpty(component.getComponentType())
            || TextUtils.isEmpty(component.getRef()) || component.getLayoutPosition() == null
            || component.getLayoutSize() == null) {
      return;
    }
    setLayout(component);
  }

  /**
   * layout view
   */
  public void setLayout(WXComponent component) {
    setLayoutSize(component.getLayoutSize());
    setLayoutPosition(component.getLayoutPosition());
    setPaddings(component.getPadding());
    setMargins(component.getMargin());
    setBorders(component.getBorder());

    boolean isRTL = component.isLayoutRTL();
    setIsLayoutRTL(isRTL);
    if (isRTL != component.isLastLayoutDirectionRTL) {
      component.isLastLayoutDirectionRTL = isRTL;
      layoutDirectionDidChanged(isRTL);
    }

    parseAnimation();

    boolean nullParent = mParent == null;//parent is nullable

    //offset by sibling
    int siblingOffset = nullParent ? 0 : mParent.getChildrenLayoutTopOffset();

    CSSShorthand parentPadding = (nullParent ? new CSSShorthand() : mParent.getPadding());
    CSSShorthand parentBorder = (nullParent ? new CSSShorthand() : mParent.getBorder());

    int realWidth = (int) getLayoutSize().getWidth();
    int realHeight = (int) getLayoutSize().getHeight();

    int realLeft = 0;
    int realTop = 0;
    int realRight = 0;

    if (isFixed()) {
      realLeft = (int) (getLayoutPosition().getLeft() - getInstance().getRenderContainerPaddingLeft());
      realTop = (int) (getLayoutPosition().getTop() - getInstance().getRenderContainerPaddingTop()) + siblingOffset;
    } else {
      realLeft = (int) (getLayoutPosition().getLeft() -
              parentPadding.get(CSSShorthand.EDGE.LEFT) - parentBorder.get(CSSShorthand.EDGE.LEFT));
      realTop = (int) (getLayoutPosition().getTop() -
              parentPadding.get(CSSShorthand.EDGE.TOP) - parentBorder.get(CSSShorthand.EDGE.TOP)) + siblingOffset;
    }

    realRight = (int) getMargin().get(CSSShorthand.EDGE.RIGHT);
    int realBottom = (int) getMargin().get(CSSShorthand.EDGE.BOTTOM);

    Point rawOffset = new Point(
            (int) getLayoutPosition().getLeft(),
            (int) getLayoutPosition().getTop());

    if (mPreRealWidth == realWidth && mPreRealHeight == realHeight && mPreRealLeft == realLeft && mPreRealRight == realRight && mPreRealTop == realTop) {
      return;
    }

    if (this instanceof WXCell && realHeight >= WXPerformance.VIEW_LIMIT_HEIGHT && realWidth>=WXPerformance.VIEW_LIMIT_WIDTH){
      mInstance.getApmForInstance().updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_CELL_EXCEED_NUM,1);
      mInstance.getWXPerformance().cellExceedNum++;
      if (WXAnalyzerDataTransfer.isOpenPerformance){
        WXAnalyzerDataTransfer.transferPerformance(getInstanceId(),"details",WXInstanceApm.KEY_PAGE_STATS_CELL_EXCEED_NUM,
            String.format(Locale.ROOT, "cell:ref:%s,[w:%d,h:%d],attrs:%s,styles:%s",getRef(),realWidth,realHeight,getAttrs(),getStyles())
        );
      }

    }

    mAbsoluteY = (int) (nullParent ? 0 : mParent.getAbsoluteY() + getCSSLayoutTop());
    mAbsoluteX = (int) (nullParent ? 0 : mParent.getAbsoluteX() + getCSSLayoutLeft());

    if (mHost == null) {
      return;
    }

    //calculate first screen time
    if (!(mHost instanceof ViewGroup) && mAbsoluteY + realHeight > mInstance.getWeexHeight() + 1) {
      if (!mInstance.mEnd){
        mInstance.onOldFsRenderTimeLogic();
      }
      if (!mInstance.isNewFsEnd){
        mInstance.isNewFsEnd = true;
        mInstance.getApmForInstance().arriveNewFsRenderTime();
      }
    }

    MeasureOutput measureOutput = measure(realWidth, realHeight);
    realWidth = measureOutput.width;
    realHeight = measureOutput.height;

    setComponentLayoutParams(realWidth, realHeight, realLeft, realTop, realRight, realBottom, rawOffset);
  }

  private void setComponentLayoutParams(int realWidth, int realHeight, int realLeft, int realTop,
                                        int realRight, int realBottom, Point rawOffset) {
    if(getInstance() == null || getInstance().isDestroy()){
      return;
    }

    FlatGUIContext UIImp = getInstance().getFlatUIContext();
    WidgetContainer ancestor;
    Widget widget;
    if (UIImp != null && (ancestor = UIImp.getFlatComponentAncestor(this)) != null) {
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
      if (isFixed()) {
        setFixedHostLayoutParams(mHost, realWidth, realHeight, realLeft, realRight, realTop,
                realBottom);
      } else {
        setHostLayoutParams(mHost, realWidth, realHeight, realLeft, realRight, realTop, realBottom);
      }
      recordInteraction(realWidth,realHeight);
      mPreRealWidth = realWidth;
      mPreRealHeight = realHeight;
      mPreRealLeft = realLeft;
      mPreRealRight = realRight;
      mPreRealTop = realTop;
      onFinishLayout();
      // restore box shadow
      updateBoxShadow();
    }
  }

  /**
   * layout direction is changed
   * basic class is a empty implementation
   * subclass can override this method do some RTL necessary things
   * such as WXText
   */
  protected void layoutDirectionDidChanged(boolean isRTL) {

  }

  private void recordInteraction(int realWidth,int realHeight){
    if (!mIsAddElementToTree){
      return;
    }
    mIsAddElementToTree = false;
    if (null == mParent){
      interactionAbsoluteX = 0;
      interactionAbsoluteY = 0;
    }else {
      float cssTop = getCSSLayoutTop();
      float cssLeft = getCSSLayoutLeft();
      interactionAbsoluteX = (int)(this.isFixed() ? cssLeft : mParent.interactionAbsoluteX + mParent.mChildrensWidth + cssLeft);
      interactionAbsoluteY = (int)(this.isFixed() ? cssTop  : mParent.interactionAbsoluteY + cssTop);
      //fix for slider impl ,and interactionTime calculate if component is out screen
      if (WXBasicComponentType.SLIDER.equalsIgnoreCase(mParent.getComponentType()) || WXBasicComponentType.CYCLE_SLIDER.equalsIgnoreCase(mParent.getComponentType())){
        if (!WXBasicComponentType.INDICATOR.equalsIgnoreCase(getComponentType())){
          mParent.mChildrensWidth += (int)(realWidth + cssLeft);
        }
      }
    }

    if (null == getInstance().getApmForInstance().instanceRect){
      getInstance().getApmForInstance().instanceRect = new Rect();
    }
    Rect instanceRect = getInstance().getApmForInstance().instanceRect;
    instanceRect.set(0,0,mInstance.getWeexWidth(),mInstance.getWeexHeight());
    boolean inScreen =
          instanceRect.contains(interactionAbsoluteX,interactionAbsoluteY) //leftTop
              || instanceRect.contains(interactionAbsoluteX+realWidth,interactionAbsoluteY)//rightTop
              || instanceRect.contains(interactionAbsoluteX,interactionAbsoluteY+realHeight)//leftBottom
              || instanceRect.contains(interactionAbsoluteX+realWidth,interactionAbsoluteY+realHeight);//rightBottom
    mInstance.onChangeElement(this,!inScreen);
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
      if (lp instanceof ViewGroup.MarginLayoutParams) {
        width = lp.width;
        height = lp.height;
        left = ((ViewGroup.MarginLayoutParams) lp).leftMargin;
        right = ((ViewGroup.MarginLayoutParams) lp).rightMargin;
        top = ((ViewGroup.MarginLayoutParams) lp).topMargin;
        bottom = ((ViewGroup.MarginLayoutParams) lp).bottomMargin;
      }
    }
    widget.setLayout(width, height, left, right, top, bottom, childOffset);

    if (widget instanceof AndroidViewWidget && ((AndroidViewWidget) widget).getView()!=null) {
      //TODO generic method if ever possible
      setHostLayoutParams((T) ((AndroidViewWidget) widget).getView(),
              width, height, childOffset.x, right, childOffset.y, bottom);
    }
  }

  public int getLayoutTopOffsetForSibling() {
    return 0;
  }

  protected void setHostLayoutParams(T host, int width, int height, int left, int right, int top, int bottom) {
    ViewGroup.LayoutParams lp;
    if (mParent == null) {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(width, height);
        this.setMarginsSupportRTL(params, left, top, right, bottom);
        lp = params;
    } else {
        lp = mParent.getChildLayoutParams(this, host, width, height, left, right, top, bottom);
    }
    if (lp != null) {
        host.setLayoutParams(lp);
    }
  }

  private void setFixedHostLayoutParams(T host, int width, int height, int left, int right, int top, int bottom){
    FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);

    params.width = width;
    params.height = height;

    this.setMarginsSupportRTL(params, left, top, right, bottom);

    host.setLayoutParams(params);
    mInstance.moveFixedView(host);

    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("Weex_Fixed_Style", "WXComponent:setLayout :" + left + " " + top + " " + width + " " + height);
      WXLogUtils.d("Weex_Fixed_Style", "WXComponent:setLayout Left:" + getStyles().getLeft() + " " + (int) getStyles().getTop());
    }
  }

  protected void updateBoxShadow() {
    if (!BoxShadowUtil.isBoxShadowEnabled()) {
//      WXLogUtils.w("BoxShadow", "box-shadow disabled");
      return;
    }

    if (getStyles() != null) {
      Object boxShadow = getStyles().get(Constants.Name.BOX_SHADOW);
      Object shadowQuality = getAttrs().get(Constants.Name.SHADOW_QUALITY);
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

      float[] radii = new float[]{0, 0, 0, 0, 0, 0, 0, 0};
      WXStyle style = getStyles();
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
//      WXLogUtils.w("BoxShadow", "box-shadow disabled");
      return;
    }

    if (getStyles() != null) {
      Object obj = getStyles().get(Constants.Name.BOX_SHADOW);
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

  /**
   * After component's layout result is apply to view. May be invoke multiple times since
   * DOM can be changed in js runtime.
   */
  protected void onFinishLayout() {
    Object param = getStyles() != null ? getStyles().get(Constants.Name.BACKGROUND_IMAGE) : null;
    if (param != null) {
      setBackgroundImage(param.toString());
    }
  }

  /**
   * measure
   */
  protected MeasureOutput measure(int width, int height) {
    MeasureOutput measureOutput = new MeasureOutput();

    if (mFixedProp != 0) {
      measureOutput.width = mFixedProp;
      measureOutput.height = mFixedProp;
    } else {
      measureOutput.width = width;
      measureOutput.height = height;
    }
    return measureOutput;
  }

  @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
  protected void setAriaHidden(boolean isHidden) {
    View host = getHostView();
    if (host != null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
      host.setImportantForAccessibility(isHidden ? View.IMPORTANT_FOR_ACCESSIBILITY_NO : View.IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
  }

  protected void setAriaLabel(String label) {
    View host = getHostView();
    if (host != null) {
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
          try {
            super.onInitializeAccessibilityNodeInfo(host, info);
            info.setRoleDescription(finalRole);
          } catch (Throwable e) {
            WXLogUtils.e("SetRole failed!");
          }
        }
      };
      ViewCompat.setAccessibilityDelegate(host, delegate);
    }
  }

  /**
   * Avoid large size view fail in GPU-Animation.
   *
   * @param fixedSize
   */
  private void setFixedSize(String fixedSize) {
    if (PROP_FS_MATCH_PARENT.equals(fixedSize)) {
      mFixedProp = ViewGroup.LayoutParams.MATCH_PARENT;
    } else if (PROP_FS_WRAP_CONTENT.equals(fixedSize)) {
      mFixedProp = ViewGroup.LayoutParams.WRAP_CONTENT;
    } else {
      mFixedProp = 0;
      return;
    }
    if (mHost != null) {
      ViewGroup.LayoutParams layoutParams = mHost.getLayoutParams();
      if (layoutParams != null) {
        layoutParams.height = mFixedProp;
        layoutParams.width = mFixedProp;
        mHost.setLayoutParams(layoutParams);
      }
    }
  }

  /**
   * Add new event to component,this will post a task to DOM thread to add event.
   *
   * @param type
   */
  protected void appendEventToDOM(String type) {
//    WXSDKManager.getInstance().getWXDomManager().postAction(getInstanceId(), Actions.getAddEvent(getRef(), type), false);
  }

  public View getRealView() {
    return mHost;
  }

  /**
   * Judge whether need to set an onTouchListener.<br>
   * As there is only one onTouchListener in each view, so all the gesture that use onTouchListener should put there.
   *
   * @param type eventType {@link Event}
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
    if(WXGesture.isStopPropagation(type)){
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
      if (container.getRef().equals(ROOT)) {
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
    if (this instanceof Scrollable) {
      return (Scrollable) this;
    }
    return null;
  }

  public WXVContainer getParent() {
    return mParent;
  }

  /**
   * create view
   */
  public final void createView() {
    if (!isLazy()) {
      createViewImpl();
    }
  }

  protected void createViewImpl() {
    if (mContext != null) {
      mHost = initComponentHostView(mContext);
      if (mHost == null && !isVirtualComponent()) {
        //compatible
        initView();
      }
      if (mHost != null) {
        if(mHost.getId() == View.NO_ID)
          mHost.setId(WXViewUtils.generateViewId());
        if(TextUtils.isEmpty(mHost.getContentDescription()) && WXEnvironment.isApkDebugable()){
          mHost.setContentDescription(getRef());
        }
        ComponentObserver observer;
        if ((observer = getInstance().getComponentObserver()) != null) {
          observer.onViewCreated(this, mHost);
        }
      }
      onHostViewInitialized(mHost);
    } else {
      WXLogUtils.e("createViewImpl", "Context is null");
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
   *
   * @param context
   * @return
   */
  protected T initComponentHostView(@NonNull Context context) {
    /**
     * compatible old initView
     */
    return null;
  }

  /**
   * Called after host view init. <br>
   * Any overriding methods should invoke this method at the right time, to ensure the cached animation can be triggered correctly.
   * (the animation will be cached when {@link #isLazy()} is true)
   *
   * @param host the host view
   */
  @CallSuper
  protected void onHostViewInitialized(T host) {
    if (mAnimationHolder != null) {
//      Performs cached animation
      mAnimationHolder.execute(mInstance, this);
    }
    setActiveTouchListener();
  }

  public T getHostView() {
    return mHost;
  }

  /**
   * use {@link #getHostView()} instead
   *
   * @return
   */
  @Deprecated
  public View getView() {
    return mHost;
  }

  public int getAbsoluteY() {
    return mAbsoluteY;
  }

  public int getAbsoluteX() {
    return mAbsoluteX;
  }

  public void removeEvent(String type) {
    if (TextUtils.isEmpty(type)) {
      return;
    }

    if (type.equals(Constants.Event.LAYEROVERFLOW))
      removeLayerOverFlowListener(getRef());

    if(getEvents() != null){
      getEvents().remove(type);
    }
    if(mAppendEvents != null) {
      mAppendEvents.remove(type);//only clean append events, not dom's events.
    }
    if(mGestureType != null){
      mGestureType.remove(type);
    }
    removeEventFromView(type);
  }

  protected void removeEventFromView(String type) {
    if (type.equals(Constants.Event.CLICK) && getRealView() != null && mHostClickListeners != null) {
      if(mClickEventListener == null){
        mClickEventListener = new OnClickListenerImp();
      }
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

  public void removeAllEvent() {
    if (getEvents().size() < 1) {
      return;
    }
    WXEvent events = getEvents();
    int size = events.size();
    for (int i=0; i<size; i++) {
      if(i >= events.size()){
        break;
      }
      String event = events.get(i);
      if (event == null) {
        continue;
      }
      removeEventFromView(event);
    }
    if(mAppendEvents!=null) {
      mAppendEvents.clear();//only clean append events, not dom's events.
    }
    if(mGestureType != null){
      mGestureType.clear();
    }
    mGesture = null;
    if (getRealView() != null &&
            getRealView() instanceof WXGestureObservable) {
      ((WXGestureObservable) getRealView()).registerGestureListener(null);
    }
    if (mHost != null) {
      mHost.setOnFocusChangeListener(null);
      if (mHostClickListeners != null && mHostClickListeners.size() > 0) {
        mHostClickListeners.clear();
        mHost.setOnClickListener(null);
      }
    }
  }

  public final void removeStickyStyle() {
    if (isSticky()) {
      Scrollable scroller = getParentScroller();
      if (scroller != null) {
        scroller.unbindStickStyle(this);
      }
    }
  }

  public boolean isSticky() {
    return getStyles().isSticky();
  }

  public boolean isFixed() {
    return getStyles().isFixed();
  }

  public void setDisabled(boolean disabled) {
    mIsDisabled = disabled;
    if (mHost == null) {
      return;
    }
    mHost.setEnabled(!disabled);
  }

  public boolean isDisabled() {
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
            WXViewUtils.setBackGround(mHost, mRippleBackground, this);
          } else {
            LayerDrawable layerDrawable = new LayerDrawable(new Drawable[]{mRippleBackground, mBackgroundDrawable});
            WXViewUtils.setBackGround(mHost, layerDrawable, this);
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
      if (getStyles() != null && getStyles().getPesudoResetStyles() != null) {
        Map<String, Object> resetStyles = getStyles().getPesudoResetStyles();

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
            @SuppressLint("CanvasSize")
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
      Shader shader = WXResourceUtils.getShader(bgImage, getLayoutSize().getWidth(), getLayoutSize().getHeight());
      getOrCreateBorder().setImage(shader);
    }
  }
  private boolean shouldCancelHardwareAccelerate() {
    IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
    boolean cancel_hardware_accelerate = true;
    if (adapter != null) {
      try {
        cancel_hardware_accelerate = Boolean.parseBoolean(adapter
                .getConfig("android_weex_test_gpu",
                        "cancel_hardware_accelerate",
                        "true"));
      }catch (Exception e){
        WXLogUtils.e(WXLogUtils.getStackTrace(e));
      }
      WXLogUtils.i("cancel_hardware_accelerate : " + cancel_hardware_accelerate);
    }
    return cancel_hardware_accelerate;
  }

  public void setOpacity(float opacity) {
    if (opacity >= 0 && opacity <= 1 && mHost.getAlpha() != opacity) {
      int limit = WXRenderManager.getOpenGLRenderLimitValue();
      if (isLayerTypeEnabled()) {
        mHost.setLayerType(View.LAYER_TYPE_HARDWARE, null);
      }
      if(isLayerTypeEnabled() && shouldCancelHardwareAccelerate() && limit > 0 && (getLayoutHeight() > limit ||
              getLayoutWidth() > limit)){
        mHost.setLayerType(View.LAYER_TYPE_NONE,null);
      }
      mHost.setAlpha(opacity);
    }
  }

  public void setBorderRadius(String key, float borderRadius) {
    if (borderRadius >= 0) {
      switch (key) {
        case Constants.Name.BORDER_RADIUS:
          getOrCreateBorder().setBorderRadius(
              CORNER.ALL, WXViewUtils.getRealSubPxByWidth(borderRadius, mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_TOP_LEFT_RADIUS:
          getOrCreateBorder().setBorderRadius(CORNER.BORDER_TOP_LEFT, WXViewUtils.getRealSubPxByWidth(borderRadius, mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
          getOrCreateBorder().setBorderRadius(CORNER.BORDER_TOP_RIGHT, WXViewUtils.getRealSubPxByWidth(borderRadius, mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
          getOrCreateBorder().setBorderRadius(CORNER.BORDER_BOTTOM_RIGHT, WXViewUtils.getRealSubPxByWidth(borderRadius, mInstance.getInstanceViewPortWidth()));
          break;
        case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
          getOrCreateBorder().setBorderRadius(CORNER.BORDER_BOTTOM_LEFT, WXViewUtils.getRealSubPxByWidth(borderRadius, mInstance.getInstanceViewPortWidth()));
          break;
      }
    }
  }

  public void setBorderWidth(String key, float borderWidth) {
    if (borderWidth >= 0) {
      switch (key) {
        case Constants.Name.BORDER_WIDTH:
          getOrCreateBorder().setBorderWidth(CSSShorthand.EDGE.ALL, borderWidth);
          break;
        case Constants.Name.BORDER_TOP_WIDTH:
          getOrCreateBorder().setBorderWidth(CSSShorthand.EDGE.TOP, borderWidth);
          break;
        case Constants.Name.BORDER_RIGHT_WIDTH:
          getOrCreateBorder().setBorderWidth(CSSShorthand.EDGE.RIGHT, borderWidth);
          break;
        case Constants.Name.BORDER_BOTTOM_WIDTH:
          getOrCreateBorder().setBorderWidth(CSSShorthand.EDGE.BOTTOM, borderWidth);
          break;
        case Constants.Name.BORDER_LEFT_WIDTH:
          getOrCreateBorder().setBorderWidth(CSSShorthand.EDGE.LEFT, borderWidth);
          break;
        default:
          break;
      }
    }
  }

  public void setBorderStyle(String key, String borderStyle) {
    if (!TextUtils.isEmpty(borderStyle)) {
      switch (key) {
        case Constants.Name.BORDER_STYLE:
          getOrCreateBorder().setBorderStyle(CSSShorthand.EDGE.ALL, borderStyle);
          break;
        case Constants.Name.BORDER_RIGHT_STYLE:
          getOrCreateBorder().setBorderStyle(CSSShorthand.EDGE.RIGHT, borderStyle);
          break;
        case Constants.Name.BORDER_BOTTOM_STYLE:
          getOrCreateBorder().setBorderStyle(CSSShorthand.EDGE.BOTTOM, borderStyle);
          break;
        case Constants.Name.BORDER_LEFT_STYLE:
          getOrCreateBorder().setBorderStyle(CSSShorthand.EDGE.LEFT, borderStyle);
          break;
        case Constants.Name.BORDER_TOP_STYLE:
          getOrCreateBorder().setBorderStyle(CSSShorthand.EDGE.TOP, borderStyle);
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
            getOrCreateBorder().setBorderColor(CSSShorthand.EDGE.ALL, colorInt);
            break;
          case Constants.Name.BORDER_TOP_COLOR:
            getOrCreateBorder().setBorderColor(CSSShorthand.EDGE.TOP, colorInt);
            break;
          case Constants.Name.BORDER_RIGHT_COLOR:
            getOrCreateBorder().setBorderColor(CSSShorthand.EDGE.RIGHT, colorInt);
            break;
          case Constants.Name.BORDER_BOTTOM_COLOR:
            getOrCreateBorder().setBorderColor(CSSShorthand.EDGE.BOTTOM, colorInt);
            break;
          case Constants.Name.BORDER_LEFT_COLOR:
            getOrCreateBorder().setBorderColor(CSSShorthand.EDGE.LEFT, colorInt);
            break;
        }
      }
    }
  }

  public
  @Nullable
  String getVisibility() {
    try {
      return (String) getStyles().get(Constants.Name.VISIBILITY);
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
    float elevation = getAttrs().getElevation(getInstance().getInstanceViewPortWidth());
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

  public void onActivityResult(int requestCode, int resultCode, Intent data) {

  }

  public boolean onCreateOptionsMenu(Menu menu) {
    return false;
  }

  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {

  }

  /********************************
   *  end hook Activity life cycle callback
   ********************************************************/
  public void recycled() {
    if (isFixed())
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
    if (mHost != null && mHost.getLayerType() == View.LAYER_TYPE_HARDWARE && isLayerTypeEnabled()) {
      mHost.setLayerType(View.LAYER_TYPE_NONE, null);
    }
    removeAllEvent();
    removeStickyStyle();

    View view;
    if (isFixed() && (view = getHostView()) != null) {
      getInstance().removeFixedView(view);
    }

    if(contentBoxMeasurement!=null){
      contentBoxMeasurement.destroy();
      contentBoxMeasurement = null;
    }
    mIsDestroyed = true;
    if(animations!=null) {
      animations.clear();
    }
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
    mPreRealRight = 0;
    mPreRealWidth = 0;
    mPreRealHeight = 0;
    mPreRealTop = 0;
//    mHost = null;
    return original;
  }

  public void clearPreLayout() {
    mPreRealLeft = 0;
    mPreRealRight = 0;
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

  public boolean containsEvent(String event) {
    return getEvents().contains(event) || (mAppendEvents!=null && mAppendEvents.contains(event));
  }

  public void notifyAppearStateChange(String wxEventType, String direction) {
    if (containsEvent(Constants.Event.APPEAR) || containsEvent(Constants.Event.DISAPPEAR)) {
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
   *
   * @return false component add subview
   */
  public boolean isVirtualComponent() {
    return mType == TYPE_VIRTUAL;
  }

  public void removeVirtualComponent() {
  }

  public int getType() {
    return mType;
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
   *
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

  private void setActiveTouchListener() {
    boolean hasActivePesudo = getStyles().getPesudoStyles().containsKey(Constants.PSEUDO.ACTIVE);
    View view;
    if (hasActivePesudo && (view = getRealView()) != null) {
      boolean hasTouchConsumer = isConsumeTouch();
      view.setOnTouchListener(new TouchActivePseudoListener(this, !hasTouchConsumer));
    }
  }

  protected boolean isConsumeTouch() {
    return (mHostClickListeners != null && mHostClickListeners.size() > 0) || mGesture != null;
  }

  @Override
  public void updateActivePseudo(boolean isSet) {
    setPseudoClassStatus(Constants.PSEUDO.ACTIVE, isSet);
  }

  /**
   * @param clzName like ':active' or ':active:enabled'
   * @param status
   */
  protected void setPseudoClassStatus(String clzName, boolean status) {
    WXStyle styles = getStyles();
    Map<String, Map<String, Object>> pesudoStyles = styles.getPesudoStyles();

    if (pesudoStyles == null || pesudoStyles.size() == 0) {
      return;
    }
    if(mPesudoStatus == null){
      mPesudoStatus = new PesudoStatus();
    }
    Map<String, Object> resultStyles = mPesudoStatus.updateStatusAndGetUpdateStyles(
            clzName,
            status,
            pesudoStyles,
            styles.getPesudoResetStyles());

    if (null != resultStyles) {
      if (status) {
        mPseudoResetGraphicSize = new GraphicSize(getLayoutSize().getWidth(), getLayoutSize().getHeight());
        if (resultStyles.keySet().contains(Constants.Name.WIDTH)) {
          getLayoutSize().setWidth(WXViewUtils.getRealPxByWidth(WXUtils.parseFloat(styles.getPesudoResetStyles().get(Constants.Name.WIDTH + Constants.PSEUDO.ACTIVE)), getViewPortWidth()));
        } else if (resultStyles.keySet().contains(Constants.Name.HEIGHT)){
          getLayoutSize().setHeight(WXViewUtils.getRealPxByWidth(WXUtils.parseFloat(styles.getPesudoResetStyles().get(Constants.Name.HEIGHT + Constants.PSEUDO.ACTIVE)), getViewPortWidth()));
        }
      } else {
        if (null != mPseudoResetGraphicSize) {
          setLayoutSize(mPseudoResetGraphicSize);
        }
      }
    }

    updateStyleByPesudo(resultStyles);
  }

  private void updateStyleByPesudo(Map<String, Object> styles) {
    new GraphicActionUpdateStyle(getInstance(), getRef(), styles, getPadding(), getMargin(), getBorder(), true)
            .executeActionOnRender();
    if (getLayoutWidth() == 0 && getLayoutWidth() == 0) {
    } else {
      WXBridgeManager.getInstance().setStyleWidth(getInstanceId(), getRef(), getLayoutWidth());
      WXBridgeManager.getInstance().setStyleHeight(getInstanceId(), getRef(), getLayoutHeight());
//      WXBridgeManager.getInstance().calculateLayout(getInstanceId(), getRef(), false);
    }
  }

  public int getStickyOffset() {
    return mStickyOffset;
  }

  public boolean canRecycled() {
    return (!isFixed() || !isSticky()) && getAttrs().canRecycled();
  }

  /**
   * Sets the offset for the sticky
   *
   * @param stickyOffset child[y]-parent[y]
   */
  public void setStickyOffset(int stickyOffset) {
    mStickyOffset = stickyOffset;
  }

  /**
   * For now, this method respect the result of {@link WXSDKInstance#isLayerTypeEnabled()}
   *
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
   * Trigger a updateStyles invoke to relayout current page
   */
  public void notifyNativeSizeChanged(int w, int h) {
    if (!mNeedLayoutOnAnimation) {
      return;
    }

    final WXBridgeManager manager = WXBridgeManager.getInstance();
    manager.setStyleWidth(getInstanceId(), getRef(), w);
    manager.setStyleHeight(getInstanceId(), getRef(), h);
  }

  public static final int STATE_DOM_FINISH = 0;
  public static final int STATE_UI_FINISH = 1;
  public static final int STATE_ALL_FINISH = 2;
  @IntDef({STATE_DOM_FINISH, STATE_UI_FINISH, STATE_ALL_FINISH})
  @Retention(RetentionPolicy.SOURCE)
  @Target(ElementType.PARAMETER)
  public @interface RenderState {
  }

  @CallSuper
  public void onRenderFinish(@RenderState int state) {
    if (WXTracing.isAvailable()) {
      double uiTime = Stopwatch.nanosToMillis(mTraceInfo.uiThreadNanos);
      if (state == STATE_ALL_FINISH || state == STATE_DOM_FINISH) {
        WXTracing.TraceEvent domEvent = WXTracing.newEvent("DomExecute", getInstanceId(), mTraceInfo.rootEventId);
        domEvent.ph = "X";
        domEvent.ts = mTraceInfo.domThreadStart;
        domEvent.tname = "DOMThread";
        domEvent.name = getComponentType();
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
          uiEvent.name = getComponentType();
          uiEvent.classname = getClass().getSimpleName();
          if (getParent() != null) {
            uiEvent.parentRef = getParent().getRef();
          }
          uiEvent.submit();
        } else {
          if (WXEnvironment.isApkDebugable() && !isLazy()) {
//            WXLogUtils.w("onRenderFinish", "createView() not called");
          }
        }
      }
    }
  }

  protected boolean isRippleEnabled() {
    try {
      Object obj = getAttrs().get(Constants.Name.RIPPLE_ENABLED);
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
      if(!WXBasicComponentType.RECYCLE_LIST.equals(getParent().getComponentType())){
          NativeRenderObjectUtils.nativeRenderObjectChildWaste(getRenderObjectPtr(), waste);
      }

     if(waste){
        //update dom not show, and put style to hidden
        getStyles().put(Constants.Name.VISIBILITY, Constants.Value.HIDDEN);
        //if component not init, mark lazy init when use, reduce view count
        if(getHostView() == null){
          if(!mLazy){
            lazy(true);
          }
        }else{
          getHostView().setVisibility(View.GONE);
        }
      }else{
       getStyles().put(Constants.Name.VISIBILITY, Constants.Value.VISIBLE);
        if(getHostView() == null){
          if(mLazy) { // when parent is lazy just mark node lazy false
            if(mParent != null && mParent.isLazy()){
              lazy(false);
            }else{
              Statements.initLazyComponent(this, mParent);
            }
          }
        }else{
          getHostView().setVisibility(View.VISIBLE);
        }
      }
    }
  }


  /** component uniquie key id in native for recycle-list,
   *  should be unique for every native component differ with ref
   *  */
  public  String getViewTreeKey(){
    if(mViewTreeKey == null){
      if(getParent() == null){
        mViewTreeKey = hashCode() + "_" + getRef();
      }else{
        mViewTreeKey = hashCode() + "_" + getRef() + "_" + getParent().indexOf(this);
      }
    }
    return mViewTreeKey;
  }

  private String mViewTreeKey;

  public WXTransition getTransition() {
    return mTransition;
  }

  public void setTransition(WXTransition transition) {
    this.mTransition = transition;
  }

  public void addAnimationForElement(Map<String, Object> animMap) {
    if(animMap!=null && !animMap.isEmpty()){
      if(animations == null){
        animations = new ConcurrentLinkedQueue<>();
      }
      animations.add(new Pair<>(getRef(),animMap));
    }
  }

  private void parseAnimation() {
    if (null == animations) {
      return;
    }
    for (final Pair<String, Map<String, Object>> pair : animations) {
      if (!TextUtils.isEmpty(pair.first)) {
        final WXAnimationBean animationBean = createAnimationBean(pair.first, pair.second);
        if (animationBean != null) {
          GraphicActionAnimation action = new GraphicActionAnimation(getInstance(), getRef(), animationBean);
          action.executeAction();
        }
      }
    }
    animations.clear();
  }


  private WXAnimationBean createAnimationBean(String ref,Map<String, Object> style){
    if (style != null) {
      try {
        Object transform = style.get(Constants.Name.TRANSFORM);
        if (transform instanceof String && !TextUtils.isEmpty((String) transform)) {
          String transformOrigin = (String) style.get(Constants.Name.TRANSFORM_ORIGIN);
          WXAnimationBean animationBean = new WXAnimationBean();
          int width = (int) getLayoutWidth();
          int height = (int) getLayoutHeight();
          animationBean.styles = new WXAnimationBean.Style();
          animationBean.styles.init(transformOrigin, (String) transform, width, height,WXSDKManager.getInstanceViewPortWidth(getInstanceId()),
                  getInstance());
          return animationBean;
        }
      }catch (RuntimeException e){
        WXLogUtils.e("", e);
        return null;
      }
    }
    return null;
  }

  /**
   * node is lazy
   * */
  private boolean mLazy = false;

  /***/
  public void lazy(boolean lazy) {
    mLazy = lazy;
  }

  public long getRenderObjectPtr(){
    if(getBasicComponentData().isRenderPtrEmpty()){
      getBasicComponentData().setRenderObjectPr(NativeRenderObjectUtils.nativeGetRenderObject(getInstanceId(), getRef()));
    }
    return getBasicComponentData().getRenderObjectPr();
  }


  public void updateNativeAttr(String key, Object value){
    if(key == null){
      return;
    }
    if(value == null){
      value  = "";
    }
    getBasicComponentData().getAttrs().put(key, value);
    NativeRenderObjectUtils.nativeUpdateRenderObjectAttr(getRenderObjectPtr(), key, value.toString());
  }

  public  void  nativeUpdateAttrs(Map<String, Object> dynamic){
    Set<Map.Entry<String, Object>> entries = dynamic.entrySet();
    /**
     * diff attrs, see attrs has update, remove none update attrs
     * */
    Iterator<Map.Entry<String, Object>> iterator = entries.iterator();
    while (iterator.hasNext()){
      Map.Entry<String, Object> objectEntry = iterator.next();
      if(objectEntry.getKey() == null){
        continue;
      }
      updateNativeAttr(objectEntry.getKey(), objectEntry.getValue());
    }
  }


  public void updateNativeStyle(String key, Object value){
    if(key == null){
      return;
    }
    if(value == null){
      value  = "";
    }
    getBasicComponentData().getStyles().put(key, value);
    NativeRenderObjectUtils.nativeUpdateRenderObjectStyle(getRenderObjectPtr(), key, value.toString());
  }

  public  void  updateNativeStyles(Map<String, Object> dynamic){
    Set<Map.Entry<String, Object>> entries = dynamic.entrySet();
    /**
     * diff attrs, see attrs has update, remove none update attrs
     * */
    Iterator<Map.Entry<String, Object>> iterator = entries.iterator();
    while (iterator.hasNext()){
      Map.Entry<String, Object> objectEntry = iterator.next();
      if(objectEntry.getKey() == null){
        continue;
      }
      updateNativeStyle(objectEntry.getKey(), objectEntry.getValue());
    }
  }

  public void addLayerOverFlowListener(String ref) {
    if (getInstance() != null)
      getInstance().addLayerOverFlowListener(ref);
  }

  public void removeLayerOverFlowListener(String ref) {
    if (getInstance() != null)
      getInstance().removeLayerOverFlowListener(ref);
  }
}
