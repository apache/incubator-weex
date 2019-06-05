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
package com.taobao.weex;

import static com.taobao.weex.common.WXErrorCode.WX_ERR_JSC_CRASH;
import static com.taobao.weex.common.WXErrorCode.WX_ERR_RELOAD_PAGE;
import static com.taobao.weex.http.WXHttpUtil.KEY_USER_AGENT;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.annotation.WorkerThread;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ScrollView;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.adapter.IDrawableLoader;
import com.taobao.weex.adapter.IWXConfigAdapter;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.adapter.IWXImgLoaderAdapter;
import com.taobao.weex.adapter.IWXJscProcessManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.adapter.URIAdapter;
import com.taobao.weex.appfram.websocket.IWebSocketAdapter;
import com.taobao.weex.bridge.EventResult;
import com.taobao.weex.bridge.NativeInvokeHelper;
import com.taobao.weex.bridge.SimpleJSCallback;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXModuleManager;
import com.taobao.weex.bridge.WXParams;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.common.WXConfig;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.common.WXRequest;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.http.WXHttpUtil;
import com.taobao.weex.instance.InstanceOnFireEventInterceptor;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.action.GraphicActionAddElement;
import com.taobao.weex.ui.component.NestedContainer;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXEmbed;
import com.taobao.weex.ui.flat.FlatGUIContext;
import com.taobao.weex.ui.view.WXScrollView;
import com.taobao.weex.utils.Trace;
import com.taobao.weex.utils.WXDeviceUtils;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXReflectionUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import com.taobao.weex.utils.cache.RegisterCache;

import java.io.Serializable;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.concurrent.ConcurrentHashMap;


/**
 * Each instance of WXSDKInstance represents an running weex instance.
 * It can be a pure weex view, or mixed with native view
 */
public class WXSDKInstance implements IWXActivityStateListener,View.OnLayoutChangeListener {

  private static  final  String SOURCE_TEMPLATE_BASE64_MD5 = "templateSourceBase64MD5";

  /**
   * Devtool protocol
   */
  public static String ACTION_DEBUG_INSTANCE_REFRESH = "DEBUG_INSTANCE_REFRESH";
  public static String ACTION_INSTANCE_RELOAD = "INSTANCE_RELOAD";

  //Performance
  public boolean mEnd = false;
  public boolean mHasCreateFinish = false;
  public static final String BUNDLE_URL = "bundleUrl";
  private IWXUserTrackAdapter mUserTrackAdapter;
  private IWXRenderListener mRenderListener;
  private IWXStatisticsListener mStatisticsListener;
  /** package **/ Context mContext;
  private final String mInstanceId;
  private RenderContainer mRenderContainer;
  private WXComponent mRootComp;
  private boolean mRendered;
  private WXRefreshData mLastRefreshData;
  private NestedInstanceInterceptor mNestedInstanceInterceptor;
  private String mBundleUrl = "";
  public static String requestUrl = "requestUrl";
  private boolean isDestroy=false;
  private boolean hasException = false;
  private boolean isRenderSuccess = false;
  private Map<String,Serializable> mUserTrackParams;
  private NativeInvokeHelper mNativeInvokeHelper;
  private boolean isCommit=false;
  private WXGlobalEventReceiver mGlobalEventReceiver=null;
  private boolean trackComponent;
  private boolean enableLayerType = true;
  private boolean mNeedValidate = false;
  private boolean mNeedReLoad = false;
  private boolean mUseScroller = false;
  private int mInstanceViewPortWidth = 750;
  private WXInstanceApm mApmForInstance;
  private @NonNull
  FlatGUIContext mFlatGUIContext =new FlatGUIContext();

  private Map<String,String> mContainerInfo;

  public boolean isNewFsEnd = false;

  /**
   * bundle type
   */
  public WXBridgeManager.BundType bundleType;
  public long mRenderStartNanos;
  public int mExecJSTraceId = WXTracing.nextId();

  private boolean isViewDisAppear = false;

  /**
   *for network tracker
   */
  public String mwxDims[] = new String [5];
  public long measureTimes[] = new long [5];

  public WeakReference<String> templateRef;
  public Map<String,List<String>> responseHeaders = new HashMap<>();

  /**
   * Render strategy.
   */
  private WXRenderStrategy mRenderStrategy = WXRenderStrategy.APPEND_ASYNC;

  private boolean mDisableSkipFrameworkInit = false;

  /**
   * Render start time
   */
  public long mRenderStartTime;
  /**
   * Refresh start time
   */
  private long mRefreshStartTime;
  private WXPerformance mWXPerformance;
  private ScrollView mScrollView;
  private WXScrollView.WXScrollViewListener mWXScrollViewListener;

  private List<OnWXScrollListener> mWXScrollListeners;

  private List<String> mLayerOverFlowListeners;

  private WXSDKInstance mParentInstance;

  /**
   * Default Width And Viewport is 750,
   * when screen width change, we adjust viewport to adapter screen change
   * */
  private boolean mAutoAdjustDeviceWidth = WXEnvironment.AUTO_ADJUST_ENV_DEVICE_WIDTH;


  public List<String> getLayerOverFlowListeners() {
    return mLayerOverFlowListeners;
  }

  public void addLayerOverFlowListener(String ref) {
    if (mLayerOverFlowListeners == null)
      mLayerOverFlowListeners = new ArrayList<>();
    mLayerOverFlowListeners.add(ref);
  }

  public void removeLayerOverFlowListener(String ref) {
    if (mLayerOverFlowListeners != null)
      mLayerOverFlowListeners.remove(ref);
  }

  /**
   * whether we are in preRender mode
   * */
  private volatile boolean isPreRenderMode;

  private boolean mCurrentGround = false;
  private ComponentObserver mComponentObserver;
  private Map<String, GraphicActionAddElement> inactiveAddElementAction = new ArrayMap<>();

  private Map<Long, ContentBoxMeasurement> mContentBoxMeasurements = new ArrayMap<>();

  private List<InstanceOnFireEventInterceptor> mInstanceOnFireEventInterceptorList;


  /**
   * network handler
   */
  public interface ImageNetworkHandler {
    public String fetchLocal(String url);
  }

  public interface StreamNetworkHandler {
    public String fetchLocal(String url);
  }

  public interface CustomFontNetworkHandler {
    public String fetchLocal(String url);
  }

  private ImageNetworkHandler mImageNetworkHandler;

  private StreamNetworkHandler mStreamNetworkHandler;

  private CustomFontNetworkHandler mCustomFontNetworkHandler;

  public ImageNetworkHandler getImageNetworkHandler() {
    return mImageNetworkHandler;
  }

  public void setImageNetworkHandler(ImageNetworkHandler imageNetworkHandler) {
    this.mImageNetworkHandler = imageNetworkHandler;
  }

  public StreamNetworkHandler getStreamNetworkHandler() {
    return mStreamNetworkHandler;
  }

  public void setStreamNetworkHandler(StreamNetworkHandler streamNetworkHandler) {
    this.mStreamNetworkHandler = streamNetworkHandler;
  }

  public CustomFontNetworkHandler getCustomFontNetworkHandler() {
    return mCustomFontNetworkHandler;
  }

  public void setCustomFontNetworkHandler(CustomFontNetworkHandler customFontNetworkHandler) {
    this.mCustomFontNetworkHandler = customFontNetworkHandler;
  }

  /**
   * set make weexCore run in single process mode
   * @param flag true means weexCore run in single process mode or multi process mode
   */
  public void setUseSingleProcess(boolean flag) {
    WXBridgeManager.getInstance().setUseSingleProcess(flag);
  }

  /**
   * set open SandBox
   * @param flag
   */
  public void setUseSandBox(boolean flag) {
    WXBridgeManager.getInstance().setSandBoxContext(flag);
  }

  public PriorityQueue<WXEmbed> hiddenEmbeds;

  private int maxHiddenEmbedsNum = -1; //max hidden embed num, -1 standard for ulimit

  public int getMaxHiddenEmbedsNum() {
    return maxHiddenEmbedsNum;
  }

  public void setMaxHiddenEmbedsNum(int maxHiddenEmbedsNum) {
    this.maxHiddenEmbedsNum = maxHiddenEmbedsNum;
  }

  @WorkerThread
  @RestrictTo(Scope.LIBRARY)
  public void addInActiveAddElementAction(String ref, GraphicActionAddElement action){
    inactiveAddElementAction.put(ref, action);
  }

