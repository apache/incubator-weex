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
package com.alibaba.weex.commons.render;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;

import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;
import com.taobao.weex.render.image.FrameImage;
import com.taobao.weex.render.image.FrameImageProcessor;

/**
 * Created by furture on 2018/7/24.
 */

public class PicassoImageTarget implements FrameImage, Target {

    private Bitmap mBitmap;
    private String url;
    private int width;
    private int height;
    private int callbackId;
    private int loadingState;

    public PicassoImageTarget(String imageUrl, int width, int height, int callbackId) {
        this.url = imageUrl;
        this.width = width;
        this.height = height;
        this.callbackId = callbackId;
    }

    @Override
    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom loadedFrom) {
        mBitmap = bitmap;
        FrameImageProcessor.toRenderSupportImage(this);
    }

    @Override
    public void onBitmapFailed(Drawable drawable) {
        loadingState = LOADING_FAILED;
        FrameImageProcessor.notifyFrameImage(null, getCallbackId());
    }

    @Override
    public void onPrepareLoad(Drawable drawable) {

    }

    @Override
    public int getCallbackId() {
        return callbackId;
    }

    @Override
    public String getUrl() {
        return url;
    }


    @Override
    public Bitmap getBitmap() {
        return mBitmap;
    }

    @Override
    public int getLoadingState() {
        return loadingState;
    }

    @Override
    public void onSupportedBitmap(Bitmap bitmap) {
        mBitmap = bitmap;
        loadingState = LOADING_SUCCESS;
        FrameImageProcessor.notifyFrameImage(bitmap, getCallbackId());
    }
}
