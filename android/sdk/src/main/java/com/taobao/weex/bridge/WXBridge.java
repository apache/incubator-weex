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

import android.text.TextUtils;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Communication interface for Java code and JavaScript code.
 */
class WXBridge implements IWXBridge {

  public static final String TAG = "WXBridge";

  /**
   * Init JSFrameWork
   *
   * @param framework assets/main.js
   */
  public native int initFramework(String framework, WXParams params);


  /**
   * Execute JavaScript function
   *
   * @param instanceId
   * @param namespace  default global
   * @param function   function string name
   * @param args       WXJSObject array
   */
  public native int execJS(String instanceId, String namespace, String function, WXJSObject[] args);


  /**
   * register Weex Service
   *
   * @param javascript  code
   */
  public native int execJSService(String javascript);

  /**
   * Take v8's heap snapshot
   * @param filename the name of the file to be written.
   */
  public native void takeHeapSnapshot(String filename);


  /**
   * Called to clear all unhandled c++ timer when an instance is
   * destroyed
   * @param instanceId the destoyed instance's id.
   */
  public native void clearInstanceCallback(String instanceId);

  /**
   * JavaScript uses this methods to call Android code
   *
   * @param instanceId
   * @param tasks
   * @param callback
   */

  public int callNative(String instanceId, byte [] tasks, String callback) {
     return callNative(instanceId,new String(tasks),callback);
  }

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

  /**
   * JSF render Node by callAddElement
   */
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
   * Report JavaScript error.
   *
   * @param instanceId
   * @param func       exception function
   * @throws String exception
   */
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
    Object object =  WXBridgeManager.getInstance().callNativeModule(instanceId,module,method,argArray,options);
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
    if(!TextUtils.isEmpty(version)) {
      WXEnvironment.JS_LIB_SDK_VERSION = version;
    }
  }
}