  @WorkerThread
  @RestrictTo(Scope.LIBRARY)
  public void removeInActiveAddElmentAction(String ref){
    inactiveAddElementAction.remove(ref);
  }

  @WorkerThread
  @RestrictTo(Scope.LIBRARY)
  public GraphicActionAddElement getInActiveAddElementAction(String ref){
    return inactiveAddElementAction.get(ref);
  }

  /**
   * If anchor is created manually(etc. define a layout xml resource ),
   * be aware do not add it to twice when {@link IWXRenderListener#onViewCreated(WXSDKInstance, View)}.
   * @param a
   */
  public void setRenderContainer(RenderContainer a){
    if(a != null) {
      a.setSDKInstance(this);
      a.addOnLayoutChangeListener(this);
    }

    mRenderContainer = a;
    if (mRenderContainer != null && mRenderContainer.getLayoutParams() != null
            && mRenderContainer.getLayoutParams().width == ViewGroup.LayoutParams.WRAP_CONTENT) {
      WXBridgeManager.getInstance().post(new Runnable() {
        @Override
        public void run() {
          WXBridgeManager.getInstance().setRenderContentWrapContentToCore(true, getInstanceId());
        }
      });
    } else {
      WXBridgeManager.getInstance().post(new Runnable() {
        @Override
        public void run() {
          WXBridgeManager.getInstance().setRenderContentWrapContentToCore(false, getInstanceId());
        }
      });
    }
  }

  private int mMaxDeepLayer;

  public boolean isTrackComponent() {
    return trackComponent;
  }

  public void setTrackComponent(boolean trackComponent) {
    this.trackComponent = trackComponent;
  }

  /**
   * Tell whether it is enabled to change the layerType
   * {@link android.view.View#setLayerType(int, Paint)}
   * @return True for enable to change the layerType of component, false otherwise. The default
   * is True
   */
  public boolean isLayerTypeEnabled() {
    return enableLayerType;
  }

  /**
   * Enable the ability of changing layerType. e.g. {@link android.view.View#setLayerType(int, Paint)}
   * Disable the ability of changing layerType will have tremendous <strong>performance
   * punishment</strong>.
   *
   * <strong>Do not</strong> set this to false unless you know exactly what you are doing.
   * @param enable True for enable to change the layerType of component, false otherwise. The default
   * is True
   */
  public void enableLayerType(boolean enable) {
    enableLayerType = enable;
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public @NonNull
  FlatGUIContext getFlatUIContext(){
    return mFlatGUIContext;
  }

  public boolean isNeedValidate() {
    return mNeedValidate;
  }

  public boolean isNeedReLoad() {
    return mNeedReLoad;
  }

  public void setNeedLoad(boolean load) {
    mNeedReLoad = load;
  }

  public boolean isUseScroller() {
    return mUseScroller;
  }

  public void setUseScroller(boolean use) {
    mUseScroller = use;
  }

  public void setInstanceViewPortWidth(int instanceViewPortWidth) {
    this.mInstanceViewPortWidth = instanceViewPortWidth;
    this.mAutoAdjustDeviceWidth = false;
  }

  public void setAutoAdjustDeviceWidth(boolean autoAdjustViewPort){
    this.mAutoAdjustDeviceWidth = autoAdjustViewPort;
  }

  public boolean isAutoAdjustDeviceWidth(){
    return mAutoAdjustDeviceWidth;
  }

  private void setDeviceDisplay(float deviceWith, float deviceHeight, float scale){
    WXBridgeManager.getInstance().setDeviceDisplay(getInstanceId(), deviceWith, deviceHeight, scale);
  }



  public int getInstanceViewPortWidth(){
    return mInstanceViewPortWidth;
  }

  public interface OnInstanceVisibleListener{
    void onAppear();
    void onDisappear();
  }
  private List<OnInstanceVisibleListener> mVisibleListeners = new ArrayList<>();

  public WXSDKInstance(Context context) {
    mInstanceId = WXSDKManager.getInstance().generateInstanceId();
    init(context);
  }

  //for preInit
  public WXSDKInstance(){
    mInstanceId = WXSDKManager.getInstance().generateInstanceId();
    mWXPerformance = new WXPerformance(mInstanceId);
    mApmForInstance = new WXInstanceApm(mInstanceId);
    WXSDKManager.getInstance().getAllInstanceMap().put(mInstanceId,this);
  }


  /**
   * For unittest only.
   */
  @RestrictTo(Scope.TESTS)
  WXSDKInstance(Context context,String id) {
    mInstanceId = id;
    init(context);
  }

  public WXComponent getRootComponent() {
    return mRootComp;
  }

  public void setNestedInstanceInterceptor(NestedInstanceInterceptor interceptor){
    mNestedInstanceInterceptor = interceptor;
  }

  public final WXSDKInstance createNestedInstance(NestedContainer container){
    WXSDKInstance sdkInstance = newNestedInstance();
    if(mNestedInstanceInterceptor != null){
      mNestedInstanceInterceptor.onCreateNestInstance(sdkInstance,container);
    }
    if(sdkInstance != null){
        sdkInstance.setComponentObserver(this.getComponentObserver());
    }
    return sdkInstance;
  }

  protected WXSDKInstance newNestedInstance() {
    return new WXSDKInstance(mContext);
  }
  public boolean isHasException() {
    return hasException;
  }

  public void setHasException(boolean hasException) {
    this.hasException = hasException;
  }
  public void addOnInstanceVisibleListener(OnInstanceVisibleListener l){
    mVisibleListeners.add(l);
  }

  public void removeOnInstanceVisibleListener(OnInstanceVisibleListener l){
    mVisibleListeners.remove(l);
  }

  public void init(Context context) {
    RegisterCache.getInstance().idle(true);
    mContext = context;
    mContainerInfo = new HashMap<>(4);
    mNativeInvokeHelper = new NativeInvokeHelper(mInstanceId);

    if (null == mWXPerformance){
      mWXPerformance = new WXPerformance(mInstanceId);
    }
    if (null == mApmForInstance){
      mApmForInstance = new WXInstanceApm(mInstanceId);
    }

    mWXPerformance.WXSDKVersion = WXEnvironment.WXSDK_VERSION;
    mWXPerformance.JSLibInitTime = WXEnvironment.sJSLibInitTime;

    mUserTrackAdapter=WXSDKManager.getInstance().getIWXUserTrackAdapter();

    WXSDKManager.getInstance().getAllInstanceMap().put(mInstanceId,this);

    mContainerInfo.put(WXInstanceApm.KEY_PAGE_PROPERTIES_CONTAINER_NAME, context instanceof Activity
            ? context.getClass().getSimpleName()
            :"unKnowContainer"
    );
    mContainerInfo.put(WXInstanceApm.KEY_PAGE_PROPERTIES_INSTANCE_TYPE,"page");

    WXBridgeManager.getInstance().checkJsEngineMultiThread();
    mDisableSkipFrameworkInit = isDisableSkipFrameworkInDataRender();
  }

  /**
   * Set a Observer for component.
   * This observer will be called in each component, should not doing
   * anything will impact render performance.
   *
   * @param observer
   */
  public void setComponentObserver(ComponentObserver observer){
    mComponentObserver = observer;
  }

  public ComponentObserver getComponentObserver(){
    return mComponentObserver;
  }

  public NativeInvokeHelper getNativeInvokeHelper() {
    return mNativeInvokeHelper;
  }

  @Deprecated
  public void setBizType(String bizType) {
    if (!TextUtils.isEmpty(bizType)) {
      mWXPerformance.bizType = bizType;
    }
  }

  public ScrollView getScrollView() {
    return mScrollView;
  }

  public void setRootScrollView(ScrollView scrollView) {
    mScrollView = scrollView;
    if (mWXScrollViewListener != null && mScrollView instanceof WXScrollView) {
      ((WXScrollView) mScrollView).addScrollViewListener(mWXScrollViewListener);
    }
  }

  @Deprecated
  public void registerScrollViewListener(WXScrollView.WXScrollViewListener scrollViewListener) {
    mWXScrollViewListener = scrollViewListener;
  }

  @Deprecated
  public WXScrollView.WXScrollViewListener getScrollViewListener() {
    return mWXScrollViewListener;
  }

  @Deprecated
  public void setIWXUserTrackAdapter(IWXUserTrackAdapter adapter) {
  }

  public void setContainerInfo(String key,String val){
    mContainerInfo.put(key,val);
  }

  public Map<String, String> getContainerInfo() {
    return mContainerInfo;
  }

  /**
   * Render template asynchronously, use {@link WXRenderStrategy#APPEND_ASYNC} as render strategy
   * @param template bundle js
   * @param options  os   iphone/android/ipad
   *                 weexversion    Weex version(like 1.0.0)
   *                 appversion     App version(like 1.0.0)
   *                 devid        Device id(like Aqh9z8dRJNBhmS9drLG5BKCmXhecHUXIZoXOctKwFebH)
   *                 sysversion    Device system version(like 5.4.4、7.0.4, should be used with os)
   *                 sysmodel     Device model(like iOS:"MGA82J/A", android:"MI NOTE LTE")
   *                 Time    UNIX timestamp, UTC+08:00
   *                 TTID(Optional)
   *                 MarkertId
   *                 Appname(Optional)  tm,tb,qa
   *                 Bundleurl(Optional)  template url
   * @param jsonInitData Initial data for rendering
   */
  public void render(String template, Map<String, Object> options, String jsonInitData) {
    render(template, options, jsonInitData, WXRenderStrategy.APPEND_ASYNC);
  }

  /**
   * Render template asynchronously
   * @param template bundle js
   * @param options  os   iphone/android/ipad
   *                 weexversion    Weex version(like 1.0.0)
   *                 appversion     App version(like 1.0.0)
   *                 devid        Device id(like Aqh9z8dRJNBhmS9drLG5BKCmXhecHUXIZoXOctKwFebH)
   *                 sysversion    Device system version(like 5.4.4、7.0.4, should be used with os)
   *                 sysmodel     Device model(like iOS:"MGA82J/A", android:"MI NOTE LTE")
   *                 Time    UNIX timestamp, UTC+08:00
   *                 TTID(Optional)
   *                 MarkertId
   *                 Appname(Optional)  tm,tb,qa
   *                 Bundleurl(Optional)  template url
   * @param jsonInitData Initial data for rendering
   * @param flag     RenderStrategy {@link WXRenderStrategy}
   */
  @Deprecated
  public void render(String template, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag) {
    render(WXPerformance.DEFAULT, template, options, jsonInitData, flag);
  }

  /**
   * Render template asynchronously
   *
   * @param pageName, used for performance log.
   * @param template bundle js
   * @param options  os   iphone/android/ipad
   *                 weexversion    Weex version(like 1.0.0)
   *                 appversion     App version(like 1.0.0)
   *                 devid        Device id(like Aqh9z8dRJNBhmS9drLG5BKCmXhecHUXIZoXOctKwFebH)
   *                 sysversion    Device system version(like 5.4.4、7.0.4, should be used with os)
   *                 sysmodel     Device model(like iOS:"MGA82J/A", android:"MI NOTE LTE")
   *                 Time    UNIX timestamp, UTC+08:00
   *                 TTID(Optional)
   *                 MarkertId
   *                 Appname(Optional)  tm,tb,qa
   *                 Bundleurl(Optional)  template url
   * @param jsonInitData Initial data for rendering
   * @param flag     RenderStrategy {@link WXRenderStrategy}
   */
  public void render(String pageName, String template, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag) {
    render(pageName, new Script(template), options, jsonInitData, flag);
  }

  public void render(String pageName, Script template, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag) {
    mWXPerformance.beforeInstanceRender(mInstanceId);

    if(WXEnvironment.isApkDebugable() && WXPerformance.DEFAULT.equals(pageName)){

      if (getUIContext() != null) {
        new AlertDialog.Builder(getUIContext())
                .setTitle("Error: Missing pageName")
                .setMessage("We highly recommend you to set pageName. Call" +
                        "\nWXSDKInstance#render(String pageName, String template, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag)\n" +
                        "to fix it.")
                .show();
      }

      return;
    }
    renderInternal(pageName,template,options,jsonInitData,flag);
  }

  /**
   * Render binary template asynchronously in DATA_RENDER_BINARY strategy.
   */
  public void render(String pageName, byte[] template, Map<String, Object> options, String jsonInitData) {
    render(pageName, new Script(template), options, jsonInitData, WXRenderStrategy.DATA_RENDER_BINARY);
  }

  private void ensureRenderArchor(){
    if(mRenderContainer == null){
      if (getContext() != null) {
        setRenderContainer(new RenderContainer(getContext()));
        mRenderContainer.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        mRenderContainer.setBackgroundColor(Color.TRANSPARENT);
        mRenderContainer.setSDKInstance(this);
        mRenderContainer.addOnLayoutChangeListener(this);
      }
    }
  }

  private void renderInternal(String pageName,
                              String template,
                              Map<String, Object> options,
                              String jsonInitData,
                              WXRenderStrategy flag) {
    if (mRendered || TextUtils.isEmpty(template)) {
      return;
    }
    renderInternal(pageName, new Script(template), options, jsonInitData, flag);
  }

  private boolean isPreInit = false;
  public boolean isPreInitMode(){
    return isPreInit;
  }

  private boolean isPreDownLoad =false;
  public boolean isPreDownLoad(){
    return isPreDownLoad;
  }

  public void onInstanceReady(){
    WXLogUtils.e("test->","onInstanceReady");
    mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_CONTAINER_READY);
    if (!isPreInit && !isPreDownLoad){
      return;
    }
    mApmForInstance.onInstanceReady(isPreDownLoad);
    if (isPreDownLoad){
        mHttpListener.onInstanceReady();
    }
  }

