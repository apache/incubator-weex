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

import static com.taobao.weex.bridge.WXBridgeManager.KEY_ARGS;
import static com.taobao.weex.bridge.WXBridgeManager.KEY_METHOD;
import static com.taobao.weex.bridge.WXBridgeManager.METHOD_CALLBACK;
import static com.taobao.weex.bridge.WXBridgeManager.METHOD_CALL_JS;
import static com.taobao.weex.common.WXJSBridgeMsgType.MODULE_INTERVAL;
import static com.taobao.weex.common.WXJSBridgeMsgType.MODULE_TIMEOUT;

import android.os.Handler;
import android.os.Message;
import android.support.annotation.FloatRange;
import android.support.annotation.IntDef;
import android.support.annotation.IntRange;
import android.support.annotation.VisibleForTesting;
import android.util.SparseArray;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXHashMap;
import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.utils.WXJsonUtils;
import com.taobao.weex.utils.WXLogUtils;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.HashMap;
import com.taobao.weex.utils.WXUtils;

public class WXTimerModule extends WXModule implements Destroyable, Handler.Callback {

  @IntDef({MODULE_TIMEOUT, MODULE_INTERVAL})
  @Retention(RetentionPolicy.SOURCE)
  @interface MessageType {}

  private final static String TAG = "timer";
  private Handler handler;
  private SparseArray<Integer> antiIntAutoBoxing;

  public WXTimerModule() {
    handler = new Handler(WXBridgeManager.getInstance().getJSLooper(), this);
    antiIntAutoBoxing = new SparseArray<>();
  }


  @JSMethod(uiThread = false)
  public void setTimeout(@IntRange(from = 1) int funcId, @FloatRange(from = 0) float delay) {
    if(mWXSDKInstance != null) {
      postOrHoldMessage(MODULE_TIMEOUT, funcId, (int) delay, WXUtils.parseInt(mWXSDKInstance.getInstanceId()));
      if (null != mWXSDKInstance.getWXPerformance()){
        mWXSDKInstance.getWXPerformance().timerInvokeCount++;
      }
      mWXSDKInstance.getApmForInstance().updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_TIMER_NUM,1);
    }
  }

  @JSMethod(uiThread = false)
  public void setInterval(@IntRange(from = 1) int funcId, @FloatRange(from = 0) float interval) {
    if(mWXSDKInstance != null) {
      postOrHoldMessage(MODULE_INTERVAL, funcId, (int) interval, WXUtils.parseInt(mWXSDKInstance.getInstanceId()));
      if (null != mWXSDKInstance.getWXPerformance()){
        mWXSDKInstance.getWXPerformance().timerInvokeCount++;
      }
      mWXSDKInstance.getApmForInstance().updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_TIMER_NUM,1);
    }
  }

  @JSMethod(uiThread = false)
  public void clearTimeout(@IntRange(from = 1) int funcId) {
    if (funcId <= 0) {
      return;
    }
    removeOrHoldMessage(MODULE_TIMEOUT, funcId);
  }

  @JSMethod(uiThread = false)
  public void clearInterval(@IntRange(from = 1) int funcId) {
    if (funcId <= 0) {
      return;
    }
    removeOrHoldMessage(MODULE_INTERVAL, funcId);
  }

  @Override
  public void destroy() {
    if (handler != null) {
      if(WXEnvironment.isApkDebugable()) {
        WXLogUtils.d(TAG, "Timer Module removeAllMessages: ");
      }
      handler.removeCallbacksAndMessages(null);
      antiIntAutoBoxing.clear();
    }
  }

  @Override
  public boolean handleMessage(Message msg) {
    boolean ret = false;
    WXJSObject[] args;
    if (msg != null) {
      int what = msg.what;
      if(WXEnvironment.isApkDebugable()) {
        WXLogUtils.d(TAG, "Timer Module handleMessage : " + msg.what);
      }
      switch (what) {
        case MODULE_TIMEOUT:
          if (msg.obj == null) {
            break;
          }
          checkIfTimerInBack(msg.arg1);
          args = createTimerArgs(msg.arg1, (Integer) msg.obj, false);
          WXBridgeManager.getInstance().invokeExecJS(String.valueOf(msg.arg1), null, METHOD_CALL_JS, args, true);
          ret = true;
          break;
        case MODULE_INTERVAL:
          if (msg.obj == null) {
            break;
          }
          checkIfTimerInBack(msg.arg1);
          postMessage(MODULE_INTERVAL, (Integer) msg.obj, msg.arg2, msg.arg1);
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

  private void checkIfTimerInBack(int instanceId){
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(String.valueOf(instanceId));
    if (null == instance){
      return;
    }
    if (instance.isViewDisAppear()){
      instance.getApmForInstance().updateDiffStats(WXInstanceApm.KEY_PAGE_TIMER_BACK_NUM,1);
    }
  }

  @VisibleForTesting
  void setHandler(Handler handler) {
    this.handler = handler;
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

  private void postOrHoldMessage(@MessageType final int what,final int funcId,final int interval,final int instanceId) {
    if(mWXSDKInstance.isPreRenderMode()) {
      postMessage(what,funcId,interval,instanceId);
    } else {
      postMessage(what,funcId,interval,instanceId);
    }
  }

  private void removeOrHoldMessage(@MessageType final int what,final int funcId) {
    if(mWXSDKInstance.isPreRenderMode()) {
      handler.removeMessages(what, antiIntAutoBoxing.get(funcId, funcId));
    } else {
      handler.removeMessages(what, antiIntAutoBoxing.get(funcId, funcId));
    }
  }

  private void postMessage(@MessageType int what,
                           @IntRange(from = 1) int funcId,
                           @IntRange(from = 0) int interval, int instanceId) {
    if (interval < 0 || funcId <= 0) {
      WXLogUtils.e(TAG, "interval < 0 or funcId <=0");
    } else {
      if(antiIntAutoBoxing.get(funcId) == null) {
        antiIntAutoBoxing.put(funcId, funcId);
      }
      Message message = handler
              .obtainMessage(what, instanceId, interval, antiIntAutoBoxing.get(funcId));
      handler.sendMessageDelayed(message, interval);
    }
  }
}
