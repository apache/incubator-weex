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
package com.alibaba.weex.extend.render;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;
import com.taobao.weex.render.image.BitmapTarget;
import com.taobao.weex.render.image.RenderBitmapProcessor;
import com.taobao.weex.render.view.DocumentView;

/**
 * Created by furture on 2018/7/24.
 */

public class PicassoImageTarget implements BitmapTarget, Target {
    private Bitmap mBitmap;
    private String ref;
    private String url;
    private boolean isPlaceholder;
    private int loadingState;
    private DocumentView documentView;

    public PicassoImageTarget(DocumentView documentView, String ref, String url, boolean isPlaceholder) {
        this.loadingState = LOADING;
        this.documentView = documentView;
        this.ref = ref;
        this.url = url;
        this.isPlaceholder = isPlaceholder;
    }

    @Override
    public String getRef() {
        return ref;
    }

    @Override
    public String getUrl() {
        return url;
    }

    @Override
    public boolean isPlaceholder() {
        return isPlaceholder;
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
    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {
        mBitmap = bitmap;
        RenderBitmapProcessor.toRenderSupportBitmap(this);
    }

    @Override
    public void onSupportedBitmap(Bitmap bitmap) {
        mBitmap = bitmap;
        loadingState = LOADING_SUCCESS;
        documentView.onLoadImageTarget(this);
    }

    @Override
    public void onBitmapFailed(Drawable errorDrawable) {
        loadingState = LOADING_FAILED;
        documentView.onLoadImageTarget(this);
    }


    @Override
    public void onPrepareLoad(Drawable placeHolderDrawable) {

    }
}
