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
package org.apache.weex;

/**
 * Created by shiwentao on 2017/8/24.
 */

import android.annotation.SuppressLint;
import android.os.Build;
import android.util.Log;
import android.view.Choreographer;
import org.apache.weex.common.WXErrorCode;
import java.lang.ref.WeakReference;

public class WeexFrameRateControl {
    private static final long VSYNC_FRAME = 1000 / 60;
    private WeakReference<VSyncListener> mListener;
    private final Choreographer mChoreographer;
    private final Choreographer.FrameCallback mVSyncFrameCallback;
    private final Runnable runnable;

    public interface VSyncListener {
        void OnVSync();
    }

    public WeexFrameRateControl(VSyncListener listener) {
        mListener = new WeakReference<>(listener);
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
            mChoreographer = Choreographer.getInstance();
            mVSyncFrameCallback = new Choreographer.FrameCallback() {
                @SuppressLint("NewApi")
                @Override
                public void doFrame(long frameTimeNanos) {
                    VSyncListener vSyncListener;
                    if (mListener != null && (vSyncListener=mListener.get()) != null) {
                        try {
                            vSyncListener.OnVSync();
                            mChoreographer.postFrameCallback(mVSyncFrameCallback);
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
            runnable = null;
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
            mChoreographer = null;
            mVSyncFrameCallback = null;
        }
    }

    @SuppressLint("NewApi")
    public void start() {
        if (mChoreographer != null) {
            mChoreographer.postFrameCallback(mVSyncFrameCallback);
        }
        else if(runnable!=null){
            WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(runnable, VSYNC_FRAME);
        }
    }

    @SuppressLint("NewApi")
    public void stop() {
        if (mChoreographer != null) {
            mChoreographer.removeFrameCallback(mVSyncFrameCallback);
        }else if(runnable!=null){
            WXSDKManager.getInstance().getWXRenderManager().removeTask(runnable);
        }
    }
}
