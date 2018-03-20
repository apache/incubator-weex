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
import android.widget.Toast;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.squareup.okhttp.Response;
import com.squareup.okhttp.ws.WebSocket;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.websocket.WXWebSocketManager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import okio.BufferedSource;

/**
 * websocket bridge
 *
 * @author gulin
 */
public class WXWebsocketBridge implements IWXBridge, WXWebSocketManager.JSDebuggerCallback {

  private WXBridgeManager mJsManager;
  private volatile boolean mInit = false;

  public WXWebsocketBridge(WXBridgeManager jm) {
    mJsManager = jm;
    WXWebSocketManager.getInstance().registerListener(this);
  }

  @Override
  public int execJS(String instanceId, String namespace, String function,
                    WXJSObject[] args) {
    if (!mInit || TextUtils.isEmpty(instanceId)
            || TextUtils.isEmpty(function)) {
      return -1;
    }

    Map<String, Object> map = new HashMap<>();
    map.put("method", function);

    ArrayList<Object> array = new ArrayList<>();
    int argsCount = args == null ? 0 : args.length;
    for (int i = 0; i < argsCount; i++) {
      if (args[i].type != WXJSObject.String) {
        array.add(JSON.parse(args[i].data.toString()));
      } else {
        array.add(args[i].data);
      }
    }
    map.put("arguments", array);
    WXWebSocketManager.getInstance().sendMessage(JSON.toJSONString(map));
    return 0;
  }

  @Override
  public byte[] execJSWithResult(String instanceId, String namespace, String function, WXJSObject[] args) {
    // TODO
    return new byte[0];
  }


  @Override
  public int execJSService(String javascript) {
    return 0;
  }

