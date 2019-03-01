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
package com.taobao.weex.render.frame;

import android.graphics.SurfaceTexture;
import android.view.Surface;

/**
 * Created by furture on 2018/8/29.
 */

public class SurfaceTextureHolder {


    private SurfaceTexture mSurfaceTexture;
    private Surface mSurface;
    private int mWidth;
    private int mHeight;
    private volatile boolean mIsDestory;
    private long mNativeWindow;

    public SurfaceTextureHolder(SurfaceTexture surfaceTexture, int width, int height) {
        this.mSurfaceTexture = surfaceTexture;
        this.mWidth = width;
        this.mHeight = height;
    }

    public void setSurfaceTexture(SurfaceTexture surfaceTexture, int width, int height) {
        this.mSurfaceTexture = surfaceTexture;
        this.mWidth = width;
        this.mHeight = height;
    }

    public boolean isDestory() {
        return mIsDestory;
    }

    public void setDestory(boolean destory) {
        mIsDestory = destory;
        if(mIsDestory){
            mSurfaceTexture = null;
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        if(mIsDestory){
            return null;
        }
        return mSurfaceTexture;
    }

    public void setSurface(Surface surface) {
        this.mSurface = surface;
    }

    public Surface createSurface() {
        if(mSurface == null){
            this.mSurface = new Surface(mSurfaceTexture);
        }
        return mSurface;
    }

    public Surface getSurface() {
        return mSurface;
    }

    public int getWidth() {
        return mWidth;
    }


    public int getHeight() {
        return mHeight;
    }

    public long getNativeWindow() {
        return mNativeWindow;
    }

    public void setNativeWindow(long nativeWindow) {
        this.mNativeWindow = nativeWindow;
    }
}
