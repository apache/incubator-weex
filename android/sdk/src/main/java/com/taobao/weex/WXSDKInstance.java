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

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ScrollView;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.adapter.IDrawableLoader;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.adapter.IWXImgLoaderAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.adapter.URIAdapter;
import com.taobao.weex.appfram.websocket.IWebSocketAdapter;
import com.taobao.weex.bridge.NativeInvokeHelper;
import com.taobao.weex.bridge.SimpleJSCallback;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXModuleManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.common.WXRequest;
import com.taobao.weex.common.WXResponse;
import com.taobao.weex.dom.DomContext;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXDomTask;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.http.WXHttpUtil;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.component.NestedContainer;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.view.WXScrollView;
import com.taobao.weex.ui.view.WXScrollView.WXScrollViewListener;
import com.taobao.weex.utils.Trace;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXReflectionUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import static com.taobao.weex.http.WXHttpUtil.KEY_USER_AGENT;


/**
 * Each instance of WXSDKInstance represents an running weex instance.
 * It can be a pure weex view, or mixed with native view
 */
public class WXSDKInstance implements IWXActivityStateListener,DomContext, View.OnLayoutChangeListener {

  //Performance
  public boolean mEnd = false;
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
  private boolean isDestroy=false;
  private Map<String,Serializable> mUserTrackParams;
  private NativeInvokeHelper mNativeInvokeHelper;
  private boolean isCommit=false;
  private WXGlobalEventReceiver mGlobalEventReceiver=null;
  private boolean trackComponent;
  private boolean enableLayerType = true;
  private boolean mNeedValidate = false;
  private static volatile int mViewPortWidth = 750;
  private int mInstanceViewPortWidth = 750;

  public long mRenderStartNanos;
  public int mExecJSTraceId = WXTracing.nextId();

  /**
   * Render strategy.
   */
  private WXRenderStrategy mRenderStrategy = WXRenderStrategy.APPEND_ASYNC;
  /**
   * Render start time
   */
  private long mRenderStartTime;
  /**
   * Refresh start time
   */
  private long mRefreshStartTime;
  private WXPerformance mWXPerformance;
  private ScrollView mScrollView;
  private WXScrollViewListener mWXScrollViewListener;

  private List<OnWXScrollListener> mWXScrollListeners;

  /**
   * whether we are in preRender mode
   * */
  private volatile boolean isPreRenderMode;

  private LayoutFinishListener mLayoutFinishListener;

  private boolean mCurrentGround = false;
  private ComponentObserver mComponentObserver;

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

  public boolean isNeedValidate() {
    return mNeedValidate;
  }

  /*
  *  Warning: use setInstanceViewPortWidth instead.
  *  store custom ViewPort Width
  */
  @Deprecated
  public void setViewPortWidth(int viewPortWidth) {
    mViewPortWidth = viewPortWidth;
  }

  /**
   * Warning: use getInstanceViewPortWidth instead.
   * @return
   */
  @Deprecated
  public static int getViewPortWidth() {
    return mViewPortWidth;
  }

  public void setInstanceViewPortWidth(int instanceViewPortWidth) {
    this.mInstanceViewPortWidth = instanceViewPortWidth;
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

  /**
   * For unittest only.
   */
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
    return sdkInstance;
  }

  protected WXSDKInstance newNestedInstance() {
    return new WXSDKInstance(mContext);
  }

  public void addOnInstanceVisibleListener(OnInstanceVisibleListener l){
    mVisibleListeners.add(l);
  }

  public void removeOnInstanceVisibleListener(OnInstanceVisibleListener l){
    mVisibleListeners.remove(l);
  }

  public void init(Context context) {
    mContext = context;
    mNativeInvokeHelper = new NativeInvokeHelper(mInstanceId);

    mWXPerformance = new WXPerformance();
    mWXPerformance.WXSDKVersion = WXEnvironment.WXSDK_VERSION;
    mWXPerformance.JSLibInitTime = WXEnvironment.sJSLibInitTime;

    mUserTrackAdapter=WXSDKManager.getInstance().getIWXUserTrackAdapter();
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
    if (mWXScrollViewListener != null) {
      ((WXScrollView) mScrollView).addScrollViewListener(mWXScrollViewListener);
    }
  }

