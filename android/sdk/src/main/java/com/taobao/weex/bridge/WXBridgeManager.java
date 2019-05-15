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
package com.taobao.weex.bridge;

import static com.taobao.weex.bridge.WXModuleManager.createDomModule;

import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.UiThread;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.util.Log;
import android.util.Pair;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.Script;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXConfigAdapter;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.adapter.IWXJsFileLoaderAdapter;
import com.taobao.weex.adapter.IWXJscProcessManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.common.IWXDebugConfig;
import com.taobao.weex.common.WXConfig;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXException;
import com.taobao.weex.common.WXJSBridgeMsgType;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.ui.WXComponentRegistry;
import com.taobao.weex.ui.action.ActionReloadPage;
import com.taobao.weex.ui.action.BasicGraphicAction;
import com.taobao.weex.ui.action.GraphicActionAddElement;
import com.taobao.weex.ui.action.GraphicActionAddEvent;
import com.taobao.weex.ui.action.GraphicActionAppendTreeCreateFinish;
import com.taobao.weex.ui.action.GraphicActionCreateBody;
import com.taobao.weex.ui.action.GraphicActionCreateFinish;
import com.taobao.weex.ui.action.GraphicActionLayout;
import com.taobao.weex.ui.action.GraphicActionMoveElement;
import com.taobao.weex.ui.action.GraphicActionRefreshFinish;
import com.taobao.weex.ui.action.GraphicActionRemoveElement;
import com.taobao.weex.ui.action.GraphicActionRemoveEvent;
import com.taobao.weex.ui.action.GraphicActionRenderSuccess;
import com.taobao.weex.ui.action.GraphicActionUpdateAttr;
import com.taobao.weex.ui.action.GraphicActionUpdateStyle;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.module.WXDomModule;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import com.taobao.weex.utils.WXWsonJSONSwitch;
import com.taobao.weex.utils.batch.BactchExecutor;
import com.taobao.weex.utils.batch.Interceptor;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.io.Serializable;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

/**
 * Manager class for communication between JavaScript and Android.
 * <ol>
 * <li>
 * Handle Android to JavaScript call, can be one of the following
 * <ul>
 * <li>{@link #createInstance(String, String, Map, String)}</li>
 * <li>{@link #destroyInstance(String)}</li>
 * <li>{@link #refreshInstance(String, WXRefreshData)}</li>
 * <li>{@link #registerModules(Map)}</li>
 * <li>{@link #registerComponents(List)}</li>
 * <li>{@link #invokeCallJSBatch(Message)}</li>
 * </ul>
 * </li>
 * <li>
 * Handle JavaScript to Android call
 * </li>
 * <li>
 * Handle next tick of message.
 * </li>
 * </ol>
 */
public class WXBridgeManager implements Callback, BactchExecutor {

  private static Class clazz_debugProxy = null;
  public static final String METHOD_CREATE_INSTANCE = "createInstance";
  public static final String METHOD_CREATE_PAGE_WITH_CONTENT = "CreatePageWithContent";
  public static final String METHOD_UPDATE_COMPONENT_WITH_DATA = "UpdateComponentData";
  public static final String METHOD_DESTROY_INSTANCE = "destroyInstance";
  public static final String METHOD_CALL_JS = "callJS";
  public static final String METHOD_SET_TIMEOUT = "setTimeoutCallback";
  public static final String METHOD_REGISTER_MODULES = "registerModules";
  public static final String METHOD_REGISTER_COMPONENTS = "registerComponents";
  public static final String METHOD_FIRE_EVENT = "fireEvent";
  public static final String METHD_FIRE_EVENT_SYNC = "fireEventSync";
  public static final String METHD_COMPONENT_HOOK_SYNC = "componentHook";
  public static final String METHOD_CALLBACK = "callback";
  public static final String METHOD_REFRESH_INSTANCE = "refreshInstance";
  public static final String METHOD_FIRE_EVENT_ON_DATA_RENDER_NODE = "fireEventOnDataRenderNode";
  public static final String METHOD_NOTIFY_TRIM_MEMORY = "notifyTrimMemory";
  public static final String METHOD_NOTIFY_SERIALIZE_CODE_CACHE =
          "notifySerializeCodeCache";
  public static final String METHOD_CREATE_INSTANCE_CONTEXT = "createInstanceContext";

  public static final String KEY_METHOD = "method";
  public static final String KEY_ARGS = "args";

  // args
  public static final String COMPONENT = "component";
  public static final String REF = "ref";
  public static final String MODULE = "module";
  public static final String METHOD = "method";
  public static final String KEY_PARAMS = "params";
  public static final String ARGS = "args";
  public static final String OPTIONS = "options";
  public static final String INITLOGFILE = "/jsserver_start.log";
  private static final String NON_CALLBACK = "-1";
  private static final String UNDEFINED = "undefined";
  private static final String BUNDLE_TYPE = "bundleType";
  private static final String RENDER_STRATEGY = "renderStrategy";
  private static final int INIT_FRAMEWORK_OK = 1;
  private static final int CRASHREINIT = 50;
  static volatile WXBridgeManager mBridgeManager;
  private static long LOW_MEM_VALUE = 120;
  private volatile static int reInitCount = 1;
  private static String crashUrl = null;
  private static long lastCrashTime = 0;

  private static String mRaxApi = null;
//  private static String mRaxExtApi = null;

  // add for clound setting, default value is true
  // can use it to control weex sandbox
  // if true will open weex sandbox for multi context
  private volatile static boolean isSandBoxContext = true;

  // add for cloud setting, default value is false.
  // weexcore use single process or not
  private static boolean isUseSingleProcess = false;
  // add for cloud setting, default value is false.
  // jsEngine use multiThread or not
  private volatile static boolean isJsEngineMultiThreadEnable = false;

  public enum BundType {
    Vue,
    Rax,
    Others
  };

  private static final boolean BRIDGE_LOG_SWITCH = false;

  /**
   * Whether JS Framework(main.js) has been initialized.
   */
  private volatile static boolean mInit = false;

  private static String globalConfig = "none";
  private static String GLOBAL_CONFIG_KEY = "global_switch_config";

  private static Map<String, String> mWeexCoreEnvOptions = new HashMap<>();

  /**
   * package
   **/
  Handler mJSHandler;
  /**
   * next tick tasks, can set priority
   */
  private WXHashMap<String, ArrayList<WXHashMap<String, Object>>> mNextTickTasks = new WXHashMap<>();
  /**
   * JSThread
   */
  private WXThread mJSThread;
  private IWXBridge mWXBridge;
  private Object mWxDebugProxy;

  private boolean mMock = false;

  private List<Map<String, Object>> mRegisterComponentFailList = new ArrayList<>(8);
  private List<Map<String, Object>> mRegisterModuleFailList = new ArrayList<>(8);
  private List<String> mRegisterServiceFailList = new ArrayList<>(8);
  private HashSet<String> mDestroyedInstanceId = new HashSet<>();
  private StringBuilder mLodBuilder = new StringBuilder(50);
  private Interceptor mInterceptor;
  private WXParams mInitParams;

  private WXBridgeManager() {
    initWXBridge(WXEnvironment.sRemoteDebugMode);
    mJSThread = new WXThread("WeexJSBridgeThread", this);
    mJSHandler = mJSThread.getHandler();
  }

  public static WXBridgeManager getInstance() {
    if (mBridgeManager == null) {
      synchronized (WXBridgeManager.class) {
        if (mBridgeManager == null) {
          mBridgeManager = new WXBridgeManager();
        }
      }
    }
    return mBridgeManager;
  }

  public void setUseSingleProcess(final boolean flag) {
    if (flag != isUseSingleProcess) {
      isUseSingleProcess = flag;
//      //we should reinit framework if js framework has been initialized
//      if (isJSFrameworkInit()) {
//        if (isJSThread()) {
//          WXSDKEngine.reload();
//        } else {
//          post(new Runnable() {
//            @Override
//            public void run() {
//              WXSDKEngine.reload();
//            }
//          });
//        }
//      }
    }
  }
  public boolean jsEngineMultiThreadEnable() {
    return isJsEngineMultiThreadEnable;
  }

  public void checkJsEngineMultiThread() {
    boolean flag = false;
    IWXJscProcessManager wxJscProcessManager = WXSDKManager.getInstance().getWXJscProcessManager();
    if(wxJscProcessManager!=null) {
      flag = wxJscProcessManager.enableBackupThread();
    }

    if(flag == WXBridgeManager.isJsEngineMultiThreadEnable) {
        return;
    }

    WXBridgeManager.isJsEngineMultiThreadEnable = flag;
    //we should reinit framework if js framework has been initialized
    if (isJSFrameworkInit()) {
      if (isJSThread()) {
        WXSDKEngine.reload();
      } else {
        post(new Runnable() {
          @Override
          public void run() {
            WXSDKEngine.reload();
          }
        });
      }
    }
  }
  public void setSandBoxContext(final boolean flag) {
    if (flag != isSandBoxContext) {
      isSandBoxContext = flag;
      // use diff context reinit jsf
      if (isJSThread()) {

        setJSFrameworkInit(false);
        WXModuleManager.resetAllModuleState();
        String jsf = "";
        if (!isSandBoxContext) {
          jsf = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
        } else {
          jsf = WXFileUtils.loadAsset("weex-main-jsfm.js", WXEnvironment.getApplication());
        }
        initFramework(jsf);
        WXServiceManager.reload();
        WXModuleManager.reload();
        WXComponentRegistry.reload();
      } else {
        post(new Runnable() {
          @Override
          public void run() {
            setJSFrameworkInit(false);
            WXModuleManager.resetAllModuleState();
            String jsf = "";
            if (!isSandBoxContext) {
              jsf = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
            } else {
              jsf = WXFileUtils.loadAsset("weex-main-jsfm.js", WXEnvironment.getApplication());
            }
            initFramework(jsf);
            WXServiceManager.reload();
            WXModuleManager.reload();
            WXComponentRegistry.reload();
          }
        }, null);
      }
    }
  }

  // setJSFrameworkInit and isJSFrameworkInit may use on diff thread
  // use volatile
  private boolean isJSFrameworkInit() {
    return mInit;
  }

  private void setJSFrameworkInit(boolean init) {
    mInit = init;
    if (init == true) {
      onJsFrameWorkInitSuccees();
    }
  }

  private void initWXBridge(boolean remoteDebug) {
    if (remoteDebug && WXEnvironment.isApkDebugable()) {
      WXEnvironment.sDebugServerConnectable = true;
    }

    if (WXEnvironment.sDebugServerConnectable && (WXEnvironment.isApkDebugable() || WXEnvironment.sForceEnableDevTool)) {
      if (WXEnvironment.getApplication() != null) {
        try {
          if (clazz_debugProxy == null) {
            clazz_debugProxy = Class.forName("com.taobao.weex.devtools.debug.DebugServerProxy");
          }
          if (clazz_debugProxy != null) {
            Constructor constructor = clazz_debugProxy.getConstructor(Context.class, IWXDebugConfig.class);
            if (constructor != null) {
              mWxDebugProxy = constructor.newInstance(WXEnvironment.getApplication(),
                      new IWXDebugConfig() {
                        @Override
                        public WXBridgeManager getWXJSManager() {
                          return WXBridgeManager.this;
                        }

                        @Override
                        public WXDebugJsBridge getWXDebugJsBridge() {
                          return new WXDebugJsBridge();
                        }
                      });
              if (mWxDebugProxy != null) {
                Method method_start = clazz_debugProxy.getMethod("start");
                if (method_start != null) {
                  method_start.invoke(mWxDebugProxy);
                }
              }
            }
          }
        } catch (Throwable e) {
          //Ignore, It will throw Exception on Release environment
        }
        WXServiceManager.execAllCacheJsService();
      } else {
        WXLogUtils.e("WXBridgeManager", "WXEnvironment.sApplication is null, skip init Inspector");
      }
    }
    if (remoteDebug && mWxDebugProxy != null) {
      try {
        if (clazz_debugProxy == null ) {
          clazz_debugProxy = Class.forName("com.taobao.weex.devtools.debug.DebugServerProxy");
        }
        if (clazz_debugProxy != null) {
          Method method_getWXBridge = clazz_debugProxy.getMethod("getWXBridge");
          if (method_getWXBridge != null) {
            mWXBridge = (IWXBridge) method_getWXBridge.invoke(mWxDebugProxy);
          }
        }
      } catch (Throwable e) {
        //Ignore, It will throw Exception on Release environment
      }
    } else {
      mWXBridge = new WXBridge();
    }
  }

