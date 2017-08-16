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
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
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
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRefreshData;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.WXDomModule;
import com.taobao.weex.dom.action.Action;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.dom.action.TraceableAction;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import com.taobao.weex.utils.batch.BactchExecutor;
import com.taobao.weex.utils.batch.Interceptor;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import static com.taobao.weex.bridge.WXModuleManager.getDomModule;

/**
 * Manager class for communication between JavaScript and Android.
 * <ol>
 *   <li>
 *     Handle Android to JavaScript call, can be one of the following
 *     <ul>
 *       <li>{@link #createInstance(String, String, Map, String)}</li>
 *       <li>{@link #destroyInstance(String)}</li>
 *       <li>{@link #refreshInstance(String, WXRefreshData)}</li>
 *       <li>{@link #registerModules(Map)}</li>
 *       <li>{@link #registerComponents(List)}</li>
 *       <li>{@link #invokeCallJSBatch(Message)}</li>
 *     </ul>
 *   </li>
 *   <li>
 *     Handle JavaScript to Android call
 *   </li>
 *   <li>
 *     Handle next tick of message.
 *   </li>
 * </ol>
 */
public class WXBridgeManager implements Callback,BactchExecutor {

  public static final String METHOD_CREATE_INSTANCE = "createInstance";
  public static final String METHOD_DESTROY_INSTANCE = "destroyInstance";
  public static final String METHOD_CALL_JS = "callJS";
  public static final String METHOD_SET_TIMEOUT = "setTimeoutCallback";
  public static final String METHOD_REGISTER_MODULES = "registerModules";
  public static final String METHOD_REGISTER_COMPONENTS = "registerComponents";
  public static final String METHOD_FIRE_EVENT = "fireEvent";
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
  public static final String ARGS = "args";
  public static final String OPTIONS = "options";
  private static final String NON_CALLBACK = "-1";
  private static final String UNDEFINED = "undefined";

  private static final int INIT_FRAMEWORK_OK = 1;

  private static long LOW_MEM_VALUE = 120;

  static volatile WXBridgeManager mBridgeManager;


  /**
   * next tick tasks, can set priority
   */
  private WXHashMap<String, ArrayList<WXHashMap<String, Object>>> mNextTickTasks = new WXHashMap<>();

  /**
   * JSThread
   */
  private WXThread mJSThread;
  /** package **/ Handler mJSHandler;
  private IWXBridge mWXBridge;
  private IWXDebugProxy mWxDebugProxy;

  private boolean mMock = false;
  /**
   * Whether JS Framework(main.js) has been initialized.
   */
  private boolean mInit =false;

  private boolean isJSFrameworkInit(){
    return mInit;
  }

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