  @Deprecated
  public void registerScrollViewListener(WXScrollViewListener scrollViewListener) {
    mWXScrollViewListener = scrollViewListener;
  }

  @Deprecated
  public WXScrollViewListener getScrollViewListener() {
    return mWXScrollViewListener;
  }

  @Deprecated
  public void setIWXUserTrackAdapter(IWXUserTrackAdapter adapter) {
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
    if(WXEnvironment.isApkDebugable() && WXPerformance.DEFAULT.equals(pageName)){
       WXLogUtils.e("Please set your pageName or your js bundle url !!!!!!!");
       return;
    }
    renderInternal(pageName,template,options,jsonInitData,flag);
  }

  private void ensureRenderArchor(){
    if(mRenderContainer == null){
      mRenderContainer = new RenderContainer(getContext());
      mRenderContainer.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
      mRenderContainer.setBackgroundColor(Color.TRANSPARENT);
      mRenderContainer.setSDKInstance(this);
      mRenderContainer.addOnLayoutChangeListener(this);
    }
  }

  private void renderInternal(String pageName,
                              String template,
                              Map<String, Object> options,
                              String jsonInitData,
                              WXRenderStrategy flag){
    if (mRendered || TextUtils.isEmpty(template)) {
      return;
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

    mWXPerformance.pageName = pageName;
    mWXPerformance.JSTemplateSize = template.length() / 1024f;

    mRenderStartTime = System.currentTimeMillis();
    mRenderStrategy = flag;

    WXSDKManager.getInstance().setCrashInfo(WXEnvironment.WEEX_CURRENT_KEY,pageName);

    WXSDKManager.getInstance().createInstance(this, template, renderOptions, jsonInitData);
    mRendered = true;

    if (TextUtils.isEmpty(mBundleUrl)) {
      mBundleUrl = pageName;
    }
  }

  private void renderByUrlInternal(String pageName,
                                   final String url,
                                   Map<String, Object> options,
                                   final String jsonInitData,
                                   final WXRenderStrategy flag) {

    ensureRenderArchor();
    pageName = wrapPageName(pageName, url);
    mBundleUrl = url;
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

    Uri uri = Uri.parse(url);
    if (uri != null && TextUtils.equals(uri.getScheme(), "file")) {
      render(pageName, WXFileUtils.loadFileOrAsset(assembleFilePath(uri), mContext), renderOptions, jsonInitData, flag);
      return;
    }

    IWXHttpAdapter adapter = WXSDKManager.getInstance().getIWXHttpAdapter();

    WXRequest wxRequest = new WXRequest();
    wxRequest.url = rewriteUri(Uri.parse(url),URIAdapter.BUNDLE).toString();
    if (wxRequest.paramMap == null) {
      wxRequest.paramMap = new HashMap<String, String>();
    }
    wxRequest.paramMap.put(KEY_USER_AGENT, WXHttpUtil.assembleUserAgent(mContext,WXEnvironment.getConfig()));
    WXHttpListener httpListener =
        new WXHttpListener(pageName, renderOptions, jsonInitData, flag, System.currentTimeMillis());
    httpListener.setSDKInstance(this);
    adapter.sendRequest(wxRequest, (IWXHttpAdapter.OnHttpListener) httpListener);
  }

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
      pageName=url;
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

  @Override
  public Context getUIContext() {
    return mContext;
  }

  public String getInstanceId() {
    return mInstanceId;
  }

  public Context getContext() {
    if(mContext == null){
      WXLogUtils.e("WXSdkInstance mContext == null");
    }
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

  public void setLayoutFinishListener(@Nullable LayoutFinishListener listener) {
    this.mLayoutFinishListener = listener;
  }

  public LayoutFinishListener getLayoutFinishListener() {
    return this.mLayoutFinishListener;
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
      WXLogUtils.w("Warning :Component tree has not build completely,onActivityCreate can not be call!");
    }

    mGlobalEventReceiver=new WXGlobalEventReceiver(this);
    getContext().registerReceiver(mGlobalEventReceiver,new IntentFilter(WXGlobalEventReceiver.EVENT_ACTION));
  }

  @Override
  public void onActivityStart() {

    // module listen Activity onActivityCreate
    WXModuleManager.onActivityStart(getInstanceId());
    if(mRootComp != null) {
      mRootComp.onActivityStart();
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely,onActivityStart can not be call!");
    }

  }

  public boolean onCreateOptionsMenu(Menu menu) {

    WXModuleManager.onCreateOptionsMenu(getInstanceId(),menu);
    if(mRootComp != null) {
      mRootComp.onCreateOptionsMenu(menu);
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely,onActivityStart can not be call!");
    }
    return true;
  }


  @Override
  public void onActivityPause() {
    onViewDisappear();
    if(!isCommit){
      Set<String> componentTypes= WXComponentFactory.getComponentTypesByInstanceId(getInstanceId());
      if(componentTypes!=null && componentTypes.contains(WXBasicComponentType.SCROLLER)){
        mWXPerformance.useScroller=1;
      }
      mWXPerformance.maxDeepViewLayer=getMaxDeepLayer();
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
      WXLogUtils.w("Warning :Component tree has not build completely,onActivityPause can not be call!");
    }

    WXLogUtils.i("Application onActivityPause()");
    if (!mCurrentGround) {
      WXLogUtils.i("Application to be in the backround");
      Intent intent = new Intent(WXGlobalEventReceiver.EVENT_ACTION);
      intent.putExtra(WXGlobalEventReceiver.EVENT_NAME, Constants.Event.PAUSE_EVENT);
      intent.putExtra(WXGlobalEventReceiver.EVENT_WX_INSTANCEID, getInstanceId());
      mContext.sendBroadcast(intent);
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
      WXLogUtils.w("Warning :Component tree has not build completely, onActivityResume can not be call!");
    }

    if (mCurrentGround) {
      WXLogUtils.i("Application  to be in the foreground");
      Intent intent = new Intent(WXGlobalEventReceiver.EVENT_ACTION);
      intent.putExtra(WXGlobalEventReceiver.EVENT_NAME, Constants.Event.RESUME_EVENT);
      intent.putExtra(WXGlobalEventReceiver.EVENT_WX_INSTANCEID, getInstanceId());
      mContext.sendBroadcast(intent);
      mCurrentGround = false;
    }

    onViewAppear();

    setViewPortWidth(mInstanceViewPortWidth);
  }

  @Override
  public void onActivityStop() {

    // notify onActivityResume callback to module
    WXModuleManager.onActivityStop(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityStop();
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely, onActivityStop can not be call!");
    }


  }

  @Override
  public void onActivityDestroy() {
    WXModuleManager.onActivityDestroy(getInstanceId());

    if(mRootComp != null) {
      mRootComp.onActivityDestroy();
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely, onActivityDestroy can not be call!");
    }

    destroy();
  }

  @Override
  public boolean onActivityBack() {

    WXModuleManager.onActivityBack(getInstanceId());

    if(mRootComp != null) {
      return mRootComp.onActivityBack();
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely, onActivityBack can not be call!");
    }

    return false;
  }

  public boolean onBackPressed() {
    WXComponent comp = getRootComponent();
    if(comp != null) {
      WXEvent events= comp.getDomObject().getEvents();
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
      WXLogUtils.w("Warning :Component tree has not build completely, onActivityResult can not be call!");
    }
  }


  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {

    WXModuleManager.onRequestPermissionsResult(getInstanceId(),requestCode,permissions,grantResults);

    if(mRootComp != null) {
       mRootComp.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }else{
      WXLogUtils.w("Warning :Component tree has not build completely, onRequestPermissionsResult can not be call!");
    }
  }

  /********************************
   *  end hook Activity life cycle callback
   ********************************************************/

  public void onViewDisappear(){
    WXComponent comp = getRootComponent();
    if(comp != null) {
      fireEvent(comp.getRef(), Constants.Event.VIEWDISAPPEAR, null, null);
      //call disappear of nested instances
      for(OnInstanceVisibleListener instance:mVisibleListeners){
        instance.onDisappear();
      }
    }
  }

  public void onViewAppear(){
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
      runOnUiThread(new Runnable() {

        @Override
        public void run() {
          if ( mContext != null) {
            onViewAppear();
            View wxView= mRenderContainer;
            if(WXEnvironment.isApkDebugable() && WXSDKManager.getInstance().getIWXDebugAdapter()!=null){
              wxView = WXSDKManager.getInstance().getIWXDebugAdapter().wrapContainer(WXSDKInstance.this,wxView);
            }
            if(mRenderListener != null) {
              mRenderListener.onViewCreated(WXSDKInstance.this, wxView);
            }
            if (mStatisticsListener != null) {
              mStatisticsListener.onFirstView();
            }
          }
        }
      });
    }
  }