  public void stopRemoteDebug() {
    if (mWxDebugProxy != null) {
      try {
        if (clazz_debugProxy == null) {
          clazz_debugProxy = Class.forName("com.taobao.weex.devtools.debug.DebugServerProxy");
        }
        if (clazz_debugProxy != null) {
          Method method_stop = clazz_debugProxy.getMethod("stop", boolean.class);
          if (method_stop != null) {
            method_stop.invoke(mWxDebugProxy, true);
          }
        }
      } catch (Throwable e) {
        //Ignore, It will throw Exception on Release environment
      }
    }
  }

  public Object callModuleMethod(String instanceId, String moduleStr, String methodStr, JSONArray args) {
    return callModuleMethod(instanceId, moduleStr, methodStr, args, null);
  }

  public Object callModuleMethod(String instanceId, String moduleStr, String methodStr, JSONArray args, JSONObject options) {
    WXSDKInstance wxsdkInstance = WXSDKManager.getInstance()
            .getSDKInstance(instanceId);
    if (wxsdkInstance == null) {
      return null;
    }
    if (wxsdkInstance.isNeedValidate()
            && WXSDKManager.getInstance().getValidateProcessor() != null) {
      WXValidateProcessor.WXModuleValidateResult validateResult = WXSDKManager
              .getInstance().getValidateProcessor()
              .onModuleValidate(wxsdkInstance, moduleStr, methodStr, args, options);
      if (validateResult == null) {
        return null;
      }
      if (validateResult.isSuccess) {
        return WXModuleManager.callModuleMethod(instanceId, moduleStr, methodStr,
                args);
      } else {
        JSONObject validateInfo = validateResult.validateInfo;
        if (validateInfo != null) {
          WXLogUtils.e("[WXBridgeManager] module validate fail. >>> " + validateInfo.toJSONString());
        }
        return validateInfo;
      }
    }
    try {
      return WXModuleManager.callModuleMethod(instanceId, moduleStr, methodStr, args);
    }catch(NumberFormatException e){
      ArrayMap<String, String> ext = new ArrayMap<>();
      ext.put("moduleName", moduleStr);
      ext.put("methodName", methodStr);
      ext.put("args", args.toJSONString());
      WXLogUtils.e("[WXBridgeManager] callNative : numberFormatException when parsing string to numbers in args", ext.toString());
      return null;
    }
  }

  /**
   * Model switch. For now, debug model and release model are supported
   */
  public void restart() {
    setJSFrameworkInit(false);
    WXModuleManager.resetAllModuleState();
    initWXBridge(WXEnvironment.sRemoteDebugMode);
    mWXBridge.resetWXBridge(WXEnvironment.sRemoteDebugMode);
  }

  /**
   * Set current Instance
   *
   * @param instanceId {@link WXSDKInstance#mInstanceId}
   */
  public synchronized void setStackTopInstance(final String instanceId) {
    post(new Runnable() {

      @Override
      public void run() {
        mNextTickTasks.setStackTopInstance(instanceId);
      }
    }, instanceId);
  }

  @Override
  public void post(Runnable r) {
    if (mInterceptor != null && mInterceptor.take(r)) {
      //task is token by the interceptor
      return;
    }
    if (mJSHandler == null) {
      return;
    }

    mJSHandler.post(WXThread.secure(r));
  }

  @Override
  public void setInterceptor(Interceptor interceptor) {
    mInterceptor = interceptor;
  }

  public void post(Runnable r, Object token) {
    if (mJSHandler == null) {
      return;
    }

    Message m = Message.obtain(mJSHandler, WXThread.secure(r));
    m.obj = token;
    m.sendToTarget();
  }

  public void postDelay(Runnable r,long delayMillis){
    if (mJSHandler == null) {
      return;
    }
    mJSHandler.postDelayed(WXThread.secure(r),delayMillis);
  }

  void setTimeout(String callbackId, String time) {
    Message message = Message.obtain();
    message.what = WXJSBridgeMsgType.SET_TIMEOUT;
    TimerInfo timerInfo = new TimerInfo();
    timerInfo.callbackId = callbackId;
    timerInfo.time = (long) Float.parseFloat(time);
    message.obj = timerInfo;

    mJSHandler.sendMessageDelayed(message, timerInfo.time);
  }

  public void sendMessageDelayed(Message message, long delayMillis) {
    if (message == null || mJSHandler == null || mJSThread == null
            || !mJSThread.isWXThreadAlive() || mJSThread.getLooper() == null) {
      return;
    }
    mJSHandler.sendMessageDelayed(message, delayMillis);
  }

  public void removeMessage(int what, Object obj) {
    if (mJSHandler == null || mJSThread == null
            || !mJSThread.isWXThreadAlive() || mJSThread.getLooper() == null) {
      return;
    }
    mJSHandler.removeMessages(what, obj);
  }

