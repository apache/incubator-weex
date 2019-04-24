/**
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

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXWsonJSONSwitch;

import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

/**
 * Communication interface for Java code and JavaScript code.
 */

public class WXBridge implements IWXBridge {

  private native int nativeInitFrameworkEnv(String framework, WXParams params, String cacheDir, boolean pieSupport);

  private native int nativeInitFramework(String framework, WXParams params);

  private native void nativeRefreshInstance(String instanceId, String namespace, String function, WXJSObject[] args);

  private native int nativeExecJS(String instanceId, String name, String function, WXJSObject[] args);

  private native int nativeExecJSService(String javascript);

  public native byte[] nativeExecJSWithResult(String instanceId, String _namespace, String _function, WXJSObject[] args);
  public native void nativeExecJSWithCallback(String instanceId, String _namespace, String _function, WXJSObject[] args, long callbackId);

  public native int nativeCreateInstanceContext(String instanceId, String name, String function, WXJSObject[] args);

  public native int nativeDestoryInstance(String instanceId, String name, String function, WXJSObject[] args);

  public native String nativeExecJSOnInstance(String instanceId, String script, int type);

  public native void nativeFireEventOnDataRenderNode(String instanceId, String ref, String type, String data, String domChanges);

  public native void nativeInvokeCallbackOnDataRender(String instanceId, String callbackId, String data, boolean keepAlive);

  public native void nativeRegisterModuleOnDataRenderNode( String data);

  public native void nativeRegisterComponentOnDataRenderNode( String data);

  private native void nativeTakeHeapSnapshot(String filename);

  private native void nativeBindMeasurementToRenderObject(long ptr);

  private native void nativeSetRenderContainerWrapContent(boolean wrap, String instanceId);

  public native long[] nativeGetFirstScreenRenderTime(String instanceId);

  public native long[] nativeGetRenderFinishTime(String instanceId);

  private native void nativeSetDefaultHeightAndWidthIntoRootDom(String instanceId, float defaultWidth, float defaultHeight, boolean isWidthWrapContent, boolean isHeightWrapContent);

  private native void nativeOnInstanceClose(String instanceId);

  private native void nativeForceLayout(String instanceId);

  private native boolean nativeNotifyLayout(String instanceId);

  private native void nativeSetStyleWidth(String instanceId, String ref, float value);

  private native void nativeSetStyleHeight(String instanceId, String ref, float value);

  private native void nativeSetMargin(String instanceId, String ref, int edge, float value);

  private native void nativeSetPadding(String instanceId, String ref, int edge, float value);

  private native void nativeSetPosition(String instanceId, String ref, int edge, float value);

  private native void nativeMarkDirty(String instanceId, String ref, boolean dirty);

  private native void nativeSetDeviceDisplay(String instanceId, float width, float height, float scale);

  private native void nativeRegisterCoreEnv(String key, String value);

  private native void nativeResetWXBridge(Object bridge, String className);

  /**
   * Update Init Framework Params
   * */
  private native void nativeUpdateInitFrameworkParams(String key, String value, String desc);

  /**
   * update global config,
   * @param config params
   * */
  public native void nativeUpdateGlobalConfig(String config);

  public static final boolean MULTIPROCESS = true;


  @Override
  public void updateInitFrameworkParams(String key, String value, String desc){
     nativeUpdateInitFrameworkParams(key, value, desc);
  }

  @Override
  public int initFramework(String framework, WXParams params) {
    return nativeInitFramework(framework, params);
  }

  @Override
  public int initFrameworkEnv(String framework, WXParams params, String cacheDir, boolean pieSupport) {
    if (MULTIPROCESS) {
      return nativeInitFrameworkEnv(framework, params, cacheDir, pieSupport);
    } else {
      return nativeInitFramework(framework, params);
    }
  }

  @Override
  public void refreshInstance(String instanceId, String namespace, String function, WXJSObject[] args) {
    nativeRefreshInstance(instanceId, namespace, function, args);
  }

  @Override
  public int execJS(String instanceId, String namespace, String function, WXJSObject[] args) {
    return nativeExecJS(instanceId, namespace, function, args);
  }