  /**
   * call back when update finish
   */
  public void onUpdateFinish() {
    WXLogUtils.d("Instance onUpdateSuccess");
  }


  public void runOnUiThread(Runnable action) {
    WXSDKManager.getInstance().postOnUiThread(action, 0);
  }

  public void onRenderSuccess(final int width, final int height) {
    firstScreenRenderFinished();

    long time = System.currentTimeMillis() - mRenderStartTime;
    WXLogUtils.renderPerformanceLog("onRenderSuccess", time);
    WXLogUtils.renderPerformanceLog("   invokeCreateInstance",mWXPerformance.communicateTime);
    WXLogUtils.renderPerformanceLog("   TotalCallNativeTime", mWXPerformance.callNativeTime);
    WXLogUtils.renderPerformanceLog("       TotalJsonParseTime", mWXPerformance.parseJsonTime);
    WXLogUtils.renderPerformanceLog("   TotalBatchTime", mWXPerformance.batchTime);
    WXLogUtils.renderPerformanceLog("       TotalCssLayoutTime", mWXPerformance.cssLayoutTime);
    WXLogUtils.renderPerformanceLog("       TotalApplyUpdateTime", mWXPerformance.applyUpdateTime);
    WXLogUtils.renderPerformanceLog("       TotalUpdateDomObjTime", mWXPerformance.updateDomObjTime);


    mWXPerformance.totalTime = time;
    if(mWXPerformance.screenRenderTime<0.001){
      mWXPerformance.screenRenderTime =  time;
    }
    mWXPerformance.componentCount = WXComponent.mComponentNum;
    if(WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(WXLogUtils.WEEX_PERF_TAG, "mComponentNum:" + WXComponent.mComponentNum);
    }
    WXComponent.mComponentNum = 0;
    if (mRenderListener != null && mContext != null) {
      runOnUiThread(new Runnable() {

        @Override
        public void run() {
          if (mRenderListener != null && mContext != null) {
            mRenderListener.onRenderSuccess(WXSDKInstance.this, width, height);
            if (mUserTrackAdapter != null) {
              WXPerformance performance=new WXPerformance();
              performance.errCode=WXErrorCode.WX_SUCCESS.getErrorCode();
              performance.args=getBundleUrl();
              mUserTrackAdapter.commit(mContext,null,IWXUserTrackAdapter.JS_BRIDGE,performance,getUserTrackParams());
            }
            if (WXEnvironment.isApkDebugable()) {
              WXLogUtils.d(WXLogUtils.WEEX_PERF_TAG, mWXPerformance.toString());
            }
          }
        }
      });
    }
    if(!WXEnvironment.isApkDebugable()){
      Log.e("weex_perf",mWXPerformance.getPerfData());
    }
  }

