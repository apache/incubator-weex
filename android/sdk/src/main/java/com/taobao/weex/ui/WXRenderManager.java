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
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Manager class for render operation, mainly for managing {@link RenderActionContextImpl}.
 * This is <strong>not</strong> a thread-safe class
 */
public class WXRenderManager {

  private ConcurrentHashMap<String, RenderActionContextImpl> mRegistries;
  private WXRenderHandler mWXRenderHandler;

  public WXRenderManager() {
    mRegistries = new ConcurrentHashMap<>();
    mWXRenderHandler = new WXRenderHandler();
  }

  public RenderActionContext getRenderContext(String instanceId) {
    return mRegistries.get(instanceId);
  }

  public @Nullable WXComponent getWXComponent(String instanceId, String ref) {
    if(instanceId == null || TextUtils.isEmpty(ref)){
      return null;
    }
    RenderActionContext stmt = getRenderContext(instanceId);
    return stmt == null?null:stmt.getComponent(ref);
  }

  public WXSDKInstance getWXSDKInstance(String instanceId) {
    RenderActionContextImpl statement = mRegistries.get(instanceId);
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
   * @param instanceId {@link WXSDKInstance#mInstanceId}
   */
  public void removeRenderStatement(String instanceId) {
    if (!WXUtils.isUiThread()) {
      throw new WXRuntimeException("[WXRenderManager] removeRenderStatement can only be called in main thread");
    }
    RenderActionContextImpl statement = mRegistries.remove(instanceId);
    if (statement != null) {
      statement.destroy();
    }
  }

  //TODO Use runnable temporarily
  public void runOnThread(final String instanceId, final IWXRenderTask task) {
    mWXRenderHandler.post(WXThread.secure(new Runnable() {

      @Override
      public void run() {
        if (mRegistries.get(instanceId) == null) {
          return;
        }
        task.execute();
      }
    }));
  }

  public void runOnThread(final String instanceId, final RenderAction action) {
    mWXRenderHandler.post(WXThread.secure(new Runnable() {

      @Override
      public void run() {
        if (mRegistries.get(instanceId) == null) {
          return;
        }
        action.executeRender(getRenderContext(instanceId));
      }
    }));
  }

  public void registerInstance(WXSDKInstance instance) {
    mRegistries.put(instance.getInstanceId(), new RenderActionContextImpl(instance));
  }

  public void setLayout(String instanceId, String ref, WXDomObject domObject) {
    RenderActionContextImpl statement = mRegistries.get(instanceId);
    if (statement == null) {
      return;
    }
    statement.setLayout(ref, domObject);
  }

  /**
   * Set extra info, other than attribute and style
   */
  public void setExtra(String instanceId, String ref, Object extra) {
    RenderActionContextImpl statement = mRegistries.get(instanceId);
    if (statement == null) {
      return;
    }
    statement.setExtra(ref, extra);
  }

  public List<WXSDKInstance> getAllInstances() {
    ArrayList<WXSDKInstance> instances = null;
    if (mRegistries != null && !mRegistries.isEmpty()) {
      instances = new ArrayList<WXSDKInstance>();
      for (Map.Entry<String, RenderActionContextImpl> entry : mRegistries.entrySet()) {
        RenderActionContextImpl renderStatement = entry.getValue();
        if (renderStatement != null) {
          instances.add(renderStatement.getWXSDKInstance());
        }
      }
    }
    return instances;
  }

  public void registerComponent(String instanceId, String ref, WXComponent comp) {
    RenderActionContextImpl statement = mRegistries.get(instanceId);
    if (statement != null) {
      statement.registerComponent(ref,comp);
    }
  }

}
