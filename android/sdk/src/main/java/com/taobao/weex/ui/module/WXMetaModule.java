/*
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
package com.taobao.weex.ui.module;

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.text.TextUtils;
import android.widget.Toast;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by zhengshihan on 16/12/20.
 */

public class WXMetaModule extends WXModule {

    public static final String WIDTH = "width";
    public static final String DEVICE_WIDTH = "device-width";

    @JSMethod(uiThread = false)
    public void setViewport(String param) {
        if (!TextUtils.isEmpty(param)) {
            try {
                param = URLDecoder.decode(param, "utf-8");
                JSONObject jsObj = JSON.parseObject(param);
                Context cxt = mWXSDKInstance.getContext();
                if (DEVICE_WIDTH.endsWith(jsObj.getString(WIDTH))) {
                    int width = (int)(WXViewUtils.getScreenWidth(cxt)/WXViewUtils.getScreenDensity(cxt));
                    mWXSDKInstance.setViewPortWidth(width);
                    mWXSDKInstance.setInstanceViewPortWidth(width);
                } else {
                    int width = jsObj.getInteger(WIDTH);
                    if (width > 0) {
                        mWXSDKInstance.setViewPortWidth(width);
                        mWXSDKInstance.setInstanceViewPortWidth(width);
                    }
                }
            } catch (Exception e) {
                WXLogUtils.e("[WXModalUIModule] alert param parse error ", e);
            }
        }
    }


    @JSMethod(uiThread = true)
    public void openLog(String open) {
        Application application = WXEnvironment.getApplication();
        if(application == null){
            return;
        }
        ApplicationInfo info = application.getApplicationInfo();
        if((info.flags & ApplicationInfo.FLAG_DEBUGGABLE)!= 0){
            if(WXUtils.getBoolean(open, true)) {
                WXEnvironment.setApkDebugable(true);
                if(mWXSDKInstance != null) {
                    Toast.makeText(mWXSDKInstance.getContext(), "log open success", Toast.LENGTH_SHORT).show();
                }
            }else{
                WXEnvironment.setApkDebugable(false);
                if(mWXSDKInstance != null) {
                    Toast.makeText(mWXSDKInstance.getContext(), "log close success", Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

    @JSMethod(uiThread = false)
    public void getPageInfo(JSCallback callback) {
        if(callback == null){
            return;
        }
        List<WXSDKInstance> instances = WXSDKManager.getInstance().getWXRenderManager().getAllInstances();
        Map<String,Object> map = new HashMap<>(4);
        for(WXSDKInstance instance : instances){
            if(TextUtils.isEmpty(instance.getBundleUrl())){
                continue;
            }
            map.put(instance.getBundleUrl(), instance.getTemplateInfo());
        }
        callback.invoke(map);
    }
}
