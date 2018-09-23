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

import com.taobao.weex.render.threads.GpuThread;
import com.taobao.weex.render.threads.IoThread;
import com.taobao.weex.render.view.DocumentView;

import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Created by furture on 2018/7/24.
 */

public class RenderManager {


    private Map<Long,WeakReference<DocumentView>> documentsMap;
    private Handler uiHandler;
    private Handler gpuHandler;
    private AtomicLong atomicLong;
    private Handler ioHandler;




    private RenderManager(){
        documentsMap = new ConcurrentHashMap<>();
        uiHandler = new Handler(Looper.getMainLooper());
        gpuHandler = new Handler(GpuThread.getThread().getLooper());
        atomicLong = new AtomicLong(0);
    }

    public Handler getUiHandler(){
        return uiHandler;
    }

    public void registerDocument(long key, DocumentView render){
        WeakReference<DocumentView> surfaceRenderWeakReference = new WeakReference<DocumentView>(render);
       documentsMap.put(key, surfaceRenderWeakReference);
    }

    public long nextDocumentKey(){
        return atomicLong.incrementAndGet();
    }

    public DocumentView getDocument(long key){
        WeakReference<DocumentView> surfaceRenderWeakReference = documentsMap.get(key);
        if(surfaceRenderWeakReference == null){
            return null;
        }
        return surfaceRenderWeakReference.get();
    }

    public Map<Long,WeakReference<DocumentView>> getDocumentsMap(){
         return documentsMap;
    }


    public void onActivityResumed(Activity activity) {
        Set<Map.Entry<Long,WeakReference<DocumentView>>> entries = documentsMap.entrySet();
        for(Map.Entry<Long,WeakReference<DocumentView>> entry : entries){
            DocumentView documentView = entry.getValue().get();
            if(documentView == null){
                continue;
            }
            if(isAcivityDocument(documentView, activity)){
                documentView.invalidate();
            }
        }
    }

    public void onActivityPaused(Activity activity) {

    }

    public void onActivityDestroyed(Activity activity) {
        Set<Map.Entry<Long,WeakReference<DocumentView>>> entries = documentsMap.entrySet();
        for(Map.Entry<Long,WeakReference<DocumentView>> entry : entries){
            DocumentView documentView = entry.getValue().get();
            if(documentView == null){
                continue;
            }
            if(isAcivityDocument(documentView, activity)){
                documentView.destory();
            }
        }
    }

    private boolean isAcivityDocument(DocumentView documentView, Activity activity){
        if(documentView.getContext() == activity || documentView.getContext() == activity.getBaseContext()){
            return true;
        }
        return false;
    }



    public void removeDocument(long key){
        documentsMap.remove(key);
    }

    public Handler getGpuHandler() {
        return gpuHandler;
    }

    public Handler getIoHandler(){
        if(ioHandler == null){
            synchronized (this){
                if(ioHandler == null) {
                    ioHandler = new Handler(IoThread.getThread().getLooper());
                }
            }
        }
        return ioHandler;
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
}
