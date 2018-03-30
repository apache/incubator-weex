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
package com.taobao.weex.dom.text;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXTextDomObject;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.ref.WeakReference;

/**
 * Created by furture on 2018/2/24.
 */

public class FontBroadcastReceiver extends BroadcastReceiver {

    private WeakReference<WXTextDomObject> wxTextDomObjectRef;

    private String mFontFamily;

    public FontBroadcastReceiver(WXTextDomObject wxTextDomObject, String mFontFamily) {
        this.wxTextDomObjectRef = new WeakReference<WXTextDomObject>(wxTextDomObject);
        this.mFontFamily = mFontFamily;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String fontFamily = intent.getStringExtra("fontFamily");
        if (!mFontFamily.equals(fontFamily)) {
            return;
        }
        WXTextDomObject wxTextDomObject = wxTextDomObjectRef.get();
        if(wxTextDomObject == null){
            return;
        }
        if(wxTextDomObject.isDestroy() || wxTextDomObject.getDomContext() == null){
            return;
        }

        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(wxTextDomObject.getDomContext().getInstanceId());
        if(domActionContext == null){
            return;
        }
        WXDomObject domObject = domActionContext.getDomByRef(wxTextDomObject.getRef());
        if(domObject == null){
            return;
        }
        domObject.markDirty();
        domActionContext.markDirty();
        WXSDKManager.getInstance().getWXDomManager().sendEmptyMessageDelayed(WXDomHandler.MsgType.WX_DOM_START_BATCH, 2);
        if(WXEnvironment.isApkDebugable()) {
            WXLogUtils.d("WXText", "Font family " + fontFamily + " is available");
        }
    }
}
