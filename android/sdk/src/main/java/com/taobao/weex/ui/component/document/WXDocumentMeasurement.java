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
package com.taobao.weex.ui.component.document;

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.layout.MeasureMode;
import com.taobao.weex.render.event.OnDocumentSizeChangedListener;
import com.taobao.weex.render.view.DocumentView;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/8/23.
 */

public class WXDocumentMeasurement extends ContentBoxMeasurement implements OnDocumentSizeChangedListener, Runnable {


    private WXDocumentComponent documentComponent;
    private float lastComputedWidth;
    private float lastComputedHeight;
    private float maxHeight;
    private float maxWidth;

    public WXDocumentMeasurement(WXDocumentComponent documentComponent) {
        this.documentComponent = documentComponent;
        this.maxHeight = WXViewUtils.getScreenHeight(documentComponent.getContext())*2;
        this.maxWidth = maxHeight;
    }



    @Override
    public void measureInternal(float width, float height, int widthMeasureMode, int heightMeasureMode) {
        if(documentComponent.getDocumentView() == null){
            return;
        }
        if(documentComponent.getDocumentView().getDocumentWidth() <= 0
                || documentComponent.getDocumentView().getDocumentHeight() <= 0){
            return;
        }
        if(widthMeasureMode != MeasureMode.EXACTLY){
            mMeasureWidth = documentComponent.getDocumentView().getDocumentWidth();
            if(mMeasureWidth > maxWidth){
                mMeasureWidth  = maxWidth;
            }
        }else{
            mMeasureWidth = width;
        }
        if(heightMeasureMode != MeasureMode.EXACTLY){
            mMeasureHeight = documentComponent.getDocumentView().getDocumentHeight();
            if(mMeasureHeight > maxHeight){
                mMeasureHeight  = maxHeight;
            }
        }else{
            mMeasureHeight= height;
        }
    }

    @Override
    public void layoutBefore() {

    }

    @Override
    public void layoutAfter(float computedWidth, float computedHeight) {
        if(computedWidth <= 0 || computedHeight <= 0 || Float.isNaN(computedHeight)){
            return;
        }
        if(computedWidth >= maxWidth){
            return;
        }
        if(computedHeight >= maxHeight){
            return;
        }

        componentShouldInit();
        if(documentComponent.getDocumentView() == null){
            return;
        }
        if(lastComputedWidth != computedWidth || lastComputedHeight != computedHeight){
            lastComputedWidth = computedWidth;
            lastComputedHeight = computedHeight;

            Map<String,Object> map = null;
            if(shouldUpdateHeight(computedHeight)){
                int pixelHeight = (int) computedHeight;
                if(pixelHeight != documentComponent.getDocumentView().getDocumentHeight()){
                    if(map == null){
                        map = new HashMap<>();
                    }
                    map.put(Constants.Name.HEIGHT, computedHeight*750/ WXViewUtils.getScreenWidth());
                }
            }
            if(shouldUpdateWidth(computedWidth)){
                int pixelWidth = (int) computedWidth;
                if(pixelWidth != documentComponent.getDocumentView().getDocumentWidth()){
                    if(map == null){
                        map = new HashMap<>();
                    }
                    map.put(Constants.Name.WIDTH, computedWidth*750/WXViewUtils.getScreenWidth());
                }
            }
            if(map != null && map.size() > 0){
                documentComponent.updateStyles(map);
                Log.e("Weex", "Weex layout measure update styles " + JSON.toJSONString(map)
                + "  " + documentComponent.getDocumentView().getDocumentWidth());
            }
        }
    }

    private boolean shouldUpdateHeight(float computedHeight){
        if(computedHeight <= 0){
            return false;
        }
        if(documentComponent.getStyles().get(Constants.Name.HEIGHT) != null){
            float styleHeight = WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(documentComponent.getStyles().get(Constants.Name.HEIGHT),
                    documentComponent.getViewPortWidth()), documentComponent.getViewPortWidth());
            if(Math.abs(styleHeight - (int)computedHeight) <= 0.1){
                return false;
            }
        }
        return true;
    }

    private boolean shouldUpdateWidth(float computedWidth){
        if(computedWidth <= 0){
            return false;
        }
        if(documentComponent.getStyles().get(Constants.Name.WIDTH) != null){
            float styleWidth = WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(documentComponent.getStyles().get(Constants.Name.WIDTH),
                    documentComponent.getViewPortWidth()), documentComponent.getViewPortWidth());
            if(Math.abs(styleWidth - (int)computedWidth) <= 0.1){
                return false;
            }
        }
        return true;
    }


    @Override
    public void onSizeChanged(final DocumentView documentView, final int width, final int height) {
        if(documentView.getDocumentHeight() != height || documentView.getDocumentWidth() != width){
            return;
        }
        synchronized (documentComponent){
            if(documentComponent.isDestoryed()){
                return;
            }
            componentShouldInit();
            if(documentComponent.getStyles().containsKey(Constants.Name.WIDTH) && documentComponent.getStyles().containsKey(Constants.Name.HEIGHT)){
                return;
            }
            WXBridgeManager.getInstance().markDirty(documentComponent.getInstanceId(), documentComponent.getRef(), true);
        }

        WXSDKManager.getInstance().getWXBridgeManager().removeCallback(this);
        WXSDKManager.getInstance().getWXBridgeManager().postAtFrontOfQueue(this);
    }

    @Override
    public void run() {
        if(documentComponent.isDestoryed()){
            return;
        }
        WXBridgeManager.getInstance().markDirty(documentComponent.getInstanceId(), documentComponent.getRef(), true);
    }

    private void componentShouldInit(){
        if(!documentComponent.isDocumentShouldInited()){
            if(documentComponent.getLayoutSize().getWidth() > 0 && documentComponent.getLayoutSize().getHeight() > 0){
                WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(documentComponent.getInstanceId(), new InitDocumentViewAction(documentComponent));
            }
            documentComponent.setDocumentShouldInited(true);
        }
    }
}
