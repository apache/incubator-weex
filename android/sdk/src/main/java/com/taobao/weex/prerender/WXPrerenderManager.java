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
package com.taobao.weex.prerender;

import android.content.Context;
import android.support.annotation.Nullable;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXRenderStrategy;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by luciolong on 03/08/2018.
 */
public class WXPrerenderManager {
    private volatile ConcurrentHashMap<String, WXSDKInstance> mPreloadInstanceMap = new ConcurrentHashMap<>();

    private static class SingletonHolder {
        private static final WXPrerenderManager INSTANCE = new WXPrerenderManager();
    }

    public static WXPrerenderManager getInstance() {
        return SingletonHolder.INSTANCE;
    }

    private WXPrerenderManager() {
    }

    public WXSDKInstance preRenderByUrl(Context context, String pageName, final String url,
                                        Map<String, Object> options, final String jsonInitData,
                                        final int width, final int height, final WXRenderStrategy flag) {
        return preRenderByUrl(context, pageName, url, options, jsonInitData, width, height, flag, PreRenderContext.LAYOUT_MODE_SPECIFIC, PreRenderContext.DEFAULT_OFFSET);
    }

    public WXSDKInstance preRenderByUrl(Context context, String pageName, final String url,
                                        Map<String, Object> options, final String jsonInitData,
                                        final int width, final int height, final WXRenderStrategy flag, final int layoutMode) {
        return preRenderByUrl(context, pageName, url, options, jsonInitData, width, height, flag, layoutMode, PreRenderContext.DEFAULT_OFFSET);
    }

    public WXSDKInstance preRenderByUrl(Context context, String pageName, final String url,
                                        Map<String, Object> options, final String jsonInitData,
                                        final int width, final int height, final WXRenderStrategy flag,
                                        final int layoutMode, final float offset) {
        if (!mPreloadInstanceMap.containsKey(url)) {
            WXSDKInstance instance = new WXSDKInstance(context);
            instance.preRenderByUrl(pageName, url, options, jsonInitData, width, height, flag, layoutMode, offset);
            mPreloadInstanceMap.put(url, instance);
            return instance;
        } else {
            return mPreloadInstanceMap.get(url);
        }
    }

    public WXSDKInstance preRenderByTemplate(Context context, String pageName, final String url,
                                             String template, Map<String, Object> options, final String jsonInitData,
                                             final int width, final int height, final WXRenderStrategy flag) {
        return preRenderByTemplate(context, pageName, url, template, options, jsonInitData, width, height, flag, PreRenderContext.LAYOUT_MODE_SPECIFIC, PreRenderContext.DEFAULT_OFFSET);
    }

    public WXSDKInstance preRenderByTemplate(Context context, String pageName, final String url,
                                             String template, Map<String, Object> options, final String jsonInitData,
                                             final int width, final int height, final WXRenderStrategy flag, final int layoutMode) {
        return preRenderByTemplate(context, pageName, url, template, options, jsonInitData, width, height, flag, layoutMode, PreRenderContext.DEFAULT_OFFSET);
    }

    public WXSDKInstance preRenderByTemplate(Context context, String pageName, final String url,
                                             String template, Map<String, Object> options, final String jsonInitData,
                                             final int width, final int height, final WXRenderStrategy flag,
                                             final int layoutMode, final float offset) {
        if (!mPreloadInstanceMap.containsKey(url)) {
            WXSDKInstance instance = new WXSDKInstance(context);
            instance.preRenderByTemplateWithLayoutMode(pageName, template, options, jsonInitData, width, height, flag, layoutMode, offset);
            mPreloadInstanceMap.put(url, instance);
            return instance;
        } else {
            return mPreloadInstanceMap.get(url);
        }
    }

    @Nullable
    public WXSDKInstance fetchPreload(String url) {
        WXSDKInstance instance = mPreloadInstanceMap.get(url);
        if (instance != null) {
            mPreloadInstanceMap.remove(url);
        }
        return instance;
    }

}
