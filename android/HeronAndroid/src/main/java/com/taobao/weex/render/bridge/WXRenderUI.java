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
package com.taobao.weex.render.bridge;

import android.content.Context;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.render.frame.TouchHelper;
import com.taobao.weex.render.image.FrameImage;
import com.taobao.weex.render.sdk.RenderSDK;

public class WXRenderUI {


    private static final WXRenderUI mRenderUI = new WXRenderUI();
    private boolean mInited;


    public static WXRenderUI getInstance(){
        return mRenderUI;
    }


    public void ensureInit(){
        if(!mInited){
            initRenderUISDK();
        }
    }

    private synchronized void initRenderUISDK(){
        if(!mInited){
            try{
                System.loadLibrary("heron");
                Context context = WXEnvironment.getApplication().getApplicationContext();
                DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
                nativeInitRenderUI(displayMetrics.widthPixels, displayMetrics.heightPixels, "");
                mInited = true;
            }catch (Throwable e){
                Log.e("Weex", "Weex Exception " + e);
            }
          }
    }

    public long frameRenderAttach(String ref, Surface surface, int width, int height, boolean isSurfaceView){
        return nativeFrameRenderAttach(ref, surface, width, height, isSurfaceView);
    }

    public void frameRenderDettach(String ref, Surface surface, long nativeWindow){
        nativeFrameRenderDettach(ref, surface, nativeWindow);
    }


    public void frameRenderOnTouchEvent(String ref, MotionEvent event) {
        int pointerCount = event.getPointerCount();
        int actionMask = event.getActionMasked();
        int enumAction = TouchHelper.converActionMaskToRender(actionMask);
        if(enumAction < 0){
            return;
        }
        for(int i=0; i<pointerCount; i++){
            nativeFrameRenderOnTouchEvent(ref, enumAction, event.getPointerId(i), event.getX(i), event.getY(i), System.currentTimeMillis());
        }
    }





    public void frameRenderOnResume(String ref){
        nativeFrameRenderOnResume(ref);
    }

    public void frameRenderOnPause(String ref) {
        nativeFrameRenderOnPause(ref);
    }

    public void frameRenderOnLowMemory(String ref) {
        nativeFrameRenderOnLowMemory(ref);
    }

    public void frameRenderSizeChanged(String ref, int width, int height) {
        nativeFrameRenderSizeChanged(ref, width, height);
    }


    public void onVsync(long frameTime) {
        nativeFrameOnVsync(frameTime);
    }


    public void getBitmap(final String url, int width, int height, int callbackId) {
        if(TextUtils.isEmpty(url) || RenderSDK.getInstance().getImageAdapter() == null){
            doImageCallback(callbackId, null,  -1);
            return;
        }

        FrameImage target = RenderSDK.getInstance().getImageAdapter().getFrameImage(callbackId);
        if(target != null) {
            if (target.getLoadingState() == FrameImage.LOADING) {
                doImageCallback(callbackId, null,  -1);
                return;
            }
            if (target.getLoadingState() == FrameImage.LOADING_SUCCESS) {
                doImageCallback(callbackId, target.getBitmap(),  -1);
                return;
            }
        }
        FrameImage loadImageTarget = RenderSDK.getInstance().getImageAdapter().requestFrameImage(url, width, height, callbackId);
        if(loadImageTarget == null){
            doImageCallback(callbackId, null,  -1);
            return;
        }
        RenderSDK.getInstance().getImageAdapter().putFrameImage(callbackId, loadImageTarget);
    }




    public void doImageCallback(int callbackId, Bitmap bitmap, int state){
        nativeFrameImageCallback(callbackId, bitmap,  state);
        RenderSDK.getInstance().getImageAdapter().removeFrameImage(callbackId);
    }


    private native void nativeInitRenderUI(int screenWidth, int screenHeight, String param);
    private native long nativeFrameRenderAttach(String ref, Surface surface, int width, int height, boolean isSurfaceView);
    private native void nativeFrameRenderSizeChanged(String ref, int width, int height);
    private native void nativeFrameRenderDettach(String ref, Surface surface, long nativeWindow);
    private native void nativeFrameRenderOnTouchEvent(String ref, int enumAction, int pointerId, float x, float y, long eventTime);
    private native void nativeFrameRenderOnResume(String ref);
    private native void nativeFrameRenderOnPause(String ref);
    private native void nativeFrameRenderOnLowMemory(String ref);
    private native void nativeFrameOnVsync(long frameTimeMS);
    private native void nativeFrameImageCallback(int callbackId, Bitmap bitmap, int state);
}
