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

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;

import com.taobao.weex.adapter.IDrawableLoader;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.adapter.IWXImgLoaderAdapter;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.adapter.IWXJsFileLoaderAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.appfram.clipboard.WXClipboardModule;
import com.taobao.weex.appfram.navigator.IActivityNavBarSetter;
import com.taobao.weex.appfram.navigator.INavigator;
import com.taobao.weex.appfram.navigator.WXNavigatorModule;
import com.taobao.weex.appfram.pickers.WXPickersModule;
import com.taobao.weex.appfram.storage.IWXStorageAdapter;
import com.taobao.weex.appfram.storage.WXStorageModule;
import com.taobao.weex.appfram.websocket.WebSocketModule;
import com.taobao.weex.bridge.ModuleFactory;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXModuleManager;
import com.taobao.weex.bridge.WXServiceManager;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.TypeModuleFactory;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXException;
import com.taobao.weex.common.WXInstanceWrap;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.http.WXStreamModule;
import com.taobao.weex.ui.ExternalLoaderComponentHolder;
import com.taobao.weex.ui.IExternalComponentGetter;
import com.taobao.weex.ui.IExternalModuleGetter;
import com.taobao.weex.ui.IFComponentHolder;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.WXComponentRegistry;
import com.taobao.weex.ui.animation.WXAnimationModule;
import com.taobao.weex.ui.component.Textarea;
import com.taobao.weex.ui.component.WXA;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXEmbed;
import com.taobao.weex.ui.component.WXHeader;
import com.taobao.weex.ui.component.WXImage;
import com.taobao.weex.ui.component.WXIndicator;
import com.taobao.weex.ui.component.WXInput;
import com.taobao.weex.ui.component.WXLoading;
import com.taobao.weex.ui.component.WXLoadingIndicator;
import com.taobao.weex.ui.component.WXRefresh;
import com.taobao.weex.ui.component.WXScroller;
import com.taobao.weex.ui.component.WXSlider;
import com.taobao.weex.ui.component.WXSliderNeighbor;
import com.taobao.weex.ui.component.WXSwitch;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.WXVideo;
import com.taobao.weex.ui.component.WXWeb;
import com.taobao.weex.ui.component.list.HorizontalListComponent;
import com.taobao.weex.ui.component.list.SimpleListComponent;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.component.list.WXListComponent;
import com.taobao.weex.ui.component.list.template.WXRecyclerTemplateList;
import com.taobao.weex.ui.config.AutoScanConfigRegister;
import com.taobao.weex.ui.module.WXLocaleModule;
import com.taobao.weex.ui.module.WXMetaModule;
import com.taobao.weex.ui.module.WXModalUIModule;
import com.taobao.weex.ui.module.WXTimerModule;
import com.taobao.weex.ui.module.WXWebViewModule;
import com.taobao.weex.utils.LogLevel;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXSoInstallMgrSdk;
import com.taobao.weex.utils.batch.BatchOperationHelper;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

import static com.taobao.weex.WXEnvironment.CORE_SO_NAME;

public class WXSDKEngine implements Serializable {

  public static final String JS_FRAMEWORK_RELOAD="js_framework_reload";
  private static final String V8_SO_NAME = CORE_SO_NAME;
  private volatile static boolean mIsInit = false;
  private volatile static boolean mIsSoInit = false;
  private static final Object mLock = new Object();
  private static final String TAG = "WXSDKEngine";

  /**
   * Deprecated. Use {@link #initialize(Application, InitConfig)} instead.
   */
  @Deprecated
  public static void init(Application application) {
    init(application, null);
  }

  /**
   * Deprecated. Use {@link #initialize(Application, InitConfig)} instead.
   */
  @Deprecated
  public static void init(Application application, IWXUserTrackAdapter utAdapter) {
    init(application, utAdapter, null);
  }

  /**
   * Deprecated. Use {@link #initialize(Application, InitConfig)} instead.
   */
  @Deprecated
  public static void init(Application application, IWXUserTrackAdapter utAdapter, String framework) {
    initialize(application,
            new InitConfig.Builder()
                    .setUtAdapter(utAdapter)
                    .build()
    );
  }


