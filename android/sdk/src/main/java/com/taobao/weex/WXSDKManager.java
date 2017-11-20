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

import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.adapter.DefaultUriAdapter;
import com.taobao.weex.adapter.DefaultWXHttpAdapter;
import com.taobao.weex.adapter.ICrashInfoReporter;
import com.taobao.weex.adapter.IDrawableLoader;
import com.taobao.weex.adapter.ITracingAdapter;
import com.taobao.weex.adapter.IWXDebugAdapter;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.adapter.IWXImgLoaderAdapter;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.adapter.IWXSoLoaderAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.adapter.URIAdapter;
import com.taobao.weex.appfram.navigator.IActivityNavBarSetter;
import com.taobao.weex.appfram.storage.DefaultWXStorage;
import com.taobao.weex.appfram.storage.IWXStorageAdapter;
import com.taobao.weex.appfram.websocket.IWebSocketAdapter;
import com.taobao.weex.appfram.websocket.IWebSocketAdapterFactory;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXModuleManager;
import com.taobao.weex.bridge.WXValidateProcessor;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.common.WXWorkThreadManager;
import com.taobao.weex.dom.WXDomManager;
import com.taobao.weex.ui.WXRenderManager;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Manger class for weex context.
 */
public class WXSDKManager {

  private static volatile WXSDKManager sManager;
  private static AtomicInteger sInstanceId = new AtomicInteger(0);
  private final WXDomManager mWXDomManager;
  private final WXWorkThreadManager mWXWorkThreadManager;
  private WXBridgeManager mBridgeManager;
  /** package **/ WXRenderManager mWXRenderManager;

  private IWXUserTrackAdapter mIWXUserTrackAdapter;
  private IWXImgLoaderAdapter mIWXImgLoaderAdapter;
  private IWXSoLoaderAdapter mIWXSoLoaderAdapter;
  private IDrawableLoader mDrawableLoader;
  private IWXHttpAdapter mIWXHttpAdapter;
  private IWXDebugAdapter mIWXDebugAdapter;
  private IActivityNavBarSetter mActivityNavBarSetter;

  private ICrashInfoReporter mCrashInfo;

  private IWXJSExceptionAdapter mIWXJSExceptionAdapter;

  private IWXStorageAdapter mIWXStorageAdapter;
  private IWXStatisticsListener mStatisticsListener;
  private URIAdapter mURIAdapter;
  private IWebSocketAdapterFactory mIWebSocketAdapterFactory;
  private ITracingAdapter mTracingAdapter;
  private WXValidateProcessor mWXValidateProcessor;
  // Tell weexv8 to initialize v8, default is true.
  private boolean mNeedInitV8 = true;

  private List<InstanceLifeCycleCallbacks> mLifeCycleCallbacks;

  private static final int DEFAULT_VIEWPORT_WIDTH = 750;

  private WXSDKManager() {
    this(new WXRenderManager());
  }

  private WXSDKManager(WXRenderManager renderManager) {
    mWXRenderManager = renderManager;
    mWXDomManager = new WXDomManager(mWXRenderManager);
    mBridgeManager = WXBridgeManager.getInstance();
    mWXWorkThreadManager = new WXWorkThreadManager();
  }

  /**
   * Used in junit test
   */
  static void initInstance(WXRenderManager renderManager){
    sManager = new WXSDKManager(renderManager);
  }

  public void registerStatisticsListener(IWXStatisticsListener listener) {
    mStatisticsListener = listener;
  }

  public IWXStatisticsListener getWXStatisticsListener() {
    return mStatisticsListener;
  }

  public void onSDKEngineInitialize() {
    if (mStatisticsListener != null) {
      mStatisticsListener.onSDKEngineInitialize();
    }
  }

  public void setNeedInitV8(boolean need) {
    mNeedInitV8 = need;
  }

  public boolean needInitV8() {
    return mNeedInitV8;
  }

  public void takeJSHeapSnapshot(String path) {
    File file = new File(path);
    if (!file.exists()) {
      if (!file.mkdir()) {
        return;
      }
    }

    String name = String.valueOf(sInstanceId.get());
    String filename = path;

    if (!path.endsWith(File.separator)) {
      filename += File.separator;
    }
    filename += name;
    filename += ".heapsnapshot";

    mBridgeManager.takeJSHeapSnapshot(filename);
  }

  public static WXSDKManager getInstance() {
    if (sManager == null) {
      synchronized (WXSDKManager.class) {
        if(sManager == null) {
          sManager = new WXSDKManager();
        }
      }
    }
    return sManager;
  }

  public static int getInstanceViewPortWidth(String instanceId){
    WXSDKInstance instance = getInstance().getSDKInstance(instanceId);
    if (instance == null) {
      return DEFAULT_VIEWPORT_WIDTH;
    }
    return instance.getInstanceViewPortWidth();
  }

  static void setInstance(WXSDKManager manager){
    sManager = manager;
  }

