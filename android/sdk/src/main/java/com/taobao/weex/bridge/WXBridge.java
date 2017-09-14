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
import com.taobao.weex.utils.WXLogUtils;

/**
 * Communication interface for Java code and JavaScript code.
 */

public class WXBridge implements IWXBridge {

  private native int nativeInitFramework(String framework, WXParams params);
  private native int nativeExecJS(String instanceId, String name, String function, WXJSObject[] args);
  private native void nativeOnVsync(String instanceId);
  private native int nativeExecJSService(String javascript);
  private native void nativeTakeHeapSnapshot(String filename);


  @Override
  public int initFramework(String framework, WXParams params) {
    return nativeInitFramework(framework, params);
  }

  @Override
  public int execJS(String instanceId, String namespace, String function, WXJSObject[] args) {
    return nativeExecJS(instanceId, namespace, function, args);
  }

  @Override
  public void onVsync(String instanceId) {
    nativeOnVsync(instanceId);
  }

  @Override
  public int execJSService(String javascript) {
    return nativeExecJSService(javascript);
  }

  @Override
  public void takeHeapSnapshot(String filename) {
    nativeTakeHeapSnapshot(filename);
  }


  public static final String TAG = "WXBridge";

  public int callNative(String instanceId, byte [] tasks, String callback) {
     return callNative(instanceId,new String(tasks),callback);
  }

  @Override
  public int callNative(String instanceId, String tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callNative(instanceId, tasks, callback);
    }catch (Throwable e){
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callNative throw exception:"+e.getMessage());
      }
    }

    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    if(WXEnvironment.isApkDebugable()){
      if(errorCode == IWXBridge.DESTROY_INSTANCE){
        WXLogUtils.w("destroyInstance :"+instanceId+" JSF must stop callNative");
      }
    }
    return errorCode;
  }

  public int callAddElement(String instanceId, String ref,byte[] dom,String index, String callback) {
    return callAddElement(instanceId,ref, new String(dom),index,callback);
  }

  @Override
  public int callAddElement(String instanceId, String ref,String dom,String index, String callback) {

    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;

    try {
      errorCode = WXBridgeManager.getInstance().callAddElement(instanceId, ref,dom,index, callback);
    }catch (Throwable e){
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        e.printStackTrace();
        WXLogUtils.e(TAG,"callNative throw error:"+e.getMessage());
      }
    }

    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    if(WXEnvironment.isApkDebugable()){
      if(errorCode == IWXBridge.DESTROY_INSTANCE){
        WXLogUtils.w("destroyInstance :"+instanceId+" JSF must stop callNative");
      }
    }
    return errorCode;
  }

  /**
   * JavaScript uses this methods to call Android code
   *
   * @param instanceId
   * @param tasks
   * @param callback
   */

  public int callCreateBody(String instanceId, byte [] tasks, String callback) {
    return callCreateBody(instanceId,new String(tasks),callback);
  }

  @Override
  public int callCreateBody(String instanceId, String tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callCreateBody(instanceId, tasks, callback);
    }catch (Throwable e){
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callCreateBody throw exception:"+e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }


  public void reportJSException(String instanceId, String func, String exception) {
    WXBridgeManager.getInstance().reportJSException(instanceId, func, exception);
  }


  /**
   * Bridge module Js Method
   * support Sync or Async through setting  Annotation as {@link com.taobao.weex.annotation.JSMethod }
   * @param instanceId  Instance ID
   * @param module  the name of module
   * @param method  the name of method
   * @param arguments  the arguments of the method
   * @param options  option arguments for extending
   * @return  the result
   */
  @Override
  public Object callNativeModule(String instanceId, String module, String method, byte [] arguments, byte [] options) {

    JSONArray argArray = JSON.parseArray(new String(arguments));
    JSONObject optionsObj = null;
    if (options != null) {
      optionsObj = JSON.parseObject(new String(options));
    }
    Object object =  WXBridgeManager.getInstance().callNativeModule(instanceId,module,method,argArray,optionsObj);
    return new WXJSObject(object);
  }

  /**
   * Bridge component Js Method
   * @param instanceId  Instance ID
   * @param componentRef  the ref of component
   * @param method  the name of method
   * @param arguments  the arguments of the method
   * @param options  option arguments for extending
   */
  @Override
  public void callNativeComponent(String instanceId, String componentRef, String method, byte [] arguments, byte [] options) {
    JSONArray argArray = JSON.parseArray(new String(arguments));
     WXBridgeManager.getInstance().callNativeComponent(instanceId,componentRef,method,argArray,options);
  }

  public void setTimeoutNative(String callbackId, String time) {
    WXBridgeManager.getInstance().setTimeout(callbackId, time);
  }

  public void setJSFrmVersion(String version) {
    if(version != null) {
      WXEnvironment.JS_LIB_SDK_VERSION = version;
    }
  }

  @Override
  public int callUpdateFinish(String instanceId, byte [] tasks, String callback) {

    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callCreateBody throw exception:"+e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callCreateFinish(String instanceId, byte [] tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callCreateFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callCreateFinish throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callRefreshFinish(String instanceId, byte [] tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRefreshFinish(instanceId, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callCreateFinish throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callUpdateAttrs(String instanceId, String ref, byte [] tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateAttrs(instanceId, ref, new String(tasks), callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callUpdateAttrs throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callUpdateStyle(String instanceId, String ref, byte [] tasks, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callUpdateStyle(instanceId, ref, new String(tasks), callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callUpdateStyle throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callRemoveElement(String instanceId, String ref, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveElement(instanceId, ref, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callRemoveElement throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callMoveElement(String instanceId, String ref, String parentref, String index, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      // Integer.parseInt(index)
      errorCode = WXBridgeManager.getInstance().callMoveElement(instanceId, ref, parentref, index, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callMoveElement throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callAddEvent(String instanceId, String ref, String event, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callAddEvent(instanceId, ref, event, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callAddEvent throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  @Override
  public int callRemoveEvent(String instanceId, String ref, String event, String callback) {
    long start = System.currentTimeMillis();
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if(instance != null) {
      instance.firstScreenCreateInstanceTime(start);
    }
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callRemoveEvent(instanceId, ref, event, callback);
    } catch (Throwable e) {
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"callRemoveEvent throw exception:" + e.getMessage());
      }
    }
    if(instance != null) {
      instance.callNativeTime(System.currentTimeMillis() - start);
    }
    return errorCode;
  }

  public void reportServerCrash(String instanceId, String crashFile) {
    int errorCode = IWXBridge.INSTANCE_RENDERING;
    try {
      errorCode = WXBridgeManager.getInstance().callReportCrashReloadPage(instanceId, crashFile);

      // upload crash log
    }catch (Throwable e){
      //catch everything during call native.
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e(TAG,"reloadPageNative throw exception:"+e.getMessage());
      }
    }
  }

}