  public void preInit(String pageName,
                      String script,
                      Map<String, Object> options,
                      String jsonInitData,
                      WXRenderStrategy flag){
    isPreInit = true;
    mRenderStrategy = flag;
    Map<String, Object> renderOptions = options;
    if (renderOptions == null) {
      renderOptions = new HashMap<>();
    }
    //mApmForInstance.doInit();
    mApmForInstance.isReady = false;
    WXSDKManager.getInstance().createInstance(this, new Script(script), renderOptions, jsonInitData);
  }

  public void preDownLoad(String url,
                          Map<String, Object> options,
                          String jsonInitData,
                          WXRenderStrategy flag){
    this.isPreDownLoad =true;
    mRenderStrategy = flag;
    mApmForInstance.isReady = false;
    renderByUrl(url,url,options,jsonInitData,flag);
  }


  private void renderInternal(String pageName,
                              Script template,
                              Map<String, Object> options,
                              String jsonInitData,
                              WXRenderStrategy flag){
    if (mRendered || template == null || template.isEmpty()) {
      return;
    }

    mRenderStrategy = flag;

    //some case ,from render(template),but not render (url)
    if (!mApmForInstance.hasInit()){
      mApmForInstance.doInit();
    }
    mApmForInstance.setPageName(pageName);
    mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_RENDER_ORGIGIN);

    mWXPerformance.pageName = (TextUtils.isEmpty(pageName) ? "defaultBundleUrl":pageName);
    if (TextUtils.isEmpty(mBundleUrl)) {
      mBundleUrl = mWXPerformance.pageName;
    }

    if (WXTracing.isAvailable()) {
      WXTracing.TraceEvent traceEvent = WXTracing.newEvent("executeBundleJS", mInstanceId, -1);
      traceEvent.traceId = mExecJSTraceId;
      traceEvent.iid = mInstanceId;
      traceEvent.tname = "JSThread";
      traceEvent.ph = "B";
      traceEvent.submit();
      mRenderStartNanos = System.nanoTime();
    }

    ensureRenderArchor();

    Map<String, Object> renderOptions = options;
    if (renderOptions == null) {
      renderOptions = new HashMap<>();
    }

    if (WXEnvironment.sDynamicMode && !TextUtils.isEmpty(WXEnvironment.sDynamicUrl) && renderOptions.get("dynamicMode") == null) {
      renderOptions.put("dynamicMode", "true");
      renderByUrl(pageName, WXEnvironment.sDynamicUrl, renderOptions, jsonInitData, flag);
      return;
    }