  public static boolean isInitialized(){
    synchronized(mLock) {

      return mIsInit && WXEnvironment.JsFrameworkInit;
    }
  }

  public static boolean isSoInitialized(){
    synchronized(mLock) {
      return mIsSoInit;
    }
  }

  /**
   *
   * @param application
   * @param config initial configurations or null
   */
  public static void initialize(Application application,InitConfig config){
    synchronized (mLock) {
      if (mIsInit) {
        return;
      }
      long start = System.currentTimeMillis();
      WXEnvironment.sSDKInitStart = start;
      if(WXEnvironment.isApkDebugable()){
        WXEnvironment.sLogLevel = LogLevel.DEBUG;
      }else{
        if(WXEnvironment.sApplication != null){
          WXEnvironment.sLogLevel = LogLevel.WARN;
        }else {
          WXLogUtils.e(TAG,"WXEnvironment.sApplication is " + WXEnvironment.sApplication);
        }
      }
      doInitInternal(application,config);
      registerApplicationOptions(application);
      WXEnvironment.sSDKInitInvokeTime = System.currentTimeMillis()-start;
      WXLogUtils.renderPerformanceLog("SDKInitInvokeTime", WXEnvironment.sSDKInitInvokeTime);
      mIsInit = true;
    }
  }

  private static void registerApplicationOptions(final Application application) {

    if (application == null) {
      WXLogUtils.e(TAG, "RegisterApplicationOptions application is null");
      return;
    }

    Resources resources = application.getResources();
    registerCoreEnv("screen_width_pixels", String.valueOf(resources.getDisplayMetrics().widthPixels));
    registerCoreEnv("screen_height_pixels", String.valueOf(resources.getDisplayMetrics().heightPixels));

    int resourceId = resources.getIdentifier("status_bar_height", "dimen", "android");
    if (resourceId > 0) {
      int statusBarHeight = resources.getDimensionPixelSize(resourceId);
      registerCoreEnv("status_bar_height", String.valueOf(statusBarHeight));
    }
  }

  private static void doInitInternal(final Application application,final InitConfig config){
    WXEnvironment.sApplication = application;
    if(application == null){
      WXLogUtils.e(TAG, " doInitInternal application is null");
      WXExceptionUtils.commitCriticalExceptionRT(null,
              WXErrorCode.WX_KEY_EXCEPTION_SDK_INIT,
              "doInitInternal",
              WXErrorCode.WX_KEY_EXCEPTION_SDK_INIT.getErrorMsg() + "WXEnvironment sApplication is null",
              null);
    }
    WXEnvironment.JsFrameworkInit = false;

    WXBridgeManager.getInstance().post(new Runnable() {
      @Override
      public void run() {
        long start = System.currentTimeMillis();
        WXSDKManager sm = WXSDKManager.getInstance();
        sm.onSDKEngineInitialize();
        if(config != null ) {
          sm.setInitConfig(config);
        }
        WXSoInstallMgrSdk.init(application,
                sm.getIWXSoLoaderAdapter(),
                sm.getWXStatisticsListener());
        mIsSoInit = WXSoInstallMgrSdk.initSo(V8_SO_NAME, 1, config!=null?config.getUtAdapter():null);
        if (!mIsSoInit) {
          WXExceptionUtils.commitCriticalExceptionRT(null,
                  WXErrorCode.WX_KEY_EXCEPTION_SDK_INIT,
                  "doInitInternal",
                  WXErrorCode.WX_KEY_EXCEPTION_SDK_INIT.getErrorMsg() + "isSoInit false",
                  null);
          return;
        }
        sm.initScriptsFramework(config!=null?config.getFramework():null);

        WXEnvironment.sSDKInitExecuteTime = System.currentTimeMillis() - start;
        WXLogUtils.renderPerformanceLog("SDKInitExecuteTime", WXEnvironment.sSDKInitExecuteTime);
      }
    });
    register();
  }

  @Deprecated
  public static void init(Application application, String framework, IWXUserTrackAdapter utAdapter, IWXImgLoaderAdapter imgLoaderAdapter, IWXHttpAdapter httpAdapter) {
    initialize(application,
            new InitConfig.Builder()
                    .setUtAdapter(utAdapter)
                    .setHttpAdapter(httpAdapter)
                    .setImgAdapter(imgLoaderAdapter)
                    .build()
    );
  }