  public Object callNativeModule(String instanceId, String module, String method, JSONArray arguments, Object options) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(module) || TextUtils.isEmpty(method)) {
      WXLogUtils.d("[WXBridgeManager] call callNativeModule arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callNativeModule",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
              .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    try {
      if (WXDomModule.WXDOM.equals(module)) {
        WXDomModule dom = WXModuleManager.getDomModule(instanceId);
        return dom.callDomMethod(method, arguments);
      } else {
        return callModuleMethod(instanceId, module,
              method, arguments);
      }
    } catch (Exception e) {
      String err = "[WXBridgeManager] callNative exception: " + WXLogUtils.getStackTrace(e);
      WXLogUtils.e(err);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callNativeModule",
              err, null);
    }

    return null;
  }

  public Object callNativeModule(String instanceId, String module, String method, JSONArray arguments, JSONObject options) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(module) || TextUtils.isEmpty(method)) {
      WXLogUtils.d("[WXBridgeManager] call callNativeModule arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callNativeModule",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
              .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    try {
      if (WXDomModule.WXDOM.equals(module)) {
        WXDomModule dom = WXModuleManager.getDomModule(instanceId);
        if(dom != null){
          return dom.callDomMethod(method, arguments);
        } else {
          createDomModule(WXSDKManager.getInstance().getSDKInstance(instanceId));
        }
      } else {
        return callModuleMethod(instanceId, module,
              method, arguments, options);
      }
    } catch (Exception e) {
      String err = "[WXBridgeManager] callNativeModule exception: " + WXLogUtils.getStackTrace(e);
      WXLogUtils.e(err);
    }

    return null;
  }

  public Object callNativeComponent(String instanceId, String componentRef, String method, JSONArray arguments, Object options) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(componentRef) || TextUtils.isEmpty(method)) {
      WXLogUtils.d("[WXBridgeManager] call callNativeComponent arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callNativeComponent",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callNativeComponent >>>> instanceId:").append(instanceId)
              .append(", componentRef:").append(componentRef).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    try {
      WXDomModule dom = WXModuleManager.getDomModule(instanceId);
      if (null != dom){
        dom.invokeMethod(componentRef, method, arguments);
      }else {
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
        if(null == instance || !instance.isDestroy()){
          WXLogUtils.e("WXBridgeManager","callNativeComponent exception :null == dom ,method:"+method);
        }
      }

    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callNativeComponent exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callNativeComponent",
              WXLogUtils.getStackTrace(e), null);
    }
    return null;
  }

  /**
   * Dispatch the native task to be executed.
   *
   * @param instanceId {@link WXSDKInstance#mInstanceId}
   * @param tasks      tasks to be executed
   * @param callback   next tick id
   */
  public int callNative(String instanceId, JSONArray tasks, String callback) {

    if (TextUtils.isEmpty(instanceId) || tasks == null) {
      WXLogUtils.d("[WXBridgeManager] call callNative arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callNative",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callNative >>>> instanceId:").append(instanceId)
              .append(", tasks:").append(tasks).append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }


    long parseNanos = System.nanoTime();
    JSONArray array = tasks;
    parseNanos = System.nanoTime() - parseNanos;

    if (null != array && array.size() > 0) {
      int size = array.size();
      try {
        JSONObject task;
        for (int i = 0; i < size; ++i) {
          task = (JSONObject) array.get(i);
          if (task != null && WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
            Object module = task.get(MODULE);
            if (module != null) {
              if (WXDomModule.WXDOM.equals(module)) {
                WXDomModule dom = WXModuleManager.getDomModule(instanceId);
                dom.callDomMethod(task, parseNanos);
              } else {
                JSONObject optionObj = task.getJSONObject(OPTIONS);
                callModuleMethod(instanceId, (String) module,
                        (String) task.get(METHOD), (JSONArray) task.get(ARGS), optionObj);
              }
            } else if (task.get(COMPONENT) != null) {
              WXDomModule dom = WXModuleManager.getDomModule(instanceId);
              dom.invokeMethod((String) task.get(REF), (String) task.get(METHOD), (JSONArray) task.get(ARGS));
            } else {
              throw new IllegalArgumentException("unknown callNative");
            }
          }
        }
      } catch (Exception e) {
        WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
        WXExceptionUtils.commitCriticalExceptionRT(instanceId,
                WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callNative",
                WXLogUtils.getStackTrace(e) , null);
      }
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  // callUpdateFinish
  public int callUpdateFinish(String instanceId, String callback) {

    if (TextUtils.isEmpty(instanceId)) {
      WXLogUtils.d("[WXBridgeManager] call callUpdateFinish arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callUpdateFinish",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callUpdateFinish >>>> instanceId:").append(instanceId)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callUpdateFinish",
              WXLogUtils.getStackTrace(e), null);
    }

    if (callback == null || callback.isEmpty() || UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  // callRefreshFinish
  public int callRefreshFinish(String instanceId, String callback) {

    if (TextUtils.isEmpty(instanceId)) {
      WXLogUtils.d("[WXBridgeManager] call callRefreshFinish arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callRefreshFinish",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callRefreshFinish >>>> instanceId:").append(instanceId)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
        if (instance != null) {
          GraphicActionRefreshFinish action = new GraphicActionRefreshFinish(instance);
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
        }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRefreshFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callRefreshFinish",
              WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;

  }

  public int callReportCrashReloadPage(String instanceId, String crashFile) {
    boolean isCrashFileEmpty = TextUtils.isEmpty(crashFile);
    try {
      String url = null;
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        url = instance.getBundleUrl();
      }
      if(!isCrashFileEmpty) {
        try {
            if (WXEnvironment.getApplication() != null) {
                crashFile = mInitParams.getCrashFilePath() + crashFile;
                Log.d("jsengine", "callReportCrashReloadPage crashFile:" + crashFile);
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }
        callReportCrash(crashFile, instanceId, url);
      } else {
         commitJscCrashAlarmMonitor(IWXUserTrackAdapter.JS_BRIDGE, WXErrorCode.WX_ERR_RELOAD_PAGE, "reboot jsc Engine", instanceId, url);
      }

      if (reInitCount > CRASHREINIT) {
        return IWXBridge.INSTANCE_RENDERING_ERROR;
      }
      reInitCount++;
      // reinit frame work
      setJSFrameworkInit(false);
      WXModuleManager.resetAllModuleState();
      initScriptsFramework("");

      if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
        return IWXBridge.DESTROY_INSTANCE;
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callReportCrashReloadPage exception: ", e);
    }
    try {

      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        boolean reloadThisInstance = shouldReloadCurrentInstance(
                WXSDKManager.getInstance().getSDKInstance(instanceId).getBundleUrl());
        new ActionReloadPage(instanceId, reloadThisInstance).executeAction();
      }

    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callReloadPage exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callReportCrashReloadPage",
              WXLogUtils.getStackTrace(e), null);
    }
    return IWXBridge.INSTANCE_RENDERING_ERROR;
  }

  public boolean shouldReloadCurrentInstance(String aUrl) {
    long time = System.currentTimeMillis();
    String bizUrl = aUrl;
    IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
    if (adapter != null) {
        boolean check_biz_url = Boolean.parseBoolean(adapter
                .getConfig("android_weex_ext_config",
                        "check_biz_url",
                        "true"));
        WXLogUtils.e("check_biz_url : " + check_biz_url);
        if(check_biz_url && !TextUtils.isEmpty(aUrl)) {
            Uri uri = Uri.parse(aUrl);
            if(uri != null) {
                bizUrl = uri.buildUpon().clearQuery().build().toString();
            }
        }
    }

    if (crashUrl == null ||
            (crashUrl != null && !crashUrl.equals(bizUrl)) ||
            ((time - lastCrashTime) > 15000)) {
      crashUrl = bizUrl;
      lastCrashTime = time;
      return true;
    }
    lastCrashTime = time;
    return false;
  }

  public void callReportCrash(String crashFile, final String instanceId, final String url) {
    // statistic weex core process crash
    Date date = new Date();
    DateFormat format = new SimpleDateFormat("yyyyMMddHHmmss");
    String time = format.format(date);
    final String origin_filename = crashFile + "." + time;
    File oldfile = new File(crashFile);
    File newfile = new File(origin_filename);
    if (oldfile.exists()) {
      oldfile.renameTo(newfile);
    }
    Thread t = new Thread(new Runnable() {
      public void run() {
        try {
          File file = new File(origin_filename);
          if (file.exists()) {
            if (file.length() > 0) {
              StringBuilder result = new StringBuilder();
              try {
                BufferedReader br = new BufferedReader(new FileReader(origin_filename));
                String s = null;
                // boolean foundStart = false;
                while ((s = br.readLine()) != null) {
                  if ("".equals(s)) {
                    continue;
                  }
                  // if (("r0:").equals(s)) {
                  //  break;
                  // }
                  result.append(s + "\n");
                }
                commitJscCrashAlarmMonitor(IWXUserTrackAdapter.JS_BRIDGE, WXErrorCode.WX_ERR_JSC_CRASH, result.toString(), instanceId, url);
                br.close();
              } catch (Exception e) {
                e.printStackTrace();
              }
            } else {
              WXLogUtils.e("[WXBridgeManager] callReportCrash crash file is empty");
            }
            if(!WXEnvironment.isApkDebugable())
              file.delete();
          }
        } catch (Throwable throwable) {
          WXLogUtils.e("[WXBridgeManager] callReportCrash exception: ", throwable);
        }
      }
    });
    t.start();
  }

  private void getNextTick(final String instanceId, final String callback) {
    addJSTask(METHOD_CALLBACK, instanceId, callback, "{}");
    sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
  }

  private void getNextTick(final String instanceId) {
    addJSTask(METHOD_CALLBACK, instanceId, "", "{}");
    sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
  }

  public String syncExecJsOnInstanceWithResult(final String instanceId, final String js, final int type) {
    final CountDownLatch waitLatch = new CountDownLatch(1);
    EventResult callback = new EventResult(){
      @Override
      public void onCallback(Object result) {
        super.onCallback(result);
        waitLatch.countDown();
      }
    };
    try{
      execJSOnInstance(callback, instanceId, js, type);
      waitLatch.await(100, TimeUnit.MILLISECONDS);
      if (callback != null && callback.getResult() != null) {
        return  callback.getResult().toString();
      }
      return "";
    }catch (Throwable e){
      WXLogUtils.e("syncCallExecJsOnInstance", e);
      return  "";
    }
  }

  public void loadJsBundleInPreInitMode(final String instanceId, final String js){
    post(new Runnable() {
      @Override
      public void run() {
        invokeExecJSOnInstance(instanceId, js, -1);
        WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
        if (null != instance && instance.isPreInitMode()){
          instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_END);
        }
      }
    });
  }

  /**
   * ref, type, data, domChanges
   * */
  public EventResult syncCallJSEventWithResult(final String method, final String instanceId, final List<Object> params, final Object... args) {
    final CountDownLatch waitLatch = new CountDownLatch(1);
    EventResult callback = new EventResult(){
      @Override
      public void onCallback(Object result) {
        super.onCallback(result);
        waitLatch.countDown();
      }
    };
    try{
      asyncCallJSEventWithResult(callback, method, instanceId, params, args);
      waitLatch.await(100, TimeUnit.MILLISECONDS);
      return  callback;
    }catch (Exception e){
      WXLogUtils.e("syncCallJSEventWithResult", e);
      return  callback;
    }
  }

  public void asyncCallJSEventVoidResult(final String method, final String instanceId, final List<Object> params, final Object... args) {
    post(new Runnable() {
      @Override
      public void run() {
        try{
          if (args == null || args.length == 0) {
            return;
          }

          ArrayList<Object> argsList = new ArrayList<>();
          for (Object arg : args) {
            argsList.add(arg);
          }
          if (params != null) {
            ArrayMap map = new ArrayMap(4);
            map.put(KEY_PARAMS, params);
            argsList.add(map);
          }

          WXHashMap<String, Object> task = new WXHashMap<>();
          task.put(KEY_METHOD, method);
          task.put(KEY_ARGS, argsList);
          Object[] tasks = {task};
          WXJSObject[] jsArgs = {
                  new WXJSObject(WXJSObject.String, instanceId),
                  WXWsonJSONSwitch.toWsonOrJsonWXJSObject(tasks)};
          invokeExecJS(String.valueOf(instanceId), null, METHOD_CALL_JS, jsArgs, true);
          jsArgs[0] = null;
          jsArgs = null;
        }catch (Exception e){
          WXLogUtils.e("asyncCallJSEventVoidResult" , e);
        }
      }
    });
  }

  /**
   * aync call js event and return result in eventResult callback
   * */
  private void asyncCallJSEventWithResult(final EventResult eventCallback, final String method, final String instanceId, final List<Object> params, final Object... args) {
    post(new Runnable() {
      @Override
      public void run() {
        Object result = null;
        try{
          if (args == null || args.length == 0) {
            return;
          }

          ArrayList<Object> argsList = new ArrayList<>();
          for (Object arg : args) {
            argsList.add(arg);
          }
          if (params != null) {
            ArrayMap map = new ArrayMap(4);
            map.put(KEY_PARAMS, params);
            argsList.add(map);
          }

          WXHashMap<String, Object> task = new WXHashMap<>();
          task.put(KEY_METHOD, method);
          task.put(KEY_ARGS, argsList);
          Object[] tasks = {task};
          WXJSObject[] jsArgs = {
                  new WXJSObject(WXJSObject.String, instanceId),
                  WXWsonJSONSwitch.toWsonOrJsonWXJSObject(tasks)};
          ResultCallback<byte[]> resultCallback = null;
          if (eventCallback != null) {
            resultCallback = new ResultCallback<byte[]>() {
              @Override
              public void onReceiveResult(byte[] result) {
                JSONArray arrayResult = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(result);
                if(arrayResult != null && arrayResult.size() > 0){
                  eventCallback.onCallback(arrayResult.get(0));
                }
              }
            };
          }
          invokeExecJSWithCallback(String.valueOf(instanceId), null, METHOD_CALL_JS,
                  jsArgs, resultCallback, true);
          jsArgs[0] = null;
        }catch (Exception e){
          WXLogUtils.e("asyncCallJSEventWithResult" , e);
        }
      }
    });
  }

  private void addJSEventTask(final String method, final String instanceId, final List<Object> params, final Object... args) {
    post(new Runnable() {
      @Override
      public void run() {
        if (args == null || args.length == 0) {
          return;
        }

        ArrayList<Object> argsList = new ArrayList<>();
        for (Object arg : args) {
          argsList.add(arg);
        }
        if (params != null) {
          ArrayMap map = new ArrayMap(4);
          map.put(KEY_PARAMS, params);
          argsList.add(map);
        }

        WXHashMap<String, Object> task = new WXHashMap<>();
        task.put(KEY_METHOD, method);
        task.put(KEY_ARGS, argsList);


        if (mNextTickTasks.get(instanceId) == null) {
          ArrayList<WXHashMap<String, Object>> list = new ArrayList<>();
          list.add(task);
          mNextTickTasks.put(instanceId, list);
        } else {
          mNextTickTasks.get(instanceId).add(task);
        }
      }
    });
  }

  private void addJSTask(final String method, final String instanceId, final Object... args) {
    addJSEventTask(method, instanceId, null, args);
  }

  private void sendMessage(String instanceId, int what) {
    Message msg = Message.obtain(mJSHandler);
    msg.obj = instanceId;
    msg.what = what;
    msg.sendToTarget();
  }

  /**
   * Initialize JavaScript framework
   *
   * @param framework String representation of the framework to be init.
   */
  public synchronized void initScriptsFramework(String framework) {
    Message msg = mJSHandler.obtainMessage();
    msg.obj = framework;
    msg.what = WXJSBridgeMsgType.INIT_FRAMEWORK;
    msg.setTarget(mJSHandler);
    msg.sendToTarget();
  }

  @Deprecated
  public void fireEvent(final String instanceId, final String ref,
                        final String type, final Map<String, Object> data) {
    this.fireEvent(instanceId, ref, type, data, null);
  }

  /**
   * Do not direct invoke this method in Components, use {@link WXSDKInstance#fireEvent(String, String, Map, Map)} instead.
   *
   * @param instanceId
   * @param ref
   * @param type
   * @param data
   * @param domChanges
   */
  @Deprecated
  public void fireEvent(final String instanceId, final String ref,
                        final String type, final Map<String, Object> data, final Map<String, Object> domChanges) {
    fireEventOnNode(instanceId, ref, type, data, domChanges);
  }

  /**
   * Notify the JavaScript about the event happened on Android
   */
  public void fireEventOnNode(final String instanceId, final String ref,
                              final String type, final Map<String, Object> data, final Map<String, Object> domChanges) {
    fireEventOnNode(instanceId, ref, type, data, domChanges, null, null);
  }

  /**
   * Notify the JavaScript about the event happened on Android
   */
  public void fireEventOnNode(final String instanceId, final String ref,
                              final String type, final Map<String, Object> data,
                              final Map<String, Object> domChanges, List<Object> params){
    fireEventOnNode(instanceId, ref, type, data, domChanges, params, null);
  }

  public void fireEventOnNode(final String instanceId, final String ref,
                              final String type, final Map<String, Object> data,
                              final Map<String, Object> domChanges, List<Object> params,  EventResult callback) {
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)
            || TextUtils.isEmpty(type) || mJSHandler == null) {
      return;
    }
    if (!checkMainThread()) {
      throw new WXRuntimeException(
              "fireEvent must be called by main thread");
    }
    WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
    if (instance != null && (instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER ||
            instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY)) {
      fireEventOnDataRenderNode(instanceId, ref, type, data, domChanges);
    } else {
      if(callback == null) {
        addJSEventTask(METHOD_FIRE_EVENT, instanceId, params, ref, type, data, domChanges);
        sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
      }else{
        asyncCallJSEventWithResult(callback, METHD_FIRE_EVENT_SYNC, instanceId, params, ref, type, data, domChanges);
      }
    }
  }

  private void fireEventOnDataRenderNode(final String instanceId, final String ref,
                                         final String type, final Map<String, Object> data, final Map<String, Object> domChanges) {
    mJSHandler.postDelayed(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        try {
          WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
          long start = System.currentTimeMillis();
          if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d("fireEventOnDataRenderNode >>>> instanceId:" + instanceId
                + ", data:" + data);
          }
          if (mWXBridge instanceof WXBridge) {
            ((WXBridge) mWXBridge).fireEventOnDataRenderNode(instanceId, ref, type,
                (data == null || data.isEmpty()) ? "{}" : JSON.toJSONString(data),
                (domChanges == null || domChanges.isEmpty()) ? "{}" : JSON.toJSONString(domChanges));
          }
          WXLogUtils.renderPerformanceLog("fireEventOnDataRenderNode", System.currentTimeMillis() - start);
        } catch (Throwable e) {
          String err = "[WXBridgeManager] fireEventOnDataRenderNode " + WXLogUtils.getStackTrace(e);
          WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "fireEventOnDataRenderNode",
              err, null);
          WXLogUtils.e(err);
        }
      }
    }), 0);
  }

  private boolean checkMainThread() {
    return Looper.myLooper() == Looper.getMainLooper();
  }


  /**
   * Invoke JavaScript callback. Use {@link JSCallback} instead.
   *
   * @see #callback(String, String, String)
   */
  @Deprecated
  public void callback(String instanceId, String callback, String data) {
    callback(instanceId, callback, data, false);
  }

  /**
   * Invoke JavaScript callback. Use {@link JSCallback} instead.
   */
  @Deprecated
  public void callback(final String instanceId, final String callback,
                       final Map<String, Object> data) {
    callback(instanceId, callback, data, false);
  }

  /**
   * Use {@link JSCallback} instead.
   *
   * @param instanceId Weex Instance Id
   * @param callback   callback referenece handle
   * @param data       callback data
   * @param keepAlive  if keep callback instance alive for later use
   */
  @Deprecated
  public void callback(final String instanceId, final String callback,
                       final Object data, boolean keepAlive) {
    callbackJavascript(instanceId, callback, data, keepAlive);
  }

  /**
   * Callback to Javascript function.
   *
   * @param instanceId Weex Instance Id
   * @param callback   callback referenece handle
   * @param data       callback data
   * @param keepAlive  if keep callback instance alive for later use
   */
  void callbackJavascript(final String instanceId, final String callback,
                          final Object data, boolean keepAlive) {
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(callback)
            || mJSHandler == null) {
      return;
    }

    WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
    if (instance != null && (instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY)) {
      callbackJavascriptOnDataRender(instanceId, callback, data, keepAlive);
    } else {
      addJSTask(METHOD_CALLBACK, instanceId, callback, data, keepAlive);
      sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
    }
  }

  void callbackJavascriptOnDataRender(final String instanceId, final String callback, final Object data, final boolean keepAlive){
    mJSHandler.postDelayed(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        try {
          long start = System.currentTimeMillis();
          String data_str = JSON.toJSONString(data);
          if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d("callbackJavascriptOnDataRender >>>> instanceId:" + instanceId
                + ", data:" + data_str);
          }
          if (mWXBridge instanceof WXBridge) {
            ((WXBridge) mWXBridge).invokeCallbackOnDataRender(instanceId, callback,data_str ,keepAlive);
          }
          WXLogUtils.renderPerformanceLog("callbackJavascriptOnDataRender", System.currentTimeMillis() - start);
        } catch (Throwable e) {
          String err = "[WXBridgeManager] callbackJavascriptOnDataRender " + WXLogUtils.getStackTrace(e);
          WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callbackJavascriptOnDataRender",
              err, null);
          WXLogUtils.e(err);
        }
      }
    }), 0);
  }

  /**
   * Refresh instance
   */
  public void refreshInstance(final String instanceId, final WXRefreshData jsonData) {
    if (TextUtils.isEmpty(instanceId) || jsonData == null) {
      return;
    }
    mJSHandler.postDelayed(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        invokeRefreshInstance(instanceId, jsonData);
      }
    }), 0);
  }

  private void invokeRefreshInstance(String instanceId, WXRefreshData refreshData) {
    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (!isSkipFrameworkInit(instanceId) && !isJSFrameworkInit()) {
        if (instance != null) {
          instance.onRenderError(
                  WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                  WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg()
                          + "invokeRefreshInstance FAILED for JSFrameworkInit FAILED, intance will invoke instance.onRenderError"
          );
        }
        String err = "[WXBridgeManager] invokeRefreshInstance: framework.js uninitialized.";
        WXLogUtils.e(err);
        return;
      }
      long start = System.currentTimeMillis();
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("refreshInstance >>>> instanceId:" + instanceId
                + ", data:" + refreshData.data + ", isDirty:" + refreshData.isDirty);
      }

      if (refreshData.isDirty) {
        return;
      }
      WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
              instanceId);
      WXJSObject dataObj = new WXJSObject(WXJSObject.JSON,
              refreshData.data == null ? "{}" : refreshData.data);
      WXJSObject[] args = {instanceIdObj, dataObj};
      mWXBridge.refreshInstance(instanceId, null, METHOD_REFRESH_INSTANCE, args);
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeRefreshInstance " + WXLogUtils.getStackTrace(e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "invokeRefreshInstance",
              err, null);
      WXLogUtils.e(err);
    }
  }

  public void commitJscCrashAlarmMonitor(final String type, final WXErrorCode errorCode, String errMsg,
                                         String instanceId, String url) {
    if (TextUtils.isEmpty(type) || errorCode == null) {
      return;
    }

    Log.d("ReportCrash", " commitJscCrashAlarmMonitor errMsg " + errMsg);
    String method = "callReportCrash";
    String exception = "weex core process crash and restart exception";
    Map<String, String> extParams = new HashMap<String, String>();
    extParams.put("jscCrashStack", errMsg);
    IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
    if (adapter != null) {
      WXJSExceptionInfo jsException = new WXJSExceptionInfo(instanceId, url, errorCode, method, exception, extParams);
      adapter.onJSException(jsException);
      // if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.e(jsException.toString());
      // }
    }
  }

  private boolean isSkipFrameworkInit(String instanceId) {
    final WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    return isSkipFrameworkInit(instance);
  }

  private boolean isSkipFrameworkInit(WXSDKInstance instance) {
    if (instance == null) {
      return false;
    }
    return instance.skipFrameworkInit();
  }

  /**
   * Create instance.
   */
  public void createInstance(final String instanceId, final String template,
                             final Map<String, Object> options, final String data) {
    createInstance(instanceId, new Script(template), options, data);
  }

  public void createInstance(final String instanceId, final Script template,
                             final Map<String, Object> options, final String data) {
    final WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance == null) {
      WXLogUtils.e("WXBridgeManager", "createInstance failed, SDKInstance does not exist");
      return;
    }
    if (TextUtils.isEmpty(instanceId) || template == null || template.isEmpty() || mJSHandler == null) {
      instance.onRenderError(
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() +
                      " instanceId==" + instanceId + " template ==" + template + " mJSHandler== " + mJSHandler.toString()
      );
      return;
    }

    if (!isSkipFrameworkInit(instanceId) && !isJSFrameworkInit() && reInitCount == 1 && !WXEnvironment.sDebugServerConnectable) {
      instance.onRenderError(
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() +
                      " isJSFrameworkInit==" + isJSFrameworkInit() + " reInitCount == 1" );
      post(new Runnable() {
        @Override
        public void run() {
          initFramework("");
        }
      }, instanceId);
      return;
    }

    WXModuleManager.createDomModule(instance);
    instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_START);
    post(new Runnable() {
      @Override
      public void run() {
        long start = System.currentTimeMillis();
        invokeCreateInstance(instance, template, options, data);
        long end = System.currentTimeMillis();
        instance.getWXPerformance().callCreateInstanceTime = end - start;
        instance.getWXPerformance().communicateTime =  instance.getWXPerformance().callCreateInstanceTime;
      }
    }, instanceId);
  }

  private void invokeCreateInstance(@NonNull WXSDKInstance instance, Script template,
                                    Map<String, Object> options, String data) {
    // add for sandbox, will delete on sandbox ok
    if (!isSkipFrameworkInit(instance)){
      initFramework("");
    }

    if (mMock) {
      mock(instance.getInstanceId());
    } else {
      if (!isSkipFrameworkInit(instance) && !isJSFrameworkInit()) {
        String err = "[WXBridgeManager] invokeCreateInstance: framework.js uninitialized.";
        instance.onRenderError(
                WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg()
        );
        WXLogUtils.e(err);
        return;
      }

      WXModuleManager.registerWhenCreateInstance();

      try {
        BundType type = BundType.Others;
        try {
          long start = System.currentTimeMillis();
          type = getBundleType(instance.getBundleUrl(), template.getContent());

          if (WXEnvironment.isOpenDebugLog()) {
            long end = System.currentTimeMillis();
            WXLogUtils.e("end getBundleType type:" + type.toString() + " time:" + (end - start));
          }
        } catch (Throwable e) {
          e.printStackTrace();
        }

        try {
          if (options == null) {
            options = new HashMap<>();
          }
          // on file there is { "framework": "Vue" } or others
          if (options.get(BUNDLE_TYPE) == null) {
            // may vue or Rax
            if (type == BundType.Vue) {
              options.put(BUNDLE_TYPE, "Vue");
            } else if (type == BundType.Rax) {
              options.put(BUNDLE_TYPE, "Rax");
            } else {
              options.put(BUNDLE_TYPE, "Others");
            }
            Object recordBundleType = options.get(BUNDLE_TYPE);
            if (recordBundleType instanceof String && "Others".equalsIgnoreCase((String)recordBundleType)){
              //same as iOS record
              recordBundleType = "other";
            }
            if (null != recordBundleType){
              instance.getApmForInstance().addProperty(WXInstanceApm.KEY_PAGE_PROPERTIES_BUNDLE_TYPE, recordBundleType);
            }
          }
          if (options.get("env") == null) {
            options.put("env", mInitParams.toMap());
          }
        } catch (Throwable e) {
          e.printStackTrace();
        }
        instance.bundleType = type;
        if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
          WXLogUtils.d("createInstance >>>> instanceId:" + instance.getInstanceId()
                  + ", options:"
                  + WXJsonUtils.fromObjectToJSONString(options)
                  + ", data:" + data);
        }
        WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
                instance.getInstanceId());
        WXJSObject instanceObj = new WXJSObject(WXJSObject.String,
                template.getContent());

        Object extraOption = null;
        if(options != null && options.containsKey("extraOption")) {
           extraOption = options.get("extraOption");
           options.remove("extraOption");
        }

        WXJSObject extraOptionObj = new WXJSObject(WXJSObject.JSON,
                extraOption == null ? "{}"
                        : WXJsonUtils.fromObjectToJSONString(extraOption));


        WXJSObject optionsObj = new WXJSObject(WXJSObject.JSON,
                options == null ? "{}"
                        : WXJsonUtils.fromObjectToJSONString(options));
        optionsObj = optionObjConvert(isSandBoxContext, type, optionsObj);
        WXJSObject dataObj = new WXJSObject(WXJSObject.JSON,
                data == null ? "{}" : data);

        WXJSObject apiObj;
        if (type == BundType.Rax || instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER) {
          if (mRaxApi == null) {
            IWXJsFileLoaderAdapter iwxJsFileLoaderAdapter = WXSDKEngine.getIWXJsFileLoaderAdapter();
            if(iwxJsFileLoaderAdapter != null) {
              mRaxApi = iwxJsFileLoaderAdapter.loadRaxApi();
            }

            if(TextUtils.isEmpty(mRaxApi)) {
              mRaxApi =  WXFileUtils.loadAsset("weex-rax-api.js", WXEnvironment.getApplication());
            }
          }
          apiObj = new WXJSObject(WXJSObject.String,
                  mRaxApi);
        } else {
          apiObj = new WXJSObject(WXJSObject.String,
                  "");
        }

        // When render strategy is data_render, put it into options. Others keep null.
        WXJSObject renderStrategy = null;
        if (instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER) {
          renderStrategy = new WXJSObject(WXJSObject.String, WXRenderStrategy.DATA_RENDER.getFlag());
        } else if (instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY) {
          renderStrategy = new WXJSObject(WXJSObject.String, WXRenderStrategy.DATA_RENDER_BINARY.getFlag());
          // In DATA_RENDER_BINARY strategy script is binary
          instanceObj.data = template.getBinary();
        }

        WXJSObject[] args = {instanceIdObj, instanceObj, optionsObj,
                dataObj, apiObj, renderStrategy, extraOptionObj};

        instance.setTemplate(template.getContent());
        // if { "framework": "Vue" } or  { "framework": "Rax" } will use invokeCreateInstanceContext
        // others will use invokeExecJS
        if (!isSandBoxContext) {
          invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
          return;
        }
        if (type == BundType.Vue || type == BundType.Rax
                || instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER
                || instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY) {
          int ret = invokeCreateInstanceContext(instance.getInstanceId(), null, "createInstanceContext", args, false);
          instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_END);
          if(ret == 0) {
            String err = "[WXBridgeManager] invokeCreateInstance : " + instance.getTemplateInfo();

            instance.onRenderError(
                    WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                    WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() + err);
          }
          return;
        } else {
          //bad case for js bundle with out bundletype header //vue or rax
          WXExceptionUtils.commitCriticalExceptionRT(
                instance.getInstanceId(),
                WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE,
                "invokeCreateInstance",
                WXErrorCode.WX_KEY_EXCEPTION_NO_BUNDLE_TYPE.getErrorMsg(),
                null
          );

          invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
          instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_END);
          return;
        }
      } catch (Throwable e) {
        String err = "[WXBridgeManager] invokeCreateInstance " + e.getCause()
                + instance.getTemplateInfo();

        instance.onRenderError(
                WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() + err);
        WXLogUtils.e(err);
      }
    }
  }

  public WXJSObject optionObjConvert(boolean useSandBox, BundType type, WXJSObject opt) {
    if (!useSandBox) {
      return opt;
    }
    try {
      String data = opt.data.toString();
      JSONObject obj = JSON.parseObject(data);
      JSONObject optEnv;
      if ((optEnv = obj.getJSONObject("env")) != null) {
        JSONObject opts = optEnv.getJSONObject("options");
        if (opts != null) {
          for (String s : opts.keySet()) {
            optEnv.put(s, opts.getString(s));
          }
        }
      }
      return new WXJSObject(WXJSObject.JSON, obj.toString());
    } catch (Throwable e) {
      e.printStackTrace();
    }
    return opt;

  }

  /**
   * check bundleType
   * @param url
   * @param temp
   * @return
   */
  public BundType getBundleType(String url, String temp) {
    try {
      if (url != null) {
        Uri uri = Uri.parse(url);
        String type = uri.getQueryParameter(BUNDLE_TYPE);
        if ("Vue".equals(type) || "vue".equals(type)) {
          return BundType.Vue;
        } else if ("Rax".equals(type) || "rax".equals(type)) {
          return BundType.Rax;
        }
      }
      if (temp != null) {
        final String FRAMEWORK="framework", VUE="vue", RAX="rax";

        // Find the first line that starts with '//' and convert it to json
        int bundleTypeStart = temp.indexOf("//");
        int bundleTypeEnd = temp.indexOf("\n", bundleTypeStart);
        JSONObject bundleType = JSONObject.parseObject(
            temp.substring(bundleTypeStart+2, bundleTypeEnd));
        String type = bundleType.getString(FRAMEWORK);
        if(VUE.equalsIgnoreCase(type)){
          return BundType.Vue;
        }
        else if(RAX.equalsIgnoreCase(type)){
          return BundType.Rax;
        }
        else{
          // '//{ "framework": "Vue"}' is not found.
          String regEx = "(use)(\\s+)(weex:vue)";
          Pattern pattern = Pattern.compile(regEx, Pattern.CASE_INSENSITIVE);
          if (pattern.matcher(temp).find()) {
            return BundType.Vue;
          }
          regEx = "(use)(\\s+)(weex:rax)";
          pattern = Pattern.compile(regEx, Pattern.CASE_INSENSITIVE);
          if (pattern.matcher(temp).find()) {
            return BundType.Rax;
          }
        }
      }
      return BundType.Others;
    } catch (Throwable e) {
      e.printStackTrace();
      return BundType.Others;
    }
  }

  private void mock(String instanceId) {

  }

  public void destroyInstance(final String instanceId) {
    if (mJSHandler == null
            || TextUtils.isEmpty(instanceId)) {
      return;
    }
    if (mDestroyedInstanceId != null) {
      mDestroyedInstanceId.add(instanceId);
    }
    // clear message with instanceId
    mJSHandler.removeCallbacksAndMessages(instanceId);
    post(new Runnable() {
      @Override
      public void run() {
        removeTaskByInstance(instanceId);
        invokeDestroyInstance(instanceId);
      }
    }, instanceId);
  }

  private void removeTaskByInstance(String instanceId) {
    mNextTickTasks.removeFromMapAndStack(instanceId);
  }

  private void invokeDestroyInstance(String instanceId) {
    try {
      if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
        WXLogUtils.d("destroyInstance >>>> instanceId:" + instanceId);
      }
      WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
              instanceId);
      WXJSObject[] args = {instanceIdObj};
      if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
        invokeDestoryInstance(instanceId, null, METHOD_DESTROY_INSTANCE, args, true);
        // invokeExecJS(instanceId, null, METHOD_DESTROY_INSTANCE, args);
      }
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeDestroyInstance " + e.getCause();
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "invokeDestroyInstance", err, null);
      WXLogUtils.e(err);
    }
  }

  @Override
  public boolean handleMessage(Message msg) {
    if (msg == null) {
      return false;
    }

    int what = msg.what;
    switch (what) {
      case WXJSBridgeMsgType.INIT_FRAMEWORK:
        invokeInitFramework(msg);
        break;
      case WXJSBridgeMsgType.CALL_JS_BATCH:
        invokeCallJSBatch(msg);
        break;
      case WXJSBridgeMsgType.SET_TIMEOUT:
        TimerInfo timerInfo = (TimerInfo) msg.obj;
        if (timerInfo == null) {
          break;
        }
        WXJSObject obj = new WXJSObject(WXJSObject.String, timerInfo.callbackId);
        WXJSObject[] args = {obj};
        invokeExecJS("", null, METHOD_SET_TIMEOUT, args);
        break;
      case WXJSBridgeMsgType.TAKE_HEAP_SNAPSHOT:
        if (msg.obj != null) {
          String filename = (String) msg.obj;
          mWXBridge.takeHeapSnapshot(filename);
        }
        break;
      default:
        break;
    }
    return false;
  }

  private void invokeExecJS(String instanceId, String namespace, String function, WXJSObject[] args) {
    invokeExecJS(instanceId, namespace, function, args, true);
  }

  public void invokeExecJS(String instanceId, String namespace, String function,
                           WXJSObject[] args, boolean logTaskDetail) {
    if (WXEnvironment.isOpenDebugLog() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
              .append("function:").append(function);
      if (logTaskDetail) {
        mLodBuilder.append(" tasks:").append(argsToJSON(args));
      }
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    final long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null && (instance.getRenderStrategy() == WXRenderStrategy.DATA_RENDER_BINARY)) {
      Pair<Pair<String,Object>, Boolean> data = null;
      if(args.length!=2 || !(args[0].data instanceof String)
          || !(args[1].data instanceof String)
          || (data = extractCallbackArgs((String) args[1].data))==null){
        WXLogUtils.w("invokeExecJS on data render that is not a callback call");
        return;
      }
      callbackJavascriptOnDataRender(instanceId, (String) data.first.first, data.first.second, data.second);
    } else {
      mWXBridge.execJS(instanceId, namespace, function, args);
    }
    if (null != instance){
      long diff = System.currentTimeMillis()-start;
      instance.getApmForInstance().updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_CALL_JS_NUM,1);
      instance.getApmForInstance().updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_CALL_JS_TIME,diff);
      instance.callJsTime(diff);
    }
  }

  private Pair<Pair<String,Object>,Boolean> extractCallbackArgs(String data) {
    try {
      JSONArray obj = JSON.parseArray(data);
      JSONObject arg_obj = obj.getJSONObject(0);
      JSONArray args = arg_obj.getJSONArray("args");
      if (args.size()!=3){
        return null;
      }
      String method = arg_obj.getString("method");
      if (!"callback".equals(method)){
        return null;
      }

      return new Pair<Pair<String,Object>, Boolean>(new Pair<String, Object>(args.getString(0), args.getJSONObject(1)),args.getBooleanValue(2));
    } catch (Exception e) {
      return null;
    }
  }

  public int invokeCreateInstanceContext(String instanceId, String namespace, String function,
                                          WXJSObject[] args, boolean logTaskDetail) {
    WXLogUtils.d("invokeCreateInstanceContext instanceId:" + instanceId + " function:"
            + function + " isJSFrameworkInit：%d" + isJSFrameworkInit());
    mLodBuilder.append("createInstanceContext >>>> instanceId:").append(instanceId)
            .append("function:").append(function);
    if (logTaskDetail)
      mLodBuilder.append(" tasks:").append(WXJsonUtils.fromObjectToJSONString(args));
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    // }
    return mWXBridge.createInstanceContext(instanceId, namespace, function, args);
  }

  public void invokeDestoryInstance(String instanceId, String namespace, String function,
                                    WXJSObject[] args, boolean logTaskDetail) {
    // if (WXEnvironment.isApkDebugable()) {
    mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
            .append("function:").append(function);
    if (logTaskDetail)
      mLodBuilder.append(" tasks:").append(WXJsonUtils.fromObjectToJSONString(args));
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    // }
    mWXBridge.destoryInstance(instanceId, namespace, function, args);
  }

  private void execJSOnInstance(final EventResult eventCallback, final String instanceId, final String js, final int type) {
    post(new Runnable() {
      @Override
      public void run() {
        String ret = invokeExecJSOnInstance(instanceId, js, type);
        eventCallback.onCallback(ret);
      }
    });
  }

  private String invokeExecJSOnInstance(String instanceId, String js, int type) {
    // if (WXEnvironment.isApkDebugable()) {
    mLodBuilder.append("execJSOnInstance >>>> instanceId:").append(instanceId);
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    // }
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      return mWXBridge.execJSOnInstance(instanceId, js, type);
    }
    return null;
  }

  private void invokeExecJSWithCallback(String instanceId, String namespace, String function,
                                        WXJSObject[] args , ResultCallback callback, boolean logTaskDetail){
    if (WXEnvironment.isOpenDebugLog() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
              .append("function:").append(function);
      if(logTaskDetail) {
        mLodBuilder.append(" tasks:").append(argsToJSON(args));
      }
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.execJSWithCallback(instanceId, namespace, function, args, callback);
    }
  }

  public @NonNull static String argsToJSON(WXJSObject[] args) {
    StringBuilder builder = new StringBuilder();
    builder.append("[");
    for(WXJSObject object : args){
      builder.append(WXWsonJSONSwitch.fromObjectToJSONString(object));
      builder.append(",");
    }
    builder.append("]");
    return  builder.toString();
  }

  private void invokeInitFramework(Message msg) {
    String framework = "";
    if (msg.obj != null) {
      framework = (String) msg.obj;
    }

    if (WXUtils.getAvailMemory(WXEnvironment.getApplication()) > LOW_MEM_VALUE) {
      initFramework(framework);
    }
  }

  public static long sInitFrameWorkTimeOrigin;
  public static StringBuilder sInitFrameWorkMsg = new StringBuilder();

  private void initFramework(String framework) {
    if (WXSDKEngine.isSoInitialized() && !isJSFrameworkInit()) {
      sInitFrameWorkTimeOrigin = System.currentTimeMillis();
      if (TextUtils.isEmpty(framework)) {
        // if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("weex JS framework from assets");
        // }

        IWXJsFileLoaderAdapter wxJsFileLoaderAdapter = WXSDKEngine.getIWXJsFileLoaderAdapter();

        if (!isSandBoxContext) {
          if(wxJsFileLoaderAdapter != null) {
            framework = wxJsFileLoaderAdapter.loadJsFramework();
          }

          if(TextUtils.isEmpty(framework)) {
            framework = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
          }
        } else {
          if(wxJsFileLoaderAdapter != null) {
            framework = wxJsFileLoaderAdapter.loadJsFrameworkForSandBox();
          }

          if(TextUtils.isEmpty(framework)) {
            framework = WXFileUtils.loadAsset("weex-main-jsfm.js", WXEnvironment.getApplication());
          }
        }
        sInitFrameWorkMsg.append("| weex JS framework from assets, isSandBoxContext: ").append(isSandBoxContext);
      }
      if (TextUtils.isEmpty(framework)) {
        setJSFrameworkInit(false);
        sInitFrameWorkMsg.append("| framework isEmpty ");
        WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
                "initFramework", "framework is empty!! ", null);
        return;
      }
      try {
        if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
          long start = System.currentTimeMillis();
          WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkStart();
          WXEnvironment.sJSFMStartListenerTime = System.currentTimeMillis() - start;
          try {
            IWXUserTrackAdapter adapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
            if (null != adapter){
              Map<String,Serializable> params = new HashMap<>(1);
              params.put("time",String.valueOf(WXEnvironment.sJSFMStartListenerTime));
              adapter.commit(WXEnvironment.getApplication(),"sJSFMStartListener",IWXUserTrackAdapter.COUNTER,null,params);
            }
          }catch (Exception e){
            e.printStackTrace();
          }
        }

        long start = System.currentTimeMillis();
        String crashFile = "";
        try {
          crashFile = WXEnvironment.getApplication().getApplicationContext().getCacheDir().getPath();
        } catch (Exception e) {
          e.printStackTrace();
        }
        boolean pieSupport = true;
        try {
          if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            pieSupport = false;
          }
        } catch (Exception e) {
          e.printStackTrace();
        }
        sInitFrameWorkMsg.append(" | pieSupport:").append(pieSupport);
        WXLogUtils.d("[WXBridgeManager] initFrameworkEnv crashFile:" + crashFile + " pieSupport:" + pieSupport);
        // extends initFramework
        if (mWXBridge.initFrameworkEnv(framework, assembleDefaultOptions(), crashFile, pieSupport) == INIT_FRAMEWORK_OK) {
          WXEnvironment.sJSLibInitTime = System.currentTimeMillis() - start;
          WXEnvironment.sSDKInitTime = System.currentTimeMillis() - WXEnvironment.sSDKInitStart;
          setJSFrameworkInit(true);

          if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
            WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkReady();
          }

          execRegisterFailTask();
          WXEnvironment.JsFrameworkInit = true;
          registerDomModule();
          trackComponentAndModulesTime();
        } else {
          sInitFrameWorkMsg.append(" | ExecuteJavaScript fail, reInitCount").append(reInitCount);
          if (reInitCount > 1) {
            WXLogUtils.e("[WXBridgeManager] invokeReInitFramework  ExecuteJavaScript fail");
          } else {
            WXLogUtils.e("[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail");
          }
        }
      } catch (Throwable e) {
        sInitFrameWorkMsg.append(" | invokeInitFramework exception ").append(e.toString());
        if (reInitCount > 1) {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
        } else {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
        }
      }

    }
  }

  private void trackComponentAndModulesTime() {
    post(new Runnable() {
      @Override
      public void run() {
        WXEnvironment.sComponentsAndModulesReadyTime = System.currentTimeMillis() - WXEnvironment.sSDKInitStart;
      }
    });
  }

  @SuppressWarnings("unchecked")
  private void invokeCallJSBatch(Message message) {
    if (mNextTickTasks.isEmpty() || !isJSFrameworkInit()) {
      if (!isJSFrameworkInit()) {
        WXLogUtils.e("[WXBridgeManager] invokeCallJSBatch: framework.js uninitialized!!  message:" + message.toString());
      }
      return;
    }

    try {
      Object instanceId = message.obj;

      Object task = null;
      Stack<String> instanceStack = mNextTickTasks.getInstanceStack();
      int size = instanceStack.size();
      for (int i = size - 1; i >= 0; i--) {
        instanceId = instanceStack.get(i);
        task = mNextTickTasks.remove(instanceId);
        if (task != null && !((ArrayList) task).isEmpty()) {
          break;
        }
      }
      if (null != task){
        task = ((ArrayList) task).toArray();

        WXJSObject[] args = {
            new WXJSObject(WXJSObject.String, instanceId),
            WXWsonJSONSwitch.toWsonOrJsonWXJSObject(task)};

        invokeExecJS(String.valueOf(instanceId), null, METHOD_CALL_JS, args);
      }
    } catch (Throwable e) {
      WXLogUtils.e("WXBridgeManager", e);
      String err = "invokeCallJSBatch#" + WXLogUtils.getStackTrace(e);
      WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
              "invokeCallJSBatch", err, null);
    }

    // If task is not empty, loop until it is empty
    if (!mNextTickTasks.isEmpty()) {
      mJSHandler.sendEmptyMessage(WXJSBridgeMsgType.CALL_JS_BATCH);
    }
  }

  private WXParams assembleDefaultOptions() {
    checkJsEngineMultiThread();

    Map<String, String> config = WXEnvironment.getConfig();
    WXParams wxParams = new WXParams();
    wxParams.setPlatform(config.get(WXConfig.os));
    wxParams.setCacheDir(config.get(WXConfig.cacheDir));
    wxParams.setOsVersion(config.get(WXConfig.sysVersion));
    wxParams.setAppVersion(config.get(WXConfig.appVersion));
    wxParams.setWeexVersion(config.get(WXConfig.weexVersion));
    wxParams.setDeviceModel(config.get(WXConfig.sysModel));
    wxParams.setShouldInfoCollect(config.get("infoCollect"));
    wxParams.setLogLevel(config.get(WXConfig.logLevel));
    wxParams.setLayoutDirection(config.get(WXConfig.layoutDirection));
    wxParams.setUseSingleProcess(isUseSingleProcess ? "true" : "false");
    wxParams.setCrashFilePath(WXEnvironment.getCrashFilePath(WXEnvironment.getApplication().getApplicationContext()));
    wxParams.setLibJssPath(WXEnvironment.getLibJssRealPath());
    wxParams.setLibIcuPath(WXEnvironment.getLibJssIcuPath());
    wxParams.setLibLdPath(WXEnvironment.getLibLdPath());
    String libJScRealPath = WXEnvironment.getLibJScRealPath();
    wxParams.setLibJscPath(TextUtils.isEmpty(libJScRealPath)? "" : new File(libJScRealPath).getParent());
    String appName = config.get(WXConfig.appName);
    if (!TextUtils.isEmpty(appName)) {
      wxParams.setAppName(appName);
    }
    wxParams.setDeviceWidth(TextUtils.isEmpty(config.get("deviceWidth")) ? String.valueOf(WXViewUtils.getScreenWidth(WXEnvironment.sApplication)) : config.get("deviceWidth"));
    wxParams.setDeviceHeight(TextUtils.isEmpty(config.get("deviceHeight")) ? String.valueOf(WXViewUtils.getScreenHeight(WXEnvironment.sApplication)) : config.get("deviceHeight"));
    Map<String, String> customOptions = WXEnvironment.getCustomOptions();
    customOptions.put("enableBackupThread", String.valueOf(jsEngineMultiThreadEnable()));
    IWXJscProcessManager wxJscProcessManager = WXSDKManager.getInstance().getWXJscProcessManager();
    if(wxJscProcessManager != null) {
      customOptions.put("enableBackupThreadCache", String.valueOf(wxJscProcessManager.enableBackUpThreadCache()));
    }

    wxParams.setOptions(customOptions);
    wxParams.setNeedInitV8(WXSDKManager.getInstance().needInitV8());
    mInitParams = wxParams;
    return wxParams;
  }

  public WXParams getInitParams() {
    return mInitParams;
  }

  private void execRegisterFailTask() {

    if (mRegisterModuleFailList.size() > 0) {
      List<Map<String, Object>> moduleReceiver = new ArrayList<>();
      for (int i = 0, moduleCount = mRegisterModuleFailList.size(); i < moduleCount; ++i) {
        invokeRegisterModules(mRegisterModuleFailList.get(i), moduleReceiver);
      }
      mRegisterModuleFailList.clear();
      if (moduleReceiver.size() > 0) {
        mRegisterModuleFailList.addAll(moduleReceiver);
      }
    }

    if (mRegisterComponentFailList.size() > 0) {
      List<Map<String, Object>> receiver = new ArrayList<>();
      invokeRegisterComponents(mRegisterComponentFailList, receiver);
      mRegisterComponentFailList.clear();
      if (receiver.size() > 0) {
        mRegisterComponentFailList.addAll(receiver);
      }
    }

    if (mRegisterServiceFailList.size() > 0) {
      List<String> receiver = new ArrayList<>();
      for (String service : mRegisterServiceFailList) {
        invokeExecJSService(service, receiver);
      }
      mRegisterServiceFailList.clear();
      if (receiver.size() > 0) {
        mRegisterServiceFailList.addAll(receiver);
      }
    }
  }

  /**
   * Register Android module
   *
   * @param modules the format is like
   *                {'dom':['updateAttrs','updateStyle'],'event':['openUrl']}
   */

  public void registerModules(final Map<String, Object> modules) {
    if (modules != null && modules.size() != 0) {
      if (isJSThread()) {
        invokeRegisterModules(modules, mRegisterModuleFailList);
      } else {
        post(new Runnable() {
          @Override
          public void run() {
            invokeRegisterModules(modules, mRegisterModuleFailList);
          }
        }, null);
      }
    }
  }

  /**
   * Registered component
   */
  public void registerComponents(final List<Map<String, Object>> components) {
    if (mJSHandler == null || components == null
            || components.size() == 0) {
      return;
    }
    Runnable runnable = new Runnable() {
      @Override
      public void run() {
        invokeRegisterComponents(components, mRegisterComponentFailList);
      }
    };

    if(isJSThread() && isJSFrameworkInit()){
      runnable.run();
    }else{
      post(runnable, null);
    }
  }

  public void execJSService(final String service) {
    post(new Runnable() {
      @Override
      public void run() {
        invokeExecJSService(service, mRegisterServiceFailList);
      }
    });
  }

  private void invokeExecJSService(String service, List<String> receiver) {
    try {
      if (!isJSFrameworkInit()) {
        WXLogUtils.e("[WXBridgeManager] invoke execJSService: framework.js uninitialized.");
        receiver.add(service);
        return;
      }
      mWXBridge.execJSService(service);
    } catch (Throwable e) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterService:", e);

      Map<String, String> data = new HashMap<String, String>();
      data.put("inputParams",service + "||" + receiver.toString());

      WXExceptionUtils.commitCriticalExceptionRT("invokeExecJSService",
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE,
              "invokeExecJSService",
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE.getErrorMsg()
                      + "[WXBridgeManager] invokeRegisterService:"
                      + WXLogUtils.getStackTrace(e),
              data);
    }
  }

  public boolean isJSThread() {
    return mJSThread != null && mJSThread.getId() == Thread.currentThread().getId();
  }

  private void invokeRegisterModules(Map<String, Object> modules, List<Map<String, Object>> failReceiver) {
    if (modules == null || !isJSFrameworkInit()) {
      if (!isJSFrameworkInit()) {
        WXLogUtils.d("[WXinvokeRegisterModulesBridgeManager] invokeRegisterModules: framework.js uninitialized.");
      }
      failReceiver.add(modules);
      return;
    }

    WXJSObject[] args = {WXWsonJSONSwitch.toWsonOrJsonWXJSObject(modules)};
    String errorMsg = null;
    try{
      // TODO use a better way
      if (mWXBridge instanceof WXBridge) {
        ((WXBridge) mWXBridge).registerModuleOnDataRenderNode(WXJsonUtils.fromObjectToJSONString(modules));
      }
    } catch (Throwable e){
      WXLogUtils.e("Weex [data_render register err]", e);
    }
    try {
        if(0 == mWXBridge.execJS("", null, METHOD_REGISTER_MODULES, args)) {
            errorMsg = "execJS error";
        }
      try {
        Iterator<String> iter = modules.keySet().iterator();
        while (iter.hasNext()) {
          String module = iter.next();
          if (module != null) {
            WXModuleManager.resetModuleState(module, true);
            //WXLogUtils.e("[WXBridgeManager]invokeRegisterModules METHOD_REGISTER_MODULES success module:" + module);
          }
        }
      } catch (Throwable e) {
        WXLogUtils.e("Weex [invokeRegisterModules]", e);
      }
    } catch (Throwable e) {
      errorMsg = WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES.getErrorMsg() +
                " \n " + e.getMessage() + modules.entrySet().toString();
    }

    if(!TextUtils.isEmpty(errorMsg)) {
        WXLogUtils.e("[WXBridgeManager] invokeRegisterModules:", errorMsg);
        WXExceptionUtils.commitCriticalExceptionRT(null,
                WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES,
                "invokeRegisterModules", errorMsg,
                null );
    }
  }

  private void invokeRegisterComponents(List<Map<String, Object>> components, List<Map<String, Object>> failReceiver) {
    if (components == failReceiver) {
      throw new RuntimeException("Fail receiver should not use source.");
    }
    if (!isJSFrameworkInit()) {
      //WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents: framework.js uninitialized.");

      for (Map<String, Object> comp : components) {
        failReceiver.add(comp);
      }
      return;
    }
    if (components == null) {
      return;
    }

    try{
      // TODO use a better way
      if (mWXBridge instanceof WXBridge) {
        ((WXBridge) mWXBridge).registerComponentOnDataRenderNode(WXJsonUtils.fromObjectToJSONString(components));
      }
    } catch (Throwable e){
      WXLogUtils.e("Weex [data_render register err]", e);
    }

    WXJSObject[] args = {WXWsonJSONSwitch.toWsonOrJsonWXJSObject(components)};
    String errorMsg = null;
    try {
      if(0 == mWXBridge.execJS("", null, METHOD_REGISTER_COMPONENTS, args)) {
          errorMsg = "execJS error";
      }
    } catch (Throwable e) {
        errorMsg = WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_COMPONENT
                + args.toString()
                + WXLogUtils.getStackTrace(e);
    }

    if(!TextUtils.isEmpty(errorMsg)) {
        WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents ", errorMsg);
        WXExceptionUtils.commitCriticalExceptionRT(null,
                WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_COMPONENT,
                METHOD_REGISTER_COMPONENTS, errorMsg,
                null);
    }
  }

  public void destroy() {
    if (mJSThread != null) {
      mJSThread.quit();
    }
    mBridgeManager = null;
    if (mDestroyedInstanceId != null) {
      mDestroyedInstanceId.clear();
    }

  }

  /**
   * Report JavaScript Exception
   */
  public void reportJSException(String instanceId, String function,
                                String exception) {
    WXLogUtils.e("reportJSException >>>> instanceId:" + instanceId
            + ", exception function:" + function + ", exception:"
            + exception);
    WXSDKInstance instance = null;
    WXErrorCode reportErrorCode = WXErrorCode.WX_ERR_JS_EXECUTE;
    if (instanceId != null && (instance = WXSDKManager.getInstance().getSDKInstance(instanceId)) != null) {
      instance.setHasException(true);
      exception +=   "\n getTemplateInfo==" +instance.getTemplateInfo();//add network header info
      if ((METHOD_CREATE_INSTANCE.equals(function)) || !instance.isContentMd5Match()) {
        try {
          //data render mode should report exception instead of reload page,
          // so we use !isSkipFrameworkInit(instanceId) to skip the positive branch of if clause.
          if (!isSkipFrameworkInit(instanceId) && isJSFrameworkInit() && (reInitCount > 1 && reInitCount < 10) && !instance.isNeedReLoad()) {
            new ActionReloadPage(instanceId, true).executeAction();
            instance.setNeedLoad(true);
            return;
          } else {
            String errorMsg = new StringBuilder()
                .append(WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg())
                .append(", reportJSException >>>> instanceId:").append(instanceId)
                .append(", exception function:").append(function)
                .append(", exception:").append(exception)
                .append(", extInitTime:").append(System.currentTimeMillis()-WXBridgeManager.sInitFrameWorkTimeOrigin).append("ms")
                .append(", extInitErrorMsg:").append(WXBridgeManager.sInitFrameWorkMsg.toString())
                .toString();
            instance.onRenderError(//DegradPassivity to H5
                     WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                     errorMsg
            );
            if (!WXEnvironment.sInAliWeex){
              WXExceptionUtils.commitCriticalExceptionRT(instanceId, WXErrorCode.WX_RENDER_ERR_JS_CREATE_INSTANCE, function,exception,null);
            }
            return;
          }
        } catch (Exception e) {
          e.printStackTrace();
        }
      }
      if (METHOD_CREATE_INSTANCE.equals(function) && !instance.getApmForInstance().hasAddView){
        reportErrorCode = WXErrorCode.WX_RENDER_ERR_JS_CREATE_INSTANCE;
      } else if ( METHOD_CREATE_INSTANCE_CONTEXT.equals(function) && !instance.getApmForInstance().hasAddView){
        reportErrorCode = WXErrorCode.WX_RENDER_ERR_JS_CREATE_INSTANCE_CONTEXT;
      } else if ((METHOD_UPDATE_COMPONENT_WITH_DATA.equals(function) || METHOD_CREATE_PAGE_WITH_CONTENT.equals(function)) && !instance.getApmForInstance().hasAddView){
        reportErrorCode = WXErrorCode.WX_DEGRAD_EAGLE_RENDER_ERROR;
      }
      instance.onJSException(reportErrorCode.getErrorCode(), function, exception);
    }
    doReportJSException(instanceId,function,reportErrorCode,exception);
  }

  private void doReportJSException(String instanceId, String function,WXErrorCode reportCode, String exception){
    WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
    IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
    if (adapter != null) {
      String exceptionId = instanceId;

      if (TextUtils.isEmpty(instanceId)) {
        exceptionId = "instanceIdisNull";
      }

      if (instance == null) {
        if (("initFramework").equals(function)) {
          String exceptionExt = null;
          try {
            if (WXEnvironment.getApplication() != null) {
              final String fileName = WXEnvironment.getApplication().getApplicationContext().getCacheDir().getPath() + INITLOGFILE;
              try {
                File file = new File(fileName);
                if (file.exists()) {
                  if (file.length() > 0) {
                    StringBuilder result = new StringBuilder();
                    try {
                      InputStreamReader read = new InputStreamReader(new FileInputStream(file), "UTF-8");
                      BufferedReader br = new BufferedReader(read);
                      String s = null;
                      while ((s = br.readLine()) != null) {
                        result.append(s + "\n");
                      }
                      exceptionExt = result.toString();
                      br.close();
                    } catch (Exception e) {
                      e.printStackTrace();
                    }
                  }
                  file.delete();
                }
              } catch (Throwable throwable) {
                throwable.printStackTrace();
              }
            }
          } catch (Throwable e) {
            e.printStackTrace();
          }
          exception += "\n" + exceptionExt;
          WXLogUtils.e("reportJSException:" + exception);

        }
      }


      WXExceptionUtils.commitCriticalExceptionRT(exceptionId, reportCode,
            function,
          reportCode.getErrorMsg() + exception,
            null);

    }
  }

  private void registerDomModule() throws WXException {
    /** Tell Javascript Framework what methods you have. This is Required.**/
    Map<String, Object> domMap = new HashMap<>();
    domMap.put(WXDomModule.WXDOM, WXDomModule.METHODS);
    registerModules(domMap);
  }

  //This method is deprecated because of performance issue.
  @Deprecated
  public void notifyTrimMemory() {

  }

  /**
   * update js server global config, current support turn wson off
   * by pass wson_off
   * */
  public static void  updateGlobalConfig(String config) {
    if(TextUtils.isEmpty(config)){
      config = "none";
    }
    if(!TextUtils.equals(config, globalConfig)){
      globalConfig = config;
      WXEnvironment.addCustomOptions(GLOBAL_CONFIG_KEY, globalConfig);
      Runnable runnable = new Runnable() {
        @Override
        public void run() {
          if(mBridgeManager != null){
            if(mBridgeManager.isJSFrameworkInit()){
              if(mBridgeManager.mWXBridge instanceof WXBridge) {
                final WXBridge bridge = (WXBridge) mBridgeManager.mWXBridge;
                bridge.nativeUpdateGlobalConfig(globalConfig);
              }
            }
          }
          if(globalConfig.contains(WXWsonJSONSwitch.WSON_OFF)){
            WXWsonJSONSwitch.USE_WSON = false;
          }else{
            WXWsonJSONSwitch.USE_WSON = true;
          }
        }
      };
      if(mBridgeManager != null && mBridgeManager.isJSFrameworkInit()){
        mBridgeManager.post(runnable);
      }else{
        runnable.run();
      }
    }
  }

  public
  @Nullable
  Looper getJSLooper() {
    Looper ret = null;
    if (mJSThread != null) {
      ret = mJSThread.getLooper();
    }
    return ret;
  }

  public void notifySerializeCodeCache() {
    post(new Runnable() {
      @Override
      public void run() {
        if (!isJSFrameworkInit())
          return;

        invokeExecJS("", null, METHOD_NOTIFY_SERIALIZE_CODE_CACHE, new WXJSObject[0]);
      }
    });
  }

  public void takeJSHeapSnapshot(String filename) {
    Message msg = mJSHandler.obtainMessage();
    msg.obj = filename;
    msg.what = WXJSBridgeMsgType.TAKE_HEAP_SNAPSHOT;
    msg.setTarget(mJSHandler);
    msg.sendToTarget();
  }

  public static class TimerInfo {

    public String callbackId;
    public long time;
    public String instanceId;
  }

  public int callCreateBody(String pageId, String componentType, String ref,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders) {

    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(componentType) || TextUtils.isEmpty(ref)) {
      WXLogUtils.d("[WXBridgeManager] call callCreateBody arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callCreateBody",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callCreateBody >>>> pageId:").append(pageId)
              .append(", componentType:").append(componentType).append(", ref:").append(ref)
              .append(", styles:").append(styles)
              .append(", attributes:").append(attributes)
              .append(", events:").append(events);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(pageId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(pageId);
      if (instance != null) {
        final BasicGraphicAction action = new GraphicActionCreateBody(instance, ref, componentType,
                styles, attributes, events, margins, paddings, borders);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateBody exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callCreateBody",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddElement(String pageId, String componentType, String ref, int index, String parentRef,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders,boolean willLayout) {
    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(componentType) || TextUtils.isEmpty(ref)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callAddElement arguments is null");
      }
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callAddElement",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callAddElement >>>> pageId:").append(pageId)
              .append(", componentType:").append(componentType).append(", ref:").append(ref).append(", index:").append(index)
              .append(", parentRef:").append(parentRef)
              .append(", styles:").append(styles)
              .append(", attributes:").append(attributes)
              .append(", events:").append(events);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(pageId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(pageId);
      if (instance != null) {
        final GraphicActionAddElement action = new GraphicActionAddElement(instance, ref, componentType, parentRef, index,
            styles, attributes, events, margins, paddings, borders);
        if(willLayout) {
          instance.addInActiveAddElementAction(ref, action);
        }else{
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(pageId, action);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddElement exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callAddElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveElement(String instanceId, String ref) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callRemoveElement arguments is null");
      }
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callRemoveElement",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callRemoveElement >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        final BasicGraphicAction action = new GraphicActionRemoveElement(instance, ref);
        if(instance.getInActiveAddElementAction(ref)!=null){
          instance.removeInActiveAddElmentAction(ref);
        }
        else {
          WXSDKManager.getInstance().getWXRenderManager()
              .postGraphicAction(action.getPageId(), action);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveElement exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callRemoveElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callMoveElement(String instanceId, String ref, String parentref, int index) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref) || TextUtils.isEmpty(parentref)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callMoveElement arguments is null");
      }
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callMoveElement",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callMoveElement >>>> instanceId:").append(instanceId)
              .append(", parentref:").append(parentref)
              .append(", index:").append(index)
              .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        final BasicGraphicAction action = new GraphicActionMoveElement(instance, ref, parentref, index);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callMoveElement exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callMoveElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddEvent(String instanceId, String ref, String event) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref) || TextUtils.isEmpty(event)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callAddEvent arguments is null");
      }
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callAddEvent",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callAddEvent >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", event:").append(event);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        new GraphicActionAddEvent(instance, ref, event).executeActionOnRender();
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddEvent exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callAddEvent",
              WXLogUtils.getStackTrace(e), null);
    }

    // get next tick
    getNextTick(instanceId);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveEvent(String instanceId, String ref, String event) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref) || TextUtils.isEmpty(event)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callRemoveEvent arguments is null");
      }
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callRemoveEvent",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callRemoveEvent >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", event:").append(event);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        new GraphicActionRemoveEvent(instance, ref, event).executeActionOnRender();
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveEvent exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callRemoveEvent",
              WXLogUtils.getStackTrace(e), null);
    }

    // get next tick
    getNextTick(instanceId);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callUpdateStyle(String instanceId, String ref, HashMap<String, Object> styles,
                             HashMap<String, String> paddings,
                             HashMap<String, String> margins,
                             HashMap<String, String> borders) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callUpdateStyle arguments is null");
      }

      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callUpdateStyle",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callUpdateStyle >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref).append(", styles:").append(styles.toString())
              .append(", paddings:").append(paddings.toString())
                      .append(", margins:").append(margins.toString())
                              .append(", borders:").append(borders.toString());
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        final BasicGraphicAction action = new GraphicActionUpdateStyle(instance, ref, styles, paddings, margins, borders);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateStyle exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callUpdateStyle",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callUpdateAttrs(String instanceId, String ref, HashMap<String, String> attrs) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)) {
      if (WXEnvironment.isApkDebugable()){
        WXLogUtils.d("[WXBridgeManager] call callUpdateAttrs arguments is null");
      }

      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callUpdateAttrs",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callUpdateAttrs >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref).append(", attrs:").append(attrs.toString());
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        final BasicGraphicAction action = new GraphicActionUpdateAttr(instance, ref, attrs);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateAttrs exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callUpdateAttrs",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callLayout(String pageId, String ref, int top, int bottom, int left, int right, int height, int width, boolean isRTL, int index) {

    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(ref)) {
        if (WXEnvironment.isApkDebugable()){
            WXLogUtils.d("[WXBridgeManager] call callLayout arguments is null");
        }

      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callLayout",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callLayout >>>> instanceId:").append(pageId)
            .append(", ref:").append(ref).append(", height:").append(height).append(", width:").append(width)
              .append(", top:").append(top)
              .append(", bottom:").append(bottom)
              .append(", left:").append(left)
              .append(", right:").append(right);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(pageId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(pageId);
      if (instance != null) {
        GraphicSize size = new GraphicSize(width, height);
        GraphicPosition position = new GraphicPosition(left, top, right, bottom);
        GraphicActionAddElement addAction = instance.getInActiveAddElementAction(ref);
        if(addAction!=null) {
          addAction.setRTL(isRTL);
          addAction.setSize(size);
          addAction.setPosition(position);
          if(!TextUtils.equals(ref, WXComponent.ROOT)) {
            addAction.setIndex(index);
          }
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(pageId, addAction);
          instance.removeInActiveAddElmentAction(ref);
        }
        else {
          final BasicGraphicAction action = new GraphicActionLayout(instance, ref, position, size, isRTL);
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callLayout exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callLayout",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAppendTreeCreateFinish(String instanceId, String ref) {

    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)) {
      WXLogUtils.d("[WXBridgeManager] call callAppendTreeCreateFinish arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callAppendTreeCreateFinish",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callAppendTreeCreateFinish >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      GraphicActionAppendTreeCreateFinish action = new GraphicActionAppendTreeCreateFinish(instance, ref);
      WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAppendTreeCreateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callAppendTreeCreateFinish",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callCreateFinish(String instanceId) {

    if (TextUtils.isEmpty(instanceId)) {
      WXLogUtils.d("[WXBridgeManager] call callCreateFinish arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callCreateFinish",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callCreateFinish >>>> instanceId:").append(instanceId);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      long start = System.currentTimeMillis();
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        instance.firstScreenCreateInstanceTime(start);
        GraphicActionCreateFinish action = new GraphicActionCreateFinish(instance);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callCreateFinish",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRenderSuccess(String instanceId) {

    if (TextUtils.isEmpty(instanceId)) {
      WXLogUtils.d("[WXBridgeManager] call callRenderSuccess arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_RENDER_ERR_BRIDGE_ARG_NULL, "callRenderSuccess",
              "arguments is empty, INSTANCE_RENDERING_ERROR will be set", null);
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callRenderSuccess >>>> instanceId:").append(instanceId);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        GraphicActionRenderSuccess action = new GraphicActionRenderSuccess(instance);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRenderSuccess exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_BRIDGE, "callRenderSuccess",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public ContentBoxMeasurement getMeasurementFunc(String instanceId, long renderObjectPtr) {
    ContentBoxMeasurement contentBoxMeasurement = null;
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null) {
      contentBoxMeasurement = instance.getContentBoxMeasurement(renderObjectPtr);
    }
    return contentBoxMeasurement;
  }

  public void bindMeasurementToRenderObject(long ptr){
    if (isJSFrameworkInit()) {
      mWXBridge.bindMeasurementToRenderObject(ptr);
    }
  }


  /**
   * Native: Layout
   * @param instanceId
   * @return
   */
  @UiThread
  public boolean notifyLayout(String instanceId) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      return mWXBridge.notifyLayout(instanceId);
    }
    return false;
  }

  @UiThread
  public void forceLayout(String instanceId) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.forceLayout(instanceId);
    }
  }

  /**
   * native: OnInstanceClose
   * should be called on JSThread
   * @param instanceId
   */
  public void onInstanceClose(String instanceId) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.onInstanceClose(instanceId);
    }
  }

  /**
   * native: SetDefaultHeightAndWidthIntoRootDom
   * @param instanceId
   * @param defaultWidth
   * @param defaultHeight
   */
  public void setDefaultRootSize(final String instanceId, final float defaultWidth, final float defaultHeight, final boolean isWidthWrapContent, final boolean isHeightWrapContent) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setDefaultHeightAndWidthIntoRootDom(instanceId, defaultWidth, defaultHeight, isWidthWrapContent, isHeightWrapContent);
    }
  }

  public void setRenderContentWrapContentToCore(boolean wrap, final String instanceId) {
    if (isJSFrameworkInit()) {
      mWXBridge.setRenderContainerWrapContent(wrap, instanceId);
    }
  }

  public void setStyleWidth(String instanceId, String ref, float value) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setStyleWidth(instanceId, ref, value);
    }
  }

  public void setStyleHeight(String instanceId, String ref, float value) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setStyleHeight(instanceId, ref, value);
    }
  }

  public long[] getFirstScreenRenderTime(String instanceId) {
    if (isJSFrameworkInit()) {
      return mWXBridge.getFirstScreenRenderTime(instanceId);
    }
    return new long[]{0, 0, 0};
  }

  public long[] getRenderFinishTime(String instanceId) {
    if (isJSFrameworkInit()) {
      return mWXBridge.getRenderFinishTime(instanceId);
    }
    return new long[]{0, 0, 0};
  }

  public void setDeviceDisplay(final String instanceId, final float deviceWidth, final float deviceHeight, final float scale) {
    post(new Runnable() {
      @Override
      public void run() {
        mWXBridge.setDeviceDisplay(instanceId, deviceWidth, deviceHeight, scale);
      }
    });
  }

  public void updateInitDeviceParams(final String width, final String height, final String density, final String statusHeight){
    if(!isJSFrameworkInit()){
      return;
    }
    post(new Runnable() {
      @Override
      public void run() {
        mWXBridge.updateInitFrameworkParams(WXConfig.deviceWidth, width, WXConfig.deviceWidth);
      }
    });
    post(new Runnable() {
      @Override
      public void run() {
        mWXBridge.updateInitFrameworkParams(WXConfig.deviceHeight, height, WXConfig.deviceHeight);
      }
    });

    post(new Runnable() {
      @Override
      public void run() {
        mWXBridge.updateInitFrameworkParams(WXConfig.scale,  density, WXConfig.scale);
      }
    });

    if(statusHeight != null){
      post(new Runnable() {
        @Override
        public void run() {
          mWXBridge.updateInitFrameworkParams(WXConfig.androidStatusBarHeight,  statusHeight, WXConfig.androidStatusBarHeight);
        }
      });
    }
  }

  public void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setMargin(instanceId, ref, edge, value);
    }
  }

  public void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setPadding(instanceId, ref, edge, value);
    }
  }

  public void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.setPosition(instanceId, ref, edge, value);
    }
  }

  public void markDirty(String instanceId, String ref, boolean dirty) {
    if (isSkipFrameworkInit(instanceId) || isJSFrameworkInit()) {
      mWXBridge.markDirty(instanceId, ref, dirty);
    }
  }

  public int callHasTransitionPros(String instanceId, String ref, HashMap<String, String> styles) {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(instanceId, ref);
    if (null == component || null == component.getTransition() || null == component.getTransition().getProperties()) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    for(String property : component.getTransition().getProperties()){
      if(styles.containsKey(property)){
        return IWXBridge.INSTANCE_RENDERING;
      }
    }
    return IWXBridge.INSTANCE_RENDERING_ERROR;
  }

  public void registerCoreEnv(String key, String value) {
    if (isJSFrameworkInit())
      mWXBridge.registerCoreEnv(key, value);
    else
      mWeexCoreEnvOptions.put(key, value);
  }

  private void onJsFrameWorkInitSuccees() {
    for (Map.Entry<String, String> entry : mWeexCoreEnvOptions.entrySet()) {
      mWXBridge.registerCoreEnv(entry.getKey(), entry.getValue());
    }
    mWeexCoreEnvOptions.clear();
  }
}
