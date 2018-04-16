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
import com.taobao.weex.dom.CSSShorthand;

import java.util.HashMap;
import java.util.HashSet;

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
   * init Weex
   *
   * @param framework assets/main.js
   * @return
   */
  int initFrameworkEnv(String framework, WXParams params, String cacheDir, boolean pieSupport);

  /**
   * execute javascript function
   */
  int execJS(String instanceId, String namespace, String function, WXJSObject[] args);

  /**
   * execute javascript function, return execute result as json array
   */
  byte[] execJSWithResult(String instanceId, String namespace, String function, WXJSObject[] args);

  int execJSService(String javascript);

  /**
   * take the heap snapshot and serialize the heap to a local file.
   *
   * @param filename
   */
  void takeHeapSnapshot(String filename);

  /**
   * createInstance
   * @param instanceId
   * @param namespace
   * @param function
   * @param args
   * @return
   */
  int createInstanceContext(String instanceId, String namespace, String function, WXJSObject[] args);

  /**
   * destoryInstance
   * @param instanceId
   * @param namespace
   * @param function
   * @param args
   * @return
   */
  int destoryInstance(String instanceId, String namespace, String function, WXJSObject[] args);

  /**
   * execJSOnInstance
   * @param instanceId
   * @param script
   * @param type
   * @return
   */
  String execJSOnInstance(String instanceId, String script, int type);

  /**
   * js call native
   */
  int callNative(int instanceId, String tasks, String callback);

  void reportJSException(int instanceId, String func, String exception);

  Object callNativeModule(int instanceId, String module, String method, byte[] arguments, byte[] options);

  void callNativeComponent(int instanceId, int ref, String method, byte[] arguments, byte[] options);

  int callUpdateFinish(int instanceId, byte[] tasks, String callback);

  int callRefreshFinish(int instanceId, byte[] tasks, String callback);

  void reportServerCrash(String instanceId, String crashFile);


  public int callCreateBody(int instanceId, String componentType, int ref,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders);

  public int callAddElement(int instanceId, String componentType, int ref, int index, int parentRef,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders);

  int callRemoveElement(int instanceId, int ref);

  int callMoveElement(int instanceId, int ref, int parentref, int index);

  int callAddEvent(int instanceId, int ref, String event);

  int callRemoveEvent(int instanceId, int ref, String event);

  public int callUpdateStyle(int instanceId, int ref,
                             HashMap<String, Object> styles,
                             HashMap<String, String> paddings,
                             HashMap<String, String> margins,
                             HashMap<String, String> borders);

  int callUpdateAttrs(int instanceId, int ref,
                      HashMap<String, String> attrs);

  int callLayout(int instanceId, int ref, int top, int bottom, int left, int right, int height, int width);

  int callCreateFinish(int instanceId);

  int callHasTransitionPros(int instanceId, int ref, HashMap<String, String> styles);

  void setStyleWidth(int instanceId, int ref, float value);

  void setStyleHeight(int instanceId, int ref, float value);

  void setMargin(int instanceId, int ref, CSSShorthand.EDGE edge, float value);

  void setPadding(int instanceId, int ref, CSSShorthand.EDGE edge, float value);

  void setPosition(int instanceId, int ref, CSSShorthand.EDGE edge, float value);

  void markDirty(int instanceId, int ref, boolean dirty);

  void registerCoreEnv(String key, String value);

  void setViewPortWidth(int instanceId, float value);

}