  @Override
  public int callNative(String instanceId, String tasks, String callback) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callNative(instanceId, tasks, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callUpdateFinish(String instanceId, byte[] tasks, String callback) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callUpdateFinish(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callRefreshFinish(String instanceId, byte[] tasks, String callback) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callRefreshFinish(instanceId, callback);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int initFramework(String scriptsFramework, WXParams params) {
    if (!mInit) {
      return -1;
    }

    Map<String, Object> map = new HashMap<>();
    map.put("method", "evalFramework");
    ArrayList<String> args = new ArrayList<>();
    args.add(scriptsFramework);
    map.put("arguments", args);
    WXWebSocketManager.getInstance().sendMessage(JSON.toJSONString(map));
    return 0;
  }

  @Override
  public int initFrameworkEnv(String scriptsFramework, WXParams params, String cacheDir, boolean pieSupport) {
    if (!mInit) {
      return -1;
    }

    Map<String, Object> map = new HashMap<>();
    map.put("method", "evalFramework");
    ArrayList<String> args = new ArrayList<>();
    args.add(scriptsFramework);
    map.put("arguments", args);
    WXWebSocketManager.getInstance().sendMessage(JSON.toJSONString(map));
    return 0;
  }

  @Override
  public void takeHeapSnapshot(String filename) {
  }

  @Override
  public void reportJSException(String instanceId, String func, String exception) {
    if (mJsManager != null) {
      mJsManager.reportJSException(instanceId, func, exception);
    }
  }

  @Override
  public String callNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options) {
    return null;
  }

  @Override
  public void callNativeComponent(String instanceId, String componentRef, String method, byte[] arguments, byte[] options) {

  }

  @Override
  public void reportServerCrash(String instanceId, String crashFile) {

  }

  @Override
  public int callCreateBody(String pageId, String componentType, String ref, HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events, float[] margins, float[] paddings, float[] borders) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callCreateBody(pageId, componentType, ref, styles, attributes, events, margins, paddings, borders);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callAddElement(String pageId, String componentType, String ref, int index, String parentRef, HashMap<String, String> styles, HashMap<String, String> attributes, HashSet<String> events, float[] margins, float[] paddings, float[] borders) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callAddElement(pageId, componentType, ref, index, parentRef, styles, attributes, events, margins, paddings, borders);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callRemoveElement(String instanceId, String ref) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callRemoveElement(instanceId, ref);


    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callMoveElement(String instanceId, String ref, String parentref, int index) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callMoveElement(instanceId, ref, parentref, index);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callAddEvent(String instanceId, String ref, String event) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callAddEvent(instanceId, ref, event);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callRemoveEvent(String instanceId, String ref, String event) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callRemoveEvent(instanceId, ref, event);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callUpdateStyle(String instanceId, String ref, HashMap<String, Object> styles, HashMap<String, String> paddings, HashMap<String, String> margins, HashMap<String, String> borders) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callUpdateStyle(instanceId, ref, styles, paddings, margins, borders);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callUpdateAttrs(String instanceId, String ref, HashMap<String, String> attrs) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callUpdateAttrs(instanceId, ref, attrs);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callLayout(String pageId, String ref, int top, int bottom, int left, int right, int height, int width) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callLayout(pageId, ref, top, bottom, left, right, height, width);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public int callCreateFinish(String instanceId) {
    if (!mInit || mJsManager == null)
      return IWXBridge.INSTANCE_RENDERING_ERROR;
    mJsManager.callCreateFinish(instanceId);
    return IWXBridge.INSTANCE_RENDERING;
  }

  @Override
  public void onMessage(BufferedSource payload, WebSocket.PayloadType type) {
    if (type != WebSocket.PayloadType.TEXT) {
      WXLogUtils.w("Websocket received unexpected message with payload of type " + type);
      return;
    }

    String message = null;
    try {
      message = payload.readUtf8();
      JSONObject jsonObject = JSONObject.parseObject(message);
      Object name = jsonObject.get("name");
      Object value = jsonObject.get("value");
      if (name == null || value == null) {
        return;
      }
      if (name.toString().equals("callNative")) {
        JSONArray jsonArray = JSONObject.parseArray(value.toString());
        callNative(jsonArray.getString(0), jsonArray.getString(1),
                jsonArray.getString(2));
      }
    } catch (Exception e) {

    } finally {
      try {
        payload.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  @Override
  public void onSuccess(Response response) {
    if (response.isSuccessful()) {
      WXSDKManager.getInstance().postOnUiThread(new Runnable() {
        @Override
        public void run() {
          Toast.makeText(WXEnvironment.sApplication, "Has switched to DEBUG mode, you can see the DEBUG information on the browser!", Toast.LENGTH_SHORT).show();
        }
      }, 0);
    }
  }

  @Override
  public void onFailure(Throwable cause) {
    Toast.makeText(WXEnvironment.sApplication, "socket connect failure!", Toast.LENGTH_SHORT).show();
  }

  public int callHasTransitionPros(String instanceId, String ref, HashMap<String, String> styles) {
    return mJsManager.callHasTransitionPros(instanceId, ref, styles);
  }

  @Override
  public void setStyleWidth(String instanceId, String ref, float value) {
    mJsManager.setStyleWidth(instanceId, ref, value);
  }

  @Override
  public void setStyleHeight(String instanceId, String ref, float value) {
    mJsManager.setStyleHeight(instanceId, ref, value);
  }

  @Override
  public void setMargin(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mJsManager.setMargin(instanceId, ref, edge, value);
  }

  @Override
  public void setPadding(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mJsManager.setPadding(instanceId, ref, edge, value);
  }

  @Override
  public void setPosition(String instanceId, String ref, CSSShorthand.EDGE edge, float value) {
    mJsManager.setPosition(instanceId, ref, edge, value);
  }

  @Override
  public void calculateLayout(String instanceId, String ref, boolean dirty) {
    mJsManager.calculateLayout(instanceId, ref, dirty);
  }

  @Override
  public void registerCoreEnv(String key, String value) {
    mJsManager.registerCoreEnv(key, value);
  }

  @Override
  public void setViewPortWidth(String instanceId, float value) {
    mJsManager.setViewPortWidth(instanceId, value);
  }
}
