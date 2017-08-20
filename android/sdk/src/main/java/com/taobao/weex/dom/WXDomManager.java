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
package com.taobao.weex.dom;

import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.ui.WXRenderManager;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Class for managing dom operation. This class works as the client in the command pattern, it
 * will call {@link DOMActionContextImpl} for creating command object and invoking corresponding
 * operation.
 * Methods in this class normally need to be invoked in dom thread, otherwise, {@link
 * WXRuntimeException} may be thrown.
 */
public final class WXDomManager {

  private WXThread mDomThread;
  /** package **/
  Handler mDomHandler;
  private WXRenderManager mWXRenderManager;
  private ConcurrentHashMap<String, DOMActionContextImpl> mDomRegistries;

  public WXDomManager(WXRenderManager renderManager) {
    mWXRenderManager = renderManager;
    mDomRegistries = new ConcurrentHashMap<>();
    mDomThread = new WXThread("WeeXDomThread", new WXDomHandler(this));
    mDomHandler = mDomThread.getHandler();
  }

  public void sendEmptyMessageDelayed(int what, long delayMillis) {
    if (mDomHandler == null || mDomThread == null
        || !mDomThread.isWXThreadAlive() || mDomThread.getLooper() == null) {
      return;
    }
    mDomHandler.sendEmptyMessageDelayed(what, delayMillis);
  }

  public void sendMessage(Message msg) {
    sendMessageDelayed(msg, 0);
  }

  public void sendMessageDelayed(Message msg, long delay) {
    if (msg == null || mDomHandler == null || mDomThread == null
        || !mDomThread.isWXThreadAlive() || mDomThread.getLooper() == null) {
      return;
    }
    mDomHandler.sendMessageDelayed(msg,delay);
  }

  /**
   * Remove the specified dom statement. This is called when {@link WXSDKManager} destroy
   * instances.
   * @param instanceId {@link com.taobao.weex.WXSDKInstance#mInstanceId} for the instance
   */
  public void removeDomStatement(String instanceId) {
    if (!WXUtils.isUiThread()) {
      throw new WXRuntimeException("[WXDomManager] removeDomStatement");
    }
    final DOMActionContextImpl statement = mDomRegistries.remove(instanceId);
    if (statement != null) {
      post(new Runnable() {

        @Override
        public void run() {
          statement.destroy();
        }
      });
    }
  }

  public void post(Runnable task) {
    if (mDomHandler == null || task == null || mDomThread == null || !mDomThread.isWXThreadAlive()
        || mDomThread.getLooper() == null) {
      return;
    }
    mDomHandler.post(WXThread.secure(task));
  }

  /**
   * Destroy current instance
   */
  public void destroy() {
    if (mDomThread != null && mDomThread.isWXThreadAlive()) {
      mDomThread.quit();
    }
    if (mDomRegistries != null) {
      mDomRegistries.clear();
    }
    mDomHandler = null;
    mDomThread = null;
  }

  private boolean isDomThread() {
    return !WXEnvironment.isApkDebugable() || Thread.currentThread().getId() == mDomThread.getId();
  }

  /**
   * Batch the execution of {@link DOMActionContextImpl}
   */
  void batch() {
    throwIfNotDomThread();
    Iterator<Entry<String, DOMActionContextImpl>> iterator = mDomRegistries.entrySet().iterator();
    while (iterator.hasNext()) {
      iterator.next().getValue().batch();
    }
  }

  void consumeRenderTask(String instanceId) {
    throwIfNotDomThread();
    DOMActionContextImpl context = mDomRegistries.get(instanceId);
    if(context != null) {
      context.consumeRenderTasks();
    }
  }

  private void throwIfNotDomThread(){
    if (!isDomThread()) {
      throw new WXRuntimeException("dom operation must be done in dom thread");
    }
  }

  public void executeAction(String instanceId, DOMAction action, boolean createContext) {
    DOMActionContext context = mDomRegistries.get(instanceId);
    if(context == null){
      if(createContext){
        DOMActionContextImpl oldStatement = new DOMActionContextImpl(instanceId, mWXRenderManager);
        mDomRegistries.put(instanceId, oldStatement);
        context = oldStatement;
      }else{
        //Instance not existed.
        return;
      }
    }
    action.executeDom(context);

  }

  /**
   *  @param action
   * @param createContext only true when create body
   */
  public void postAction(String instanceId,DOMAction action, boolean createContext){
    postActionDelay(instanceId, action, createContext, 0);
  }

  /**
   *  @param action
   * @param createContext only true when create body
   */
  public void postActionDelay(String instanceId,DOMAction action,
                              boolean createContext, long delay){
    if(action == null){
      return;
    }
    Message msg = Message.obtain();
    msg.what = WXDomHandler.MsgType.WX_EXECUTE_ACTION;
    WXDomTask task = new WXDomTask();
    task.instanceId = instanceId;
    task.args = new ArrayList<>();
    task.args.add(action);
    task.args.add(createContext);
    msg.obj = task;
    sendMessageDelayed(msg, delay);
  }

  public void postRenderTask(@NonNull String instanceId) {
    Message msg = Message.obtain();
    msg.what = WXDomHandler.MsgType.WX_CONSUME_RENDER_TASKS;
    WXDomTask task = new WXDomTask();
    task.instanceId = instanceId;
    msg.obj = task;
    sendMessage(msg);
  }

}
