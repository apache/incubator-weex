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
import android.net.Uri;

import com.squareup.picasso.Picasso;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.URIAdapter;
import com.taobao.weex.common.WXImageSharpen;
import com.taobao.weex.common.WXImageStrategy;
import com.taobao.weex.render.image.BitmapTarget;
import com.taobao.weex.render.image.RenderImageAdapter;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.view.DocumentView;
import com.taobao.weex.ui.WXRenderManager;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXImage;

/**
 * Created by furture on 2018/8/10.
 */

public class PicassoImageAdapter extends RenderImageAdapter {
    @Override
    public BitmapTarget requestImageTarget(final DocumentView documentView, String ref, String url, int width, int height, boolean isPlaceholder) {
        WXSDKInstance instance = (WXSDKInstance) documentView.getInstance();
        if(instance != null){
            url = instance.rewriteUri(Uri.parse(url), URIAdapter.IMAGE).toString();
            WXImage image = (WXImage) WXSDKManager.getInstance().getWXRenderManager().getWXComponent(instance.getInstanceId(), ref);
            if(image == null){
                return null;
            }

            WXImageStrategy imageStrategy = new WXImageStrategy();
            imageStrategy.isClipping = true;
            WXImageSharpen imageSharpen = image.getAttrs().getImageSharpen();
            imageStrategy.isSharpen = imageSharpen == WXImageSharpen.SHARPEN;
            imageStrategy.blurRadius = 0;
            imageStrategy.instanceId = image.getInstanceId();
        }
        final String imageUrl = url;
        PicassoImageTarget loadImageTarget = new PicassoImageTarget(documentView, ref, url, isPlaceholder);
        final PicassoImageTarget requestTarget  = loadImageTarget;
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                Picasso.with(documentView.getContext()).load(Uri.parse(imageUrl)).into(requestTarget);
            }
        });
        return loadImageTarget;
    }
}
