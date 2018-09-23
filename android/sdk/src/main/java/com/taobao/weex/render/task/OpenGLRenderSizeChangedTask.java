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

import com.taobao.weex.render.bridge.RenderBridge;
import com.taobao.weex.render.log.RenderLog;
import com.taobao.weex.render.view.DocumentView;
import com.taobao.weex.render.view.SurfaceTextureHolder;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Created by furture on 2018/8/17.
 */

public class OpenGLRenderSizeChangedTask extends GLTask implements Runnable {

    private SurfaceTextureHolder surfaceTextureHolder;
    private CountDownLatch countDownLatch;

    public OpenGLRenderSizeChangedTask(DocumentView documentView, SurfaceTextureHolder surfaceTextureHolder) {
        super(documentView);
        this.surfaceTextureHolder = surfaceTextureHolder;
        this.countDownLatch = new CountDownLatch(1);
    }

    @Override
    public void run() {
        try {

            OpenGLRender openGLRender = surfaceTextureHolder.getOpenGLRender();
            if(surfaceTextureHolder.isDestory() || openGLRender == null){
                return;
            }
            openGLRender.setWidth(surfaceTextureHolder.getWidth());
            openGLRender.setHeight(surfaceTextureHolder.getHeight());
            RenderBridge.getInstance().renderSizeChanged(openGLRender.getPtr(), surfaceTextureHolder.getWidth(), surfaceTextureHolder.getHeight());

            if(surfaceTextureHolder.isDestory()){
                return;
            }

            synchronized (getDocumentView().lock){
                if(!surfaceTextureHolder.isDestory()){
                    RenderBridge.getInstance().renderSwap(openGLRender.getPtr());
                }
            }

            if(surfaceTextureHolder.isDestory()){
                return;
            }

            RenderBridge.getInstance().renderClearBuffer(openGLRender.getPtr());

            if(surfaceTextureHolder.isDestory()){
                return;
            }

            synchronized (getDocumentView().lock){
                if(!surfaceTextureHolder.isDestory()){
                    RenderBridge.getInstance().renderSwap(openGLRender.getPtr());
                }
            }

            if(surfaceTextureHolder.isDestory()){
                return;
            }

            getDocumentView().renderSizeChanged(openGLRender);
        }finally {
            countDownLatch.countDown();
        }
    }

    public void waitComplete(){
        try {
            countDownLatch.await(160, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