    mWXPerformance.JSTemplateSize = template.length() / 1024f;
    mApmForInstance.addStats(WXInstanceApm.KEY_PAGE_STATS_BUNDLE_SIZE,mWXPerformance.JSTemplateSize);
    mRenderStartTime = System.currentTimeMillis();
    WXSDKManager.getInstance().setCrashInfo(WXEnvironment.WEEX_CURRENT_KEY,pageName);;
    if(mAutoAdjustDeviceWidth && WXDeviceUtils.isAutoResize(mContext)){
         if(WXEnvironment.AUTO_UPDATE_APPLICATION_SCREEN_SIZE) {
             WXViewUtils.updateApplicationScreen(mContext);
         }
         WXParams params = WXBridgeManager.getInstance().getInitParams();
         if(params != null && !TextUtils.equals(params.getDeviceWidth(), String.valueOf(WXViewUtils.getScreenWidth(mContext)))){
           params.setDeviceWidth(String.valueOf(WXViewUtils.getScreenWidth(mContext)));
           params.setDeviceHeight(String.valueOf(WXViewUtils.getScreenHeight(mContext)));
           float density = WXEnvironment.sApplication.getResources().getDisplayMetrics().density;
           WXEnvironment.addCustomOptions(WXConfig.scale, Float.toString(density));
           String statusBarHeight =  null;
           if(WXViewUtils.getStatusBarHeight(mContext) > 0 ){
             statusBarHeight = String.valueOf(WXViewUtils.getStatusBarHeight(mContext));
           }
           WXBridgeManager.getInstance().updateInitDeviceParams(params.getDeviceWidth(),
                                                                params.getDeviceHeight(),
                                                                Float.toString(density), statusBarHeight);
           setDeviceDisplay(WXViewUtils.getScreenWidth(mContext),
                            WXViewUtils.getScreenHeight(mContext),
                            WXViewUtils.getScreenDensity(mContext));
         }
    }
    if (isPreInitMode()){
      getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_START);
      WXBridgeManager.getInstance().loadJsBundleInPreInitMode(getInstanceId(),template.getContent());
    } else {
      WXSDKManager.getInstance().createInstance(this, template, renderOptions, jsonInitData);
    }
    mRendered = true;

    final IWXJscProcessManager wxJscProcessManager = WXSDKManager.getInstance().getWXJscProcessManager();

    if(wxJscProcessManager != null && wxJscProcessManager.shouldReboot()) {
      WXSDKManager.getInstance().postOnUiThread(new Runnable() {
        @Override
        public void run() {
          if(isDestroy || hasException || isRenderSuccess) {
            return;
          }

          View containerView = getContainerView();
          if(containerView instanceof ViewGroup) {
            if(0 == ((ViewGroup) containerView).getChildCount()) {
              if(wxJscProcessManager.withException(WXSDKInstance.this)) {
                onJSException(String.valueOf(WX_ERR_RELOAD_PAGE),"jsc reboot","jsc reboot");
              }
              WXBridgeManager.getInstance().callReportCrashReloadPage(mInstanceId, null);
            }
          }
        }
      }, wxJscProcessManager.rebootTimeout());
    }
  }

  public boolean skipFrameworkInit(){
    return isDataRender() && !mDisableSkipFrameworkInit;
  }

  private boolean isDataRender() {
    return getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY || getRenderStrategy() == WXRenderStrategy.DATA_RENDER;
  }

  private void renderByUrlInternal(String pageName,
                                   final String url,
                                   Map<String, Object> options,
                                   final String jsonInitData,
                                   WXRenderStrategy flag) {

    ensureRenderArchor();
    pageName = wrapPageName(pageName, url);
    mBundleUrl = url;
    mRenderStrategy = flag;
    if(WXSDKManager.getInstance().getValidateProcessor()!=null) {
      mNeedValidate = WXSDKManager.getInstance().getValidateProcessor().needValidate(mBundleUrl);
    }

    Map<String, Object> renderOptions = options;
    if (renderOptions == null) {
      renderOptions = new HashMap<>();
    }
    if (!renderOptions.containsKey(BUNDLE_URL)) {
      renderOptions.put(BUNDLE_URL, url);
    }

    getWXPerformance().pageName = pageName;

    mApmForInstance.doInit();
    mApmForInstance.setPageName(pageName);

    Uri uri = Uri.parse(url);
    if (uri != null && TextUtils.equals(uri.getScheme(), "file")) {
      mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_START);
      String template = WXFileUtils.loadFileOrAsset(assembleFilePath(uri), mContext);
      mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_END);
      render(pageName,template , renderOptions, jsonInitData, flag);
      return;
    }

    boolean is_wlasm = false;
    if (uri != null && uri.getPath()!=null) {
      if(uri.getPath().endsWith(".wlasm")){
        is_wlasm =  true;
      }
    }
    if (is_wlasm){
      flag = WXRenderStrategy.DATA_RENDER_BINARY;
    }

    IWXHttpAdapter adapter = WXSDKManager.getInstance().getIWXHttpAdapter();

    WXRequest wxRequest = new WXRequest();
    wxRequest.url = rewriteUri(Uri.parse(url),URIAdapter.BUNDLE).toString();
    if(wxRequest != null && !TextUtils.isEmpty(wxRequest.url)){
      requestUrl = wxRequest.url;
    }else {
      requestUrl = pageName;
    }

    if (wxRequest.paramMap == null) {
      wxRequest.paramMap = new HashMap<String, String>();
    }
    wxRequest.instanceId = getInstanceId();
    wxRequest.paramMap.put(KEY_USER_AGENT, WXHttpUtil.assembleUserAgent(mContext,WXEnvironment.getConfig()));
    wxRequest.paramMap.put("isBundleRequest","true");
    mHttpListener = new WXHttpListener(this, pageName, renderOptions, jsonInitData, flag, System.currentTimeMillis());
    mHttpListener.isPreDownLoadMode = isPreDownLoad;
    mHttpListener.setSDKInstance(this);
    mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_START);
    adapter.sendRequest(wxRequest, (IWXHttpAdapter.OnHttpListener) mHttpListener);
  }

  private WXHttpListener mHttpListener = null;

  /**
   * Use {@link #render(String, String, Map, String, WXRenderStrategy)} instead.
   * @param pageName
   * @param template
   * @param options
   * @param jsonInitData
   * @param width
   * @param height
   * @param flag
   */
  @Deprecated
  public void render(String pageName, String template, Map<String, Object> options, String jsonInitData, int width, int height, WXRenderStrategy flag) {
    render(pageName,template,options,jsonInitData,flag);
  }

  /**
   * Render template asynchronously, use {@link WXRenderStrategy#APPEND_ASYNC} as render strategy
   * @param template bundle js
   */
  public void render(String template) {
    render(WXPerformance.DEFAULT, template, null, null, mRenderStrategy);
  }

  /**
   * Use {@link #render(String)} instead.
   * @param template
   * @param width
   * @param height
   */
  @Deprecated
  public void render(String template, int width, int height) {
    render(template);
  }

  /**
   * Use {@link #renderByUrl(String, String, Map, String, WXRenderStrategy)} instead.
   * @param pageName
   * @param url
   * @param options
   * @param jsonInitData
   * @param width
   * @param height
   * @param flag
   */
  @Deprecated
  public void renderByUrl(String pageName, final String url, Map<String, Object> options, final String jsonInitData, final int width, final int height, final WXRenderStrategy flag){
    renderByUrl(pageName,url,options,jsonInitData,flag);
  }

  public void renderByUrl(String pageName, final String url, Map<String, Object> options, final String jsonInitData, final WXRenderStrategy flag) {
    renderByUrlInternal(pageName,url,options,jsonInitData,flag);
  }

  private String wrapPageName(String pageName, String url) {
    if(TextUtils.equals(pageName, WXPerformance.DEFAULT)){
      pageName = url;
      WXExceptionUtils.degradeUrl = pageName;
      try {
        Uri uri=Uri.parse(url);
        if(uri!=null){
          Uri.Builder builder=new Uri.Builder();
          builder.scheme(uri.getScheme());
          builder.authority(uri.getAuthority());
          builder.path(uri.getPath());
          pageName=builder.toString();
        }
      } catch (Exception e) {
      }
    }
    return pageName;
  }

  private String assembleFilePath(Uri uri) {
    if(uri!=null && uri.getPath()!=null){
      return uri.getPath().replaceFirst("/","");
    }
    return "";
  }

  public void reloadPage(boolean reloadThis) {

    WXSDKEngine.reload();

    if (reloadThis) {
      if (mContext != null)  {
        Intent intent = new Intent();
        intent.setAction(ACTION_INSTANCE_RELOAD);
        intent.putExtra("url", mBundleUrl);
        mContext.sendBroadcast(intent);
      }
      // mRendered = false;
      //    destroy();
      // renderInternal(mPackage, mTemplate, mOptions, mJsonInitData, mFlag);
      // refreshInstance("{}");
    } else {
      IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
      if (adapter != null) {
        boolean degrade = Boolean.parseBoolean(adapter
                .getConfig("android_weex_ext_config",
                        "degrade_to_h5_if_not_reload",
                        "true"));
        WXLogUtils.e("degrade : " + degrade);
        if(degrade) {
          onJSException(String.valueOf(WX_ERR_JSC_CRASH.getErrorCode()),"jsc Crashed", "jsc Crashed degradeToH5");
        }
      }
    }
  }

  /**
   * Refresh instance asynchronously.
   * @param data the new data
   */
  public void refreshInstance(Map<String, Object> data) {
    if (data == null) {
      return;
    }
    refreshInstance(WXJsonUtils.fromObjectToJSONString(data));
  }

  /**
   * Refresh instance asynchronously.
   * @param jsonData the new data
   */
  public void refreshInstance(String jsonData) {
    if (jsonData == null) {
      return;
    }
    mRefreshStartTime = System.currentTimeMillis();
    //cancel last refresh message
    if (mLastRefreshData != null) {
      mLastRefreshData.isDirty = true;
    }

    mLastRefreshData = new WXRefreshData(jsonData, false);

    WXSDKManager.getInstance().refreshInstance(mInstanceId, mLastRefreshData);
  }

  public WXRenderStrategy getRenderStrategy() {
    return mRenderStrategy;
  }

  public Context getUIContext() {
    return mContext;
  }

  public String getInstanceId() {
    return mInstanceId;
  }

  public Context getContext() {
    return mContext;
  }

  public int getWeexHeight() {
    return mRenderContainer == null ? 0: mRenderContainer.getHeight();
  }

  public int getWeexWidth() {
    return mRenderContainer == null ? 0: mRenderContainer.getWidth();
  }


  public IWXImgLoaderAdapter getImgLoaderAdapter() {
    return WXSDKManager.getInstance().getIWXImgLoaderAdapter();
  }

  public IDrawableLoader getDrawableLoader() {
    return WXSDKManager.getInstance().getDrawableLoader();
  }

  public URIAdapter getURIAdapter(){
    return WXSDKManager.getInstance().getURIAdapter();
  }

  public Uri rewriteUri(Uri uri,String type){
    return getURIAdapter().rewrite(this,type,uri);
  }

  public IWXHttpAdapter getWXHttpAdapter() {
    return WXSDKManager.getInstance().getIWXHttpAdapter();
  }

  public IWXStatisticsListener getWXStatisticsListener() {
    return mStatisticsListener;
  }

  public @Nullable
  IWebSocketAdapter getWXWebSocketAdapter() {
    return WXSDKManager.getInstance().getIWXWebSocketAdapter();
  }

  @Deprecated
  public void reloadImages() {
    if (mScrollView == null) {
      return;
    }
  }


  public boolean isPreRenderMode() {
    return this.isPreRenderMode;
  }

  public void setPreRenderMode(final boolean isPreRenderMode) {
    WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(new Runnable() {
      @Override
      public void run() {
        WXSDKInstance.this.isPreRenderMode = isPreRenderMode;
      }
    },0);
  }

  public void setContext(@NonNull Context context) {
    this.mContext = context;
  }

  /********************************
   * begin register listener
   ********************************************************/
  public void registerRenderListener(IWXRenderListener listener) {
    mRenderListener = listener;
  }

  @Deprecated
  public void registerActivityStateListener(IWXActivityStateListener listener) {

  }

  public void registerStatisticsListener(IWXStatisticsListener listener) {
    mStatisticsListener = listener;
  }

  /**set render start time*/
  public void setRenderStartTime(long renderStartTime) {
    this.mRenderStartTime = renderStartTime;
  }

  /********************************
   * end register listener
   ********************************************************/


  /********************************
   *  begin hook Activity life cycle callback
   ********************************************************/

  @Override
  public void onActivityCreate() {

    // module listen Activity onActivityCreate
    WXModuleManager.onActivityCreate(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityCreate();
    }else{
        if (WXEnvironment.isApkDebugable()){
            WXLogUtils.w("Warning :Component tree has not build completely,onActivityCreate can not be call!");
        }
    }

    mGlobalEventReceiver=new WXGlobalEventReceiver(this);
    try {
      getContext().registerReceiver(mGlobalEventReceiver, new IntentFilter(WXGlobalEventReceiver.EVENT_ACTION));
    } catch (Throwable e) {
      // Huawei may throw a exception if register more than 500 BroadcastReceivers
      WXLogUtils.e(e.getMessage());
      mGlobalEventReceiver = null;
    }

  }

  @Override
  public void onActivityStart() {

    // module listen Activity onActivityCreate
    WXModuleManager.onActivityStart(getInstanceId());
    if(mRootComp != null) {
      mRootComp.onActivityStart();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely,onActivityStart can not be call!");

        }
    }

  }

  public boolean onCreateOptionsMenu(Menu menu) {

    WXModuleManager.onCreateOptionsMenu(getInstanceId(),menu);
    if(mRootComp != null) {
      mRootComp.onCreateOptionsMenu(menu);
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely,onActivityStart can not be call!");

        }
    }
    return true;
  }

  @Override
  public void onActivityPause() {
    onViewDisappear();
    if(!isCommit){
      if(mUseScroller){
        mWXPerformance.useScroller=1;
      }
      mWXPerformance.maxDeepViewLayer=getMaxDeepLayer();
      mWXPerformance.wxDims = mwxDims;
      mWXPerformance.measureTimes = measureTimes;
      if (mUserTrackAdapter != null) {
        mUserTrackAdapter.commit(mContext, null, IWXUserTrackAdapter.LOAD, mWXPerformance, getUserTrackParams());
      }
      isCommit=true;
    }
    // module listen Activity onActivityPause
    WXModuleManager.onActivityPause(getInstanceId());
    if(mRootComp != null) {
      mRootComp.onActivityPause();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely,onActivityPause can not be call!");
        }
    }

    if (!mCurrentGround) {
      WXLogUtils.i("Application to be in the backround");
      Intent intent = new Intent(WXGlobalEventReceiver.EVENT_ACTION);
      intent.putExtra(WXGlobalEventReceiver.EVENT_NAME, Constants.Event.PAUSE_EVENT);
      intent.putExtra(WXGlobalEventReceiver.EVENT_WX_INSTANCEID, getInstanceId());
      /**
       *  Fix NPE just like {@link #onActivityResume()}
       */
      if (null != mContext){
        mContext.sendBroadcast(intent);
      }else {
        WXEnvironment.getApplication().sendBroadcast(intent);
      }
      this.mCurrentGround = true;
    }
  }


  @Override
  public void onActivityResume() {

    // notify onActivityResume callback to module
    WXModuleManager.onActivityResume(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityResume();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely, onActivityResume can not be call!");
        }
    }

    if (mCurrentGround) {
      WXLogUtils.i("Application  to be in the foreground");
      Intent intent = new Intent(WXGlobalEventReceiver.EVENT_ACTION);
      intent.putExtra(WXGlobalEventReceiver.EVENT_NAME, Constants.Event.RESUME_EVENT);
      intent.putExtra(WXGlobalEventReceiver.EVENT_WX_INSTANCEID, getInstanceId());
      //todo tmp solution for gray version
      if (null != mContext){
        mContext.sendBroadcast(intent);
      }else {
        WXEnvironment.getApplication().sendBroadcast(intent);
      }
      this.mCurrentGround = false;
    }

    onViewAppear();
  }

  @Override
  public void onActivityStop() {

    // notify onActivityResume callback to module
    WXModuleManager.onActivityStop(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityStop();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely, onActivityStop can not be call!");
        }
    }

  }

  @Override
  public void onActivityDestroy() {
    WXModuleManager.onActivityDestroy(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityDestroy();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely, onActivityDestroy can not be call!");
        }
    }

    destroy();
  }

  @Override
  public boolean onActivityBack() {

    WXModuleManager.onActivityBack(getInstanceId());

    if(mRootComp != null) {
      return mRootComp.onActivityBack();
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely, onActivityBack can not be call!");
        }
    }

    return false;
  }

  public boolean onBackPressed() {
    WXComponent comp = getRootComponent();
    if(comp != null) {
      WXEvent events= comp.getEvents();
      boolean hasNativeBackHook = events.contains(Constants.Event.NATIVE_BACK);
      if (hasNativeBackHook) {
        EventResult result = comp.fireEventWait(Constants.Event.NATIVE_BACK, null);
        if (WXUtils.getBoolean(result.getResult(), false)) {
          return true;
        }
      }

      boolean hasBackPressed = events.contains(Constants.Event.CLICKBACKITEM);
      if (hasBackPressed) {
        fireEvent(comp.getRef(), Constants.Event.CLICKBACKITEM,null, null);
      }
      return hasBackPressed;
    }
    return false;
  }

  public void onActivityResult(int requestCode, int resultCode, Intent data){
    WXModuleManager.onActivityResult(getInstanceId(),requestCode,resultCode,data);

    if(mRootComp != null) {
      mRootComp.onActivityResult(requestCode,resultCode,data);
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w("Warning :Component tree has not build completely, onActivityResult can not be call!");
        }
    }
  }


  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {

    WXModuleManager.onRequestPermissionsResult(getInstanceId(),requestCode,permissions,grantResults);

    if(mRootComp != null) {
       mRootComp.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }else{
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.w(
                "Warning :Component tree has not build completely, onRequestPermissionsResult can not be call!");
        }
    }
  }

  /********************************
   *  end hook Activity life cycle callback
   ********************************************************/

  public void onViewDisappear(){
    isViewDisAppear = false;
    mApmForInstance.onDisAppear();
    WXComponent comp = getRootComponent();
    if(comp != null) {
      fireEvent(comp.getRef(), Constants.Event.VIEWDISAPPEAR, null, null);
      //call disappear of nested instances
      for(OnInstanceVisibleListener instance:mVisibleListeners){
        instance.onDisappear();
      }
    }
  }

  public boolean isViewDisAppear(){
    return isViewDisAppear;
  }

  public void onViewAppear(){
    isViewDisAppear = true;
    mApmForInstance.onAppear();
    WXComponent comp = getRootComponent();
    if(comp != null) {
      fireEvent( comp.getRef(), Constants.Event.VIEWAPPEAR,null, null);
      for(OnInstanceVisibleListener instance:mVisibleListeners){
        instance.onAppear();
      }
    }
  }


  public void onCreateFinish() {
    if (mContext != null) {
      onViewAppear();
      View wxView= mRenderContainer;
      if(mRenderListener != null) {
        mRenderListener.onViewCreated(WXSDKInstance.this, wxView);
      }
      if (mStatisticsListener != null) {
        mStatisticsListener.onFirstView();
      }
    }
  }

  /**
   * call back when update finish
   */
  public void onUpdateFinish() {
    if (WXEnvironment.isApkDebugable()){
      WXLogUtils.d("Instance onUpdateSuccess");
    }
  }


  public void runOnUiThread(Runnable action) {
    WXSDKManager.getInstance().postOnUiThread(action, 0);
  }

  public void onRenderSuccess(final int width, final int height) {
    isRenderSuccess = true;
    if (!isNewFsEnd){
      getApmForInstance().arriveNewFsRenderTime();
    }

    long time = System.currentTimeMillis() - mRenderStartTime;
    long[] renderFinishTime = WXBridgeManager.getInstance().getRenderFinishTime(getInstanceId());

    mWXPerformance.callBridgeTime = renderFinishTime[0];
    mWXPerformance.cssLayoutTime = renderFinishTime[1];
    mWXPerformance.parseJsonTime = renderFinishTime[2];

    mWXPerformance.totalTime = time;
    if(mWXPerformance.screenRenderTime<0.001){
      mWXPerformance.screenRenderTime =  time;
    }

    if (mRenderListener != null && mContext != null) {
      mRenderListener.onRenderSuccess(WXSDKInstance.this, width, height);
      if (mUserTrackAdapter != null) {
        WXPerformance performance=new WXPerformance(mInstanceId);
        performance.errCode=WXErrorCode.WX_SUCCESS.getErrorCode();
        performance.args=getBundleUrl();
        mUserTrackAdapter.commit(mContext,null,IWXUserTrackAdapter.JS_BRIDGE,performance,getUserTrackParams());
      }
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d(WXLogUtils.WEEX_PERF_TAG, mWXPerformance.toString());
      }
    }
    if(WXEnvironment.isPerf()){
      WXLogUtils.e("weex_perf",mWXPerformance.getPerfData());
    }
  }

  public void onRefreshSuccess(final int width, final int height) {
    if (mRenderListener != null && mContext != null) {
      mRenderListener.onRefreshSuccess(WXSDKInstance.this, width, height);
    }
  }

  public void onChangeElement(WXComponent component, boolean isOutOfScreen) {

    if (isDestroy()  || null == mRenderContainer || mWXPerformance == null ){
      return;
    }
    if (null == component || component.isIgnoreInteraction){
        return;
    }

    if (mRenderContainer.hasConsumeEvent()) {
      return;
    }

    long lastElementChangeTime = System.currentTimeMillis();

    long lazyLoadTime;

    if (mHasCreateFinish){
      lazyLoadTime = lastElementChangeTime - mWXPerformance.renderTimeOrigin;
      if (lazyLoadTime > 8000) {
        //bad case
        return;
      }
    }

    if (component.mIsAddElementToTree) {
      getWXPerformance().localInteractionViewAddCount++;
      if (!isOutOfScreen)
        getWXPerformance().interactionViewAddLimitCount++;
      component.mIsAddElementToTree = false;
    }

    if (!isOutOfScreen) {
      mApmForInstance.arriveInteraction(component);
    }
  }

  public void onRenderError(final String errCode, final String msg) {
    if (mRenderListener != null && mContext != null) {
      runOnUiThread(new Runnable() {

        @Override
        public void run() {
          if (mRenderListener != null && mContext != null) {
            mRenderListener.onException(WXSDKInstance.this, errCode, msg);
          }
        }
      });
    }
  }

  public void onJSException(final String errCode, final String function, final String exception) {
    hasException = true;
    if (mRenderListener != null && mContext != null) {
      runOnUiThread(new Runnable() {

        @Override
        public void run() {
          if (mRenderListener != null && mContext != null) {
            StringBuilder builder = new StringBuilder();
            builder.append(function);
            builder.append(exception);
            mRenderListener.onException(WXSDKInstance.this, errCode, builder.toString());
          }
        }
      });
    }
  }


  @Override
  public final void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft, int
          oldTop, int oldRight, int oldBottom) {
    if (left != oldLeft || top != oldTop || right != oldRight || bottom != oldBottom) {
      onLayoutChange(v);
    }
  }

  /**
   * Subclass should override this method to get notifications of layout change of GodView.
   * @param godView the godView.
   */
  public void onLayoutChange(View godView) {

  }

  private boolean mCreateInstance =true;
  public void firstScreenCreateInstanceTime(long time) {
    if(mCreateInstance) {
      mWXPerformance.firstScreenJSFExecuteTime = time -mRenderStartTime;
      mCreateInstance =false;
    }
  }

  public void callJsTime(final long time){
    if (!mEnd){
      mWXPerformance.fsCallJsTotalTime+=time;
      mWXPerformance.fsCallJsTotalNum++;
    }
  }

  public void onComponentCreate(WXComponent component,long createTime) {
      mWXPerformance.mActionAddElementCount++;
      mWXPerformance.mActionAddElementSumTime += createTime;
      if (!mEnd){
        mWXPerformance.fsComponentCreateTime+=createTime;
        mWXPerformance.fsComponentCount++;
      }
      mWXPerformance.componentCount++;
      mWXPerformance.componentCreateTime+=createTime;
  }

  public void callActionAddElementTime(long time) {
      mWXPerformance.mActionAddElementSumTime += time;
  }

  public void onOldFsRenderTimeLogic(){
      if (mEnd){
        return;
      }
      mEnd = true;
      if (mStatisticsListener != null && mContext != null) {
        runOnUiThread(new Runnable() {
        @Override
        public void run() {
            if (mStatisticsListener != null && mContext != null) {
              Trace.beginSection("onFirstScreen");
              mStatisticsListener.onFirstScreen();
              Trace.endSection();
            }
          }
        });
      }
      mApmForInstance.arriveFSRenderTime();
      mWXPerformance.fsRenderTime = System.currentTimeMillis();
      mWXPerformance.screenRenderTime = System.currentTimeMillis() - mRenderStartTime;
  }

  public WXSDKInstance getParentInstance() {
    return mParentInstance;
  }

  public void setParentInstance(WXSDKInstance mParentInstance) {
    this.mParentInstance = mParentInstance;
  }

  private void destroyView(View rootView) {
    try {
      if (rootView instanceof ViewGroup) {
        ViewGroup cViewGroup = ((ViewGroup) rootView);
        for (int index = 0; index < cViewGroup.getChildCount(); index++) {
          destroyView(cViewGroup.getChildAt(index));
        }

        cViewGroup.removeViews(0, ((ViewGroup) rootView).getChildCount());
        // Ensure that the viewgroup's status to be normal
        WXReflectionUtils.setValue(rootView, "mChildrenCount", 0);

      }
      if(rootView instanceof Destroyable){
        ((Destroyable)rootView).destroy();
      }
    } catch (Exception e) {
      WXLogUtils.e("WXSDKInstance destroyView Exception: ", e);
    }
  }

  public synchronized void destroy() {
    if(!isDestroy()) {
      if(mParentInstance != null){
         mParentInstance = null;
      }
      mApmForInstance.onEnd();
      if(mRendered) {
        WXSDKManager.getInstance().destroyInstance(mInstanceId);
      }

      if (mGlobalEventReceiver != null) {
        getContext().unregisterReceiver(mGlobalEventReceiver);
        mGlobalEventReceiver = null;
      }
      if (mRootComp != null) {
        mRootComp.destroy();
        destroyView(mRenderContainer);
        mRootComp = null;
      }

      if (mGlobalEvents != null) {
        mGlobalEvents.clear();
      }

      if (mComponentObserver != null) {
        mComponentObserver = null;
      }

      if (mLayerOverFlowListeners != null) {
        mLayerOverFlowListeners.clear();
      }

      getFlatUIContext().destroy();
      mFlatGUIContext = null;
      mInstanceOnFireEventInterceptorList = null;
      mWXScrollListeners = null;
      mRenderContainer = null;
      mNestedInstanceInterceptor = null;
      mUserTrackAdapter = null;
      mScrollView = null;
      mContext = null;
      mRenderListener = null;
      isDestroy = true;
      mStatisticsListener = null;
      if(responseHeaders != null){
        responseHeaders.clear();
      }
      if(templateRef != null){
        templateRef = null;
      }
      if (null != mContentBoxMeasurements) {
        mContentBoxMeasurements.clear();
      }
      mWXPerformance.afterInstanceDestroy(mInstanceId);

      WXBridgeManager.getInstance().post(new Runnable() {
        @Override
        public void run() {
          WXBridgeManager.getInstance().onInstanceClose(getInstanceId());
          inactiveAddElementAction.clear();
        }
      });

      //when report error in @WXExceptionUtils
      // instance may had destroy and remove,
      // so we delay remove from allInstanceMap
      WXBridgeManager.getInstance().postDelay(new Runnable() {
        @Override
        public void run() {
          WXSDKManager.getInstance().getAllInstanceMap().remove(mInstanceId);
        }
      },1000);
    }
  }

  public boolean isDestroy(){
    return isDestroy;
  }

  /**
   * @return If you use render () the return value may be empty
   */
  public @Nullable String getBundleUrl() {
    return mBundleUrl;
  }

  public View getRootView() {
    if (mRootComp == null)
      return null;
    return mRootComp.getRealView();
  }

  public View getContainerView() {
    return mRenderContainer;
  }

  @Deprecated
  public void setBundleUrl(String url){
    mBundleUrl = url;
    if(WXSDKManager.getInstance().getValidateProcessor()!=null) {
      mNeedValidate = WXSDKManager.getInstance().getValidateProcessor().needValidate(mBundleUrl);
    }
  }

  public void onRootCreated(WXComponent root) {
    this.mRootComp = root;
    this.mRootComp.mDeepInComponentTree =1;
    mRenderContainer.addView(root.getHostView());


    setSize(mRenderContainer.getWidth(),mRenderContainer.getHeight());
  }

  /**
   * Move fixed view to container ,except it's already moved.
   * @param fixedChild
   */
  public void moveFixedView(View fixedChild){
    if(mRenderContainer != null) {
      ViewGroup parent;
      if((parent = (ViewGroup) fixedChild.getParent()) != null){
        if (parent != mRenderContainer) {
          parent.removeView(fixedChild);
          mRenderContainer.addView(fixedChild);
        }
      }else{
        mRenderContainer.addView(fixedChild);
      }
    }
  }

  public void removeFixedView(View fixedChild){
    if(mRenderContainer != null) {
      mRenderContainer.removeView(fixedChild);
    }
  }

  public int getRenderContainerPaddingLeft() {
    if(mRenderContainer != null) {
      return mRenderContainer.getPaddingLeft();
    }
    return 0;
  }

  public int getRenderContainerPaddingRight() {
    if(mRenderContainer != null) {
      return mRenderContainer.getPaddingRight();
    }
    return 0;
  }

  public int getRenderContainerPaddingTop() {
    if(mRenderContainer != null) {
      return mRenderContainer.getPaddingTop();
    }
    return 0;
  }

  public synchronized List<OnWXScrollListener> getWXScrollListeners() {
    return mWXScrollListeners;
  }

  public synchronized void registerOnWXScrollListener(OnWXScrollListener wxScrollListener) {
    if(mWXScrollListeners==null){
      mWXScrollListeners=new ArrayList<>();
    }
    mWXScrollListeners.add(wxScrollListener);
  }

  static int sScreenHeight = -1;
  public void setSize(int width, int height) {
    if (width > 0 && height > 0 & !isDestroy && mRendered && mRenderContainer != null) {
        if (sScreenHeight < 0){
            sScreenHeight = WXViewUtils.getScreenHeight(getContext());
        }
        if (sScreenHeight>0){
            double screenRatio = (double)height/(double)sScreenHeight *100;
            if(screenRatio>100){
              screenRatio =100;
            }
            getApmForInstance().addStats(WXInstanceApm.KEY_PAGE_STATS_BODY_RATIO,screenRatio);
        }
      ViewGroup.LayoutParams layoutParams = mRenderContainer.getLayoutParams();
      if (layoutParams != null) {
        final float realWidth = width;
        final float realHeight = height;
        if (mRenderContainer.getWidth() != width || mRenderContainer.getHeight() != height) {
          layoutParams.width = width;
          layoutParams.height = height;
          mRenderContainer.setLayoutParams(layoutParams);
        }

        if (mRootComp != null && layoutParams != null) {
          final boolean isWidthWrapContent = layoutParams.width == ViewGroup.LayoutParams.WRAP_CONTENT;
          final boolean isHeightWrapContent = layoutParams.height == ViewGroup.LayoutParams.WRAP_CONTENT;

          WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
              WXBridgeManager.getInstance().setDefaultRootSize(getInstanceId(), realWidth, realHeight, isWidthWrapContent,
                      isHeightWrapContent);
            }
          });
        }
      }
    }
  }

  /*Global Event*/
  private HashMap<String, List<String>> mGlobalEvents = new HashMap<>();

  public void fireGlobalEventCallback(String eventName, Map<String,Object> params){
    List<String> callbacks=mGlobalEvents.get(eventName);
    if(callbacks!=null){
      for(String callback:callbacks){
        WXSDKManager.getInstance().callback(mInstanceId,callback,params,true);
      }
    }
  }

  /**
   * Fire event callback on a element.
   * @param elementRef
   * @param type
   * @param data
   * @param domChanges
   */
  public void fireEvent(String elementRef,final String type, final Map<String, Object> data,final Map<String, Object> domChanges, List<Object> eventArgs){
    fireEvent(elementRef, type, data, domChanges, eventArgs, null);
  }

  public void fireEvent(String elementRef,final String type, final Map<String, Object> data,final Map<String, Object> domChanges, List<Object> eventArgs, EventResult callback) {
    onInterceptInstanceEvent(getInstanceId(), elementRef, type, data, domChanges);
    if (null != mWXPerformance && mWXPerformance.fsCallEventTotalNum<Integer.MAX_VALUE){
      mWXPerformance.fsCallEventTotalNum++;
    }
    mApmForInstance.updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_CALL_EVENT_NUM,1);
    WXBridgeManager.getInstance().fireEventOnNode(getInstanceId(),elementRef,type,data,domChanges, eventArgs, callback);
  }


  /**
   * Fire event callback on a element.
   * @param elementRef
   * @param type
   * @param data
   * @param domChanges
   */
  public void fireEvent(String elementRef,final String type, final Map<String, Object> data,final Map<String, Object> domChanges){
    fireEvent(elementRef, type, data, domChanges, null);
  }

  public void fireEvent(String elementRef,final String type, final Map<String, Object> data){
    fireEvent(elementRef,type,data,null);
  }

  public void fireEvent(String ref, String type){
    fireEvent(ref,type,new HashMap<String, Object>());
  }

  protected void addEventListener(String eventName, String callback) {
    if (TextUtils.isEmpty(eventName) || TextUtils.isEmpty(callback)) {
      return;
    }
    List<String> callbacks = mGlobalEvents.get(eventName);
    if (callbacks == null) {
      callbacks = new ArrayList<>();
      mGlobalEvents.put(eventName, callbacks);
    }
    callbacks.add(callback);
  }
  protected void removeEventListener(String eventName, String callback) {
    if (TextUtils.isEmpty(eventName) || TextUtils.isEmpty(callback)) {
      return;
    }
    List<String> callbacks = mGlobalEvents.get(eventName);
    if (callbacks != null) {
      callbacks.remove(callback);
    }
  }

  protected void removeEventListener(String eventName) {
    if (TextUtils.isEmpty(eventName)) {
      return;
    }
    mGlobalEvents.remove(eventName);
  }

  /**
   * Notifies WEEX that this event has occurred
   * @param eventName WEEX register event
   * @param module Events occur in this Module
   * @param params The parameters to be notified to WEEX are required
   */
  public void fireModuleEvent(String eventName, WXModule module,Map<String, Object> params) {
    if (TextUtils.isEmpty(eventName) || module == null) {
      return;
    }

    Map<String, Object> event = new HashMap<>();
    event.put("type", eventName);
    event.put("module", module.getModuleName());
    event.put("data", params);

    List<String> callbacks = module.getEventCallbacks(eventName);
    if (callbacks != null) {
      for (String callback : callbacks) {
        SimpleJSCallback jsCallback = new SimpleJSCallback(mInstanceId, callback);
        if (module.isOnce(callback)) {
          jsCallback.invoke(event);
        } else {
          jsCallback.invokeAndKeepAlive(event);
        }
      }
    }
  }

  /**
   * Check whether the current module registered the event
   * @param eventName EventName register in weex
   * @param module Events occur in this Module
   * @return  register->true
   */
  public boolean checkModuleEventRegistered(String eventName,WXModule module) {
    if (module != null) {
      List<String> events = module.getEventCallbacks(eventName);
      if (events != null && events.size() > 0) {
        return true;
      }
    }
    return false;
  }

  public WXPerformance getWXPerformance(){
    return mWXPerformance;
  }

  public WXInstanceApm getApmForInstance() {
    return mApmForInstance;
  }

  public Map<String, Serializable> getUserTrackParams() {
    return mUserTrackParams;
  }

  public void addUserTrackParameter(String key,Serializable value){
    if(this.mUserTrackParams == null){
      this.mUserTrackParams = new ConcurrentHashMap<>();
    }
    mUserTrackParams.put(key,value);
  }

  public void clearUserTrackParameters(){
    if(this.mUserTrackParams != null){
      this.mUserTrackParams.clear();
    }
  }

  public void removeUserTrackParameter(String key){
    if(this.mUserTrackParams != null){
      this.mUserTrackParams.remove(key);
    }
  }

  public int getMaxDeepLayer() {
    return mMaxDeepLayer;
  }

  public void setMaxDeepLayer(int maxDeepLayer) {
    mMaxDeepLayer = maxDeepLayer;
    mApmForInstance.updateMaxStats(WXInstanceApm.KEY_PAGE_STATS_MAX_DEEP_VIEW,maxDeepLayer);
  }

  public void setMaxDomDeep(int maxDomDeep){
    mApmForInstance.updateMaxStats(WXInstanceApm.KEY_PAGE_STATS_MAX_DEEP_DOM,maxDomDeep);
    if (null == mWXPerformance){
      return;
    }
    if (mWXPerformance.maxDeepVDomLayer <= maxDomDeep){
      mWXPerformance.maxDeepVDomLayer = maxDomDeep;
    }
  }

  public void onHttpStart(){
    if (!mEnd){
      mWXPerformance.fsRequestNum++;
    }
  }

  /**
   * return md5, and bytes length
   * */
  public String getTemplateInfo() {
    String template = getTemplate();
    if(template == null){
      return " template md5 null ,httpHeader:" + JSONObject.toJSONString(responseHeaders);
    }
    if(TextUtils.isEmpty(template)){
      return " template md5  length 0 ,httpHeader" + JSONObject.toJSONString(responseHeaders);
    }
    try {
      byte[] bts = template.getBytes("UTF-8");
      String sourceMD5 = WXFileUtils.md5(bts);
      String sourceBase64MD5 = WXFileUtils.base64Md5(bts);
      ArrayList<String> sourceMD5List = new ArrayList<>();
      ArrayList<String> sourceBase64MD5List = new ArrayList<>();
      sourceMD5List.add(sourceMD5);
      sourceBase64MD5List.add(sourceBase64MD5);
      responseHeaders.put("templateSourceMD5", sourceMD5List);
      responseHeaders.put(SOURCE_TEMPLATE_BASE64_MD5, sourceBase64MD5List);
      return " template md5 " + sourceMD5 + " length " +   bts.length
              + " base64 md5 " + sourceBase64MD5
              + " response header " + JSONObject.toJSONString(responseHeaders);
    } catch (Exception e) {
      return "template md5 getBytes error";
    }

  }

  /**
   * check template header md5 match with header  content-md5
   * */
  public boolean isContentMd5Match(){
    if(responseHeaders == null){
      return true;
    }
    List<String> contentMD5s = responseHeaders.get("Content-Md5");
    if(contentMD5s == null){
      contentMD5s  = responseHeaders.get("content-md5");
    }
    if(contentMD5s == null || contentMD5s.size() <= 0){
      return true;
    }
    String md5 = contentMD5s.get(0);

    List<String> sourceBase64Md5 = responseHeaders.get(SOURCE_TEMPLATE_BASE64_MD5);
    if(sourceBase64Md5 == null){
      getTemplateInfo();
      sourceBase64Md5 = responseHeaders.get(SOURCE_TEMPLATE_BASE64_MD5);
    }
    if(sourceBase64Md5 == null || sourceBase64Md5.size() == 0){
      return  true;
    }
    return  md5.equals(sourceBase64Md5.get(0));
  }

  public String getTemplate() {
    if(templateRef == null){
      return  null;
    }
    return templateRef.get();
  }

  public void setTemplate(String template) {
    this.templateRef = new WeakReference<String>(template);
  }

  public interface NestedInstanceInterceptor {
    void onCreateNestInstance(WXSDKInstance instance, NestedContainer container);
  }

  public void OnVSync() {
    boolean forceLayout = WXBridgeManager.getInstance().notifyLayout(getInstanceId());
    if(forceLayout) {
      WXBridgeManager.getInstance().post(new Runnable() {
        @Override
        public void run() {
          WXBridgeManager.getInstance().forceLayout(getInstanceId());
        }
      });
    }
  }

  public void addContentBoxMeasurement(long renderObjectPtr, ContentBoxMeasurement contentBoxMeasurement) {
    mContentBoxMeasurements.put(renderObjectPtr, contentBoxMeasurement);
  }

  public ContentBoxMeasurement getContentBoxMeasurement(long renderObjectPtr) {
    return mContentBoxMeasurements.get(renderObjectPtr);
  }


  private void onInterceptInstanceEvent(String instanceId, String elementRef, String type, Map<String, Object> data, Map<String, Object> domChanges) {
    if(this.mInstanceOnFireEventInterceptorList == null){
      return;
    }
    for(InstanceOnFireEventInterceptor instanceOnFireEventInterceptor : this.mInstanceOnFireEventInterceptorList){
      instanceOnFireEventInterceptor.onInterceptFireEvent(instanceId, elementRef, type, data, domChanges);
    }
  }

  public List<InstanceOnFireEventInterceptor> getInstanceOnFireEventInterceptorList(){
    if(this.mInstanceOnFireEventInterceptorList == null){
      this.mInstanceOnFireEventInterceptorList = new ArrayList<>();
    }
    return mInstanceOnFireEventInterceptorList;
  }


  public void addInstanceOnFireEventInterceptor(InstanceOnFireEventInterceptor instanceOnFireEventInterceptor) {
    if(instanceOnFireEventInterceptor == null){
      return;
    }
    if(!getInstanceOnFireEventInterceptorList().contains(instanceOnFireEventInterceptor)){
      getInstanceOnFireEventInterceptorList().add(instanceOnFireEventInterceptor);
    }
  }

  private static boolean isDisableSkipFrameworkInDataRender() {
    IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
    if (adapter == null) {
      return false;
    }
    String result = adapter.getConfig("wxeagle", "disable_skip_framework_init", "false");
    return "true".equals(result);
  }
}
