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

import android.graphics.Bitmap;
import android.os.Build;
import android.text.TextUtils;
import android.view.Surface;

import com.taobao.weex.render.RenderSDK;
import com.taobao.weex.render.image.BitmapTarget;
import com.taobao.weex.render.image.RenderBitmapImageCache;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.threads.GpuThread;
import com.taobao.weex.render.view.DocumentView;

import java.io.UnsupportedEncodingException;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

/**
 * Created by furture on 2018/7/24.
 */

public class RenderBridge {

    private static final RenderBridge renderBridge = new RenderBridge();

    public static RenderBridge getInstance(){
        return renderBridge;
    }

    /**
     * @param documentKey documentView key
     * @param url imageUrl
     * @param width content image layout width
     * @param height content image layout height
     * the method is called in gpu thread
     * */
    public static Bitmap getImage(long documentKey, String ref, String url, int width, int height, boolean isPlaceholder){
        return RenderBridge.getInstance().getBitmap(documentKey, ref, url, width, height, isPlaceholder);
    }


    public static boolean isImagePremultiplied(Bitmap bitmap){
        if(bitmap == null){
            return true;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            return bitmap.isPremultiplied();
        }
        return  true;
    }

    public boolean initSDK(int screenWidth, int screenHeight, float density){
        return nativeInitSDK(screenWidth, screenHeight, density);
    }


    public long initOpenGLRender(Surface surface, int width, int height){
        return nativeInitOpenGLRender(surface, width, height);
    }

    public void destroyOpenGLRender(long ptr, Surface surface){
         nativeDestroyOpenGLRender(ptr, surface);
    }


    public String actionEvent(long mNativeDocument, int type, int x, int y) {
        return nativeHandleEvent(mNativeDocument, type, x, y);
    }



    public long createDocument(long key) {
        return nativeCreateDocument(key);
    }

    public boolean invalidate(long mNativeDocument, long mRender) {
        return nativeInvalidate(mNativeDocument, mRender);
    }


    public void layoutIfNeed(long ptr){
        nativeLayoutIfNeed(ptr);
    }

    public void renderClearBuffer(long openGLRenderPtr) {
         nativeClearRenderBuffer(openGLRenderPtr);
    }

    public void renderSizeChanged(long openGLRenderPtr, int width, int height) {
        nativeRenderSizeChanged(openGLRenderPtr, width, height);
    }

    public void renderSwap(long openGLRenderPtr) {
        nativeRenderSwap(openGLRenderPtr);
    }



    public void destroyDocument(long mNativeDocument) {
        nativeDestroyDocument(mNativeDocument);
    }

    public void createBody(long ptr, String ref, Map<String,String> styles, Map<String,String> attrs, Collection<String> events){
        nativeCreateBody(ptr, ref, toNativeMap(styles), toNativeMap(attrs), toNativeSet(events));
    }

    public void actionAddElement(long ptr, String ref, String type, String parentRef, int index, Map<String,String> styles, Map<String,String> attrs, Collection<String> events){
        nativeActionAddElement(ptr, ref, type, parentRef, index, toNativeMap(styles), toNativeMap(attrs), toNativeSet(events));
    }

    public void actionUpdateAttrs(long nativeDocument, String ref, Map<String, String> attrs) {
        nativeActionUpdateAttrs(nativeDocument, ref, toNativeMap(attrs));
    }

    public void actionUpdateStyles(long nativeDocument, String ref, Map<String, String> styles) {
        nativeActionUpdateStyles(nativeDocument, ref, toNativeMap(styles));
    }

    public void actionAddEvent(long nativeDocument, String ref, String event) {
        if(TextUtils.isEmpty(event) || TextUtils.isEmpty(ref)){
            return;
        }
        nativeActionAddEvent(nativeDocument, ref, event);
    }

    public void actionRemoveEvent(long nativeDocument, String ref, String event) {
        if(TextUtils.isEmpty(event) || TextUtils.isEmpty(ref)){
            return;
        }
        nativeActionRemoveEvent(nativeDocument, ref, event);
    }


    public void actionRemoveElement(long nativeDocument, String ref) {
        nativeActionRemoveElement(nativeDocument, ref);
    }


    public void actionMoveElement(long nativeDocument, String ref, String parentRef, int index){
        nativeActionMoveElement(nativeDocument, ref, parentRef, index);
    }

    public void actionRefreshFont(final long nativeDocument, final String fontFamilyName){
        if(Thread.currentThread() != GpuThread.getThread()){
            RenderManager.getInstance().getGpuHandler().post(new Runnable() {
                @Override
                public void run() {
                    actionRefreshFont(nativeDocument, fontFamilyName);
                }
            });
            return;
        }
        nativeRefreshFont(nativeDocument, fontFamilyName);
    }

    public void addFont(final String fontFamilyName, final String fontPath){
        if(Thread.currentThread() != GpuThread.getThread()){
            RenderManager.getInstance().getGpuHandler().post(new Runnable() {
                @Override
                public void run() {
                    addFont(fontFamilyName, fontPath);
                }
            });
            return;
        }
        nativeAddFont(fontFamilyName, fontPath);
    }


    public boolean hasFont(final String fontFamilyName, final String fontPath){
        return nativeHasFont(fontFamilyName, fontPath);
    }