  @Override
  public void execJSWithCallback(String instanceId, String namespace, String function, WXJSObject[] args, ResultCallback callback) {
    if (callback == null) {
      execJS(instanceId, namespace, function, args);
    }
    nativeExecJSWithCallback(instanceId, namespace, function, args,
            ResultCallbackManager.generateCallbackId(callback));
  }

  // Result from js engine
  @CalledByNative
  public void onReceivedResult(long callbackId, byte[] result) {
    ResultCallback callback = ResultCallbackManager.removeCallbackById(callbackId);
    if (callback != null) {
       callback.onReceiveResult(result);
    }
  }

  @Override
  public int execJSService(String javascript) {
    return nativeExecJSService(javascript);
  }


  @Override
  public void takeHeapSnapshot(String filename) {
    nativeTakeHeapSnapshot(filename);
  }

  @Override
  public int createInstanceContext(String instanceId, String name, String function, WXJSObject[] args) {
    return nativeCreateInstanceContext(instanceId, name, function, args);
  }

  @Override
  public int destoryInstance(String instanceId, String name, String function, WXJSObject[] args) {
    return nativeDestoryInstance(instanceId, name, function, args);
  }

  @Override
  public String execJSOnInstance(String instanceId, String script, int type) {
    return nativeExecJSOnInstance(instanceId, script, type);
  }

  public static final String TAG = "WXBridge";

  /**
   * JavaScript uses this methods to call Android code
   *
   * @param instanceId
   * @param tasks
   * @param callback
   */
  @CalledByNative
  public int callNative(String instanceId, byte[] tasks, String callback) {
    return callNative(instanceId, (JSONArray) JSON.parseArray(new String(tasks)), callback);
  }

  @Override
  public int callNative(String instanceId, String tasks, String callback) {
    try{
      return callNative(instanceId, JSONArray.parseArray(tasks), callback);
    }catch (Exception e){
      WXLogUtils.e(TAG, "callNative throw exception: " + e.getMessage());
      return IWXBridge.INSTANCE_RENDERING;
    }
  }

  private int callNative(String instanceId, JSONArray tasks, String callback){
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callNative(instanceId, tasks, callback);
    } catch (Throwable e) {
      WXLogUtils.e(TAG, "callNative throw exception:" + e.getMessage());
    }

