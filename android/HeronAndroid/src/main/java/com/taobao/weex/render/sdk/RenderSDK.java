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
package com.taobao.weex.render.sdk;

import android.app.Application;

import com.taobao.weex.render.bridge.WXRenderUI;
import com.taobao.weex.render.image.FrameImageAdapter;
import com.taobao.weex.render.lifecycle.RenderLifecycleCallback;

public class RenderSDK {


    private Application mApplication;
    private FrameImageAdapter mImageAdapter;
    private static RenderSDK mRenderSDK;
    private RenderLifecycleCallback mRenderLifecycleCallback;
    private boolean mInited;

    private RenderSDK(){
        mRenderLifecycleCallback = new RenderLifecycleCallback();
    }

    public static RenderSDK getInstance(){
        if(mRenderSDK == null){
            synchronized (RenderSDK.class){
                if(mRenderSDK == null){
                    mRenderSDK = new RenderSDK();
                }
            }
        }
        return mRenderSDK;
    }


    public Application getApplication() {
        return mApplication;
    }

    public RenderSDK setApplication(Application application) {
        this.mApplication = application;
        return this;
    }


    public FrameImageAdapter getImageAdapter() {
        return mImageAdapter;
    }

    public RenderSDK setImageAdapter(FrameImageAdapter imageAdapter) {
        this.mImageAdapter = imageAdapter;
        return this;
    }

    public void init(){
        if(!mInited){
            WXRenderUI.getInstance().ensureInit();
            mApplication.unregisterActivityLifecycleCallbacks(mRenderLifecycleCallback);
            mApplication.unregisterComponentCallbacks(mRenderLifecycleCallback);
            mApplication.registerComponentCallbacks(mRenderLifecycleCallback);
            mApplication.registerActivityLifecycleCallbacks(mRenderLifecycleCallback);
            mInited = true;
        }
    }


}