  public static void setJSExcetptionAdapter(IWXJSExceptionAdapter excetptionAdapter){
    WXSDKManager.getInstance().setIWXJSExceptionAdapter(excetptionAdapter);
  }

  private static void register() {
    BatchOperationHelper batchHelper = new BatchOperationHelper(WXBridgeManager.getInstance());
    try {
      registerComponent(
              new SimpleComponentHolder(
                      WXText.class,
                      new WXText.Creator()
              ),
              false,
              WXBasicComponentType.TEXT
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXDiv.class,
                      new WXDiv.Ceator()
              ),
              false,
              WXBasicComponentType.CONTAINER,
              WXBasicComponentType.DIV,
              WXBasicComponentType.HEADER,
              WXBasicComponentType.FOOTER
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXImage.class,
                      new WXImage.Creator()
              ),
              false,
              WXBasicComponentType.IMAGE,
              WXBasicComponentType.IMG
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXScroller.class,
                      new WXScroller.Creator()
              ),
              false,
              WXBasicComponentType.SCROLLER
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXSlider.class,
                      new WXSlider.Creator()
              ),
              true,
              WXBasicComponentType.SLIDER,
              WXBasicComponentType.CYCLE_SLIDER
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXSliderNeighbor.class,
                      new WXSliderNeighbor.Creator()
              ),
              true,
              WXBasicComponentType.SLIDER_NEIGHBOR
      );
      registerComponent(
              new SimpleComponentHolder(
                      WXCell.class,
                      new WXCell.Creator()
              ),
              true,
              WXBasicComponentType.CELL);
      registerComponent(
              new SimpleComponentHolder(
                      WXListComponent.class,
                      new WXListComponent.Creator()
              ),
              true,
              WXBasicComponentType.LIST,
              WXBasicComponentType.VLIST,
              WXBasicComponentType.RECYCLER,
              WXBasicComponentType.WATERFALL);

      String simpleList = "simplelist";
      registerComponent(SimpleListComponent.class,false,simpleList);
      registerComponent(WXRecyclerTemplateList.class, false,WXBasicComponentType.RECYCLE_LIST);
      registerComponent(HorizontalListComponent.class,false,WXBasicComponentType.HLIST);
      registerComponent(WXBasicComponentType.CELL_SLOT, WXCell.class, true);
      registerComponent(WXBasicComponentType.INDICATOR, WXIndicator.class, true);
      registerComponent(WXBasicComponentType.VIDEO, WXVideo.class, false);
      registerComponent(WXBasicComponentType.INPUT, WXInput.class, false);
      registerComponent(WXBasicComponentType.TEXTAREA, Textarea.class,false);
      registerComponent(WXBasicComponentType.SWITCH, WXSwitch.class, false);
      registerComponent(WXBasicComponentType.A, WXA.class, false);
      registerComponent(WXBasicComponentType.EMBED, WXEmbed.class, true);
      registerComponent(WXBasicComponentType.WEB, WXWeb.class);
      registerComponent(WXBasicComponentType.REFRESH, WXRefresh.class);
      registerComponent(WXBasicComponentType.LOADING, WXLoading.class);
      registerComponent(WXBasicComponentType.LOADING_INDICATOR, WXLoadingIndicator.class);
      registerComponent(WXBasicComponentType.HEADER, WXHeader.class);

      registerModule("modal", WXModalUIModule.class);
      registerModule("instanceWrap", WXInstanceWrap.class);
      registerModule("animation", WXAnimationModule.class);
      registerModule("webview", WXWebViewModule.class);
      registerModule("navigator", WXNavigatorModule.class);
      registerModule("stream", WXStreamModule.class);
      registerModule("timer", WXTimerModule.class);
      registerModule("storage", WXStorageModule.class);
      registerModule("clipboard", WXClipboardModule.class);
      registerModule("globalEvent",WXGlobalEventModule.class);
      registerModule("picker", WXPickersModule.class);
      registerModule("meta", WXMetaModule.class);
      registerModule("webSocket", WebSocketModule.class);
      registerModule("locale", WXLocaleModule.class);
    } catch (WXException e) {
      WXLogUtils.e("[WXSDKEngine] register:", e);
    }
    AutoScanConfigRegister.doScanConfig();
    batchHelper.flush();
  }

  /**
   *
   * Register component. The registration is singleton in {@link WXSDKEngine} level
   * @param type name of component. Same as type field in the JS.
   * @param clazz the class of the {@link WXComponent} to be registered.
   * @param appendTree true for appendTree flag
   * @return true for registration success, false for otherwise.
   * @throws WXException Throws exception if type conflicts.
   */
  public static boolean registerComponent(String type, Class<? extends WXComponent> clazz, boolean appendTree) throws WXException {
    return registerComponent(clazz, appendTree,type);
  }

  public static boolean registerComponent(String type, IExternalComponentGetter componentGetter, boolean appendTree) throws WXException {
    return registerComponent(new ExternalLoaderComponentHolder(type,componentGetter), appendTree,type);
  }

  /**
   *
   * Register component. The registration is singleton in {@link WXSDKEngine} level
   * @param clazz the class of the {@link WXComponent} to be registered.
   * @param appendTree true for appendTree flag
   * @return true for registration success, false for otherwise.
   * @param names names(alias) of component. Same as type field in the JS.
   * @throws WXException Throws exception if type conflicts.
   */
  public static boolean registerComponent(Class<? extends WXComponent> clazz, boolean appendTree,String ... names) throws WXException {
    if(clazz == null){
      return false;
    }
    SimpleComponentHolder holder = new SimpleComponentHolder(clazz);
    return registerComponent(holder,appendTree,names);
  }


  public static boolean registerComponent(IFComponentHolder holder, boolean appendTree, String ... names) throws WXException {
    boolean result =  true;
    try {
      for (String name : names) {
        Map<String, Object> componentInfo = new HashMap<>();
        if (appendTree) {
          componentInfo.put("append", "tree");
        }
        result = result && WXComponentRegistry.registerComponent(name, holder, componentInfo);
      }
      return result;
    } catch (Throwable e) {
      e.printStackTrace();
      return result;
    }
  }

  /**
   * Register module. This is a wrapper method for
   * {@link #registerModule(String, Class, boolean)}. The module register here only need to
   * be singleton in {@link WXSDKInstance} level.
   * @param moduleName  module name
   * @param moduleClass module to be registered.
   * @return true for registration success, false for otherwise.
   * {@link WXModuleManager#registerModule(String, ModuleFactory, boolean)}
   */
  public static <T extends WXModule> boolean registerModule(String moduleName, Class<T> moduleClass,boolean global) throws WXException {
    return moduleClass != null && registerModule(moduleName, new TypeModuleFactory<>(moduleClass), global);
  }

  /**
   * Register module. This is a wrapper method for
   * {@link #registerModule(String, Class, boolean)}. The module register here only need to
   * be singleton in {@link WXSDKInstance} level.
   * @param moduleName  module name
   * @param factory module factory to be registered. You can override {@link DestroyableModuleFactory#buildInstance()} to customize module creation.
   * @return true for registration success, false for otherwise.
   * {@link WXModuleManager#registerModule(String, ModuleFactory, boolean)}
   */
  public static <T extends WXModule> boolean registerModuleWithFactory(String moduleName, DestroyableModuleFactory factory, boolean global) throws WXException {
    return registerModule(moduleName, factory,global);
  }


  public static <T extends WXModule> boolean registerModuleWithFactory(String moduleName, IExternalModuleGetter factory, boolean global) throws WXException {
    return registerModule(moduleName, factory.getExternalModuleClass(moduleName,WXEnvironment.getApplication()),global);
  }

  public static <T extends WXModule> boolean registerModule(String moduleName, ModuleFactory factory, boolean global) throws WXException {
    return WXModuleManager.registerModule(moduleName, factory,global);
  }

  public static boolean registerModule(String moduleName, Class<? extends WXModule> moduleClass) throws WXException {
    return registerModule(moduleName, moduleClass,false);
  }

  public static boolean registerService(String name, String serviceScript, Map<String, Object> options) {
    return WXServiceManager.registerService(name, serviceScript, options);
  }

  public static boolean unRegisterService(String name) {
    return WXServiceManager.unRegisterService(name);
  }

  /**
   * module implement {@link Destroyable}
   */
  public static abstract class DestroyableModule extends WXModule implements Destroyable {}

  public static  abstract  class DestroyableModuleFactory<T extends DestroyableModule> extends TypeModuleFactory<T> {
    public DestroyableModuleFactory(Class<T> clz) {
      super(clz);
    }
  }

  public static void callback(String instanceId, String funcId, Map<String, Object> data) {
    WXSDKManager.getInstance().callback(instanceId, funcId, data);
  }

  /**
   * Model switch, only applicable for developer model
   * @param debug
   */
  public static void restartBridge(boolean debug) {
    WXEnvironment.sDebugMode = debug;
    WXSDKManager.getInstance().restartBridge();
  }

  public static boolean registerComponent(String type, Class<? extends WXComponent> clazz) throws WXException {
    return WXComponentRegistry.registerComponent(type, new SimpleComponentHolder(clazz),new HashMap<String, Object>());
  }

  public static boolean registerComponent(Map<String, Object> componentInfo, Class<? extends WXComponent> clazz) throws WXException {
    if(componentInfo == null){
      return false;
    }
    String type = (String)componentInfo.get("type");
    if(TextUtils.isEmpty(type)){
      return false;
    }
    return WXComponentRegistry.registerComponent(type,new SimpleComponentHolder(clazz), componentInfo);
  }

  public static void addCustomOptions(String key, String value) {
    WXEnvironment.addCustomOptions(key, value);
  }

  public static IWXUserTrackAdapter getIWXUserTrackAdapter() {
    return WXSDKManager.getInstance().getIWXUserTrackAdapter();
  }

  public static IWXImgLoaderAdapter getIWXImgLoaderAdapter() {
    return WXSDKManager.getInstance().getIWXImgLoaderAdapter();
  }

  public static IDrawableLoader getDrawableLoader() {
    return WXSDKManager.getInstance().getDrawableLoader();
  }

  public static IWXHttpAdapter getIWXHttpAdapter() {
    return WXSDKManager.getInstance().getIWXHttpAdapter();
  }

  public static IWXStorageAdapter getIWXStorageAdapter() {
    return WXSDKManager.getInstance().getIWXStorageAdapter();
  }


  public static IWXJsFileLoaderAdapter getIWXJsFileLoaderAdapter() {
    return WXSDKManager.getInstance().getIWXJsFileLoaderAdapter();
  }

  public static IActivityNavBarSetter getActivityNavBarSetter() {
    return WXSDKManager.getInstance().getActivityNavBarSetter();
  }

  public static INavigator getNavigator() {
    return WXSDKManager.getInstance().getNavigator();
  }

  public static  void setNavigator(INavigator navigator) {
    WXSDKManager.getInstance().setNavigator(navigator);
  }

  public static void setActivityNavBarSetter(IActivityNavBarSetter activityNavBarSetter) {
    WXSDKManager.getInstance().setActivityNavBarSetter(activityNavBarSetter);
  }

  public static void reload(final Context context,String framework, boolean remoteDebug) {
    WXEnvironment.sRemoteDebugMode = remoteDebug;
    WXBridgeManager.getInstance().restart();
    WXBridgeManager.getInstance().initScriptsFramework(framework);

    WXModuleManager.reload();
    WXComponentRegistry.reload();
    WXSDKManager.getInstance().postOnUiThread(new Runnable() {
      @Override
      public void run() {
        LocalBroadcastManager.getInstance(context).sendBroadcast(new Intent(JS_FRAMEWORK_RELOAD));
      }
    }, 0);
  }
  public static void reload(final Context context, boolean remoteDebug) {
    reload(context,null,remoteDebug);
  }

  public static void reload() {
    reload(WXEnvironment.getApplication(), WXEnvironment.sRemoteDebugMode);
  }

  public static void registerCoreEnv(String key, String value) {
    WXBridgeManager.getInstance().registerCoreEnv(key, value);
  }
}
