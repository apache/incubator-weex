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

import android.content.Context;
import android.text.TextUtils;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.net.URLDecoder;

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
}
