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
package com.taobao.weex.render.image;

import android.util.LruCache;

import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by furture on 2018/8/20.
 */

public class RenderBitmapImageCache {

    private LruCache<Long,WeakReference<Map<String, BitmapTarget>>> imageTargetsCache;

    private RenderBitmapImageCache(){
         imageTargetsCache   = new LruCache<>(512);
    }


    private static RenderBitmapImageCache cacheInstance;
    public static RenderBitmapImageCache getInstance(){
        if(cacheInstance == null){
            synchronized (RenderBitmapImageCache.class){
                if(cacheInstance == null) {
                    cacheInstance = new RenderBitmapImageCache();
                }
            }
        }
        return cacheInstance;
    }


    public BitmapTarget getImageTarget(long documentKey, String imageUrl){
        Map<String, BitmapTarget> cache = getImageTarget(documentKey);
        if(cache == null){
            return null;
        }
        return cache.get(imageUrl);
    }

    private Map<String, BitmapTarget> getImageTarget(long documentKey){
        WeakReference<Map<String, BitmapTarget>> imageTargetWeakReference = imageTargetsCache.get(documentKey);
        if(imageTargetWeakReference == null){
            return null;
        }
        Map<String, BitmapTarget> imageTargetMap = imageTargetWeakReference.get();
        return imageTargetMap;
    }


    public void cacheImageTargets(long documentKey, Map<String, BitmapTarget> imageTargetMap){
        if(imageTargetMap == null || imageTargetMap.size() == 0) {
            return;
        }

        Map<String, BitmapTarget> caches =  getImageTarget(documentKey);
        if(caches == null){
            caches = new ConcurrentHashMap<>();
            imageTargetsCache.put(documentKey, new WeakReference<Map<String, BitmapTarget>>(caches));
        }
        caches.clear();
        Set<Map.Entry<String, BitmapTarget>> entrySet =  imageTargetMap.entrySet();
        for(Map.Entry<String, BitmapTarget> entry : entrySet){
            caches.put(entry.getKey(), entry.getValue());
        }
    }


}
