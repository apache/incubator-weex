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

import com.taobao.weex.bridge.ResultCallback;
import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.bridge.WXParams;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.layout.ContentBoxMeasurement;
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
   * Update InitFramework Params
   * */
  void updateInitFrameworkParams(String key, String value, String desc);


  void refreshInstance(String instanceId, String namespace, String function, WXJSObject[] args);

  /**
   * execute javascript function
   */
  int execJS(String instanceId, String namespace, String function, WXJSObject[] args);

  /**
   * execute javascript function with asynchronous callback
   */
  void execJSWithCallback(String instanceId, String namespace, String function, WXJSObject[] args, ResultCallback callback);

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
  int callNative(String instanceId, byte[] tasks, String callback);


  int callNative(String instanceId, String tasks, String callback);

  void reportJSException(String instanceId, String func, String exception);

  Object callNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options);

  void callNativeComponent(String instanceId, String ref, String method, byte[] arguments, byte[] options);

  int callUpdateFinish(String instanceId, byte[] tasks, String callback);

  int callRefreshFinish(String instanceId, byte[] tasks, String callback);

  void reportServerCrash(String instanceId, String crashFile);


  int callCreateBody(String instanceId, String componentType, String ref,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders);

  int callAddElement(String instanceId, String componentType, String ref, int index, String parentRef,
                            HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events,
                            float[] margins, float[] paddings, float[] borders, boolean willLayout);

  int callRemoveElement(String instanceId, String ref);

  int callMoveElement(String instanceId, String ref, String parentref, int index);

  int callAddEvent(String instanceId, String ref, String event);

  int callRemoveEvent(String instanceId, String ref, String event);

  int callUpdateStyle(String instanceId, String ref,
                             HashMap<String, Object> styles,
                             HashMap<String, String> paddings,
                             HashMap<String, String> margins,
                             HashMap<String, String> borders);

  int callUpdateAttrs(String instanceId, String ref,
                      HashMap<String, String> attrs);

  int callLayout(String instanceId, String ref, int top, int bottom, int left, int right, int height, int width, boolean isRTL, int index);

  int callCreateFinish(String instanceId);

  int callRenderSuccess(String instanceId);

  int callAppendTreeCreateFinish(String instanceId, String ref);

  int callHasTransitionPros(String instanceId, String ref, HashMap<String, String> styles);

  ContentBoxMeasurement getMeasurementFunc(String instanceId, long renderObjectPtr);

  void bindMeasurementToRenderObject(long ptr);

  void setRenderContainerWrapContent(boolean wrap, String instanceId);

  long[] getFirstScreenRenderTime(String instanceId);

  long[] getRenderFinishTime(String instanceId);

  void setDefaultHeightAndWidthIntoRootDom(String instanceId, float defaultWidth, float defaultHeight, boolean isWidthWrapContent, boolean isHeightWrapContent);

  void onInstanceClose(String instanceId);

  void forceLayout(String instanceId);

  boolean notifyLayout(String instanceId);

  void setStyleWidth(String instanceId, String ref, float value);

  void setStyleHeight(String instanceId, String ref, float value);

  void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value);

  void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value);

  void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value);

  void markDirty(String instanceId, String ref, boolean dirty);

  void setDeviceDisplay(String instanceId, float width, float height, float scale);

  void registerCoreEnv(String key, String value);

  void reportNativeInitStatus(String statusCode, String errorMsg);

  void setTimeoutNative(String callbackId, String time);

  void setJSFrmVersion(String version);

  void resetWXBridge(boolean remoteDebug);

}
