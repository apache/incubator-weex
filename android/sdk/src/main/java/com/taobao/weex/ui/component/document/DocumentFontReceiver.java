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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.render.font.FontWatchTask;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.view.DocumentView;

import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.Set;


/**
 * Created by furture on 2018/8/30.
 */

public class DocumentFontReceiver extends BroadcastReceiver {

    private WXSDKInstance instance;
    private FontWatchTask fontWatchTask;

    public DocumentFontReceiver(WXSDKInstance instance, FontWatchTask fontWatchTask) {
        this.instance = instance;
        this.fontWatchTask = fontWatchTask;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if(instance == null){
            return;
        }
        String fontFamily = intent.getStringExtra("fontFamily");
        String filePath = intent.getStringExtra("filePath");
        if(TextUtils.equals(fontFamily, fontWatchTask.getFontFaimly())){
            LocalBroadcastManager.getInstance(instance.getContext()).unregisterReceiver(this);
            Map<Long,WeakReference<DocumentView>> maps =  RenderManager.getInstance().getDocumentsMap();
            Set<Map.Entry<Long,WeakReference<DocumentView>>> entrySet = maps.entrySet();
            for(Map.Entry<Long,WeakReference<DocumentView>> entry : entrySet){
                DocumentView documentView = entry.getValue().get();
                if(documentView == null){
                    continue;
                }
                if(documentView.getInstance() != instance){
                    continue;
                }
                fontWatchTask.addWatchDocument(documentView);
            }
            fontWatchTask.notifyFontReadyWithFilePath(filePath);
            instance = null;
        }
    }
}
