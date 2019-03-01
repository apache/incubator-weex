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
package com.taobao.weex.render.vsync;

import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Choreographer;

import java.lang.ref.WeakReference;

public class VsyncController implements Runnable{

    private WeakReference<VsyncListener> mVsyncListenerRef;
    private Choreographer.FrameCallback mVSyncFrameCallback;
    private boolean mStart;
    private Handler mHandler;


    public VsyncController(VsyncListener vsyncListener) {
        this.mVsyncListenerRef = new WeakReference<>(vsyncListener);
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            mHandler = new Handler(Looper.getMainLooper());
        }else{
            mVSyncFrameCallback = new Choreographer.FrameCallback(){
                @Override
                public void doFrame(long frameTimeNanos) {
                    VsyncController.this.onVsyncFrame(frameTimeNanos);
                    if(!mStart){
                        return;
                    }
                    Choreographer.getInstance().postFrameCallback(mVSyncFrameCallback);
                }
            };
        }
    }

    public void start(){
        if(!mStart){
            mStart = true;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                Choreographer.getInstance().postFrameCallback(mVSyncFrameCallback);
            }else{
                mHandler.post(this);
            }
        }
    }
    public void stop() {
        if (mStart) {
            mStart = false;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                Choreographer.getInstance().removeFrameCallback(mVSyncFrameCallback);
            }else{
                mHandler.removeCallbacks(this);
            }
        }
    }

    public void onVsyncFrame(long frameTimeNanos) {
        VsyncListener vsyncListener = mVsyncListenerRef.get();
        if(vsyncListener == null){
            return;
        }
        if(!mStart){
            return;
        }
        vsyncListener.onVsync(frameTimeNanos);
    }

    @Override
    public void run() {
       long start = System.currentTimeMillis();
       long frameTimeNanos = start*1000000;
       onVsyncFrame(frameTimeNanos);
       long used = System.currentTimeMillis()-start;
       requestNextFrame(used);
    }

    private void requestNextFrame(long used) {
        long nextFrameTime = 16 - used -1;
        if(nextFrameTime <= 0){
            mHandler.post(this);
        }else{
            mHandler.postDelayed(this, nextFrameTime);
        }
    }


    public interface VsyncListener {
        void onVsync(long frameTimeMS);
    }
}