  private void initWXBridge(boolean remoteDebug) {
    if (remoteDebug && WXEnvironment.isApkDebugable()) {
      WXEnvironment.sDebugServerConnectable = true;
    }

    if (mWxDebugProxy != null) {
      mWxDebugProxy.stop(false);
    }
    if (WXEnvironment.sDebugServerConnectable && WXEnvironment.isApkDebugable()) {
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
      return  callModuleMethod(instanceId, moduleStr, methodStr, args, null);
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
    mInit = false;
    initWXBridge(WXEnvironment.sRemoteDebugMode);
  }

  /**
   * Set current Instance
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
  public void post(Runnable r){
    if(mInterceptor != null && mInterceptor.take(r)){
      //task is token by the interceptor
      return;
    }
    if (mJSHandler == null){
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

  public void sendMessageDelayed(Message message, long delayMillis){
    if (message == null || mJSHandler == null || mJSThread == null
        || !mJSThread.isWXThreadAlive() || mJSThread.getLooper() == null) {
      return;
    }
    mJSHandler.sendMessageDelayed(message,delayMillis);
  }

  public void removeMessage(int what,Object obj){
    if (mJSHandler == null || mJSThread == null
        || !mJSThread.isWXThreadAlive() || mJSThread.getLooper() == null) {
      return;
    }
    mJSHandler.removeMessages(what, obj);
  }

    public Object callNativeModule(String instanceId, String module, String method, JSONArray arguments, Object options) {

        if (WXEnvironment.isApkDebugable()) {
            mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
                    .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
            WXLogUtils.d(mLodBuilder.substring(0));
            mLodBuilder.setLength(0);
        }

        try {
            if(WXDomModule.WXDOM.equals(module)){
              WXDomModule dom = getDomModule(instanceId);
              return dom.callDomMethod(method,arguments);
            }else {
              return callModuleMethod(instanceId, module,
                      method, arguments);
            }
        } catch (Exception e) {
            WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
            commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE, "[WXBridgeManager] callNativeModule exception " + e.getCause());
        }

        return null;
    }
    public Object callNativeModule(String instanceId, String module,String method, JSONArray arguments, JSONObject options) {

        if (WXEnvironment.isApkDebugable()) {
            mLodBuilder.append("[WXBridgeManager] callNativeModule >>>> instanceId:").append(instanceId)
                    .append(", module:").append(module).append(", method:").append(method).append(", arguments:").append(arguments);
            WXLogUtils.d(mLodBuilder.substring(0));
            mLodBuilder.setLength(0);
        }

        try {
            if(WXDomModule.WXDOM.equals(module)){
              WXDomModule dom = getDomModule(instanceId);
              return dom.callDomMethod(method,arguments);
            }else {
              return callModuleMethod(instanceId, module,
                      method, arguments, options);
            }
        } catch (Exception e) {
            WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
            commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE, "[WXBridgeManager] callNativeModule exception " + e.getCause());
        }

        return null;
    }

    public Object callNativeComponent(String instanceId, String componentRef, String method, JSONArray arguments, Object options) {
        if (WXEnvironment.isApkDebugable()) {
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
            commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE, "[WXBridgeManager] callNativeModule exception " + e.getCause());
        }
        return null;
    }

  /**
   * Dispatch the native task to be executed.
     *
   * @param instanceId {@link WXSDKInstance#mInstanceId}
   * @param tasks tasks to be executed
   * @param callback next tick id
   */
  public int callNative(String instanceId, String tasks, String callback) {
    if (TextUtils.isEmpty(tasks)) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[WXBridgeManager] callNative: call Native tasks is null");
      }
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,"[WXBridgeManager] callNative: call Native tasks is null");
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callNative >>>> instanceId:").append(instanceId)
          .append(", tasks:").append(tasks).append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId!=null &&mDestroyedInstanceId.contains(instanceId)){
      return IWXBridge.DESTROY_INSTANCE;
    }


    long start = System.currentTimeMillis();
    long parseNanos = System.nanoTime();
    JSONArray array = JSON.parseArray(tasks);
    parseNanos = System.nanoTime() - parseNanos;

    if(WXSDKManager.getInstance().getSDKInstance(instanceId)!=null) {
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
            if(target != null){
              if(WXDomModule.WXDOM.equals(target)){
                WXDomModule dom = getDomModule(instanceId);
                dom.callDomMethod(task,parseNanos);
              }else {
                JSONObject optionObj = task.getJSONObject(OPTIONS);
                callModuleMethod(instanceId, (String) target,
                        (String) task.get(METHOD), (JSONArray) task.get(ARGS), optionObj);
              }
            }else if(task.get(COMPONENT) != null){
              //call component
              WXDomModule dom = getDomModule(instanceId);
              dom.invokeMethod((String) task.get(REF),(String) task.get(METHOD),(JSONArray) task.get(ARGS));
            }else{
              throw new IllegalArgumentException("unknown callNative");
            }
          }
        }
      } catch (Exception e) {
        WXLogUtils.e("[WXBridgeManager] callNative exception: ", e);
        commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,"[WXBridgeManager] callNative exception "+e.getCause());
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
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[WXBridgeManager] callCreateBody: call CreateBody tasks is null");
      }
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_CREATEBODY,"[WXBridgeManager] callCreateBody: call CreateBody tasks is null");
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callCreateBody >>>> instanceId:").append(instanceId)
              .append(", tasks:").append(tasks).append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }


    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)){
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
        domModule.postAction((DOMAction)action, true);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mParseJsonNanos = nanosTemp;
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).onStartDomExecute(instanceId, "createBody", "_root", domObject.getString("type"), tasks);
        }
      }
    } catch (Exception e) {
        WXLogUtils.e("[WXBridgeManager] callCreateBody exception: ", e);
        commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_CREATEBODY,"[WXBridgeManager] callCreateBody exception "+e.getCause());
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
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callUpdateFinish >>>> instanceId:").append(instanceId)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)){
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getUpdateFinish();
        domModule.postAction((DOMAction)action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateFinish exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,"[WXBridgeManager] callUpdateFinish exception "+e.getCause());
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
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callCreateFinish >>>> instanceId:").append(instanceId)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getCreateFinish();
        domModule.postAction((DOMAction)action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callCreateFinish exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERROR_DOM_CREATEFINISH,"[WXBridgeManager] callCreateFinish exception " + e.getCause());
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
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callRefreshFinish >>>> instanceId:").append(instanceId)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRefreshFinish();
        domModule.postAction((DOMAction)action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRefreshFinish exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERROR_DOM_REFRESHFINISH,"[WXBridgeManager] callRefreshFinish exception " + e.getCause());
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
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[WXBridgeManager] callUpdateAttrs: call UpdateAttrs tasks is null");
      }
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_UPDATEATTRS,"[WXBridgeManager] callUpdateAttrs: call UpdateAttrs tasks is null");
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callUpdateAttrs >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", task:").append(task)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
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
        domModule.postAction((DOMAction)action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).mParseJsonNanos = parseNanos;
          ((TraceableAction) action).onStartDomExecute(instanceId, "updateAttrs", domObject.getString("ref"), domObject.getString("type"), task);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateAttrs exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_UPDATEATTRS,"[WXBridgeManager] callUpdateAttrs exception " + e.getCause());
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
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[WXBridgeManager] callUpdateStyle: call UpdateStyle tasks is null");
      }
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_UPDATESTYLE,"[WXBridgeManager] callUpdateStyle: call UpdateStyle tasks is null");
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callUpdateStyle >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", task:").append(task)
              .append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
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
        domModule.postAction((DOMAction)action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).mParseJsonNanos = nanosTemp;
          ((TraceableAction) action).mStartMillis = start;
          ((TraceableAction) action).onStartDomExecute(instanceId, "updateStyle", ref, domObject.getString("type"), domObject.toJSONString());
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callUpdateStyle exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_UPDATESTYLE,"[WXBridgeManager] callUpdateStyle exception " + e.getCause());
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

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callRemoveElement >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRemoveElement(ref);
        domModule.postAction((DOMAction)action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "removeElement", ref, null, ref);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveElement exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_REMOVEELEMENT,"[WXBridgeManager] callRemoveElement exception " + e.getCause());
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

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callMoveElement >>>> instanceId:").append(instanceId)
              .append(", parentref:").append(parentref)
              .append(", index:").append(index)
              .append(", ref:").append(ref);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getMoveElement(ref, parentref, Integer.parseInt(index));
        domModule.postAction((DOMAction)action, false);
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callMoveElement exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_MOVEELEMENT,"[WXBridgeManager] callMoveElement exception " + e.getCause());
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddEvent(String instanceId, String ref, String event, String callback) {

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callAddEvent >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", event:").append(event);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge.DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getAddEvent(ref, event);
        domModule.postAction((DOMAction)action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "addEvent", ref, null, event);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callAddEvent exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_ADDEVENT,"[WXBridgeManager] callAddEvent exception " + e.getCause());
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callRemoveEvent(String instanceId, String ref, String event, String callback) {

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callRemoveEvent >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref)
              .append(", event:").append(event);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId != null && mDestroyedInstanceId.contains(instanceId)) {
      return IWXBridge .DESTROY_INSTANCE;
    }

    try {
      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXDomModule domModule = getDomModule(instanceId);
        Action action = Actions.getRemoveEvent(ref, event);
        domModule.postAction((DOMAction)action, false);

        if (WXTracing.isAvailable() && action instanceof TraceableAction) {
          ((TraceableAction) action).onStartDomExecute(instanceId, "removeEvent", ref, null, event);
        }
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXBridgeManager] callRemoveEvent exception: ", e);
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_DOM_REMOVEEVENT,"[WXBridgeManager] callRemoveEvent exception " + e.getCause());
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  public int callAddElement(String instanceId, String ref,String dom,String index, String callback){

    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("[WXBridgeManager] callNative::callAddElement >>>> instanceId:").append(instanceId)
              .append(", ref:").append(ref).append(", dom:").append(dom).append(", callback:").append(callback);
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }

    if(mDestroyedInstanceId!=null && mDestroyedInstanceId.contains(instanceId)){
      return IWXBridge.DESTROY_INSTANCE;
    }


    if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
      long start = System.currentTimeMillis();
      long nanosTemp = System.nanoTime();
      JSONObject domObject = JSON.parseObject(dom);
      nanosTemp = System.nanoTime() - nanosTemp;

      if (WXSDKManager.getInstance().getSDKInstance(instanceId) != null) {
        WXSDKManager.getInstance().getSDKInstance(instanceId).jsonParseTime(System.currentTimeMillis() - start);
      }
      WXDomModule domModule = getDomModule(instanceId);
      DOMAction addElementAction = Actions.getAddElement(domObject, ref,Integer.parseInt(index));
      domModule.postAction(addElementAction, false);

      if (WXTracing.isAvailable() && addElementAction instanceof TraceableAction) {
        ((TraceableAction) addElementAction).mParseJsonNanos = nanosTemp;
        ((TraceableAction) addElementAction).mStartMillis = start;
        ((TraceableAction) addElementAction).onStartDomExecute(instanceId, "addElement", domObject.getString("ref"), domObject.getString("type"), dom);
      }
    }

    if (UNDEFINED.equals(callback) || NON_CALLBACK.equals(callback)) {
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    }
    // get next tick
    getNextTick(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;

  }

  private void getNextTick(final String instanceId, final String callback) {
    addJSTask(METHOD_CALLBACK,instanceId, callback, "{}");
    sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
  }


  private void addJSTask(final String method, final String instanceId, final Object... args) {
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

  private void sendMessage(String instanceId, int what) {
    Message msg = Message.obtain(mJSHandler);
    msg.obj = instanceId;
    msg.what = what;
    msg.sendToTarget();
  }

  /**
   * Initialize JavaScript framework
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
                        final String type, final Map<String, Object> data){
    this.fireEvent(instanceId, ref, type, data, null);
  }

  /**
   * Do not direct invoke this method in Components, use {@link WXSDKInstance#fireEvent(String, String, Map, Map)} instead.
   * @param instanceId
   * @param ref
   * @param type
   * @param data
   * @param domChanges
   */
  @Deprecated
  public void fireEvent(final String instanceId, final String ref,
                        final String type, final Map<String, Object> data,final Map<String, Object> domChanges) {
    fireEventOnNode(instanceId,ref,type,data,domChanges);
  }

  /**
   * Notify the JavaScript about the event happened on Android
   */
  public void fireEventOnNode(final String instanceId, final String ref,
                        final String type, final Map<String, Object> data,final Map<String, Object> domChanges) {
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(ref)
        || TextUtils.isEmpty(type) || mJSHandler == null) {
      return;
    }
    if (!checkMainThread()) {
      throw new WXRuntimeException(
          "fireEvent must be called by main thread");
    }
    addJSTask(METHOD_FIRE_EVENT, instanceId, ref, type, data,domChanges);
    sendMessage(instanceId, WXJSBridgeMsgType.CALL_JS_BATCH);
  }

  private boolean checkMainThread() {
    return Looper.myLooper() == Looper.getMainLooper();
  }


  /**
   * Invoke JavaScript callback. Use {@link JSCallback} instead.
   * @see #callback(String, String, String)
   */
  @Deprecated
  public void callback(String instanceId, String callback,String data) {
    callback(instanceId, callback,data,false);
  }

  /**
   * Invoke JavaScript callback. Use {@link JSCallback} instead.
   */
  @Deprecated
  public void callback(final String instanceId, final String callback,
                       final Map<String, Object> data){
    callback(instanceId,callback,data,false);
  }

  /**
   * Use {@link JSCallback} instead.
   * @param instanceId Weex Instance Id
   * @param callback  callback referenece handle
   * @param data callback data
   * @param keepAlive if keep callback instance alive for later use
     */
  @Deprecated
  public void callback(final String instanceId, final String callback,
                       final Object data,boolean keepAlive) {
    callbackJavascript(instanceId,callback,data,keepAlive);
  }

  /**
   * Callback to Javascript function.
   * @param instanceId Weex Instance Id
   * @param callback  callback referenece handle
   * @param data callback data
   * @param keepAlive if keep callback instance alive for later use
   */
  void callbackJavascript(final String instanceId, final String callback,
                          final Object data, boolean keepAlive) {
    if (TextUtils.isEmpty(instanceId) || TextUtils.isEmpty(callback)
        || mJSHandler == null) {
      return;
    }

    addJSTask(METHOD_CALLBACK, instanceId, callback, data,keepAlive);
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
          instance.onRenderError(WXRenderErrorCode.WX_CREATE_INSTANCE_ERROR,
                                 "createInstance failed!");
        }
        String err = "[WXBridgeManager] invokeRefreshInstance: framework.js uninitialized.";
        commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,err);
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
      String err = "[WXBridgeManager] invokeRefreshInstance " + e.getCause();
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,err);
      WXLogUtils.e(err);
    }
  }

  public void commitJSBridgeAlarmMonitor(String instanceId, WXErrorCode errCode, String errMsg) {
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance == null || errCode == null) {
      return;
    }
    // TODO: We should move WXPerformance and IWXUserTrackAdapter
    // into a adapter level.
    // comment out the line below to prevent commiting twice.
    //instance.commitUTStab(WXConst.JS_BRIDGE, errCode, errMsg);

    IWXUserTrackAdapter adapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
    if (adapter == null) {
      return;
    }
    WXPerformance performance = new WXPerformance();
    performance.args=instance.getBundleUrl();
    performance.errCode=errCode.getErrorCode();
    if (errCode != WXErrorCode.WX_SUCCESS) {
      performance.appendErrMsg(TextUtils.isEmpty(errMsg)?errCode.getErrorMsg():errMsg);
      WXLogUtils.e("wx_monitor",performance.toString());
    }
    adapter.commit(WXEnvironment.getApplication(), null, IWXUserTrackAdapter.JS_BRIDGE, performance, instance.getUserTrackParams());
  }

  public void commitJSFrameworkAlarmMonitor(final String type, final WXErrorCode errorCode, String errMsg) {
    if (TextUtils.isEmpty(type) || errorCode == null) {
      return;
    }
    if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
      WXSDKManager.getInstance().getWXStatisticsListener().onException("0",
          errorCode.getErrorCode(),
          TextUtils.isEmpty(errMsg) ? errorCode.getErrorMsg() : errMsg);
    }

    final IWXUserTrackAdapter userTrackAdapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
    if (userTrackAdapter == null) {
      return;
    }
    WXPerformance performance = new WXPerformance();
    performance.errCode = errorCode.getErrorCode();
    if (errorCode != WXErrorCode.WX_SUCCESS) {
      performance.appendErrMsg(TextUtils.isEmpty(errMsg)?errorCode.getErrorMsg():errMsg);
      WXLogUtils.e("wx_monitor",performance.toString());
    }
    userTrackAdapter.commit(WXEnvironment.getApplication(), null, type, performance, null);
  }


  /**
   * Create instance.
   */
  public void createInstance(final String instanceId, final String template,
                             final Map<String, Object> options, final String data) {
    final WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance == null){
      WXLogUtils.e("WXBridgeManager","createInstance failed, SDKInstance is not exist");
      return;
    }
    if ( TextUtils.isEmpty(instanceId)
        || TextUtils.isEmpty(template) || mJSHandler == null) {
      instance.onRenderError(WXRenderErrorCode.WX_CREATE_INSTANCE_ERROR, "createInstance fail!");
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
        instance.onRenderError(WXRenderErrorCode.WX_CREATE_INSTANCE_ERROR, "createInstance "
                                                                             + "fail!");
        String err = "[WXBridgeManager] invokeCreateInstance: framework.js uninitialized.";
        commitJSBridgeAlarmMonitor(instance.getInstanceId(), WXErrorCode.WX_ERR_INVOKE_NATIVE,err);
        WXLogUtils.e(err);
        return;
      }
      try {
        if (WXEnvironment.isApkDebugable()) {
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
        invokeExecJS(instance.getInstanceId(), null, METHOD_CREATE_INSTANCE, args,false);
      } catch (Throwable e) {
        instance.onRenderError(WXRenderErrorCode.WX_CREATE_INSTANCE_ERROR,
                                 "createInstance failed!");
        String err = "[WXBridgeManager] invokeCreateInstance " + e.getCause();
        commitJSBridgeAlarmMonitor(instance.getInstanceId(), WXErrorCode.WX_ERR_INVOKE_NATIVE,err);
        WXLogUtils.e(err);
      }
    }
  }

  private void mock(String instanceId) {

  }

  public void destroyInstance(final String instanceId) {
    if ( mJSHandler == null
        || TextUtils.isEmpty(instanceId)) {
      return;
    }
    if(mDestroyedInstanceId!=null) {
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
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("destroyInstance >>>> instanceId:" + instanceId);
      }
      WXJSObject instanceIdObj = new WXJSObject(WXJSObject.String,
                                                instanceId);
      WXJSObject[] args = {instanceIdObj};
      invokeExecJS(instanceId, null, METHOD_DESTROY_INSTANCE, args);
    } catch (Throwable e) {
      String err = "[WXBridgeManager] invokeDestroyInstance " + e.getCause();
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_INVOKE_NATIVE,err);
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
        if(timerInfo == null){
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
                            WXJSObject[] args,boolean logTaskDetail){
    if (WXEnvironment.isApkDebugable()) {
      mLodBuilder.append("callJS >>>> instanceId:").append(instanceId)
              .append("function:").append(function);
      if(logTaskDetail)
        mLodBuilder.append(" tasks:").append(WXJsonUtils.fromObjectToJSONString(args));
      WXLogUtils.d(mLodBuilder.substring(0));
      mLodBuilder.setLength(0);
    }
    mWXBridge.execJS(instanceId, namespace, function, args);
  }

  private void invokeInitFramework(Message msg) {
    String framework = "";
    if (msg.obj != null) {
      framework = (String) msg.obj;
    }

    if(WXUtils.getAvailMemory(WXEnvironment.getApplication()) > LOW_MEM_VALUE) {
      initFramework(framework);
    }
  }

  private void initFramework(String framework){
    if (!isJSFrameworkInit()) {
      if (TextUtils.isEmpty(framework)) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("weex JS framework from assets");
        }
        framework = WXFileUtils.loadAsset("main.js", WXEnvironment.getApplication());
      }
      if (TextUtils.isEmpty(framework)) {
        mInit = false;
        commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK, WXErrorCode.WX_ERR_JS_FRAMEWORK, "JS Framework is empty!");
        return;
      }
      try {
        if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
          WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkStart();
        }

        long start = System.currentTimeMillis();
        if(mWXBridge.initFramework(framework, assembleDefaultOptions())==INIT_FRAMEWORK_OK){
          WXEnvironment.sJSLibInitTime = System.currentTimeMillis() - start;
          WXLogUtils.renderPerformanceLog("initFramework", WXEnvironment.sJSLibInitTime);
          WXEnvironment.sSDKInitTime = System.currentTimeMillis() - WXEnvironment.sSDKInitStart;
          WXLogUtils.renderPerformanceLog("SDKInitTime", WXEnvironment.sSDKInitTime);
          mInit = true;

          if (WXSDKManager.getInstance().getWXStatisticsListener() != null) {
            WXSDKManager.getInstance().getWXStatisticsListener().onJsFrameworkReady();
          }

          execRegisterFailTask();
          WXEnvironment.JsFrameworkInit = true;
          registerDomModule();
          commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK, WXErrorCode.WX_SUCCESS, "success");
        }else{
          WXLogUtils.e("[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail");
          String err="[WXBridgeManager] invokeInitFramework  ExecuteJavaScript fail";
          commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK, WXErrorCode.WX_ERR_JS_FRAMEWORK, err);
        }
      } catch (Throwable e) {
        WXLogUtils.e("[WXBridgeManager] invokeInitFramework ", e);
        String err="[WXBridgeManager] invokeInitFramework exception!#"+e.toString();
        commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK, WXErrorCode.WX_ERR_JS_FRAMEWORK, err);
      }
    }
  }

  @SuppressWarnings("unchecked")
  private void invokeCallJSBatch(Message message) {
    if (mNextTickTasks.isEmpty() || !isJSFrameworkInit()) {
      if (!isJSFrameworkInit()) {
        WXLogUtils.e("[WXBridgeManager] invokeCallJSBatch: framework.js uninitialized!!  message:"+message.toString());
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
              new WXJSObject(WXJSObject.JSON,
                      WXJsonUtils.fromObjectToJSONString(task))};

      invokeExecJS(String.valueOf(instanceId), null, METHOD_CALL_JS, args);

    } catch (Throwable e) {
      WXLogUtils.e("WXBridgeManager", e);
      String err="invokeCallJSBatch#"+e.toString();
      commitJSBridgeAlarmMonitor(message.obj.toString(), WXErrorCode.WX_ERR_JS_EXECUTE,err);
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
   * @param modules the format is like
   *                {'dom':['updateAttrs','updateStyle'],'event':['openUrl']}
   */

  public void registerModules(final Map<String, Object> modules) {
    if (modules != null && modules.size() != 0) {
      if(isJSThread()){
        invokeRegisterModules(modules, mRegisterModuleFailList);
      }
      else{
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
    if ( mJSHandler == null || components == null
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
      commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK,WXErrorCode.WX_ERR_JS_EXECUTE,"invokeRegisterService");
    }
  }

  private boolean isJSThread() {
    return mJSThread != null && mJSThread.getId() == Thread.currentThread().getId();
  }

  private void invokeRegisterModules(Map<String, Object> modules, List<Map<String, Object>> failReceiver) {
    if (modules == null || !isJSFrameworkInit()) {
      if (!isJSFrameworkInit()) {
        WXLogUtils.e("[WXBridgeManager] invokeRegisterModules: framework.js uninitialized.");
      }
      failReceiver.add(modules);
      return;
    }

    WXJSObject[] args = {new WXJSObject(WXJSObject.JSON,
                                        WXJsonUtils.fromObjectToJSONString(modules))};
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_MODULES, args);
    } catch (Throwable e) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterModules:", e);
      commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK,WXErrorCode.WX_ERR_JS_EXECUTE,"invokeRegisterModules");
    }
  }

  private void invokeRegisterComponents(List<Map<String, Object>> components, List<Map<String, Object>> failReceiver) {
    if(components == failReceiver){
      throw new RuntimeException("Fail receiver should not use source.");
    }
    if (!isJSFrameworkInit()) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents: framework.js uninitialized.");

      for (Map<String,Object> comp:components){
        failReceiver.add(comp);
      }
      return;
    }
    if(components == null){
      return;
    }

    WXJSObject[] args = {new WXJSObject(WXJSObject.JSON,
                                        WXJsonUtils.fromObjectToJSONString(components))};
    try {
      mWXBridge.execJS("", null, METHOD_REGISTER_COMPONENTS, args);
    } catch (Throwable e) {
      WXLogUtils.e("[WXBridgeManager] invokeRegisterComponents ", e);
      commitJSFrameworkAlarmMonitor(IWXUserTrackAdapter.JS_FRAMEWORK,WXErrorCode.WX_ERR_JS_EXECUTE,"invokeRegisterComponents");
    }
  }

  public void destroy() {
    if (mJSThread != null) {
      mJSThread.quit();
    }
    mBridgeManager = null;
    if(mDestroyedInstanceId!=null){
      mDestroyedInstanceId.clear();
    }

  }

  /**
   * Report JavaScript Exception
   */
  public void reportJSException(String instanceId, String function,
                                String exception) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.e("reportJSException >>>> instanceId:" + instanceId
                   + ", exception function:" + function + ", exception:"
                   + exception);
    }
    WXSDKInstance instance;
    if (instanceId != null && (instance = WXSDKManager.getInstance().getSDKInstance(instanceId)) != null) {
      instance.onJSException(WXErrorCode.WX_ERR_JS_EXECUTE.getErrorCode(), function, exception);

      String err = "function:" + function + "#exception:" + exception;
      commitJSBridgeAlarmMonitor(instanceId, WXErrorCode.WX_ERR_JS_EXECUTE, err);

      IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
      if (adapter != null) {
        WXJSExceptionInfo jsException = new WXJSExceptionInfo(instanceId, instance.getBundleUrl(), WXErrorCode.WX_ERR_JS_EXECUTE.getErrorCode(), function, exception, null);
        adapter.onJSException(jsException);
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d(jsException.toString());
        }
      }
    }
  }

  public static class TimerInfo {

    public String callbackId;
    public long time;
    public String instanceId;
  }

  private void registerDomModule() throws WXException {
    /** Tell Javascript Framework what methods you have. This is Required.**/
    Map<String,Object> domMap=new HashMap<>();
    domMap.put(WXDomModule.WXDOM,WXDomModule.METHODS);
    registerModules(domMap);
  }

  //This method is deprecated because of performance issue.
  @Deprecated
  public void notifyTrimMemory() {

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

}