  public void onRefreshSuccess(final int width, final int height) {
    WXLogUtils.renderPerformanceLog("onRefreshSuccess", (System.currentTimeMillis() - mRefreshStartTime));
    if (mRenderListener != null && mContext != null) {
      runOnUiThread(new Runnable() {

        @Override
        public void run() {
          if (mRenderListener != null && mContext != null) {
            mRenderListener.onRefreshSuccess(WXSDKInstance.this, width, height);
          }
        }
      });
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

  public void callNativeTime(long time) {
    mWXPerformance.callNativeTime += time;
  }

  public void jsonParseTime(long time) {
    mWXPerformance.parseJsonTime += time;
  }

  public void firstScreenRenderFinished() {
    if(mEnd == true)
       return;

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

    mWXPerformance.screenRenderTime = System.currentTimeMillis() - mRenderStartTime;
    WXLogUtils.renderPerformanceLog("firstScreenRenderFinished", mWXPerformance.screenRenderTime);
    WXLogUtils.renderPerformanceLog("   firstScreenJSFExecuteTime", mWXPerformance.firstScreenJSFExecuteTime);
    WXLogUtils.renderPerformanceLog("   firstScreenCallNativeTime", mWXPerformance.callNativeTime);
    WXLogUtils.renderPerformanceLog("       firstScreenJsonParseTime", mWXPerformance.parseJsonTime);
    WXLogUtils.renderPerformanceLog("   firstScreenBatchTime", mWXPerformance.batchTime);
    WXLogUtils.renderPerformanceLog("       firstScreenCssLayoutTime", mWXPerformance.cssLayoutTime);
    WXLogUtils.renderPerformanceLog("       firstScreenApplyUpdateTime", mWXPerformance.applyUpdateTime);
    WXLogUtils.renderPerformanceLog("       firstScreenUpdateDomObjTime", mWXPerformance.updateDomObjTime);
  }

  public void batchTime(long time) {
    mWXPerformance.batchTime += time;
  }
  public void cssLayoutTime(long time) {
      mWXPerformance.cssLayoutTime += time;
  }

  public void applyUpdateTime(long time) {
      mWXPerformance.applyUpdateTime += time;
  }

  public void updateDomObjTime(long time) {
      mWXPerformance.updateDomObjTime += time;
    }


  public void createInstanceFinished(long time) {
    if (time > 0) {
      mWXPerformance.communicateTime = time;
    }
  }

  /**
   * UserTrack Log
   */
  public void commitUTStab(final String type, final WXErrorCode errorCode) {
    if (TextUtils.isEmpty(type) || errorCode == null) {
      return;
    }

    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        // Record exception if a render error happened.
        if (mStatisticsListener != null && errorCode != WXErrorCode.WX_SUCCESS) {
          mStatisticsListener.onException(mInstanceId,
                                          errorCode.getErrorCode(),
                                          errorCode.getErrorMsg());
        }

        WXPerformance performance = new WXPerformance();
        performance.errCode = errorCode.getErrorCode();
        performance.args = errorCode.getArgs();
        if (errorCode != WXErrorCode.WX_SUCCESS) {
          performance.errMsg = errorCode.getErrorMsg();
          if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d(performance.toString());
          }
        }
        if( mUserTrackAdapter!= null) {
          mUserTrackAdapter.commit(mContext, null, type, performance, getUserTrackParams());
        }
      }
    });
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
    WXSDKManager.getInstance().destroyInstance(mInstanceId);
    WXComponentFactory.removeComponentTypesByInstanceId(getInstanceId());

    if(mGlobalEventReceiver!=null){
      getContext().unregisterReceiver(mGlobalEventReceiver);
      mGlobalEventReceiver=null;
    }
    if(mRootComp != null ) {
      mRootComp.destroy();
      destroyView(mRenderContainer);
      mRenderContainer = null;
      mRootComp = null;
    }

    if(mGlobalEvents!=null){
      mGlobalEvents.clear();
    }

    if(mComponentObserver != null){
        mComponentObserver = null;
    }

    mNestedInstanceInterceptor = null;
    mUserTrackAdapter = null;
    mScrollView = null;
    mContext = null;
    mRenderListener = null;
    isDestroy = true;
    mStatisticsListener = null;
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

  public synchronized List<OnWXScrollListener> getWXScrollListeners() {
    return mWXScrollListeners;
  }

  public synchronized void registerOnWXScrollListener(OnWXScrollListener wxScrollListener) {
    if(mWXScrollListeners==null){
      mWXScrollListeners=new ArrayList<>();
    }
    mWXScrollListeners.add(wxScrollListener);
  }

  private void updateRootComponentStyle(JSONObject style) {
    Message message = Message.obtain();
    WXDomTask task = new WXDomTask();
    task.instanceId = getInstanceId();
    if (task.args == null) {
      task.args = new ArrayList<>();
    }
    task.args.add(WXDomObject.ROOT);
    task.args.add(style);
    message.obj = task;
    message.what = WXDomHandler.MsgType.WX_DOM_UPDATE_STYLE;
    WXSDKManager.getInstance().getWXDomManager().sendMessage(message);
  }

  public void setSize(int width, int height) {
    if (width < 0 || height < 0 || isDestroy || !mRendered) {
      return;
    }
    float realWidth = WXViewUtils.getWebPxByWidth(width,getInstanceViewPortWidth());
    float realHeight = WXViewUtils.getWebPxByWidth(height,getInstanceViewPortWidth());

    View godView = mRenderContainer;
    if (godView != null) {
      ViewGroup.LayoutParams layoutParams = godView.getLayoutParams();
      if (layoutParams != null) {
        if(godView.getWidth() != width || godView.getHeight() != height) {
          layoutParams.width = width;
          layoutParams.height = height;
          godView.setLayoutParams(layoutParams);
        }

        JSONObject style = new JSONObject();
        WXComponent rootComponent = mRootComp;

        if(rootComponent == null){
          return;
        }
        style.put(Constants.Name.DEFAULT_WIDTH, realWidth);
        style.put(Constants.Name.DEFAULT_HEIGHT, realHeight);
        updateRootComponentStyle(style);
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
  public void fireEvent(String elementRef,final String type, final Map<String, Object> data,final Map<String, Object> domChanges){
    WXBridgeManager.getInstance().fireEventOnNode(getInstanceId(),elementRef,type,data,domChanges);
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
  }

  /**
   * load bundle js listener
   */
  class WXHttpListener implements IWXHttpAdapter.OnHttpListener {

    private String pageName;
    private Map<String, Object> options;
    private String jsonInitData;
    private WXRenderStrategy flag;
    private WXSDKInstance instance;
    private long startRequestTime;
    private int traceId;

    private WXHttpListener(String pageName, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag, long startRequestTime) {
      this.pageName = pageName;
      this.options = options;
      this.jsonInitData = jsonInitData;
      this.flag = flag;
      this.startRequestTime = startRequestTime;
      this.traceId = WXTracing.nextId();

      if (WXTracing.isAvailable()) {
        WXTracing.TraceEvent event = WXTracing.newEvent("downloadBundleJS", mInstanceId, -1);
        event.iid = mInstanceId;
        event.tname = "Network";
        event.ph = "B";
        event.traceId = traceId;
        event.submit();
      }
    }
    
    public void setSDKInstance(WXSDKInstance instance) {
      this.instance = instance;
    }

    @Override
    public void onHttpStart() {
      if (this.instance != null
          && this.instance.getWXStatisticsListener() != null) {
        this.instance.getWXStatisticsListener().onHttpStart();
      }
    }

    @Override
    public void onHeadersReceived(int statusCode,Map<String,List<String>> headers) {
      if (this.instance != null
          && this.instance.getWXStatisticsListener() != null) {
        this.instance.getWXStatisticsListener().onHeadersReceived();
      }
    }

    @Override
    public void onHttpUploadProgress(int uploadProgress) {

    }

    @Override
    public void onHttpResponseProgress(int loadedLength) {

    }

    @Override
    public void onHttpFinish(WXResponse response) {
      if (this.instance != null
          && this.instance.getWXStatisticsListener() != null) {
        this.instance.getWXStatisticsListener().onHttpFinish();
      }

      if (WXTracing.isAvailable()) {
        WXTracing.TraceEvent event = WXTracing.newEvent("downloadBundleJS", mInstanceId, -1);
        event.traceId = traceId;
        event.tname = "Network";
        event.ph = "E";
        event.extParams = new HashMap<>();
        event.extParams.put("BundleSize", response.originalData.length);
        event.submit();
      }

      mWXPerformance.networkTime = System.currentTimeMillis() - startRequestTime;
      if(response.extendParams!=null){
        Object actualNetworkTime=response.extendParams.get("actualNetworkTime");
        mWXPerformance.actualNetworkTime=actualNetworkTime instanceof Long?(long)actualNetworkTime:0;
        WXLogUtils.renderPerformanceLog("actualNetworkTime", mWXPerformance.actualNetworkTime);

        Object pureNetworkTime=response.extendParams.get("pureNetworkTime");
        mWXPerformance.pureNetworkTime=pureNetworkTime instanceof Long?(long)pureNetworkTime:0;
        WXLogUtils.renderPerformanceLog("pureNetworkTime", mWXPerformance.pureNetworkTime);

        Object connectionType=response.extendParams.get("connectionType");
        mWXPerformance.connectionType=connectionType instanceof String?(String)connectionType:"";

        Object packageSpendTime=response.extendParams.get("packageSpendTime");
        mWXPerformance.packageSpendTime=packageSpendTime instanceof Long ?(long)packageSpendTime:0;

        Object syncTaskTime=response.extendParams.get("syncTaskTime");
        mWXPerformance.syncTaskTime=syncTaskTime instanceof Long ?(long)syncTaskTime:0;

        Object requestType=response.extendParams.get("requestType");
        mWXPerformance.requestType=requestType instanceof String?(String)requestType:"";

        if("network".equals(requestType) && mUserTrackAdapter!=null){
          WXPerformance performance=new WXPerformance();
          if(!TextUtils.isEmpty(mBundleUrl)){
            try {
              performance.args= Uri.parse(mBundleUrl).buildUpon().clearQuery().toString();
            } catch (Exception e) {
              performance.args=pageName;
            }
          }
          if(!"200".equals(response.statusCode)){
            performance.errCode=WXErrorCode.WX_ERR_JSBUNDLE_DOWNLOAD.getErrorCode();
            performance.appendErrMsg(response.errorCode);
            performance.appendErrMsg("|");
            performance.appendErrMsg(response.errorMsg);
          }else if("200".equals(response.statusCode) && (response.originalData==null || response.originalData.length<=0)){
            performance.errCode=WXErrorCode.WX_ERR_JSBUNDLE_DOWNLOAD.getErrorCode();
            performance.appendErrMsg(response.statusCode);
            performance.appendErrMsg("|template is null!");
          }else {
            performance.errCode=WXErrorCode.WX_SUCCESS.getErrorCode();
          }
          if (mUserTrackAdapter != null) {
            mUserTrackAdapter.commit(getContext(), null, IWXUserTrackAdapter.JS_DOWNLOAD, performance, null);
          }
        }
      }
      WXLogUtils.renderPerformanceLog("networkTime", mWXPerformance.networkTime);
      if (response!=null && response.originalData!=null && TextUtils.equals("200", response.statusCode)) {
        String template = new String(response.originalData);
        render(pageName, template, options, jsonInitData, flag);
      } else if (TextUtils.equals(WXRenderErrorCode.WX_USER_INTERCEPT_ERROR, response.statusCode)) {
        WXLogUtils.d("user intercept");
        onRenderError(WXRenderErrorCode.WX_USER_INTERCEPT_ERROR,response.errorMsg);
      } else {
        onRenderError(WXRenderErrorCode.WX_NETWORK_ERROR, response.errorMsg);
      }
    }
  }

  public interface NestedInstanceInterceptor {
    void onCreateNestInstance(WXSDKInstance instance, NestedContainer container);
  }
}
