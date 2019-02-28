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
package com.taobao.weex.render.manager;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;

import com.taobao.weex.render.bridge.WXRenderUI;
import com.taobao.weex.render.frame.RenderFrame;
import com.taobao.weex.render.threads.IoThread;
import com.taobao.weex.render.vsync.VsyncController;

import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by furture on 2018/7/24.
 */

public class RenderManager implements VsyncController.VsyncListener {


    private Handler mUiHandler;
    private Handler mIoHandler;
    private VsyncController mVsyncController;
    private Map<String,WeakReference<RenderFrame>> mRenderFrameMap;

    private RenderManager(){
        mRenderFrameMap = new ConcurrentHashMap<>();
        mUiHandler = new Handler(Looper.getMainLooper());
        mVsyncController = new VsyncController(this);
        mVsyncController.start();
    }

    public Handler getUiHandler(){
        return mUiHandler;
    }

    public void registerRenderView(String key, RenderFrame render){
        WeakReference<RenderFrame> surfaceRenderWeakReference = new WeakReference<RenderFrame>(render);
       mRenderFrameMap.put(key, surfaceRenderWeakReference);
    }

    public RenderFrame getRenderView(String key){
        WeakReference<RenderFrame> surfaceRenderWeakReference = mRenderFrameMap.get(key);
        if(surfaceRenderWeakReference == null){
            return null;
        }
        return surfaceRenderWeakReference.get();
    }

    public Map<String,WeakReference<RenderFrame>> getRenderFrameMap(){
         return mRenderFrameMap;
    }


    public void onActivityResumed(Activity activity) {
        Set<Map.Entry<String,WeakReference<RenderFrame>>> entries = mRenderFrameMap.entrySet();
        for(Map.Entry<String,WeakReference<RenderFrame>> entry : entries){
            RenderFrame renderFrame = entry.getValue().get();
            if(renderFrame == null){
                continue;
            }
            if(isAcivityFrame(renderFrame, activity)){
                renderFrame.onResume();
            }
        }
    }

    public void onActivityPaused(Activity activity) {
        Set<Map.Entry<String,WeakReference<RenderFrame>>> entries = mRenderFrameMap.entrySet();
        for(Map.Entry<String,WeakReference<RenderFrame>> entry : entries){
            RenderFrame renderFrame = entry.getValue().get();
            if(renderFrame == null){
                continue;
            }
            if(isAcivityFrame(renderFrame, activity)){
                renderFrame.onPause();
            }
        }
    }

    public void onActivityDestroyed(Activity activity) {
        Set<Map.Entry<String,WeakReference<RenderFrame>>> entries = mRenderFrameMap.entrySet();
        for(Map.Entry<String,WeakReference<RenderFrame>> entry : entries){
            RenderFrame renderFrame = entry.getValue().get();
            if(renderFrame == null){
                continue;
            }
            if(isAcivityFrame(renderFrame, activity)){
                renderFrame.onDestroy();
            }
        }
    }


    public void onLowMemory() {
        Set<Map.Entry<String,WeakReference<RenderFrame>>> entries = mRenderFrameMap.entrySet();
        for(Map.Entry<String,WeakReference<RenderFrame>> entry : entries){
            RenderFrame renderFrame = entry.getValue().get();
            if(renderFrame == null){
                continue;
            }
            renderFrame.onLowMemory();
        }
    }

    private boolean isAcivityFrame(RenderFrame renderFrame, Activity activity){
        if(renderFrame.getContext() == activity || renderFrame.getContext() == activity.getBaseContext()){
            return true;
        }
        return false;
    }



    public void removeFrame(String key){
        mRenderFrameMap.remove(key);
    }



    public Handler getIoHandler(){
        if(mIoHandler == null){
            synchronized (this){
                if(mIoHandler == null) {
                    mIoHandler = new Handler(IoThread.getThread().getLooper());
                }
            }
        }
        return mIoHandler;
    }

    public static RenderManager getInstance(){
        if(renderManager == null){
            synchronized (RenderManager.class){
                if(renderManager == null) {
                    renderManager = new RenderManager();
                }
            }
        }
        return renderManager;
    }
    private static RenderManager renderManager;


    @Override
    public void onVsync(long frameTimeMS) {
        WXRenderUI.getInstance().onVsync(frameTimeMS);
    }
}
