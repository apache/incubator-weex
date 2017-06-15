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
package com.taobao.weex.common;

import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.bridge.WXParams;

/**
 * Bridge interface, native bridge and debug bridge both need to implement this interface
 */
public interface IWXBridge extends IWXObject {

  int DESTROY_INSTANCE = -1;
  int INSTANCE_RENDERING = 1;
  int INSTANCE_RENDERING_ERROR = 0;

  /**
   * init Weex
   *
   * @param framework assets/main.js
   * @return
   */
  int initFramework(String framework, WXParams params);

  /**
   * execute javascript function
   */
  int execJS(String instanceId, String namespace, String function, WXJSObject[] args);

  int execJSService(String javascript);

  /**
   * take the heap snapshot and serialize the heap to a local file.
   *
   * @param filename
   */
  void takeHeapSnapshot(String filename);

  /**
   * js call native

   */
  int callNative(String instanceId, String tasks, String callback);

  int callAddElement(String instanceId, String ref,String dom,String index, String callback);

  void reportJSException(String instanceId, String func, String exception);

  Object callNativeModule(String instanceId, String module, String method,  byte [] arguments,  byte [] options);

  void callNativeComponent(String instanceId, String componentRef, String method,  byte [] arguments,  byte [] options);

  int callCreateBody(String instanceId, String tasks, String callback);

  int callUpdateFinish(String instanceId,  byte [] tasks, String callback);

  int callCreateFinish(String instanceId,  byte [] tasks, String callback);

  int callRefreshFinish(String instanceId,  byte [] tasks, String callback);

  int callUpdateAttrs(String instanceId, String ref,  byte [] tasks, String callback);

  int callUpdateStyle(String instanceId, String ref,  byte [] tasks, String callback);

  int callRemoveElement(String instanceId, String ref, String callback);

  int callMoveElement(String instanceId, String ref, String parentref, String index, String callback);

  int callAddEvent(String instanceId, String ref, String event, String callback);

  int callRemoveEvent(String instanceId, String ref, String event, String callback);

}
