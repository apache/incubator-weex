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
import android.os.Build;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXRenderErrorCode;
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
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.WXDomModule;
import com.taobao.weex.dom.action.Action;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.dom.action.TraceableAction;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import com.taobao.weex.utils.batch.BactchExecutor;
import com.taobao.weex.utils.batch.Interceptor;
import com.taobao.weex.wson.Wson;

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
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static com.taobao.weex.bridge.WXModuleManager.getDomModule;
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
  private static final int INIT_FRAMEWORK_OK = 1;
  private static final int CRASHREINIT = 50;
  static volatile WXBridgeManager mBridgeManager;
  private static long LOW_MEM_VALUE = 120;
  private volatile static int reInitCount = 1;
  private static String crashUrl = null;
  private static long lastCrashTime = 0;

  /**
   * Whether JS Framework(main.js) has been initialized.
   */
  private volatile static boolean mInit = false;



  private static String globalConfig = "none";
  private static String GLOBAL_CONFIG_KEY = "global_switch_config";

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

  // setJSFrameworkInit and isJSFrameworkInit may use on diff thread
  // use volatile
  private boolean isJSFrameworkInit() {
      return mInit;
  }

  private void setJSFrameworkInit(boolean init) {
      mInit = init;
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
                mWxDebugProxy.start();
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

    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
          .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    try {
      if (WXDomModule.WXDOM.equals(module)) {
        WXDomModule dom = getDomModule(instanceId);
        return dom.callDomMethod(method, arguments);
      } else {
        return callModuleMethod(instanceId, module,
            method, arguments);
      }
    } catch (Exception e) {
	  String err = "[WXBridgeManager] callNative exception: " + WXLogUtils.getStackTrace(e);
      WXLogUtils.e(err);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callNativeModule",
			  err, null);
    }

    return null;
  }

  public Object callNativeModule(String instanceId, String module, String method, JSONArray arguments, JSONObject options) {

    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
          .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    try {
      if (WXDomModule.WXDOM.equals(module)) {
        WXDomModule dom = getDomModule(instanceId);
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
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callNativeModule",
			  err, null);
    }

    return null;
  }

  public Object callNativeComponent(String instanceId, String componentRef, String method, JSONArray arguments, Object options) {
    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callNativeComponent >>>> instanceId:").append(instanceId)
          .append(", componentRef:").append(componentRef).append(", method:").append(method).append(", arguments:").append(arguments);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    try {

      WXDomModule dom = getDomModule(instanceId);
      dom.invokeMethod(componentRef, method, arguments);

    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callNativeComponent",
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
    if (tasks == null || tasks.isEmpty()) {
	  String err = "[WXBridgeManager] callNative: call Native tasks is null";
      WXLogUtils.e(err);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callNative",
			  err + "| INSTANCE_RENDERING_ERROR will be set", null);

      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isOpenDebugLog()) {
    mLodBuilder.append("[WXBridgeManager] callNative >>>> instanceId:").append(instanceId)
        .append(", tasks:").append(tasks).append(", callback:").append(callback);
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }


    long start = System.currentTimeMillis();
    long parseNanos = System.nanoTime();
    JSONArray array = tasks;
    parseNanos = System.nanoTime() - parseNanos;

    if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
      WXSDKManager.getInstance().getSDKInstance(instanceId).jsonParseTime(System.currentTimeMillis() - start);
    }

    int size = array.size();
    if (size > 0) {
      try {
        JSONObject task;
        for (int i = 0; i < size; ++i) {
          task = (JSONObject) array.get(i);
          if (task != null && WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
            Object target = task.get(MODULE);
            if (target != null) {
              if (WXDomModule.WXDOM.equals(target)) {
                WXDomModule dom = getDomModule(instanceId);
                dom.callDomMethod(task, parseNanos);
              } else {
                JSONObject optionObj = task.getJSONObject(OPTIONS);
                callModuleMethod(instanceId, (String) target,
                    (String) task.get(METHOD), (JSONArray) task.get(ARGS), optionObj);
              }
            } else if (task.get(COMPONENT) != null) {
              //call component
              WXDomModule dom = getDomModule(instanceId);
              dom.invokeMethod((String) task.get(REF), (String) task.get(METHOD), (JSONArray) task.get(ARGS));
            } else {
              throw new IllegalArgumentException("unknown callNative");
            }
          }
        }
      } catch (Exception e) {
        WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
		WXExceptionUtils.commitCriticalExceptionRT(instanceId,
				WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callNative",
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

  // callCreateBody
  public int callCreateBody(String instanceId, String tasks, String callback) {
    if (TextUtils.isEmpty(tasks)) {
      // if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("[WXBridgeManager] callCreateBody: call CreateBody tasks is null");
      // }
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callCreateBody",
			  "tasks is empty, INSTANCE_RENDERING_ERROR will be set", null);

	  return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isOpenDebugLog()) {
    mLodBuilder.append("[WXBridgeManager] callCreateBody >>>> instanceId:").append(instanceId)
        .append(", tasks:").append(tasks).append(", callback:").append(callback);
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    }


    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        long start = System.currentTimeMillis();
        long nanosTemp = System.nanoTime();
        JSONObject domObject = JSON.parseObject(tasks);
        nanosTemp = System.nanoTime() - nanosTemp;

        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getCreateBody(domObject);
        domModule.postAction((DOMAction) action, true);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mParseJsonNanos = nanosTemp;
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).onStartDomExecute(instanceId, "createBody", "_root", domObject.getString("type"), tasks);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateBody exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callCreateBody",
			  WXLogUtils.getStackTrace(e), null);
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
    if (WXEnvironment.isOpenDebugLog()) {
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
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getUpdateFinish();
        domModule.postAction((DOMAction) action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateFinish exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateFinish",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  // callCreateFinish
  public int callCreateFinish(String instanceId, String callback) {
    if (WXEnvironment.isOpenDebugLog()) {
    mLodBuilder.append("[WXBridgeManager] callCreateFinish >>>> instanceId:").append(instanceId)
        .append(", callback:").append(callback);
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
     }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getCreateFinish();
        domModule.postAction((DOMAction) action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateFinish exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateFinish",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;

  }

  // callRefreshFinish
  public int callRefreshFinish(String instanceId, String callback) {
    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callRefreshFinish >>>> instanceId:").append(instanceId)
          .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRefreshFinish();
        domModule.postAction((DOMAction) action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRefreshFinish exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callRefreshFinish",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;

  }

  // callUpdateAttrs
  public int callUpdateAttrs(String instanceId, String ref, String task, String callback) {
    if (TextUtils.isEmpty(task)) {
      WXLogUtils.e("[WXBridgeManager] callUpdateAttrs: call UpdateAttrs tasks is null");
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateAttrs",
			  "call UpdateAttrs tasks is null| INSTANCE_RENDERING_ERROR will be set", null);

      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callUpdateAttrs >>>> instanceId:").append(instanceId)
          .append(", ref:").append(ref)
          .append(", task:").append(task)
          .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        long start = System.currentTimeMillis();
        long parseNanos = System.nanoTime();
        JSONObject domObject = JSON.parseObject(task);
        parseNanos = System.nanoTime() - parseNanos;

        Action action = Actions.getUpdateAttrs(ref, domObject);
        domModule.postAction((DOMAction) action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).mParseJsonNanos = parseNanos;
          ((TraceableAction) action).onStartDomExecute(instanceId, "updateAttrs", domObject.getString("ref"), domObject.getString("type"), task);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateAttrs exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateAttrs",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;

  }

  // callUpdateStyle
  public int callUpdateStyle(String instanceId, String ref, String task, String callback) {
    if (TextUtils.isEmpty(task)) {
      WXLogUtils.e("[WXBridgeManager] callUpdateStyle: call UpdateStyle tasks is null");
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateStyle",
			  "task is empty", null);
	  return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
   if (WXEnvironment.isOpenDebugLog()) {
    mLodBuilder.append("[WXBridgeManager] callUpdateStyle >>>> instanceId:").append(instanceId)
        .append(", ref:").append(ref)
        .append(", task:").append(task)
        .append(", callback:").append(callback);
    WXLogUtils.d(mLodBuilder.substring(0));
    mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        long start = System.currentTimeMillis();
        long nanosTemp = System.nanoTime();
        JSONObject domObject = JSON.parseObject(task);
        nanosTemp = System.nanoTime() - nanosTemp;

        Action action = Actions.getUpdateStyle(ref, domObject, false);
        domModule.postAction((DOMAction) action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mParseJsonNanos = nanosTemp;
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).onStartDomExecute(instanceId, "updateStyle", ref, domObject.getString("type"), domObject.toJSONString());
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateStyle exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callUpdateStyle",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  // callUpdateStyle
  public int callRemoveElement(String instanceId, String ref, String callback) {

    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callRemoveElement >>>> instanceId:").append(instanceId)
          .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRemoveElement(ref);
        domModule.postAction((DOMAction) action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "removeElement", ref, null, ref);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveElement exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callRemoveElement",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  // callMoveElement
  public int callMoveElement(String instanceId, String ref, String parentref, String index, String callback) {

    if (WXEnvironment.isOpenDebugLog()) {
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
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getMoveElement(ref, parentref, Integer.parseInt(index));
        domModule.postAction((DOMAction) action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callMoveElement exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callMoveElement",
			  WXLogUtils.getStackTrace(e), null);

	}

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddEvent(String instanceId, String ref, String event, String callback) {

   if (WXEnvironment.isOpenDebugLog()) {
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
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getAddEvent(ref, event);
        domModule.postAction((DOMAction) action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "addEvent", ref, null, event);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddEvent exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callAddEvent",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveEvent(String instanceId, String ref, String event, String callback) {

    if (WXEnvironment.isOpenDebugLog()) {
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
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRemoveEvent(ref, event);
        domModule.postAction((DOMAction) action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "removeEvent", ref, null, event);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveEvent exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callAddEvent",
			  WXLogUtils.getStackTrace(e), null);
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddElement(String instanceId, String ref, JSONObject dom, String index, String callback) {

    if (WXEnvironment.isOpenDebugLog()) {
      mLodBuilder.append("[WXBridgeManager] callNative::callAddElement >>>> instanceId:").append(instanceId)
          .append(", ref:").append(ref).append(", dom:").append(dom).append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if (mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }


    if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
      long start = System.currentTimeMillis();
      long nanosTemp = System.nanoTime();
      JSONObject domObject = dom;
      nanosTemp = System.nanoTime() - nanosTemp;

      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXSDKManager.getInstance().getSDKInstance(instanceId).jsonParseTime(System.currentTimeMillis() - start);
      }
      WXDomModule domModule = getDomModule(instanceId);
      DOMAction addElementAction = Actions.getAddElement(domObject, ref, Integer.parseInt(index));
      domModule.postAction(addElementAction, false);

      if (WXTracing.isAvailable() && addElementAction instanceof TraceableAction) {
        ((TraceableAction) addElementAction).mParseJsonNanos = nanosTemp;
        ((TraceableAction) addElementAction).mStartMillis = start;
        ((TraceableAction) addElementAction).onStartDomExecute(instanceId, "addElement", domObject.getString("ref"), domObject.getString("type"), dom.toString());
      }
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
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getReloadPage(instanceId, reloadThisInstance);
        domModule.postAction((DOMAction) action, true);
      }

    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callReloadPage exception: ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "callReloadPage",
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
    // statistic weexjsc process crash
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
                  WXJsonUtils.wsonWXJSObject(tasks)};
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
                  WXJsonUtils.wsonWXJSObject(tasks)};
          byte[] taskResult = invokeExecJSWithResult(String.valueOf(instanceId), null, METHOD_CALL_JS, jsArgs, true);
          if(eventCallback == null){
            return;
          }
          if(taskResult != null){
            JSONArray arrayResult = (JSONArray) WXJsonUtils.parseWson(taskResult);
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
				  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
				  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg()
				  + "invokeRefreshInstance FAILED for JSFrameworkInit FAILED, intance will invoke instance.onRenderError"
		  );
        }
        String err = "[WXBridgeManager] invokeRefreshInstance: framework.js uninitialized.";
        WXLogUtils.e(err);
        return;
      }
      long start = System.currentTimeMillis();
      if (WXEnvironment.isOpenDebugLog()) {
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
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "invokeRefreshInstance",
			  err, null);
      WXLogUtils.e(err);
    }
  }

//  public void commitJSBridgeAlarmMonitor(String instanceId, WXErrorCode errCode, String errMsg) {
//    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
//    if (instance == null || errCode == null) {
//      return;
//    }
//    // TODO: We should move WXPerformance and IWXUserTrackAdapter
//    // into a adapter level.
//    // comment out the line below to prevent commiting twice.
//    //instance.commitUTStab(WXConst.JS_BRIDGE, errCode, errMsg);
//
//    IWXUserTrackAdapter adapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
//    if (adapter == null) {
//      return;
//    }
//    WXPerformance performance = new WXPerformance();
//    performance.args = instance.getBundleUrl();
//    performance.errCode = errCode.getErrorCode();
//    if (errCode != WXErrorCode.WX_SUCCESS) {
//      performance.appendErrMsg(TextUtils.isEmpty(errMsg) ? errCode.getErrorMsg() : errMsg);
//      WXLogUtils.e("wx_monitor", performance.toString());
//    }
//    adapter.commit(WXEnvironment.getApplication(), null, IWXUserTrackAdapter.JS_BRIDGE, performance, instance.getUserTrackParams());
//  }

//  public void commitJSFrameworkAlarmMonitor(final String type, final WXErrorCode errorCode, String errMsg) {
//    if (TextUtils.isEmpty(type) || errorCode == null) {
//      return;
//    }
//    if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
//      WXSDKManager.getInstance().getWXStatisticsListener().onException("0",
//          errorCode.getErrorCode(),
//          TextUtils.isEmpty(errMsg) ? errorCode.getErrorMsg() : errMsg);
//    }
//
//    final IWXUserTrackAdapter userTrackAdapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
//    if (userTrackAdapter == null) {
//      return;
//    }
//    WXPerformance performance = new WXPerformance();
//    performance.errCode = errorCode.getErrorCode();
//    if (errorCode != WXErrorCode.WX_SUCCESS) {
//      performance.appendErrMsg(TextUtils.isEmpty(errMsg) ? errorCode.getErrorMsg() : errMsg);
//      WXLogUtils.e("wx_monitor", performance.toString());
//    }
//    userTrackAdapter.commit(WXEnvironment.getApplication(), null, type, performance, null);
//  }


  public void commitJscCrashAlarmMonitor(final String type, final WXErrorCode errorCode, String errMsg,
                                         String instanceId, String url) {
    if (TextUtils.isEmpty(type) || errorCode == null) {
      return;
    }

    String method = "callReportCrash";
    String exception = "weexjsc process crash and restart exception";
    Map<String, String> extParams = new HashMap<String, String>();
    extParams.put("jscCrashStack", errMsg);
    IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
    if (adapter != null) {
      WXJSExceptionInfo jsException = new WXJSExceptionInfo(instanceId, url, errorCode.getErrorCode(), method, exception, extParams);
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
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg() +
	  " instanceId==" + instanceId + " template ==" + template + " mJSHandler== " + mJSHandler.toString()
	  );
      return;
    }

    if (!isJSFrameworkInit() && reInitCount == 1 && !WXEnvironment.sDebugServerConnectable) {
      instance.onRenderError(
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg() +
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
        final long totalTime = System.currentTimeMillis() - start;
        WXSDKManager.getInstance().postOnUiThread(new Runnable() {

            @Override
            public void run() {
            instance.createInstanceFinished(totalTime);
          }
        }, 0);
      }
    }, instanceId);
  }

  private void invokeCreateInstance(@NonNull WXSDKInstance instance, String template,
                                    Map<String, Object> options, String data) {

    initFramework("");

    if (mMock) {
      mock(instance.getInstanceId());
    } else {
      if (!isJSFrameworkInit()) {
		String err = "[WXBridgeManager] invokeCreateInstance: framework.js uninitialized.";
		instance.onRenderError(
				WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
				WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg()
		);
        WXLogUtils.e(err);
        return;
      }
      try {
        if (WXEnvironment.isOpenDebugLog()) {
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
        WXJSObject dataObj = new WXJSObject(WXJSObject.JSON,
            data == null ? "{}" : data);
        WXJSObject[] args = {instanceIdObj, instanceObj, optionsObj,
            dataObj};
        instance.setTemplate(template);
        invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args, false);
      } catch (Throwable e) {
		String err = "[WXBridgeManager] invokeCreateInstance " + e.getCause()
				+ instance.getTemplateInfo();

        instance.onRenderError(
        		WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
				WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg() + err);
		WXLogUtils.e(err);
      }
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
      if (WXEnvironment.isOpenDebugLog()) {
        WXLogUtils.d("destroyInstance >>>> instanceId:" + instanceId);
      }
      WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
          instanceId);
      WXJSObject[] args = {instanceIdObj};
      if (isJSFrameworkInit()) {
        invokeExecJS(instanceId, null, METHOD_DESTROY_INSTANCE, args);
      }
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeDestroyInstance " + e.getCause();
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE.getErrorCode(), "invokeDestroyInstance", err, null);
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
    mWXBridge.execJS(instanceId, namespace, function, args);
  }

  private byte[] invokeExecJSWithResult(String instanceId, String namespace, String function,
                                       WXJSObject[] args,boolean logTaskDetail){
    if (WXEnvironment.isOpenDebugLog()) {
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
      if(object.type == WXJSObject.WSON){
        object = new WXJSObject(WXJSObject.WSON, WXJsonUtils.parseWson(((byte[]) object.data)));
      }
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
        framework = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
      }
      if (TextUtils.isEmpty(framework)) {
        setJSFrameworkInit(false);
		WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
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
          String reinitInfo = "";
          if (reInitCount > 1) {
            reinitInfo = "reinit Framework:";
          }
        } else {
          if (reInitCount > 1) {
            WXLogUtils.e("[WXBridgeManager] invokeReInitFramework  ExecuteJavaScript fail");
            String err = "[WXBridgeManager] invokeReInitFramework  ExecuteJavaScript fail reinit FrameWork";
			WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
					"initFramework", "reInitCount = " + reInitCount + err, null);
          } else {
            WXLogUtils.e("[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail");
            String err = "[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail";
			WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
					"initFramework", err, null);
          }
        }
      } catch (Throwable e) {
        if (reInitCount > 1) {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
          String err = "[WXBridgeManager] invokeInitFramework reinit FrameWork exception!#" + e.toString();
		  WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
				  "initFramework", "reInitCount ==" + reInitCount + err, null);
        } else {
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
          String err = "[WXBridgeManager] invokeInitFramework exception!#" + e.toString();
		  WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
				  "initFramework", err, null);
        }
      }

    }
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

      ArrayList<WXHashMap<String, Object>> task = null;
      Stack<String> instanceStack = mNextTickTasks.getInstanceStack();
      int size = instanceStack.size();
      for (int i = size - 1; i >= 0; i--) {
        instanceId = instanceStack.get(i);
        task = mNextTickTasks.remove(instanceId);
        if (task != null && !((ArrayList) task).isEmpty()) {
          break;
        }
      }
      Object[] tasks = ((ArrayList) task).toArray();

      WXJSObject[] args = {
          new WXJSObject(WXJSObject.String, instanceId),
          WXJsonUtils.wsonWXJSObject(tasks)};

      invokeExecJS(String.valueOf(instanceId), null, METHOD_CALL_JS, args);
      task.clear();
      for(int i=0; i<tasks.length; i++){
        args[i] = null;
      }
      args = null;
    } catch (Throwable e) {
      WXLogUtils.e("WXBridgeManager", e);
      String err = "invokeCallJSBatch#" + WXLogUtils.getStackTrace(e);
	  WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_ERR_JS_FRAMEWORK.getErrorCode(),
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
			invokeRegisterModules(modules, mRegisterComponentFailList);
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
    post(new Runnable() {
      @Override
      public void run() {
        invokeRegisterComponents(components, mRegisterComponentFailList);
      }
    }, null);
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
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE.getErrorCode(),
			  "invokeExecJSService",
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE.getErrorMsg()
					  + "[WXBridgeManager] invokeRegisterService:"
					  + WXLogUtils.getStackTrace(e),
			  data);
	}
  }

  private boolean isJSThread() {
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

    WXJSObject[] args = {new WXJSObject(WXJSObject.JSON,
        WXJsonUtils.fromObjectToJSONString(modules))};
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_MODULES, args);
    } catch (Throwable e) {
	  WXExceptionUtils.commitCriticalExceptionRT(null,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES.getErrorCode(),
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

    WXJSObject[] args = {new WXJSObject(WXJSObject.JSON,
        WXJsonUtils.fromObjectToJSONString(components))};
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_COMPONENTS, args);
    } catch (Throwable e) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents ", e);
	  WXExceptionUtils.commitCriticalExceptionRT(null,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED.getErrorCode(),
			  METHOD_REGISTER_COMPONENTS,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED.getErrorMsg()
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
      if (METHOD_CREATE_INSTANCE.equals(function) || !instance.isContentMd5Match()) {
        try {
          if (isJSFrameworkInit() && reInitCount > 1 && !instance.isNeedReLoad()) {
            // JSONObject domObject = JSON.parseObject(tasks);
            WXDomModule domModule = getDomModule(instanceId);
            Action action = Actions.getReloadPage(instanceId, true);
            domModule.postAction((DOMAction) action, true);
            instance.setNeedLoad(true);
            return;
          } else {
            instance.onRenderError(//DegradPassivity to H5
            		WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorCode(),
					WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getDegradErrorMsg() +
							"reportJSException >>>> instanceId:" + instanceId
							+ ", exception function:" + function + ", exception:"
							+ exception
			);
            return;
          }
        } catch (Exception e) {
          e.printStackTrace();
        }
      }
      instance.onJSException(WXErrorCode.WX_ERR_JS_EXECUTE.getErrorCode(), function, exception);
	}

    IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
    if (adapter != null) {
      String bundleUrl;
      String exceptionId = instanceId;

      if (instanceId == "" || instanceId == null) {
        exceptionId = "instanceIdisNull";
      }

      if (instance == null) {
        if (("initFramework").equals(function)) {
          bundleUrl = "jsExceptionBeforeRenderInstanceNull";
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

              }
            }
          } catch (Throwable e) {
            e.printStackTrace();
          }
          exception += "\n" + exceptionExt;
          WXLogUtils.e("reportJSException:" + exception);

        } else if (function == null) {
          bundleUrl = "jsExceptionInstanceAndFunctionNull";
        } else {
          bundleUrl = "jsExceptionInstanceNull" + function;
        }
      } else {
        bundleUrl = instance.getBundleUrl();
      }

//      WXJSExceptionInfo jsException = new WXJSExceptionInfo(exceptionId, bundleUrl, WXErrorCode.WX_ERR_JS_EXECUTE.getErrorCode(), function, exception, null);
//      adapter.onJSException(jsException);

	  WXExceptionUtils.commitCriticalExceptionRT(exceptionId, WXErrorCode.WX_KEY_EXCEPTION_WXBRIDGE.getErrorCode(),
			  function,
			  WXErrorCode.WX_KEY_EXCEPTION_WXBRIDGE.getErrorMsg() + exception,
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
  public static void  updateGlobalConfig(String config){
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
                bridge.updateGlobalConfig(globalConfig);
              }
            }
          }
          if(globalConfig.contains("wson_off")){
            WXJsonUtils.USE_WSON = false;
          }else{
            WXJsonUtils.USE_WSON = true;
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



}