  public IActivityNavBarSetter getActivityNavBarSetter() {
    return mActivityNavBarSetter;
  }

  public void setActivityNavBarSetter(IActivityNavBarSetter mActivityNavBarSetter) {
    this.mActivityNavBarSetter = mActivityNavBarSetter;
  }

  public void restartBridge() {
    mBridgeManager.restart();
  }

  public WXDomManager getWXDomManager() {
    return mWXDomManager;
  }

  public WXBridgeManager getWXBridgeManager() {
    return mBridgeManager;
  }

  public WXRenderManager getWXRenderManager() {
    return mWXRenderManager;
  }

  public WXWorkThreadManager getWXWorkThreadManager() {
    return mWXWorkThreadManager;
  }

  public @Nullable WXSDKInstance getSDKInstance(String instanceId) {
    return instanceId == null? null : mWXRenderManager.getWXSDKInstance(instanceId);
  }

  public void postOnUiThread(Runnable runnable, long delayMillis) {
    mWXRenderManager.postOnUiThread(WXThread.secure(runnable), delayMillis);
  }

  public void destroy() {
    if (mWXDomManager != null) {
      mWXDomManager.destroy();
    }
    if (mWXWorkThreadManager != null) {
      mWXWorkThreadManager.destroy();
    }
  }

  @Deprecated
  public void callback(String instanceId, String funcId, Map<String, Object> data) {
    mBridgeManager.callback(instanceId, funcId, data);
  }

  @Deprecated
  public void callback(String instanceId, String funcId, Map<String, Object> data,boolean keepAlive) {
    mBridgeManager.callback(instanceId, funcId, data,keepAlive);
  }

  public void initScriptsFramework(String framework) {
    mBridgeManager.initScriptsFramework(framework);
  }

  public void registerComponents(List<Map<String, Object>> components) {
    mBridgeManager.registerComponents(components);
  }

  public void registerModules(Map<String, Object> modules) {
    mBridgeManager.registerModules(modules);
  }

  /**
   * Do not direct invoke this method in Components, use {@link WXSDKInstance#fireEvent(String, String, Map, Map)} instead.
   */
  @Deprecated
  public void fireEvent(final String instanceId, String ref, String type) {
    fireEvent(instanceId, ref, type, new HashMap<String, Object>());
  }

  /**
   * FireEvent back to JS
   * Do not direct invoke this method in Components, use {@link WXSDKInstance#fireEvent(String, String, Map, Map)} instead.
   */
  @Deprecated
  public void fireEvent(final String instanceId, String ref, String type, Map<String, Object> params){
    fireEvent(instanceId,ref,type,params,null);
  }

  /**
   * Do not direct invoke this method in Components, use {@link WXSDKInstance#fireEvent(String, String, Map, Map)} instead.
   **/
  @Deprecated
  public void fireEvent(final String instanceId, String ref, String type, Map<String, Object> params,Map<String,Object> domChanges) {
    if (WXEnvironment.isApkDebugable() && Looper.getMainLooper().getThread().getId() != Thread.currentThread().getId()) {
      throw new WXRuntimeException("[WXSDKManager]  fireEvent error");
    }
    mBridgeManager.fireEventOnNode(instanceId, ref, type, params,domChanges);
  }

  void createInstance(WXSDKInstance instance, String code, Map<String, Object> options, String jsonInitData) {
    mWXRenderManager.registerInstance(instance);
    mBridgeManager.createInstance(instance.getInstanceId(), code, options, jsonInitData);
    if (mLifeCycleCallbacks != null) {
      for (InstanceLifeCycleCallbacks callbacks : mLifeCycleCallbacks) {
        callbacks.onInstanceCreated(instance.getInstanceId());
      }
    }
  }

  void refreshInstance(String instanceId, WXRefreshData jsonData) {
    mBridgeManager.refreshInstance(instanceId, jsonData);
  }

  void destroyInstance(String instanceId) {
    setCrashInfo(WXEnvironment.WEEX_CURRENT_KEY,"");
    if (TextUtils.isEmpty(instanceId)) {
      return;
    }
    if (!WXUtils.isUiThread()) {
      throw new WXRuntimeException("[WXSDKManager] destroyInstance error");
    }
    if (mLifeCycleCallbacks != null) {
      for (InstanceLifeCycleCallbacks callbacks : mLifeCycleCallbacks) {
        callbacks.onInstanceDestroyed(instanceId);
      }
    }
    mWXRenderManager.removeRenderStatement(instanceId);
    mWXDomManager.removeDomStatement(instanceId);
    mBridgeManager.destroyInstance(instanceId);
    WXModuleManager.destroyInstanceModules(instanceId);
  }

  String generateInstanceId() {
    return String.valueOf(sInstanceId.incrementAndGet());
  }

  public IWXUserTrackAdapter getIWXUserTrackAdapter() {
    return mIWXUserTrackAdapter;
  }

