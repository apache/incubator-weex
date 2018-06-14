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
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;
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

  private native int nativeExecJS(String instanceId, String name, String function, WXJSObject[] args);

  private native int nativeExecJSService(String javascript);

  public native byte[] nativeExecJSWithResult(String instanceId, String _namespace, String _function, WXJSObject[] args);

  public native int nativeCreateInstanceContext(String instanceId, String name, String function, WXJSObject[] args);

  public native int nativeDestoryInstance(String instanceId, String name, String function, WXJSObject[] args);

  public native String nativeExecJSOnInstance(String instanceId, String script, int type);

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

  private native void nativeRegisterCoreEnv(String key, String value);

  /**
   * update global config,
   * @param config params
   * */
  public native void nativeUpdateGlobalConfig(String config);

  public static final boolean MULTIPROCESS = true;

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
  public int execJS(String instanceId, String namespace, String function, WXJSObject[] args) {
    return nativeExecJS(instanceId, namespace, function, args);
  }

  @Override
  public byte[] execJSWithResult(String instanceId, String namespace, String function, WXJSObject[] args) {
    return nativeExecJSWithResult(instanceId, namespace, function, args);
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
  public Object callNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options) {
    try{
      JSONArray argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
      JSONObject optionsObj = null;
      if (options != null) {
        optionsObj = (JSONObject) WXWsonJSONSwitch.parseWsonOrJSON(options);
      }
      Object object = WXBridgeManager.getInstance().callNativeModule(instanceId, module, method, argArray, optionsObj);
      return WXWsonJSONSwitch.toWsonOrJsonWXJSObject(object);
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
  public void callNativeComponent(String instanceId, String ref, String method, byte[] arguments, byte[] optionsData) {
    try{
      JSONArray argArray = (JSONArray) WXWsonJSONSwitch.parseWsonOrJSON(arguments);
      Object options = WXWsonJSONSwitch.parseWsonOrJSON(optionsData);
      WXBridgeManager.getInstance().callNativeComponent(instanceId, ref, method, argArray, options);
    }catch (Exception e){
      WXLogUtils.e(TAG,  e);
    }
  }

  public void setTimeoutNative(String callbackId, String time) {
    WXBridgeManager.getInstance().setTimeout(callbackId, time);
  }

  public void setJSFrmVersion(String version) {
    if (version != null) {
      WXEnvironment.JS_LIB_SDK_VERSION = version;
    }
  }

  @Override
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
  public void reportServerCrash(String instanceId, String crashFile) {
    WXLogUtils.e(TAG, "reportServerCrash instanceId:" + instanceId);
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
  public int callLayout(String instanceId, String ref, int top, int bottom, int left, int right, int height, int width, int index) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callLayout(instanceId, ref, top, bottom, left, right, height, width, index);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callLayout throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
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
  public void setStyleHeight(String instanceId, String ref, float value) {
    nativeSetStyleHeight(instanceId, ref, value);
  }

  @Override
  public void registerCoreEnv(String key, String value) {
    nativeRegisterCoreEnv(key, value);
  }

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

    for (WXErrorCode e : WXErrorCode.values()) {
      if (e.getErrorType().equals(WXErrorCode.ErrorType.NATIVE_ERROR)
              && e.getErrorCode().equals(statusCode)) {
        WXExceptionUtils.commitCriticalExceptionRT(null, e, "initFramework", errorMsg, null);
        break;
      }
    }
  }
}
