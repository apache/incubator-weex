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
import android.view.View;
import android.view.ViewGroup;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.common.IWXDebugProxy;
import com.taobao.weex.common.WXConfig;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXException;
import com.taobao.weex.common.WXJSBridgeMsgType;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.ui.WXComponentRegistry;
import com.taobao.weex.ui.component.WXComponent;
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
import com.taobao.weex.ui.action.GraphicActionUpdateAttr;
import com.taobao.weex.ui.action.GraphicActionUpdateStyle;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.module.WXDomModule;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXWsonJSONSwitch;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import com.taobao.weex.utils.batch.BactchExecutor;
import com.taobao.weex.utils.batch.Interceptor;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.lang.reflect.Constructor;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

import static com.taobao.weex.bridge.WXModuleManager.createDomModule;

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

  public static final String METHOD_CREATE_INSTANCE = "createInstance";
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
  public static final String METHOD_NOTIFY_TRIM_MEMORY = "notifyTrimMemory";
  public static final String METHOD_NOTIFY_SERIALIZE_CODE_CACHE =
          "notifySerializeCodeCache";

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

  private enum BundType {
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
  private IWXDebugProxy mWxDebugProxy;

  private boolean mMock = false;

  private List<Map<String, Object>> mRegisterComponentFailList = new ArrayList<>(8);
  private List<Map<String, Object>> mRegisterModuleFailList = new ArrayList<>(8);
  private List<String> mRegisterServiceFailList = new ArrayList<>(8);
  private List<String> mDestroyedInstanceId = new ArrayList<>();
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

  public void setSandBoxContext(final boolean flag) {
    if (flag != isSandBoxContext) {
      isSandBoxContext = flag;
      // use diff context reinit jsf
      if (isJSThread()) {

        setJSFrameworkInit(false);
        WXModuleManager.resetAllModuleState();
        String jsf = "";
        if (!isSandBoxContext || WXEnvironment.sDebugServerConnectable) {
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
            if (!isSandBoxContext || WXEnvironment.sDebugServerConnectable) {
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

    if (mWxDebugProxy != null) {
      mWxDebugProxy.stop(false);
    }
    if (WXEnvironment.sDebugServerConnectable && (WXEnvironment.isApkDebugable() || WXEnvironment.sForceEnableDevTool)) {
      if (WXEnvironment.getApplication() != null) {
        try {
          Class clazz = Class.forName("com.taobao.weex.devtools.debug.DebugServerProxy");
          if (clazz != null) {
            Constructor constructor = clazz.getConstructor(Context.class, WXBridgeManager.class);
            if (constructor != null) {
              mWxDebugProxy = (IWXDebugProxy) constructor.newInstance(
                      WXEnvironment.getApplication(), WXBridgeManager.this);
              if (mWxDebugProxy != null) {
                mWxDebugProxy.start(new WXJsFunctions());
              }
            }
          }
        } catch (Throwable e) {
          //Ignore, It will throw Exception on Release environment
        }
        WXServiceManager.execAllCacheJsService();
      } else {
        WXLogUtils.e("WXBridgeManager", "WXEnvironment.sApplication is null, skip init Inspector");
        WXLogUtils.w("WXBridgeManager", new Throwable("WXEnvironment.sApplication is null when init Inspector"));
      }
    }
    if (remoteDebug && mWxDebugProxy != null) {
      mWXBridge = mWxDebugProxy.getWXBridge();
    } else {
      mWXBridge = new WXBridge();
    }
  }

  public void stopRemoteDebug() {
    if (mWxDebugProxy != null) {
      mWxDebugProxy.stop(true);
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
    return WXModuleManager.callModuleMethod(instanceId, moduleStr, methodStr, args);
  }

  /**
   * Model switch. For now, debug model and release model are supported
   */
  public void restart() {
    setJSFrameworkInit(false);
    WXModuleManager.resetAllModuleState();
    initWXBridge(WXEnvironment.sRemoteDebugMode);
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
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callNativeModule",
              err, null);
    }

    return null;
  }

  public Object callNativeModule(String instanceId, String module, String method, JSONArray arguments, JSONObject options) {

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
      String err = "[WXBridgeManager] callNative exception: " + WXLogUtils.getStackTrace(e);
      WXLogUtils.e(err);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callNativeModule",
              err, null);
    }

    return null;
  }

  public Object callNativeComponent(String instanceId, String componentRef, String method, JSONArray arguments, Object options) {
    if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("[WXBridgeManager] callNativeComponent >>>> instanceId:").append(instanceId)
              .append(", componentRef:").append(componentRef).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    try {
      WXDomModule dom = WXModuleManager.getDomModule(instanceId);
      dom.invokeMethod(componentRef, method, arguments);
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callNativeComponent",
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
    if (tasks == null) {
      String err = "[WXBridgeManager] callNative: call Native tasks is null";
      WXLogUtils.e(err);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callNative",
              err + "| INSTANCE_RENDERING_ERROR will be set", null);

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
                WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callNative",
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
//        WXDomModule domModule = getDomModule(instanceId);
//        Action action = Actions.getUpdateFinish();
//        domModule.postAction((DOMAction) action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callUpdateFinish",
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
          GraphicActionRefreshFinish action = new GraphicActionRefreshFinish(instanceId);
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
        }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRefreshFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callRefreshFinish",
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
    try {
      String url = null;
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      if (instance != null) {
        url = instance.getBundleUrl();
      }
      try {
        if (WXEnvironment.getApplication() != null) {
          crashFile = WXEnvironment.getApplication().getApplicationContext().getCacheDir().getPath() + crashFile;
          // Log.e("jsengine", "callReportCrashReloadPage crashFile:" + crashFile);
        }
      } catch (Throwable e) {
        e.printStackTrace();
      }
      callReportCrash(crashFile, instanceId, url);
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
        boolean reloadThisInstance = shouReloadCurrentInstance(
                WXSDKManager.getInstance().getSDKInstance(instanceId).getBundleUrl());
        new ActionReloadPage(instanceId, reloadThisInstance).executeAction();
      }

    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callReloadPage exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callReloadPage",
              WXLogUtils.getStackTrace(e), null);
    }
    return IWXBridge.INSTANCE_RENDERING_ERROR;
  }

  public boolean shouReloadCurrentInstance(String aUrl) {
    long time = System.currentTimeMillis();
    if (crashUrl == null ||
            (crashUrl != null && !crashUrl.equals(aUrl)) ||
            ((time - lastCrashTime) > 15000)) {
      crashUrl = aUrl;
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
          byte[] taskResult = invokeExecJSWithResult(String.valueOf(instanceId), null, METHOD_CALL_JS, jsArgs, true);
          if(eventCallback == null){
            return;
          }
          if(taskResult != null){
            JSONArray arrayResult = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(taskResult);
            if(arrayResult != null && arrayResult.size() > 0){
              result = arrayResult.get(0);
            }
          }
          eventCallback.onCallback(result);
          jsArgs[0] = null;
          taskResult = null;
          jsArgs = null;
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
    if(callback == null) {
      addJSEventTask(METHOD_FIRE_EVENT, instanceId, params, ref, type, data, domChanges);
      sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
    }else{
      asyncCallJSEventWithResult(callback, METHD_FIRE_EVENT_SYNC, instanceId, params, ref, type, data, domChanges);
    }
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

    addJSTask(METHOD_CALLBACK, instanceId, callback, data, keepAlive);
    sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
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
      if (!isJSFrameworkInit()) {
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
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
      invokeExecJS(instanceId, null, METHOD_REFRESH_INSTANCE, args);
      WXLogUtils.renderPerformanceLog("invokeRefreshInstance", System.currentTimeMillis() - start);
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeRefreshInstance " + WXLogUtils.getStackTrace(e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "invokeRefreshInstance",
              err, null);
      WXLogUtils.e(err);
    }
  }

  public void commitJscCrashAlarmMonitor(final String type, final WXErrorCode errorCode, String errMsg,
                                         String instanceId, String url) {
    if (TextUtils.isEmpty(type) || errorCode == null) {
      return;
    }

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

  /**
   * Create instance.
   */
  public void createInstance(final String instanceId, final String template,
                             final Map<String, Object> options, final String data) {
    final WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance == null) {
      WXLogUtils.e("WXBridgeManager", "createInstance failed, SDKInstance is not exist");
      return;
    }
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(template) || mJSHandler == null) {
      instance.onRenderError(
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
              WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() +
                      " instanceId==" + instanceId + " template ==" + template + " mJSHandler== " + mJSHandler.toString()
      );
      return;
    }

    if (!isJSFrameworkInit() && reInitCount == 1 && !WXEnvironment.sDebugServerConnectable) {
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
    post(new Runnable() {
      @Override
      public void run() {
        long start = System.currentTimeMillis();
        invokeCreateInstance(instance, template, options, data);
        instance.getWXPerformance().callCreateInstanceTime = System.currentTimeMillis();
        long totalTime =  instance.getWXPerformance().callCreateInstanceTime - start;
        if (totalTime > 0) {
          instance.getWXPerformance().communicateTime = totalTime;
        }
      }
    }, instanceId);
  }

  private void invokeCreateInstance(@NonNull WXSDKInstance instance, String template,
                                    Map<String, Object> options, String data) {
    // add for sandbox, will delete on sandbox ok
    initFramework("");

    if (mMock) {
      mock(instance.getInstanceId());
    } else {
      if (!isJSFrameworkInit()) {
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
          type = getBundleType(instance.getBundleUrl(), template);

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
          }
          if (options.get("env") == null) {
            options.put("env", mInitParams);
          }
        } catch (Throwable e) {
          e.printStackTrace();
        }

        if (WXEnvironment.isApkDebugable() && BRIDGE_LOG_SWITCH) {
          WXLogUtils.d("createInstance >>>> instanceId:" + instance.getInstanceId()
                  + ", options:"
                  + WXJsonUtils.fromObjectToJSONString(options)
                  + ", data:" + data);
        }
        WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
                instance.getInstanceId());
        WXJSObject instanceObj = new WXJSObject(WXJSObject.String,
                template);
        WXJSObject optionsObj = new WXJSObject(WXJSObject.JSON,
                options == null ? "{}"
                        : WXJsonUtils.fromObjectToJSONString(options));
        optionsObj = optionObjConvert(isSandBoxContext, type, optionsObj);
        WXJSObject dataObj = new WXJSObject(WXJSObject.JSON,
                data == null ? "{}" : data);

        WXJSObject apiObj;
        if (type == BundType.Rax) {
          if (mRaxApi == null) {
            mRaxApi =  WXFileUtils.loadAsset("weex-rax-api.js", WXEnvironment.getApplication());
          }
          apiObj = new WXJSObject(WXJSObject.String,
                  mRaxApi);
        } else {
          apiObj = new WXJSObject(WXJSObject.String,
                  "");
        }

        WXJSObject[] args = {instanceIdObj, instanceObj, optionsObj,
                dataObj, apiObj};

        instance.setTemplate(template);

        // if { "framework": "Vue" } or  { "framework": "Rax" } will use invokeCreateInstanceContext
        // others will use invokeExecJS
        if (!isSandBoxContext || WXEnvironment.sDebugServerConnectable) {
          invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
          return;
        }
        if (type == BundType.Vue || type == BundType.Rax) {
          invokeCreateInstanceContext(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
          return;
        } else {
          invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
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
    if (!useSandBox || type == BundType.Others || WXEnvironment.sDebugServerConnectable) {
      return opt;
    }
    try {
      String data = opt.data.toString();
      JSONObject obj = JSON.parseObject(data);
      if (obj.getJSONObject("env") != null) {
        JSONObject optEnv = obj.getJSONObject("env");
        // obj.replace()
        if (optEnv != null) {
          JSONObject opts = optEnv.getJSONObject("options");
          if (opts!= null) {
            optEnv.remove("options");
            Set<String> set = opts.keySet();
            for(Iterator it = set.iterator(); it.hasNext();) {
              String key = it.next().toString();
              optEnv.put(key, opts.getString(key));
            }
          }
        }
        obj.remove("env");
        obj.put("env", optEnv);
      }
      WXJSObject optionsObj = new WXJSObject(WXJSObject.JSON, obj.toString());
      return optionsObj;
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
        if (temp.startsWith("// { \"framework\": \"Vue\" }") ||
                temp.startsWith("// { \"framework\": \"vue\" }") ||
                temp.startsWith("// {\"framework\" : \"Vue\"}") ||
                temp.startsWith("// {\"framework\" : \"vue\"}")) {
          return BundType.Vue;
        } else if (temp.startsWith("// { \"framework\": \"Rax\" }") ||
                temp.startsWith("// { \"framework\": \"rax\" }")
                || temp.startsWith("// {\"framework\" : \"Rax\"}") ||
                temp.startsWith("// {\"framework\" : \"rax\"}")) {
          return BundType.Rax;
        } else {
          if (temp.length() > 500) {
            temp = temp.substring(0, 500);
          }
          String strTrim = temp.replaceAll("\n","").trim();
          if (strTrim.startsWith("// { \"framework\": \"Vue\" }") ||
                  strTrim.startsWith("// { \"framework\": \"vue\" }") ||
                  strTrim.startsWith("// {\"framework\" : \"Vue\"}") ||
                  strTrim.startsWith("// {\"framework\" : \"vue\"}")) {
            return BundType.Vue;
          } else if (strTrim.startsWith("// { \"framework\": \"Rax\" }") ||
                  strTrim.startsWith("// { \"framework\": \"rax\" }")
                  || strTrim.startsWith("// {\"framework\" : \"Rax\"}") ||
                  strTrim.startsWith("// {\"framework\" : \"rax\"}")) {
            return BundType.Rax;
          }

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
      if (isJSFrameworkInit()) {
        invokeDestoryInstance(instanceId, null, METHOD_DESTROY_INSTANCE, args, true);
        // invokeExecJS(instanceId, null, METHOD_DESTROY_INSTANCE, args);
      }
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeDestroyInstance " + e.getCause();
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "invokeDestroyInstance", err, null);
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
    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
              .append("function:").append(function);
      if (logTaskDetail) {
        mLodBuilder.append(" tasks:").append(argsToJSON(args));
      }
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    final long start = System.currentTimeMillis();
    mWXBridge.execJS(instanceId, namespace, function, args);
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (null != instance){
      instance.callJsTime(System.currentTimeMillis()-start);
    }
  }

  public void invokeCreateInstanceContext(String instanceId, String namespace, String function,
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
    mWXBridge.createInstanceContext(instanceId, namespace, function, args);
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
    return mWXBridge.execJSOnInstance(instanceId, js, type);
  }

  private byte[] invokeExecJSWithResult(String instanceId, String namespace, String function,
                                        WXJSObject[] args,boolean logTaskDetail){
    if (WXEnvironment.isOpenDebugLog() && BRIDGE_LOG_SWITCH) {
      mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
              .append("function:").append(function);
      if(logTaskDetail) {
        mLodBuilder.append(" tasks:").append(argsToJSON(args));
      }
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    return  mWXBridge.execJSWithResult(instanceId, namespace, function, args);
  }

  public @NonNull static String argsToJSON(WXJSObject[] args) {
    StringBuilder builder = new StringBuilder();
    builder.append("[");
    for(WXJSObject object : args){
      builder.append(WXJsonUtils.fromObjectToJSONString(object));
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

  private void initFramework(String framework) {
    if (!isJSFrameworkInit()) {
      if (TextUtils.isEmpty(framework)) {
        // if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("weex JS framework from assets");
        // }
        if (!isSandBoxContext || WXEnvironment.sDebugServerConnectable) {
          framework = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
        } else {
          framework = WXFileUtils.loadAsset("weex-main-jsfm.js", WXEnvironment.getApplication());
        }
      }
      if (TextUtils.isEmpty(framework)) {
        setJSFrameworkInit(false);
        WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
                "initFramework", "framework is empty!! ", null);
        return;
      }
      try {
        if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
          WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkStart();
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
        WXLogUtils.d("[WXBridgeManager] initFrameworkEnv crashFile:" + crashFile + " pieSupport:" + pieSupport);
        // extends initFramework
        if (mWXBridge.initFrameworkEnv(framework, assembleDefaultOptions(), crashFile, pieSupport) == INIT_FRAMEWORK_OK) {
          WXEnvironment.sJSLibInitTime = System.currentTimeMillis() - start;
          WXLogUtils.renderPerformanceLog("initFramework", WXEnvironment.sJSLibInitTime);
          WXEnvironment.sSDKInitTime = System.currentTimeMillis() - WXEnvironment.sSDKInitStart;
          WXLogUtils.renderPerformanceLog("SDKInitTime", WXEnvironment.sSDKInitTime);
          setJSFrameworkInit(true);

          if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
            WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkReady();
          }

          execRegisterFailTask();
          WXEnvironment.JsFrameworkInit = true;
          registerDomModule();
          trackComponentAndModulesTime();
          String reinitInfo = "";
          if (reInitCount > 1) {
            reinitInfo = "reinit Framework:";
//            WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_JS_FRAMEWORK_REINIT_SUCCESS,
//                    "initFramework", reinitInfo + "success", null);
          } else {
//            WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_JS_FRAMEWORK_INIT_SUCCESS,
//                    "initFramework", reinitInfo + "success", null);
          }
        } else {
          if (reInitCount > 1) {
            WXLogUtils.e("[WXBridgeManager] invokeReInitFramework  ExecuteJavaScript fail");
            String err = "[WXBridgeManager] invokeReInitFramework  ExecuteJavaScript fail reinit FrameWork";
//            WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
//                    "initFramework", "reInitCount = " + reInitCount + err, null);
          } else {
            WXLogUtils.e("[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail");
            String err = "[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail";
//            WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
//                    "initFramework", err, null);
          }
        }
      } catch (Throwable e) {
        if (reInitCount > 1) {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
          String err = "[WXBridgeManager] invokeInitFramework reinit FrameWork exception!#" + e.toString();
//          WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
//                  "initFramework", "reInitCount ==" + reInitCount + err, null);
        } else {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
          String err = "[WXBridgeManager] invokeInitFramework exception!#" + e.toString();
//          WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK,
//                  "initFramework", err, null);
        }
      }

    }
  }

  private void trackComponentAndModulesTime() {
    post(new Runnable() {
      @Override
      public void run() {
        WXEnvironment.sComponentsAndModulesReadyTime = System.currentTimeMillis() - WXEnvironment.sSDKInitStart;
        WXLogUtils.renderPerformanceLog("ComponentModulesReadyTime", WXEnvironment.sComponentsAndModulesReadyTime);
        WXLogUtils.d("ComponentModulesReadyTime " + WXEnvironment.sComponentsAndModulesReadyTime);
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
      task = ((ArrayList) task).toArray();

      WXJSObject[] args = {
              new WXJSObject(WXJSObject.String, instanceId),
              WXWsonJSONSwitch.toWsonOrJsonWXJSObject(task)};

      invokeExecJS(String.valueOf(instanceId), null, METHOD_CALL_JS, args);

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
    wxParams.setUseSingleProcess(isUseSingleProcess ? "true" : "false");
    wxParams.setLibJssPath(WXEnvironment.getLibJssRealPath());
    String appName = config.get(WXConfig.appName);
    if (!TextUtils.isEmpty(appName)) {
      wxParams.setAppName(appName);
    }
    wxParams.setDeviceWidth(TextUtils.isEmpty(config.get("deviceWidth")) ? String.valueOf(WXViewUtils.getScreenWidth(WXEnvironment.sApplication)) : config.get("deviceWidth"));
    wxParams.setDeviceHeight(TextUtils.isEmpty(config.get("deviceHeight")) ? String.valueOf(WXViewUtils.getScreenHeight(WXEnvironment.sApplication)) : config.get("deviceHeight"));
    wxParams.setOptions(WXEnvironment.getCustomOptions());
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
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_MODULES, args);
      try {
        Iterator<String> iter = modules.keySet().iterator();
        while (iter.hasNext()) {
          String module = iter.next();
          if (module != null) {
            WXModuleManager.resetModuleState(module, true);
            WXLogUtils.e("[WXBridgeManager]invokeRegisterModules METHOD_REGISTER_MODULES success module:" + module);
          }
        }
      } catch (Throwable e) {
        WXLogUtils.e("Weex [invokeRegisterModules]", e);
      }
    } catch (Throwable e) {
      WXExceptionUtils.commitCriticalExceptionRT(null,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES,
              "invokeRegisterModules", WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES.getErrorMsg() +
                      " \n " + e.getMessage() + modules.entrySet().toString(),
              null );

      WXLogUtils.e("[WXBridgeManager] invokeRegisterModules:", e);
    }
  }

  private void invokeRegisterComponents(List<Map<String, Object>> components, List<Map<String, Object>> failReceiver) {
    if (components == failReceiver) {
      throw new RuntimeException("Fail receiver should not use source.");
    }
    if (!isJSFrameworkInit()) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents: framework.js uninitialized.");

      for (Map<String, Object> comp : components) {
        failReceiver.add(comp);
      }
      return;
    }
    if (components == null) {
      return;
    }

    WXJSObject[] args = {WXWsonJSONSwitch.toWsonOrJsonWXJSObject(components)};
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_COMPONENTS, args);
    } catch (Throwable e) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents ", e);
      WXExceptionUtils.commitCriticalExceptionRT(null,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED,
              METHOD_REGISTER_COMPONENTS,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED
                      + args.toString()
                      + WXLogUtils.getStackTrace(e),
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
    if (instanceId != null && (instance = WXSDKManager.getInstance().getSDKInstance(instanceId)) != null) {
      exception +=   "\n getTemplateInfo==" +instance.getTemplateInfo();//add network header info
      if (METHOD_CREATE_INSTANCE.equals(function)) {
        try {
          if (isJSFrameworkInit() && reInitCount > 1 && !instance.isNeedReLoad()) {
            new ActionReloadPage(instanceId, true).executeAction();
            instance.setNeedLoad(true);
            return;
          } else {
            instance.onRenderError(//DegradPassivity to H5
                    WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                    WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorMsg() +
                            "reportJSException >>>> instanceId:" + instanceId
                            + ", exception function:" + function + ", exception:"
                            + exception
            );
            //doReportJSException(instanceId,function,exception);
            WXExceptionUtils.commitCriticalExceptionRT(instanceId,WXErrorCode.WX_RENDER_ERR_JS_CREATE_INSTANCE,function,exception,null);
            return;
          }
        } catch (Exception e) {
          e.printStackTrace();
        }
      }
      instance.onJSException(WXErrorCode.WX_ERR_JS_EXECUTE.getErrorCode(), function, exception);
    }
    doReportJSException(instanceId,function,exception);
  }

  private void doReportJSException(String instanceId, String function, String exception){
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
    if (adapter != null) {
      String exceptionId = instanceId;

      if (instanceId == "" || instanceId == null) {
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

      if (checkEmptyScreen(instance)){
        if (WXEnvironment.isApkDebugable()){
          WXLogUtils.e("render error 4 js error !");
        }
        WXExceptionUtils.commitCriticalExceptionRT(exceptionId, WXErrorCode.WX_RENDER_ERR_JS_RUNTIME,
                function,
                WXErrorCode.WX_RENDER_ERR_JS_RUNTIME.getErrorMsg() + exception,
                null);
      } else {
        WXExceptionUtils.commitCriticalExceptionRT(exceptionId, WXErrorCode.WX_KEY_EXCEPTION_WXBRIDGE,
                function,
                WXErrorCode.WX_KEY_EXCEPTION_WXBRIDGE.getErrorMsg() + exception,
                null);
      }
    }
  }

  private boolean checkEmptyScreen(WXSDKInstance instance){
    if (null == instance || instance.isDestroy()){
      return false;
    }
    WXComponent rootComponent = instance.getRootComponent();
    if (null == rootComponent) {
      return true;
    }

    View rootView = rootComponent.getRealView();
    if (null == rootView){
      return true;
    }

    if (rootView instanceof ViewGroup) {
      return ((ViewGroup) rootView).getChildCount() > 0;
    }else {
      return false;
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
      WXEnvironment.getCustomOptions().put(GLOBAL_CONFIG_KEY, globalConfig);
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
    long start = System.currentTimeMillis();
    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(componentType) || TextUtils.isEmpty(ref)) {
      // if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("[WXBridgeManager] callCreateBody: call CreateBody tasks is null");
      // }
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callCreateBody",
              "tasks is empty, INSTANCE_RENDERING_ERROR will be set", null);

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
      if (WXSDKManager.getInstance().getSDKInstance(pageId) != null) {
        final BasicGraphicAction action = new GraphicActionCreateBody(pageId, ref, componentType,
                styles, attributes, events, margins, paddings, borders);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateBody exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callCreateBody",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddElement(String pageId, String componentType, String ref, int index, String parentRef,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders,boolean willLayout) {
    long start = System.currentTimeMillis();
    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(componentType) || TextUtils.isEmpty(ref)) {
      WXLogUtils.d("[WXBridgeManager] callAddElement: call CreateBody tasks is null");
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callAddElement",
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
      if (WXSDKManager.getInstance().getSDKInstance(pageId) != null) {
        final GraphicActionAddElement action = new GraphicActionAddElement(pageId, ref, componentType, parentRef, index,
            styles, attributes, events, margins, paddings, borders);
        if(willLayout) {
          WXSDKManager.getInstance().getSDKInstance(pageId).addInActiveAddElementAction(ref, action);
        }else{
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(pageId, action);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddElement exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callAddElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveElement(String instanceId, String ref) {

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
        final BasicGraphicAction action = new GraphicActionRemoveElement(instanceId, ref);
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
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callRemoveElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callMoveElement(String instanceId, String ref, String parentref, int index) {

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
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        final BasicGraphicAction action = new GraphicActionMoveElement(instanceId, ref, parentref, index);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callMoveElement exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callMoveElement",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddEvent(String instanceId, String ref, String event) {

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
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        new GraphicActionAddEvent(instanceId, ref, event).executeActionOnRender();
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddEvent exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callAddEvent",
              WXLogUtils.getStackTrace(e), null);
    }

    // get next tick
    getNextTick(instanceId);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveEvent(String instanceId, String ref, String event) {

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
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        new GraphicActionRemoveEvent(instanceId, ref, event).executeActionOnRender();
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveEvent exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callAddEvent",
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
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        final BasicGraphicAction action = new GraphicActionUpdateStyle(instanceId, ref, styles, paddings, margins, borders);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateStyle exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callUpdateStyle",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callUpdateAttrs(String instanceId, String ref, HashMap<String, String> attrs) {
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
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        final BasicGraphicAction action = new GraphicActionUpdateAttr(instanceId, ref, attrs);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateAttrs exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callUpdateAttrs",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callLayout(String pageId, String ref, int top, int bottom, int left, int right, int height, int width, int index) {
    long start = System.currentTimeMillis();
    if (TextUtils.isEmpty(pageId) || TextUtils.isEmpty(ref)) {
      WXLogUtils.d("[WXBridgeManager] callLayout: call callLayout arguments is null");
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callLayout",
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
      if (WXSDKManager.getInstance().getSDKInstance(pageId) != null) {
        GraphicSize size = new GraphicSize(width, height);
        GraphicPosition position = new GraphicPosition(left, top, right, bottom);
        GraphicActionAddElement addAction = WXSDKManager.getInstance().getSDKInstance(pageId).getInActiveAddElementAction(ref);
        if(addAction!=null) {
          addAction.setSize(size);
          addAction.setPosition(position);
          if(!TextUtils.equals(ref, WXComponent.ROOT)) {
            addAction.setIndex(index);
          }
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(pageId, addAction);
          WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(pageId);
          if(instance != null){
            instance.removeInActiveAddElmentAction(ref);
          }
        }
        else {
          final BasicGraphicAction action = new GraphicActionLayout(pageId, ref, position, size);
          WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callLayout exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(pageId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callLayout",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAppendTreeCreateFinish(String instanceId, String ref) {
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
      GraphicActionAppendTreeCreateFinish action = new GraphicActionAppendTreeCreateFinish(instanceId, ref);
      WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAppendTreeCreateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callAppendTreeCreateFinish",
              WXLogUtils.getStackTrace(e), null);
    }

    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callCreateFinish(String instanceId) {
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
        GraphicActionCreateFinish action = new GraphicActionCreateFinish(instanceId);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(instanceId, action);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateFinish exception: ", e);
      WXExceptionUtils.commitCriticalExceptionRT(instanceId,
              WXErrorCode.WX_KEY_EXCEPTION_INVOKE, "callCreateFinish",
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
    mWXBridge.bindMeasurementToRenderObject(ptr);
  }


  /**
   * Native: Layout
   * @param instanceId
   * @return
   */
  @UiThread
  public boolean notifyLayout(String instanceId) {
    return mWXBridge.notifyLayout(instanceId);
  }

  @UiThread
  public void forceLayout(String instanceId) {
    mWXBridge.forceLayout(instanceId);
  }

  /**
   * native: OnInstanceClose
   * @param instanceId
   */
  public void onInstanceClose(String instanceId) {
    mWXBridge.onInstanceClose(instanceId);
  }

  /**
   * native: SetDefaultHeightAndWidthIntoRootDom
   * @param instanceId
   * @param defaultWidth
   * @param defaultHeight
   */
  public void setDefaultRootSize(final String instanceId, final float defaultWidth, final float defaultHeight, final boolean isWidthWrapContent, final boolean isHeightWrapContent) {
    mWXBridge.setDefaultHeightAndWidthIntoRootDom(instanceId, defaultWidth, defaultHeight, isWidthWrapContent, isHeightWrapContent);
  }

  public void setRenderContentWrapContentToCore(boolean wrap, final String instanceId) {
    mWXBridge.setRenderContainerWrapContent(wrap, instanceId);
  }

  public void setStyleWidth(String instanceId, String ref, float value) {
    mWXBridge.setStyleWidth(instanceId, ref, value);
  }

  public void setStyleHeight(String instanceId, String ref, float value) {
    mWXBridge.setStyleHeight(instanceId, ref, value);
  }

  public long[] getFirstScreenRenderTime(String instanceId) {
    return mWXBridge.getFirstScreenRenderTime(instanceId);
  }

  public long[] getRenderFinishTime(String instanceId) {
    return mWXBridge.getRenderFinishTime(instanceId);
  }

  public void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mWXBridge.setMargin(instanceId, ref, edge, value);
  }

  public void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mWXBridge.setPadding(instanceId, ref, edge, value);
  }

  public void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mWXBridge.setPosition(instanceId, ref, edge, value);
  }

  public void markDirty(String instanceId, String ref, boolean dirty) {
    mWXBridge.markDirty(instanceId, ref, dirty);
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
