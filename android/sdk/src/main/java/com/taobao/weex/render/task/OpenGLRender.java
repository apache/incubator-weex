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
package com.taobao.weex.render.task;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.taobao.weex.render.bridge.RenderBridge;
import com.taobao.weex.render.threads.GpuThread;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * opengl render for manager context for weex
 * Created by furture on 2018/7/23.
 */
public class OpenGLRender {

    private SurfaceTexture surfaceTexture;
    private Surface surface;
    private int width;
    private int height;
    private long ptr;
    private boolean willDestory;

    private static AtomicInteger renderCount = new AtomicInteger();

    public OpenGLRender(SurfaceTexture surfaceTexture, int width, int height) {
        this.surfaceTexture = surfaceTexture;
        this.surface = new Surface(surfaceTexture);
        this.width = width;
        this.height = height;
    }

    public void initRender(){
        if(ptr == 0){
            if(Thread.currentThread() != GpuThread.getThread()){
                throw new RuntimeException("OpenGLRender Init Must Be Call On GPU Thread");
            }
            ptr = RenderBridge.getInstance().initOpenGLRender(surface, width, height);
            renderCount.incrementAndGet();
        }
    }


    @Override
    protected void finalize() throws Throwable {
        destroy();
        super.finalize();
    }

    public boolean isDestory(){
        return ptr == 0;
    }

    public synchronized void destroy(){
        if(ptr != 0){
            renderCount.decrementAndGet();
            RenderBridge.getInstance().destroyOpenGLRender(ptr, surface);
            ptr = 0;
        }
        if(surface != null){
            surface.release();
            surface = null;
        }
    }

    public long getPtr(){
        return ptr;
    }

    public Surface getSurface() {
        return surface;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public boolean isWillDestory() {
        return willDestory;
    }

    public void setWillDestory(boolean willDestory) {
        this.willDestory = willDestory;
    }

    public static int getRenderNum(){
        return renderCount.get();
    }
}
