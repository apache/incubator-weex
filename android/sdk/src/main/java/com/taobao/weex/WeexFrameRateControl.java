/**
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
package com.taobao.weex;

/**
 * Created by shiwentao on 2017/8/24.
 */

import android.annotation.SuppressLint;
import android.os.Build;
import android.os.Build.VERSION_CODES;
import android.support.annotation.MainThread;
import android.support.annotation.RequiresApi;
import android.text.TextUtils;
import android.util.Log;
import android.view.Choreographer;
import com.taobao.weex.adapter.IWXConfigAdapter;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXErrorCode;
import java.lang.ref.WeakReference;
import java.util.concurrent.atomic.AtomicBoolean;

public class WeexFrameRateControl {
    private static final long VSYNC_FRAME = 1000 / 60;
    private final boolean choreographerInWorkerThread;
    private final AtomicBoolean taskStarted;
    private final WeakReference<VSyncListener> mListener;
    private final Runnable runnable;
    private final Choreographer.FrameCallback mVSyncFrameCallback;
    private Choreographer mChoreographer;

    public interface VSyncListener {
        void OnVSync();
    }

    public WeexFrameRateControl(VSyncListener listener) {
        mListener = new WeakReference<>(listener);
        taskStarted = new AtomicBoolean(false);
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
            runnable = null;
            mVSyncFrameCallback = createVSyncFrameCallback();
            IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
            if (adapter != null && TextUtils.equals(Boolean.TRUE.toString(),
                adapter.getConfig("WeexFrameRate", "InWorkerThread", "true"))) {
                choreographerInWorkerThread = true;
                WXBridgeManager.getInstance().post(new Runnable() {
                    @Override
                    public void run() {
                        mChoreographer = Choreographer.getInstance();
                        if(taskStarted.compareAndSet(false, true)){
                            mChoreographer.postFrameCallback(mVSyncFrameCallback);
                        }
                    }
                });
            } else {
                // This case should be entered only in abnormal case.
                choreographerInWorkerThread = false;
                mChoreographer = Choreographer.getInstance();
            }
        } else {
            // For API 15 or lower
            runnable = new Runnable() {
                @Override
                public void run() {
                    VSyncListener vSyncListener;
                    if (mListener != null && (vSyncListener = mListener.get()) != null) {
                        try {
                            vSyncListener.OnVSync();
                            WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(runnable, VSYNC_FRAME);
                        }catch (UnsatisfiedLinkError e){
                            if(vSyncListener instanceof WXSDKInstance){
                                ((WXSDKInstance) vSyncListener).onRenderError(
                                    WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED.getErrorCode(),
                                    Log.getStackTraceString(e));
                            }
                        }
                    }
                }
            };
            choreographerInWorkerThread = false;
            mChoreographer = null;
            mVSyncFrameCallback = null;
        }
    }

    @SuppressLint("NewApi")
    @MainThread
    public void start() {
        if (runnable != null) {
            WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(runnable, VSYNC_FRAME);
        } else {
            if (choreographerInWorkerThread) {
                if(taskStarted.compareAndSet(false, true)){
                    mChoreographer.postFrameCallback(mVSyncFrameCallback);
                }
            } else {
                mChoreographer.postFrameCallback(mVSyncFrameCallback);
            }
        }
    }

    @SuppressLint("NewApi")
    @MainThread
    public void stop() {
        if (runnable != null) {
            WXSDKManager.getInstance().getWXRenderManager().removeTask(runnable);
        } else {
            if (choreographerInWorkerThread) {
              if(taskStarted.compareAndSet(true, false)){
                  mChoreographer.removeFrameCallback(mVSyncFrameCallback);
              }
            } else {
                mChoreographer.removeFrameCallback(mVSyncFrameCallback);
            }
        }
    }

    @RequiresApi(api = VERSION_CODES.JELLY_BEAN)
    private Choreographer.FrameCallback createVSyncFrameCallback(){
        return new Choreographer.FrameCallback() {
            @SuppressLint("NewApi")
            @Override
            public void doFrame(long frameTimeNanos) {
                VSyncListener vSyncListener;
                if ((mListener!=null) && (vSyncListener = mListener.get()) != null) {
                    try {
                        vSyncListener.OnVSync();
                        mChoreographer.postFrameCallback(mVSyncFrameCallback);
                    } catch (UnsatisfiedLinkError e) {
                        if (vSyncListener instanceof WXSDKInstance) {
                            ((WXSDKInstance) vSyncListener).onRenderError(
                                WXErrorCode.WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED
                                    .getErrorCode(),
                                Log.getStackTraceString(e));
                        }
                    }
                }
            }
        };
    }
}