  public IWXImgLoaderAdapter getIWXImgLoaderAdapter() {
    return mIWXImgLoaderAdapter;
  }

  public IDrawableLoader getDrawableLoader() {
    return mDrawableLoader;
  }

  public IWXJSExceptionAdapter getIWXJSExceptionAdapter() {
    return mIWXJSExceptionAdapter;
  }

  public void setIWXJSExceptionAdapter(IWXJSExceptionAdapter IWXJSExceptionAdapter) {
    mIWXJSExceptionAdapter = IWXJSExceptionAdapter;
  }

  public @NonNull IWXHttpAdapter getIWXHttpAdapter() {
    if (mIWXHttpAdapter == null) {
      mIWXHttpAdapter = new DefaultWXHttpAdapter();
    }
    return mIWXHttpAdapter;
  }

  public @NonNull URIAdapter getURIAdapter() {
    if(mURIAdapter == null){
      mURIAdapter = new DefaultUriAdapter();
    }
    return mURIAdapter;
  }

  public IWXSoLoaderAdapter getIWXSoLoaderAdapter() {
    return mIWXSoLoaderAdapter;
  }

  void setInitConfig(InitConfig config){
    this.mIWXDebugAdapter = config.getDebugAdapter();
    this.mIWXHttpAdapter = config.getHttpAdapter();
    this.mIWXImgLoaderAdapter = config.getImgAdapter();
    this.mDrawableLoader = config.getDrawableLoader();
    this.mIWXStorageAdapter = config.getStorageAdapter();
    this.mIWXUserTrackAdapter = config.getUtAdapter();
    this.mURIAdapter = config.getURIAdapter();
    this.mIWebSocketAdapterFactory = config.getWebSocketAdapterFactory();
    this.mIWXJSExceptionAdapter = config.getJSExceptionAdapter();
    this.mIWXSoLoaderAdapter = config.getIWXSoLoaderAdapter();
  }

  public IWXDebugAdapter getIWXDebugAdapter() {
    return mIWXDebugAdapter;
  }

  public IWXStorageAdapter getIWXStorageAdapter(){
    if(mIWXStorageAdapter == null){
      if(WXEnvironment.sApplication != null){
        mIWXStorageAdapter = new DefaultWXStorage(WXEnvironment.sApplication);
      }else{
        WXLogUtils.e("WXStorageModule", "No Application context found,you should call WXSDKEngine#initialize() method in your application");
      }
    }
    return mIWXStorageAdapter;
  }

  /**
   * Weex embedders can use <code>notifyTrimMemory</code> to reduce
   * memory at a proper time.
   *
   * It's not a good idea to reduce memory at any time, because
   * memory trimming is a expense operation, and V8 needs to do
   * a full GC and all the inline caches get to be cleared.
   *
   * The embedder needs to make some scheduling strategies to
   * ensure that the embedded application is just on an idle time.
   * If the application use the same js bundle to render pages,
   * it's not a good idea to trim memory every time of exiting
   * pages.
   */
  public void notifyTrimMemory() {
    mBridgeManager.notifyTrimMemory();
  }

  /**
   * Weex embedders can use <code>notifySerializeCodeCache</code> to
   * serialize code caches if the jsfm has the alility to compile 'new Function'
   * against js bundles on the weex native side.
   *
   * It's a good time to serialize a code cache after exiting a weex page.
   * Then, the next time of entering the same weex page, V8 would compile
   * 'new Function' against the code cache deseriazed from the js bundle.
   */
  public void notifySerializeCodeCache() {
    mBridgeManager.notifySerializeCodeCache();
  }

  public @Nullable
  IWebSocketAdapter getIWXWebSocketAdapter() {
    if (mIWebSocketAdapterFactory != null) {
      return mIWebSocketAdapterFactory.createWebSocketAdapter();
    }
    return null;
  }

  public void registerValidateProcessor(WXValidateProcessor processor){
    this.mWXValidateProcessor = processor;
  }

  public WXValidateProcessor getValidateProcessor(){
    return mWXValidateProcessor;
  }


  public void setCrashInfoReporter(ICrashInfoReporter mCrashInfo) {
    this.mCrashInfo = mCrashInfo;
  }

  public void setCrashInfo(String key, String value) {
    if(mCrashInfo!=null){
      mCrashInfo.addCrashInfo(key,value);
    }
  }

  public void setTracingAdapter(ITracingAdapter adapter) {
    this.mTracingAdapter = adapter;
  }

  public ITracingAdapter getTracingAdapter() {
    return mTracingAdapter;
  }

  public void registerInstanceLifeCycleCallbacks(InstanceLifeCycleCallbacks callbacks) {
    if (mLifeCycleCallbacks == null) {
      mLifeCycleCallbacks = new ArrayList<>();
    }
    mLifeCycleCallbacks.add(callbacks);
  }

  public interface InstanceLifeCycleCallbacks {
    void onInstanceDestroyed(String instanceId);
    void onInstanceCreated(String instanceId);
  }
}
