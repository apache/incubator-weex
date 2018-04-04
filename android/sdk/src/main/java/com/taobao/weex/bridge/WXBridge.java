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

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;
import java.util.HashSet;

/**
 * Communication interface for Java code and JavaScript code.
 */

public class WXBridge implements IWXBridge {

  private native int nativeInitFrameworkEnv(String framework, WXParams params, String cacheDir, boolean pieSupport);

  private native int nativeInitFramework(String framework, WXParams params);

  private native int nativeExecJS(String instanceId, String name, String function, WXJSObject[] args);

  private native int nativeExecJSService(String javascript);

  private native void nativeTakeHeapSnapshot(String filename);

  private native void nativeSetStyleWidth(int instanceId, String ref, float value);

  private native void nativeSetStyleHeight(int instanceId, String ref, float value);

  private native void nativeSetMargin(int instanceId, String ref, int edge, float value);

  private native void nativeSetPadding(int instanceId, String ref, int edge, float value);

  private native void nativeSetPosition(int instanceId, String ref, int edge, float value);

  private native void nativeMarkDirty(int instanceId, String ref, boolean dirty);

  private native void nativeRegisterCoreEnv(String key, String value);

  private native void nativeSetViewPortWidth(int instanceId, float value);

  public native byte[] nativeExecJSWithResult(String instanceId, String _namespace, String _function, WXJSObject[] args);

  public native int nativeCreateInstanceContext(String instanceId, String name, String function, WXJSObject[] args);

  public native int nativeDestoryInstance(String instanceId, String name, String function, WXJSObject[] args);

  public native String nativeExecJSOnInstance(String instanceId, String script, int type);

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
    return callNative(instanceId, new String(tasks), callback);
  }

  @Override
  public int callNative(String instanceId, String tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callNative(instanceId, tasks, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      // if(WXEnvironment.isApkDebugable()){
      WXLogUtils.e(TAG, "callNative throw exception:" + e.getMessage());
      // }
    }

    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
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

    JSONArray argArray = JSON.parseArray(new String(arguments));
    JSONObject optionsObj = null;
    if (options != null) {
      optionsObj = JSON.parseObject(new String(options));
    }
    Object object = WXBridgeManager.getInstance().callNativeModule(instanceId, module, method, argArray, optionsObj);
    return new WXJSObject(object);
  }

  /**
   * Bridge component Js Method
   *
   * @param instanceId   Instance ID
   * @param componentRef the ref of component
   * @param method       the name of method
   * @param arguments    the arguments of the method
   * @param options      option arguments for extending
   */
  @Override
  public void callNativeComponent(String instanceId, String componentRef, String method, byte[] arguments, byte[] options) {
    JSONArray argArray = JSON.parseArray(new String(arguments));
    WXBridgeManager.getInstance().callNativeComponent(instanceId, componentRef, method, argArray, options);
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

    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callCreateBody throw exception:" + e.getMessage());
      }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callRefreshFinish(String instanceId, byte[] tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRefreshFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callCreateFinish throw exception:" + e.getMessage());
      }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
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
  public int callCreateBody(int instanceId, String componentType, String ref,
                                      HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                                      float[] margins, float[] paddings, float[] borders) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;

    try {
      errorCode = WXBridgeManager.getInstance().callCreateBody(String.valueOf(instanceId), componentType, ref,
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
  public int callAddElement(int instanceId, String componentType, String ref, int index, String parentRef,
                                      HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                                      float[] margins, float[] paddings, float[] borders) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;

    try {
      errorCode = WXBridgeManager.getInstance().callAddElement(String.valueOf(instanceId), componentType, ref, index, parentRef,
              styles, attributes, events, margins, paddings, borders);
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
  public int callRemoveElement(int instanceId, String ref) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(String.valueOf(instanceId));
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveElement(String.valueOf(instanceId), ref);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callRemoveElement throw exception:" + e.getMessage());
      }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callMoveElement(int instanceId, String ref, String parentref, int index) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(String.valueOf(instanceId));
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callMoveElement(String.valueOf(instanceId), ref, parentref, index);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callMoveElement throw exception:" + e.getMessage());
      }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callAddEvent(int instanceId, String ref, String event) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(String.valueOf(instanceId));
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callAddEvent(String.valueOf(instanceId), ref, event);
    } catch (Throwable e) {
      //catch everything during call native.
      // if(WXEnvironment.isApkDebugable()){
      WXLogUtils.e(TAG, "callAddEvent throw exception:" + e.getMessage());
      // }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callRemoveEvent(int instanceId, String ref, String event) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(String.valueOf(instanceId));
    if (instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveEvent(String.valueOf(instanceId), ref, event);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callRemoveEvent throw exception:" + e.getMessage());
      }
    }
    if (instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callUpdateStyle(int instanceId, String ref,
                                       HashMap<String, Object> styles,
                                       HashMap<String, String> paddings,
                                       HashMap<String, String> margins,
                                       HashMap<String, String> borders) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateStyle(String.valueOf(instanceId), ref, styles, paddings, margins, borders);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callUpdateStyle throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  public int callUpdateAttrs(int instanceId, String ref,
                                       HashMap<String, String> attrs) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateAttrs(String.valueOf(instanceId), ref, attrs);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callUpdateAttr throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  public int callLayout(int instanceId, String ref, int top, int bottom, int left, int right, int height, int width) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callLayout(String.valueOf(instanceId), ref, top, bottom, left, right, height, width);
    } catch (Throwable e) {
      //catch everything during call native.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callLayout throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  public int callCreateFinish(int instanceId) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callCreateFinish(String.valueOf(instanceId));
    } catch (Throwable e) {
      WXLogUtils.e(TAG, "callCreateFinish throw exception:" + e.getMessage());
    }
    return errorCode;
  }

  @Override
  public int callHasTransitionPros(int instanceId, String ref, HashMap<String, String> styles) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callHasTransitionPros(String.valueOf(instanceId), ref, styles);
    } catch (Throwable e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(TAG, "callHasTransitionPros throw exception:" + e.getMessage());
      }
    }
    return errorCode;
  }

  @Override
  public void setStyleWidth(String instanceId, String ref, float value) {
    nativeSetStyleWidth(Integer.parseInt(instanceId), ref, value);
  }

  @Override
  public void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetMargin(Integer.parseInt(instanceId), ref, edge.ordinal(), value);
  }

  @Override
  public void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetPadding(Integer.parseInt(instanceId), ref, edge.ordinal(), value);
  }

  @Override
  public void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    nativeSetPosition(Integer.parseInt(instanceId), ref, edge.ordinal(), value);
  }

  @Override
  public void markDirty(String instanceId, String ref, boolean dirty) {
    nativeMarkDirty(Integer.parseInt(instanceId), ref, dirty);
  }

  @Override
  public void setStyleHeight(String instanceId, String ref, float value) {
    nativeSetStyleHeight(Integer.parseInt(instanceId), ref, value);
  }

  @Override
  public void registerCoreEnv(String key, String value) {
    nativeRegisterCoreEnv(key, value);
  }

  @Override
  public void setViewPortWidth(String instanceId, float value) {
    nativeSetViewPortWidth(Integer.parseInt(instanceId), value);
  }
}
