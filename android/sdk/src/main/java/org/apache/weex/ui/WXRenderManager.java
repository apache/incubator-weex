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
package org.apache.weex.ui;

import android.opengl.GLES10;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.text.TextUtils;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXRuntimeException;
import org.apache.weex.common.WXThread;
import org.apache.weex.dom.RenderContext;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.ui.action.BasicGraphicAction;
import org.apache.weex.ui.action.GraphicActionBatchAction;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;


/**
 * Manager class for render operation, mainly for managing {@link RenderContextImpl}.
 * This is <strong>not</strong> a thread-safe class
 */
public class WXRenderManager {

  private volatile ConcurrentHashMap<String, RenderContextImpl> mRenderContext;
  private WXRenderHandler mWXRenderHandler;
  private String mCurrentBatchInstanceId = null;
  private ArrayList<Map<String,Object>> mBatchActions = new ArrayList<>();
  private final int MAX_DROP_FRAME_NATIVE_BATCH = 2000;
  private final static  String sKeyAction = "Action";
  private static int nativeBatchTimes = 0;
  private static int mOpenGLRenderLimitValue = 0;

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

    public static int getOpenGLRenderLimitValue() {
      if(mOpenGLRenderLimitValue == 0){
          int maxsize = 0;
              try {
                  EGL10 egl = (EGL10) EGLContext.getEGL();
                  EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
                  int[] vers = new int[2];
                  egl.eglInitialize(dpy, vers);
                  int[] configAttr = {
                          EGL10.EGL_COLOR_BUFFER_TYPE, EGL10.EGL_RGB_BUFFER,
                          EGL10.EGL_LEVEL, 0,
                          EGL10.EGL_SURFACE_TYPE, EGL10.EGL_PBUFFER_BIT,
                          EGL10.EGL_NONE};
                  EGLConfig[] configs = new EGLConfig[1];
                  int[] numConfig = new int[1];
                  egl.eglChooseConfig(dpy, configAttr, configs, 1, numConfig);
                  if(numConfig[0] == 0){
                      //There is something wrong with opengl environment.
                      maxsize = -1;
                      egl.eglTerminate(dpy);
                  }else {
                      EGLConfig config = configs[0];
                      int[] surfAttr = {
                              EGL10.EGL_WIDTH, 64,
                              EGL10.EGL_HEIGHT, 64,
                              EGL10.EGL_NONE};
                      EGLSurface surf = egl.eglCreatePbufferSurface(dpy, config, surfAttr);
                      final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;// missing in EGL10
                      int[] ctxAttrib = {
                              EGL_CONTEXT_CLIENT_VERSION, 1,
                              EGL10.EGL_NONE};
                      EGLContext ctx = egl.eglCreateContext(dpy, config, EGL10.EGL_NO_CONTEXT, ctxAttrib);
                      egl.eglMakeCurrent(dpy, surf, surf, ctx);
                      int[] maxSize = new int[1];
                      GLES10.glGetIntegerv(GLES10.GL_MAX_TEXTURE_SIZE, maxSize, 0);
                      egl.eglMakeCurrent(dpy, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE,
                              EGL10.EGL_NO_CONTEXT);
                      egl.eglDestroySurface(dpy, surf);
                      egl.eglDestroyContext(dpy, ctx);
                      egl.eglTerminate(dpy);
                      maxsize = maxSize[0];
                  }
              } catch(Exception e){
                  WXLogUtils.e(WXLogUtils.getStackTrace(e));
              }
          mOpenGLRenderLimitValue = maxsize;
      }
      return mOpenGLRenderLimitValue;
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
      final RenderContextImpl renderContext = mRenderContext.get(instanceId);

      /// keep actions if renderContext still exist
      ArrayList<Map<String, Object>> tmpList = null;
      if (renderContext != null) {
        tmpList = new ArrayList<>(mBatchActions);
      }

      // clear stashed actions
      this.mBatchActions.clear();
      mCurrentBatchInstanceId = null;
      nativeBatchTimes = 0;

      // return if renderContext has been destroyed
      if (renderContext == null) {
        return;
      }

      /// post actions if renderContext still exist
      ArrayList<BasicGraphicAction> actions = new ArrayList<>(tmpList.size());
      for (int i = 0 ; i < tmpList.size(); i ++) {
          Map<String, Object> item = tmpList.get(i);
          Object mustBeAction = item.get(sKeyAction);
          if (!(mustBeAction instanceof BasicGraphicAction)) {
            continue;
          }
          BasicGraphicAction tmpAction = (BasicGraphicAction)mustBeAction;
          if (tmpAction.mActionType == BasicGraphicAction.ActionTypeBatchBegin || tmpAction.mActionType == BasicGraphicAction.ActionTypeBatchEnd) {
            continue;
          }
          actions.add(tmpAction);
      }
      postGraphicAction(instanceId, new GraphicActionBatchAction(action.getWXSDKIntance(),action.getRef(), actions));
  }

  public void postGraphicAction(final String instanceId, final BasicGraphicAction action) {
    final RenderContextImpl renderContext = mRenderContext.get(instanceId);
    if (renderContext == null) {
      return;
    }

    // If more than two pages exist and mCurrentBatchInstanceId not matches new instanceId, we will post all stashed actions at once.
    // That will cause losing efficacy of batch action, but it is acceptable because it's not serious problem.
    if (mCurrentBatchInstanceId != null && instanceId != null && !mCurrentBatchInstanceId.equals(instanceId) && mBatchActions.size() > 0) {
      Map<String, Object> lastItem = mBatchActions.get(mBatchActions.size() - 1);
      Object mustBeAction = lastItem.get(sKeyAction);
      if (mustBeAction instanceof BasicGraphicAction) {
        BasicGraphicAction lastAction = (BasicGraphicAction)mustBeAction;
        postAllStashedGraphicAction(mCurrentBatchInstanceId, lastAction);
      }
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
            mCurrentBatchInstanceId = instanceId;
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
