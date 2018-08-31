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

import android.content.Context;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import android.support.v4.view.ViewCompat;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.TextureView;
import android.view.View;

import com.taobao.weex.render.accessibility.DocumentAccessibilityHelper;
import com.taobao.weex.render.event.OnEventListener;
import com.taobao.weex.render.task.GLTask;
import com.taobao.weex.render.task.GLTaskQueue;
import com.taobao.weex.render.task.OpenGLRenderDestroyTask;
import com.taobao.weex.render.task.OpenGLRenderInitTask;
import com.taobao.weex.render.task.OpenGLRenderSizeChangedTask;


/**
 * Created by furture on 2018/7/23.
 */

public class DocumentTextureView extends TextureView implements  TextureView.SurfaceTextureListener, View.OnClickListener{
   private DocumentView documentView;
    private boolean hasAttachToWindow;
    private GLTaskQueue gpuThreadTaskQueue;
    private MotionEvent lastEvent;
    private DocumentAccessibilityHelper documentAccessibilityHelper;
    private SurfaceTextureHolder surfaceTextureHolder;
    public DocumentTextureView(Context context) {
        super(context);
        initView();
    }

    public DocumentTextureView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public DocumentTextureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    public DocumentTextureView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView();
    }


    private void initView() {
        setOpaque(false);
        setSurfaceTextureListener(this);
        setOnClickListener(this);
        gpuThreadTaskQueue = new GLTaskQueue();
        if(getLayerType() != View.LAYER_TYPE_HARDWARE) {
            setLayerType(View.LAYER_TYPE_HARDWARE, null);
        }
        try{
            documentAccessibilityHelper = new DocumentAccessibilityHelper(this);
            ViewCompat.setAccessibilityDelegate(this, documentAccessibilityHelper);
        }catch (Exception e){}
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        lastEvent = event;
        return super.dispatchTouchEvent(event);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        return documentAccessibilityHelper.dispatchKeyEvent(event) ||super.dispatchKeyEvent(event);
    }

    @Override
    protected boolean dispatchHoverEvent(MotionEvent event) {
        lastEvent = event;
        return documentAccessibilityHelper.dispatchHoverEvent(event) || super.dispatchHoverEvent(event);
    }

    @Override
    public void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        documentAccessibilityHelper.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
    }

    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture, final int width, final int height) {
        if(documentView == null){
            return;
        }
        surfaceTextureHolder = new SurfaceTextureHolder(surfaceTexture, width, height);
        documentView.setPause(false);

        GLTask.waitIfTaskBlock();
        OpenGLRenderInitTask initOpenGLRenderTask = new OpenGLRenderInitTask(documentView, surfaceTextureHolder);
        gpuThreadTaskQueue.addTask(initOpenGLRenderTask);
    }



    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
        if(documentView != null){
            surfaceTextureHolder.setSurfaceTexture(surfaceTexture, width, height);
            OpenGLRenderSizeChangedTask task = new OpenGLRenderSizeChangedTask(documentView, surfaceTextureHolder);
            gpuThreadTaskQueue.addTask(task);
            task.waitComplete();
        }
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        if(surfaceTextureHolder != null){
            if(documentView != null){
                documentView.setPause(true);
            }
            surfaceTextureHolder.setDestory(true);
            OpenGLRenderDestroyTask openGLRenderDestroyTask = new OpenGLRenderDestroyTask(documentView, surfaceTextureHolder);
            gpuThreadTaskQueue.addTask(openGLRenderDestroyTask);
        }
        surfaceTextureHolder = null;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {}


    @Override
    protected void onAttachedToWindow() {
        hasAttachToWindow = true;
        GLTask.waitIfTaskBlock();
        super.onAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        hasAttachToWindow = false;
        GLTask.waitIfTaskBlock();
        super.onDetachedFromWindow();
    }

    @Override
    protected void onWindowVisibilityChanged(int visibility) {
        super.onWindowVisibilityChanged(visibility);
    }



    public boolean isHasAttachToWindow() {
        return hasAttachToWindow;
    }

    @Override
    public void onClick(View v) {
        if(documentView == null){
            return;
        }
        int[] location = new int[2];
        v.getLocationOnScreen(location);
        float x = lastEvent.getRawX()  - location[0];
        float y = lastEvent.getRawY() - location[1];
        documentView.actionEvent(OnEventListener.EVENT_TYPE_CLICK, x, y);

    }


    public boolean hasDocumentView(){
        return documentView != null;
    }


    public GLTaskQueue getTaskQueue() {
        return gpuThreadTaskQueue;
    }

    public DocumentView getDocumentView(){
        if(documentView == null){
            documentView = new DocumentView(getContext());
        }
        return documentView;
    }

    public void setDocumentView(DocumentView documentView) {
        this.documentView = documentView;
        if(hasAttachToWindow){
            throw new RuntimeException("setDocumentView should be be remove first");
        }
    }



    public void destroy(){
        if(documentView != null){
            documentView.destory();
        }
    }



    @Override
    protected void finalize() throws Throwable {
        destroy();
        super.finalize();
    }
}
