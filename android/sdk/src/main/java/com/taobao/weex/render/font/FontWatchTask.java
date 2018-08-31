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
package com.taobao.weex.render.font;

import android.text.TextUtils;

import com.taobao.weex.render.bridge.RenderBridge;
import com.taobao.weex.render.log.RenderLog;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.view.DocumentView;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by furture on 2018/8/30.
 */

public class FontWatchTask implements  Runnable{
    private List<DocumentView> documentViews;
    private boolean isLoaded;
    private String fontFaimly;
    private String fontUrl;
    private String fontPath;
    private boolean isLoading;

    public FontWatchTask(String fontFaimly, String fontUrl) {
        this.fontFaimly = fontFaimly;
        this.fontUrl = fontUrl;
        this.documentViews = new ArrayList<>();
        this.isLoaded = false;
    }

    public void notifyFontReadyWithFilePath(String filePath){
        this.fontPath = filePath;
        RenderManager.getInstance().getGpuHandler().post(this);
    }


    boolean isSame(String fontFaimly, String fontUrl){
        return this.fontFaimly.equals(fontFaimly) && this.fontUrl.equals(fontUrl);
    }


    @Override
    public void run() {
        if(TextUtils.isEmpty(fontPath)){
            synchronized (documentViews){
                documentViews.clear();
            }
            return;
        }
        if(isLoaded){
            return;
        }
        this.isLoaded = true;
        if(new File(fontPath).exists()){
            if(!RenderBridge.getInstance().hasFont(fontFaimly, fontPath)){
                RenderBridge.getInstance().addFont(fontFaimly, fontPath);
                synchronized (documentViews){
                    for(DocumentView documentView : documentViews){
                        documentView.refreshFont(fontFaimly);
                    }
                }
            }
        }else{
            //RenderLog.actionError();
        }
        synchronized (documentViews){
            documentViews.clear();
        }
    }

    public void addWatchDocument(DocumentView documentView){
        if(isLoaded){
            return;
        }
        synchronized (documentViews){
            documentViews.add(documentView);
        }
    }

    public boolean isLoaded() {
        return isLoaded;
    }

    public boolean isLoading() {
        return isLoading;
    }

    public void setLoading(boolean loading) {
        isLoading = loading;
    }

    public String getFontFaimly() {
        return fontFaimly;
    }

    public String getFontUrl() {
        return fontUrl;
    }

    public String getFontPath() {
        return fontPath;
    }
}
