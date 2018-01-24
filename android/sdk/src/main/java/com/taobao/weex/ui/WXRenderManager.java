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
package com.taobao.weex.ui;

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.RenderContext;
import com.taobao.weex.ui.action.IExecutable;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Manager class for render operation, mainly for managing {@link RenderContextImpl}.
 * This is <strong>not</strong> a thread-safe class
 */
public class WXRenderManager {

  private ConcurrentHashMap<String, RenderContextImpl> mRenderContext;
  private WXRenderHandler mWXRenderHandler;

  public WXRenderManager() {
    mRenderContext = new ConcurrentHashMap<>();
    mWXRenderHandler = new WXRenderHandler();
  }

  public RenderContext getRenderContext(String instanceId) {
    return mRenderContext.get(instanceId);
  }

  public @Nullable
  WXComponent getWXComponent(String instanceId, String ref) {
    if (instanceId == null || TextUtils.isEmpty(ref)) {
      return null;
    }
    RenderContext stmt = getRenderContext(instanceId);
    return stmt == null ? null : stmt.getComponent(ref);
  }

  public WXSDKInstance getWXSDKInstance(String instanceId) {
    RenderContextImpl statement = mRenderContext.get(instanceId);
    if (statement == null) {
      return null;
    }
    return statement.getWXSDKInstance();
  }

  public void postOnUiThread(Runnable runnable, long delayMillis) {
    mWXRenderHandler.postDelayed(WXThread.secure(runnable), delayMillis);
  }

  /**
   * Remove renderStatement, can only be invoked in UI thread.
   *
   * @param instanceId {@link WXSDKInstance#mInstanceId}
   */
  public void removeRenderStatement(String instanceId) {
    if (!WXUtils.isUiThread()) {
      throw new WXRuntimeException("[WXRenderManager] removeRenderStatement can only be called in main thread");
    }
    RenderContextImpl statement = mRenderContext.remove(instanceId);

    if (statement != null) {
      statement.destroy();
    }
    mWXRenderHandler.removeCallbacksAndMessages(null);
  }

  public void postGraphicAction(final String instanceId, final IExecutable action) {

    final RenderContextImpl renderContext = mRenderContext.get(instanceId);
    if (renderContext == null) {
      return;
    }

    Runnable renderTask = WXThread.secure(new Runnable() {
      @Override
      public void run() {
        long start = System.currentTimeMillis();
        action.executeAction();
        long time = System.currentTimeMillis() - start;
        // WXLogUtils.e("" + Thread.currentThread() + "," + Thread.currentThread().getPriority());
//        if ("com.taobao.weex.ui.action.GraphicActionAddElement".equals(action.getClass().getName())) {
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionAddElementCount();
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionAddElementTime(time);
//
//        } else if ("com.taobao.weex.ui.action.GraphicActionLayout".equals(action.getClass().getName())) {
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionLayoutCount();
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionLayoutTime(time);
//
//        } else if ("com.taobao.weex.ui.action.GraphicActionCreateBody".equals(action.getClass().getName())) {
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionCreateBodyCount();
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionCreateBodyTime(time);
//
//        } else {
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionOtherCount();
//          WXSDKManager.getInstance().getSDKInstance(instanceId).callActionOtherTime(time);
//        }
      }
    });

    mWXRenderHandler.post(renderTask);
  }

  public void registerInstance(WXSDKInstance instance) {
    mRenderContext.put(instance.getInstanceId(), new RenderContextImpl(instance));
  }

  public List<WXSDKInstance> getAllInstances() {
    ArrayList<WXSDKInstance> instances = null;
    if (mRenderContext != null && !mRenderContext.isEmpty()) {
      instances = new ArrayList<WXSDKInstance>();
      for (Map.Entry<String, RenderContextImpl> entry : mRenderContext.entrySet()) {
        RenderContextImpl renderStatement = entry.getValue();
        if (renderStatement != null) {
          instances.add(renderStatement.getWXSDKInstance());
        }
      }
    }
    return instances;
  }

  public void registerComponent(String instanceId, String ref, WXComponent comp) {
    RenderContextImpl statement = mRenderContext.get(instanceId);
    if (statement != null) {
      statement.registerComponent(ref, comp);
    }
  }

  public WXComponent unregisterComponent(String instanceId, String ref) {
    RenderContextImpl statement = mRenderContext.get(instanceId);
    if (statement != null) {
      return statement.unregisterComponent(ref);
    } else {
      return null;
    }
  }
}