    if (WXEnvironment.isApkDebugable()) {
      if (errorCode == IWXBridge.DESTROY_INSTANCE) {
        WXLogUtils.w("destroyInstance :" + instanceId + " JSF must stop callNative");
      }
    }
    return errorCode;
  }

  @CalledByNative
  public void reportJSException(String instanceId, String func, String exception) {
    WXBridgeManager.getInstance().reportJSException(instanceId, func, exception);
  }


  /**
   * Bridge module Js Method
   * support Sync or Async through setting  Annotation as {@link com.taobao.weex.annotation.JSMethod }
   *
   * @param instanceId Instance ID
   * @param module     the name of module
   * @param method     the name of method
   * @param arguments  the arguments of the method
   * @param options    option arguments for extending
   * @return the result
   */
  @Override
  @CalledByNative
  public Object callNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options) {
    try {
      long start = WXUtils.getFixUnixTime();
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      JSONArray argArray = null;
      if (arguments != null){
        // TODO use a better way
        if (instance!=null && (instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER
                || instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER_BINARY)){
          try {
            argArray = (JSONArray) JSON.parse(new String(arguments, "UTF-8"));
          } catch (Exception e) {
            // For wson use in data render mode
            argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
          }
        } else {
          argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
        }
      }
      JSONObject optionsObj = null;
      if (options != null) {
        optionsObj = (JSONObject) WXWsonJSONSwitch.parseWsonOrJSON(options);
      } else if (argArray != null) {
        final WXSDKInstance sdkInstance = WXSDKManager.getInstance().getSDKInstance(instanceId);
        if (sdkInstance != null) {
          if (WXBridgeManager.BundType.Rax.equals(sdkInstance.bundleType)) {
            Object weex_options__ = null;
            for (Object object: argArray) {
              if (object instanceof JSONObject && ((JSONObject) object).containsKey("__weex_options__")) {
                weex_options__ = ((JSONObject) object).get("__weex_options__");
              }
            }

            if (weex_options__ instanceof JSONObject)
              optionsObj = (JSONObject) weex_options__;
          }
        }

      }

      Object object = WXBridgeManager.getInstance().callNativeModule(instanceId, module, method, argArray, optionsObj);

      if (null != instance){
        instance.getApmForInstance().updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_CALL_NATIVE_NUM,1);
        instance.getApmForInstance().updateFSDiffStats(
            WXInstanceApm.KEY_PAGE_STATS_FS_CALL_NATIVE_TIME,
            WXUtils.getFixUnixTime()-start
        );
      }
      if (instance!=null && (instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER
          || instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER_BINARY)){
        try {
          if(object == null){
            return new WXJSObject(null);
          }
          if(object.getClass() == WXJSObject.class){
            return (WXJSObject) object;
          }
          return new WXJSObject(WXJSObject.JSON, WXJsonUtils.fromObjectToJSONString(object));
        } catch (Exception e) {
          // For wson use in data render mode
          return WXWsonJSONSwitch.toWsonOrJsonWXJSObject(object);
        }
      } else {
        return WXWsonJSONSwitch.toWsonOrJsonWXJSObject(object);
      }
    }catch (Exception e){
      WXLogUtils.e(TAG,  e);
      return new WXJSObject(null);
    }
  }

  /**
   * Bridge component Js Method
   *
   * @param instanceId   Instance ID
   * @param ref the ref of component
   * @param method       the name of method
   * @param arguments    the arguments of the method
   * @param options      option arguments for extending
   */
  @Override
  @CalledByNative
  public void callNativeComponent(String instanceId, String ref, String method, byte[] arguments, byte[] optionsData) {
    try{
      WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
      JSONArray argArray = null;
      if (arguments != null){
        // TODO use a better way
        if (instance!=null && (instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER
            || instance.getRenderStrategy()== WXRenderStrategy.DATA_RENDER_BINARY)){
          try {
            argArray = (JSONArray) JSON.parse(new String(arguments, "UTF-8"));
          } catch (Exception e) {
            // For wson use in data render mode
            argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
          }
        } else {
          argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
        }
      }
      Object options = WXWsonJSONSwitch.parseWsonOrJSON(optionsData);
      WXBridgeManager.getInstance().callNativeComponent(instanceId, ref, method, argArray, options);
    }catch (Exception e){
      WXLogUtils.e(TAG,  e);
    }
  }

  @Override
  @CalledByNative
  public void setTimeoutNative(String callbackId, String time) {
    WXBridgeManager.getInstance().setTimeout(callbackId, time);
  }

  @Override
  @CalledByNative
  public void setJSFrmVersion(String version) {
    if (version != null) {
      WXEnvironment.JS_LIB_SDK_VERSION = version;
    }
  }

  @Override
  public void resetWXBridge(boolean remoteDebug) {
    final String className = this.getClass().getName().replace('.', '/');
    nativeResetWXBridge(this, className);
  }

  @Override
  @CalledByNative
  public int callUpdateFinish(String instanceId, byte[] tasks, String callback) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callCreateBody throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callRefreshFinish(String instanceId, byte[] tasks, String callback) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRefreshFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callCreateFinish throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public void reportServerCrash(String instanceId, String crashFile) {
    WXLogUtils.e(TAG, "reportServerCrash instanceId:" + instanceId + " crashFile: " + crashFile);
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callReportCrashReloadPage(instanceId, crashFile);

      // upload crash log
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "reloadPageNative throw exception:" + e.getMessage());
      }
    }
  }

  @Override
  @CalledByNative
  public int callCreateBody(String instanceId, String componentType, String ref,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;

    try {
      errorCode = WXBridgeManager.getInstance().callCreateBody(instanceId, componentType, ref,
              styles, attributes, events, margins, paddings, borders);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callCreateBody throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callAddElement(String instanceId, String componentType, String ref, int index, String parentRef,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders,  boolean willLayout) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;

    try {
      errorCode = WXBridgeManager.getInstance().callAddElement(instanceId, componentType, ref, index, parentRef,
              styles, attributes, events, margins, paddings, borders, willLayout);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        e.printStackTrace();
        WXLogUtils.e(TAG, "callAddElement throw error:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callRemoveElement(String instanceId, String ref) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveElement(instanceId, ref);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callRemoveElement throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callMoveElement(String instanceId, String ref, String parentref, int index) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callMoveElement(instanceId, ref, parentref, index);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callMoveElement throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callAddEvent(String instanceId, String ref, String event) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callAddEvent(instanceId, ref, event);
    } catch (Throwable e) {
      //catch everything during call native.
      // if(WXEnvironment.isApkDebugable()){
      WXLogUtils.e(TAG, "callAddEvent throw exception:" + e.getMessage());
      // }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callRemoveEvent(String instanceId, String ref, String event) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveEvent(instanceId, ref, event);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callRemoveEvent throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callUpdateStyle(String instanceId, String ref,
                             HashMap<String, Object> styles,
                             HashMap<String, String> paddings,
                             HashMap<String, String> margins,
                             HashMap<String, String> borders) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateStyle(instanceId, ref, styles, paddings, margins, borders);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callUpdateStyle throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callUpdateAttrs(String instanceId, String ref, HashMap<String, String> attrs) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateAttrs(instanceId, ref, attrs);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callUpdateAttr throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callLayout(String instanceId, String ref, int top, int bottom, int left, int right, int height, int width, boolean isRTL, int index) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callLayout(instanceId, ref, top, bottom, left, right, height, width, isRTL, index);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callLayout throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callCreateFinish(String instanceId) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callCreateFinish(instanceId);
    } catch (Throwable e) {
      WXLogUtils.e(TAG, "callCreateFinish throw exception:" + e.getMessage());
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callRenderSuccess(String instanceId) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRenderSuccess(instanceId);
    } catch (Throwable e) {
      WXLogUtils.e(TAG, "callCreateFinish throw exception:" + e.getMessage());
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callAppendTreeCreateFinish(String instanceId, String ref) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callAppendTreeCreateFinish(instanceId, ref);
    } catch (Throwable e) {
      WXLogUtils.e(TAG, "callAppendTreeCreateFinish throw exception:" + e.getMessage());
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public int callHasTransitionPros(String instanceId, String ref, HashMap<String, String> styles) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callHasTransitionPros(instanceId, ref, styles);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callHasTransitionPros throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  @CalledByNative
  public ContentBoxMeasurement getMeasurementFunc(String instanceId, long renderObjectPtr) {
    ContentBoxMeasurement obj = null;
    try {
      obj = WXBridgeManager.getInstance().getMeasurementFunc(instanceId, renderObjectPtr);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "getMeasurementFunc throw exception:" + e.getMessage());
      }
    }
    return obj;
  }

  @Override
  public void bindMeasurementToRenderObject(long ptr){
    nativeBindMeasurementToRenderObject(ptr);
  }

  @Override
  public void setRenderContainerWrapContent(boolean wrap, String instanceId) {
    nativeSetRenderContainerWrapContent(wrap, instanceId);
  }

  @Override
  public long[] getFirstScreenRenderTime(String instanceId) {
    return nativeGetFirstScreenRenderTime(instanceId);
  }

  @Override
  public long[] getRenderFinishTime(String instanceId) {
    return nativeGetRenderFinishTime(instanceId);
  }

  @Override
  public void setDefaultHeightAndWidthIntoRootDom(String instanceId, float defaultWidth, float defaultHeight, boolean isWidthWrapContent, boolean isHeightWrapContent) {
    nativeSetDefaultHeightAndWidthIntoRootDom(instanceId, defaultWidth, defaultHeight, isWidthWrapContent, isHeightWrapContent);
  }

  @Override
  public void onInstanceClose(String instanceId) {
    nativeOnInstanceClose(instanceId);
  }

  @Override
  public void forceLayout(String instanceId) {
    nativeForceLayout(instanceId);
  }

  @Override
  public boolean notifyLayout(String instanceId) {
    return nativeNotifyLayout(instanceId);
  }

  @Override
  public void setStyleWidth(String instanceId, String ref, float value) {
    nativeSetStyleWidth(instanceId, ref, value);
  }

  @Override
  public void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetMargin(instanceId, ref, edge.ordinal(), value);
  }

  @Override
  public void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetPadding(instanceId, ref, edge.ordinal(), value);
  }

  @Override
  public void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetPosition(instanceId, ref, edge.ordinal(), value);
  }

  @Override
  public void markDirty(String instanceId, String ref, boolean dirty) {
    nativeMarkDirty(instanceId, ref, dirty);
  }

  @Override
  public void setDeviceDisplay(String instanceId, float width, float height, float scale) {
    nativeSetDeviceDisplay(instanceId, width, height, scale);
  }

  @Override
  public void setStyleHeight(String instanceId, String ref, float value) {
    nativeSetStyleHeight(instanceId, ref, value);
  }

  @Override
  public void registerCoreEnv(String key, String value) {
    nativeRegisterCoreEnv(key, value);
  }

  @CalledByNative
  public void reportNativeInitStatus(String statusCode, String errorMsg) {
    if (WXErrorCode.WX_JS_FRAMEWORK_INIT_SINGLE_PROCESS_SUCCESS.getErrorCode().equals(statusCode)
            || WXErrorCode.WX_JS_FRAMEWORK_INIT_FAILED.getErrorCode().equals(statusCode)) {
      IWXUserTrackAdapter userTrackAdapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
      if (userTrackAdapter != null) {
        Map<String, Serializable> params = new HashMap<>(3);
        params.put(IWXUserTrackAdapter.MONITOR_ERROR_CODE, statusCode);
        params.put(IWXUserTrackAdapter.MONITOR_ARG, "InitFrameworkNativeError");
        params.put(IWXUserTrackAdapter.MONITOR_ERROR_MSG, errorMsg);
        Log.e("Dyy", "reportNativeInitStatus is running and errorCode is " + statusCode + " And errorMsg is " + errorMsg);
        userTrackAdapter.commit(null, null, IWXUserTrackAdapter.INIT_FRAMEWORK, null, params);
      }

      return;
    }

    if (WXErrorCode.WX_JS_FRAMEWORK_INIT_FAILED_PARAMS_NULL.getErrorCode().equals(statusCode)) {
      WXExceptionUtils.commitCriticalExceptionRT(null, WXErrorCode.WX_JS_FRAMEWORK_INIT_FAILED_PARAMS_NULL,
              "WeexProxy::initFromParam()",
              WXErrorCode.WX_JS_FRAMEWORK_INIT_FAILED_PARAMS_NULL.getErrorMsg() + ": " + errorMsg,
              null);
      return;
    }

    for (WXErrorCode e : WXErrorCode.values()) {
      if (e.getErrorType().equals(WXErrorCode.ErrorType.NATIVE_ERROR)
              && e.getErrorCode().equals(statusCode)) {
        WXExceptionUtils.commitCriticalExceptionRT(null, e, "initFramework", errorMsg, null);
        break;
      }
    }
  }

  public void fireEventOnDataRenderNode(String instanceId, String ref, String type, String data, String domChanges) {
    nativeFireEventOnDataRenderNode(instanceId,ref,type,data, domChanges);
  }

  public void invokeCallbackOnDataRender(String instanceId, String callbackId, String data, boolean keepAlive) {
    nativeInvokeCallbackOnDataRender(instanceId,callbackId,data, keepAlive);
  }

  public void registerModuleOnDataRenderNode(String data) {
    nativeRegisterModuleOnDataRenderNode(data);
  }

  public void registerComponentOnDataRenderNode(String data) {
    nativeRegisterComponentOnDataRenderNode(data);
  }
}