    public int documentWidth(long ptr){
        return nativeDocumentWidth(ptr);
    }
    public int documentHeight(long ptr){
        return nativeDocumentHeight(ptr);
    }

    public int getBlockLayout(long ptr, String ref, int edge){
        return nativeGetBlockLayout(ptr, ref, edge);
    }

    public String getNodeAttr(long ptr, String ref, String key){
        byte [] bts =  nativeGetNodeAttr(ptr, ref, key);
        if(bts == null){
            return null;
        }
        try {
            return new String(bts, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            return null;
        }
    }

    public String getNodeType(long ptr, String ref){
        return nativeGetNodeType(ptr, ref);
    }

    public boolean nodeContainsEvent(long ptr, String ref, String event){
        return nativeNodeContainsEvent(ptr, ref, event);
    }

    public long toNativeMap(Map<String,String> styles){
        long map = nativeNewMap();
        if(styles != null){
            Set<Map.Entry<String,String>> entrySet = styles.entrySet();
            for(Map.Entry<String,String> entry : entrySet){
                if(entry.getValue() == null){
                    continue;
                }
                nativeMapPut(map, entry.getKey(), entry.getValue());
            }
        }
        return map;
    }

    public long toNativeSet(Collection<String> events){
        long set = nativeNewSet();
        if(events != null){
            for(String envent : events){
                nativeSetAdd(set, envent);
            }
        }
        return set;
    }




    public Bitmap getBitmap(long documentKey, String ref, final String url, int width, int height, boolean isPlaceholder){
        if(TextUtils.isEmpty(url)){
            return null;
        }
        if(RenderSDK.getInstance().getImageAdapter() == null){
            return null;
        }
        DocumentView documentView = RenderManager.getInstance().getDocument(documentKey);
        if(documentView == null || documentView.getOpenGLRender() == null){
            return null;
        }
        String imageKey = RenderSDK.getInstance().getImageAdapter().genImageKey(documentView, ref, url, width, height, isPlaceholder);
        BitmapTarget target = documentView.getImageTargetMap().get(imageKey);
        if(target == null){
            target = RenderBitmapImageCache.getInstance().getImageTarget(documentKey, imageKey);
            if(target != null){
                documentView.getImageTargetMap().put(imageKey, target);
            }
        }
        if(target != null) {
            if (target.getLoadingState() == BitmapTarget.LOADING) {
                return null;
            }
            if (target.getLoadingState() == BitmapTarget.LOADING_SUCCESS) {
                return target.getBitmap();
            }
        }
        BitmapTarget loadImageTarget = RenderSDK.getInstance().getImageAdapter().requestImageTarget(documentView, ref, url, width, height, isPlaceholder);
        if(loadImageTarget == null || imageKey == null){
            return null;
        }
        documentView.getImageTargetMap().put(imageKey, loadImageTarget);
        return loadImageTarget.getBitmap();
    }



    private native boolean nativeInitSDK(int screenWidth, int screenHeight, float density);
    private native long nativeInitOpenGLRender(Surface surface, int width, int height);
    private native void nativeDestroyOpenGLRender(long ptr, Surface surface);
    private native long nativeCreateDocument(long key);
    private native boolean nativeInvalidate(long ptr, long mRender);
    private native void nativeLayoutIfNeed(long ptr);
    private native String nativeHandleEvent(long mNativeDocument, int type, int x, int  y);
    private native void nativeRenderSizeChanged(long openGLRenderPtr, int width, int height);
    private native void nativeClearRenderBuffer(long openGLRenderPtr);
    private native void nativeRenderSwap(long openGLRenderPtr);
    private native void nativeDestroyDocument(long ptr);
    private native int  nativeDocumentWidth(long ptr);
    private native int  nativeDocumentHeight(long ptr);
    private native void nativeCreateBody(long ptr, String ref, long styles, long attrs, long events);
    private native void nativeActionAddElement(long ptr, String ref, String type, String parentRef, int index, long styles, long attrs, long events);
    private native void nativeActionUpdateAttrs(long ptr, String ref, long attrs);
    private native void nativeActionUpdateStyles(long ptr, String ref, long styles);
    private native void nativeActionAddEvent(long nativeDocument, String ref, String event);
    private native void nativeActionRemoveEvent(long nativeDocument, String ref, String event);
    private native void nativeActionRemoveElement(long nativeDocument, String ref);
    private native void nativeActionMoveElement(long nativeDocument, String ref, String parentRef, int index);
    private native void nativeRefreshFont(long nativeDocument, String fontFamilyName);
    private native void nativeAddFont(String fontFamilyName, String fontPath);
    private native boolean nativeHasFont(String fontFamilyName, String fontPath);
    private native int nativeGetBlockLayout(long ptr, String ref, int edge);
    private native byte[] nativeGetNodeAttr(long ptr, String ref, String key);
    private native String nativeGetNodeType(long ptr, String ref);
    private native boolean nativeNodeContainsEvent(long ptr, String ref, String event);
    private native long nativeNewMap();
    private native void nativeMapPut(long ptr, String key, String value);
    private native long nativeNewSet();
    private native void nativeSetAdd(long ptr, String value);
}
