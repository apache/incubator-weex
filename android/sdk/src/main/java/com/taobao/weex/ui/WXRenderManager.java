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
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.RenderContext;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.ui.action.BasicGraphicAction;
import com.taobao.weex.ui.action.GraphicActionBatchAction;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Manager class for render operation, mainly for managing {@link RenderContextImpl}.
 * This is <strong>not</strong> a thread-safe class
 */
public class WXRenderManager {

  private volatile ConcurrentHashMap<String, RenderContextImpl> mRenderContext;
  private WXRenderHandler mWXRenderHandler;
  private ArrayList<Map<String,Object>> mBatchActions = new ArrayList<>();
  private final int MAX_DROP_FRAME_NATIVE_BATCH = 2000;
  private final static  String sKeyAction = "Action";
  private static int nativeBatchTimes = 0;

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

  @RestrictTo(Scope.LIBRARY)
  public void postOnUiThread(Runnable runnable, long delayMillis) {
    mWXRenderHandler.postDelayed(WXThread.secure(runnable), delayMillis);
  }

  @RestrictTo(Scope.LIBRARY)
  public void postOnUiThread(Runnable runnable,final String instanceId){
    mWXRenderHandler.post(instanceId, WXThread.secure(runnable));
  }

  @RestrictTo(Scope.LIBRARY)
  public void postOnUiThread(Runnable runnable){
    mWXRenderHandler.post(WXThread.secure(runnable));
  }

  @RestrictTo(Scope.LIBRARY)
  public void removeTask(Runnable runnable){
    mWXRenderHandler.removeCallbacks(runnable);
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
    if(instanceId == null) {
      mWXRenderHandler.removeCallbacksAndMessages(null);
    } else {
      // use hashCode to match message's what.
      mWXRenderHandler.removeMessages(instanceId.hashCode());
    }
  }

  private void postAllStashedGraphicAction(final String instanceId,final BasicGraphicAction action) {
      ArrayList<Map<String, Object>> tmpList = new ArrayList<>(mBatchActions);
      this.mBatchActions.clear();
      ArrayList<BasicGraphicAction> actions = new ArrayList<>(tmpList.size());
      for (int i = 0 ; i < tmpList.size(); i ++) {
          Map<String, Object> item = tmpList.get(i);
          BasicGraphicAction tmpAction = (BasicGraphicAction)item.get(sKeyAction);
          if (tmpAction.mActionType == BasicGraphicAction.ActionTypeBatchBegin || tmpAction.mActionType == BasicGraphicAction.ActionTypeBatchEnd) {
              continue;
          }
          actions.add(tmpAction);
      }
      nativeBatchTimes = 0;
      postGraphicAction(instanceId, new GraphicActionBatchAction(action.getWXSDKIntance(),action.getRef(), actions));
  }

  public void postGraphicAction(final String instanceId, final BasicGraphicAction action) {
    final RenderContextImpl renderContext = mRenderContext.get(instanceId);
    if (renderContext == null) {
      return;
    }

    if (action.mActionType == BasicGraphicAction.ActionTypeBatchEnd) {
        postAllStashedGraphicAction(instanceId,action);
        return;
    } else if (action.mActionType == BasicGraphicAction.ActionTypeBatchBegin || this.mBatchActions.size() > 0 ) {
        nativeBatchTimes ++ ;
        if (nativeBatchTimes > MAX_DROP_FRAME_NATIVE_BATCH) {
            postAllStashedGraphicAction(instanceId,action);
        } else {
            HashMap<String, Object> item = new HashMap<>(1);
            item.put(sKeyAction, action);
            mBatchActions.add(item);
            return;
        }
    }

    mWXRenderHandler.post(instanceId, action);
  }

  public void registerInstance(WXSDKInstance instance) {
    if (instance.getInstanceId() == null) {
      WXExceptionUtils.commitCriticalExceptionRT(null,
              WXErrorCode.WX_RENDER_ERR_INSTANCE_ID_NULL,
              "registerInstance",
              WXErrorCode.WX_RENDER_ERR_INSTANCE_ID_NULL.getErrorMsg() + "instanceId is null",
              null);
    } else {
      mRenderContext.put(instance.getInstanceId(), new RenderContextImpl(instance));
    }
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
      if (null != statement.getInstance()){
        statement.getInstance().getApmForInstance().updateMaxStats(
            WXInstanceApm.KEY_PAGE_STATS_MAX_COMPONENT_NUM,
            statement.getComponentCount()
        );
      }
    }
  }

  public WXComponent unregisterComponent(String instanceId, String ref) {
    RenderContextImpl statement = mRenderContext.get(instanceId);
    if (statement != null) {
      if (null != statement.getInstance()){
        statement.getInstance().getApmForInstance().updateMaxStats(
            WXInstanceApm.KEY_PAGE_STATS_MAX_COMPONENT_NUM,
            statement.getComponentCount()
        );
      }
      return statement.unregisterComponent(ref);
    } else {
      return null;
    }
  }
}
