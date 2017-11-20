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
package com.taobao.weex.websocket;

import android.text.TextUtils;
import android.widget.Toast;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;
import com.squareup.okhttp.ws.WebSocket;
import com.squareup.okhttp.ws.WebSocketCall;
import com.squareup.okhttp.ws.WebSocketListener;
import com.taobao.weex.utils.LogLevel;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.utils.WXLogUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

import okio.Buffer;
import okio.BufferedSource;

public class WXWebSocketManager implements WebSocketListener {

  private static WXWebSocketManager sWebsocket = new WXWebSocketManager();

  private final ConcurrentHashMap<Integer, JSDebuggerCallback> mCallbacks = new ConcurrentHashMap<>();
  private WebSocket mWebSocket;
  private OkHttpClient mHttpClient;

  private boolean isSupportWebSocket =true;

  public static WXWebSocketManager getInstance() {
    return sWebsocket;
  }

  private WXWebSocketManager() {
  }

  public void connect(String url) {
    try {
      mHttpClient= (OkHttpClient) Class.forName("com.squareup.okhttp.OkHttpClient").newInstance();
    } catch (Exception e) {
      isSupportWebSocket =false;
      return;
    }
    mHttpClient.setConnectTimeout(10, TimeUnit.SECONDS);
    mHttpClient.setWriteTimeout(10, TimeUnit.SECONDS);
    // Disable timeouts for read
    mHttpClient.setReadTimeout(0, TimeUnit.MINUTES);

    Request request = new Request.Builder().url(url).build();
    WebSocketCall call = WebSocketCall.create(mHttpClient, request);
    call.enqueue(this);
  }

  public boolean isConnect(String url) {
    return mHttpClient != null;
  }

  public void registerListener(JSDebuggerCallback callback) {
    mCallbacks.put(mCallbacks.size(), callback);
  }

  public void closeQuietly() {
    if(!isSupportWebSocket){
      return;
    }
    if (mWebSocket != null) {
      try {
        mWebSocket.close(1000, "End of session");
      } catch (IOException e) {
        // swallow, no need to handle it here
      }
      mWebSocket = null;
    }
  }

  public void sendMessage(String message) {
    if(!isSupportWebSocket){
      return;
    }
    if (mWebSocket == null) {
      return;
    }
    Buffer messageBuffer = new Buffer();
    messageBuffer.writeUtf8(message);
    try {
      mWebSocket.sendMessage(WebSocket.PayloadType.TEXT, messageBuffer);
    } catch (IOException e) {
    }
  }

  @Override
  public void onMessage(BufferedSource payload, WebSocket.PayloadType type)
      throws IOException {
    if (type != WebSocket.PayloadType.TEXT) {
      WXLogUtils.w(
          "Websocket received unexpected message with payload of type "
          + type);
      return;
    }
    for (JSDebuggerCallback callback : mCallbacks.values()) {
      callback.onMessage(payload, type);
    }

    String message = null;
    try {
      message = payload.readUtf8();
      JSONObject jsonObject = JSONObject.parseObject(message);
      Object name = jsonObject.get("method");
      Object value = jsonObject.get("arguments");
      if (name == null || value == null) {
        return;
      }
      if (TextUtils.equals(name.toString(), "setLogLevel")) {
        JSONArray jsonArray = JSONObject.parseArray(value.toString());
        String level = jsonArray.get(0).toString();
        WXEnvironment.sLogLevel = LogLevel.valueOf(level.toUpperCase());
        WXLogUtils.v("into--[onMessage]setLogLevel");
      }
    } catch (Exception e) {

    } finally {
      payload.close();
    }
  }

  @Override
  public void onClose(int code, String reason) {
    mWebSocket = null;
  }

  @Override
  public void onPong(Buffer payload) {
    // ignore
  }

  private void abort(String message, Throwable cause) {
    WXLogUtils.e(
        "Error occurred, shutting down websocket connection: "
        + message);
    closeQuietly();

    for (JSDebuggerCallback callback : mCallbacks.values()) {
      callback.onFailure(cause);
    }
    mCallbacks.clear();
  }

  @Override
  public void onFailure(IOException e) {
    abort("Websocket exception", e);
    WXSDKManager.getInstance().postOnUiThread(new Runnable() {
      @Override
      public void run() {
        Toast.makeText(WXEnvironment.sApplication,"socket connect failure!",Toast.LENGTH_SHORT).show();
      }
    },0);
  }

  @Override
  public void onOpen(WebSocket webSocket, Request arg1, Response arg2)
      throws IOException {
    mWebSocket = webSocket;
    setEnvironment(WXEnvironment.getConfig());
    WXSDKManager.getInstance().postOnUiThread(new Runnable() {
      @Override
      public void run() {
        Toast.makeText(WXEnvironment.sApplication, "Has switched to DEBUG mode, you can see the DEBUG information on the browser!", Toast.LENGTH_SHORT).show();
      }
    },0);
    for (JSDebuggerCallback callback : mCallbacks.values()) {
      callback.onSuccess(arg2);
    }
    WXLogUtils.e("into--[onOpen]");
  }

  public interface JSDebuggerCallback {

    void onMessage(BufferedSource payload, WebSocket.PayloadType type);

    void onSuccess(Response response);

    void onFailure(Throwable cause);
  }

  private void setEnvironment(Map<String, String> options) {
    if(!isSupportWebSocket){
      return;
    }
    List<Object> arguments = new ArrayList<>();
    arguments.add(options);
    Map<String, Object> objectMap = new HashMap<>();
    objectMap.put("method", "setEnvironment");
    objectMap.put("arguments", arguments);
    sendMessage(JSON.toJSONString(objectMap));
    WXLogUtils.info("into--[onOpen]");
  }
}
