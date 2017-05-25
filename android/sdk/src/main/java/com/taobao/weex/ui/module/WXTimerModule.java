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
package com.taobao.weex.ui.module;

import android.os.Handler;
import android.os.Message;
import android.support.annotation.IntRange;
import android.support.annotation.VisibleForTesting;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXHashMap;
import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.WXJSBridgeMsgType;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.HashMap;

import static com.taobao.weex.bridge.WXBridgeManager.KEY_ARGS;
import static com.taobao.weex.bridge.WXBridgeManager.KEY_METHOD;
import static com.taobao.weex.bridge.WXBridgeManager.METHOD_CALLBACK;
import static com.taobao.weex.bridge.WXBridgeManager.METHOD_CALL_JS;

public class WXTimerModule extends WXModule implements Destroyable, Handler.Callback {

  private final static String TAG = "timer";
  private Handler handler;

  public WXTimerModule() {
    handler = new Handler(WXBridgeManager.getInstance().getJSLooper(), this);
  }

  @VisibleForTesting
  void setHandler(Handler handler) {
    this.handler = handler;
  }

  @JSMethod(uiThread = false)
  public void setTimeout(@IntRange(from = 1) int funcId, @IntRange(from = 0) int delay) {
    if(mWXSDKInstance != null) {
      postOrHoldMessage(WXJSBridgeMsgType.MODULE_TIMEOUT, funcId, delay, Integer.parseInt(mWXSDKInstance.getInstanceId()));
    }
  }

  @JSMethod(uiThread = false)
  public void setInterval(@IntRange(from = 1) int funcId, @IntRange(from = 0) int interval) {
    if(mWXSDKInstance != null) {
      postOrHoldMessage(WXJSBridgeMsgType.MODULE_INTERVAL, funcId, interval, Integer.parseInt(mWXSDKInstance.getInstanceId()));
    }
  }

  @JSMethod(uiThread = false)
  public void clearTimeout(@IntRange(from = 1) int funcId) {
    if (funcId <= 0) {
      return;
    }
    removeOrHoldMessage(WXJSBridgeMsgType.MODULE_TIMEOUT, funcId);
  }

  @JSMethod(uiThread = false)
  public void clearInterval(@IntRange(from = 1) int funcId) {
    if (funcId <= 0) {
      return;
    }
    removeOrHoldMessage(WXJSBridgeMsgType.MODULE_INTERVAL, funcId);
  }

  @Override
  public void destroy() {
    if (handler != null) {
      WXLogUtils.d(TAG, "Timer Module removeAllMessages: ");
      handler.removeCallbacksAndMessages(null);
    }
  }

  @Override
  public boolean handleMessage(Message msg) {
    boolean ret = false;
    WXJSObject[] args;
    if (msg != null) {
      int what = msg.what;
      WXLogUtils.d(TAG, "Timer Module handleMessage : " + msg.what);
      switch (what) {
        case WXJSBridgeMsgType.MODULE_TIMEOUT:
          if (msg.obj == null) {
            break;
          }
          args = createTimerArgs(msg.arg1, (Integer) msg.obj, false);
          WXBridgeManager.getInstance().invokeExecJS(String.valueOf(msg.arg1), null, METHOD_CALL_JS, args, true);
          ret = true;
          break;
        case WXJSBridgeMsgType.MODULE_INTERVAL:
          if (msg.obj == null) {
            break;
          }
          postMessage(WXJSBridgeMsgType.MODULE_INTERVAL, (Integer) msg.obj, msg.arg2, msg.arg1);
          args = createTimerArgs(msg.arg1, (Integer) msg.obj, true);
          WXBridgeManager.getInstance().invokeExecJS(String.valueOf(msg.arg1), null, METHOD_CALL_JS, args, true);
          ret = true;
          break;
        default:
          break;
      }
    }
    return ret;
  }

  private WXJSObject[] createTimerArgs(int instanceId, int funcId, boolean keepAlive) {
    ArrayList<Object> argsList = new ArrayList<>();
    argsList.add(funcId);
    argsList.add(new HashMap<>());
    argsList.add(keepAlive);
    WXHashMap<String, Object> task = new WXHashMap<>();
    task.put(KEY_METHOD, METHOD_CALLBACK);
    task.put(KEY_ARGS, argsList);
    Object[] tasks = {task};
    return new WXJSObject[]{
        new WXJSObject(WXJSObject.String, String.valueOf(instanceId)),
        new WXJSObject(WXJSObject.JSON,
                       WXJsonUtils.fromObjectToJSONString(tasks))};
  }

  private void postMessage(int what,
                           @IntRange(from = 1) int funcId,
                           @IntRange(from = 0) int interval, int instanceId) {
    if (interval < 0 || funcId <= 0) {
      WXLogUtils.e(TAG, "interval < 0 or funcId <=0");
    } else {
      Message message = Message.obtain();
      message.what = what;
      message.arg1 = instanceId;
      message.arg2 = interval;
      message.obj = funcId;
      handler.sendMessageDelayed(message, interval);
    }
  }


  private void postOrHoldMessage(final int what,final int funcId,final int interval,final int instanceId) {
    if(mWXSDKInstance.isPreRenderMode()) {
      WXSDKManager.getInstance().getWXDomManager().postAction(mWXSDKInstance.getInstanceId(), Actions.getExecutableRenderAction(new Runnable() {
        @Override
        public void run() {
          postMessage(what,funcId,interval,instanceId);
        }
      }),false);
    } else {
      postMessage(what,funcId,interval,instanceId);
    }
  }

  private void removeOrHoldMessage(final int what,final int funcId) {
    if(mWXSDKInstance.isPreRenderMode()) {
      WXSDKManager.getInstance().getWXDomManager().postAction(mWXSDKInstance.getInstanceId(), Actions.getExecutableRenderAction(new Runnable() {
        @Override
        public void run() {
          handler.removeMessages(what, funcId);
        }
      }),false);
    } else {
      handler.removeMessages(what, funcId);
    }
  }

}
