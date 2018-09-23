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
package com.taobao.weex.render.view;

import android.graphics.SurfaceTexture;

import com.taobao.weex.render.task.OpenGLRender;

/**
 * Created by furture on 2018/8/29.
 */

public class SurfaceTextureHolder {

    private OpenGLRender openGLRender;
    private SurfaceTexture surfaceTexture;
    private int width;
    private int height;
    private volatile boolean isDestory;

    public SurfaceTextureHolder(SurfaceTexture surfaceTexture, int width, int height) {
        this.surfaceTexture = surfaceTexture;
        this.width = width;
        this.height = height;
    }

    public SurfaceTexture getSurfaceTexture() {
        if(isDestory){
            return null;
        }
        return surfaceTexture;
    }

    public void setSurfaceTexture(SurfaceTexture surfaceTexture, int width, int height) {
        this.surfaceTexture = surfaceTexture;
        this.width = width;
        this.height = height;
    }

    public boolean isDestory() {
        return isDestory;
    }

    public void setDestory(boolean destory) {
        isDestory = destory;
        if(isDestory){
            surfaceTexture = null;
        }
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public OpenGLRender getOpenGLRender() {
        return openGLRender;
    }

    public void setOpenGLRender(OpenGLRender openGLRender) {
        this.openGLRender = openGLRender;
    }
}
