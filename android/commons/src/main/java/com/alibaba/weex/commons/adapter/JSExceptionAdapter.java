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
package com.alibaba.weex.commons.adapter;

import java.util.Map;

import com.alibaba.fastjson.JSON;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.common.WXJSExceptionInfo;

/**
 */

public class JSExceptionAdapter implements IWXJSExceptionAdapter {

    @Override
    public void onJSException(WXJSExceptionInfo wxjsExceptionInfo) {
        if (null == wxjsExceptionInfo) {
            return;
        }
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(wxjsExceptionInfo.getInstanceId());
        if (null != instance) {
            Map<String, Object> params = null;
            try {
                params = JSON.parseObject(JSON.toJSONString(wxjsExceptionInfo));
            } catch (Exception e) {
                params.put("bundleUrl", wxjsExceptionInfo.getBundleUrl());
                params.put("errorCode", wxjsExceptionInfo.getErrCode());
                params.put("exception", wxjsExceptionInfo.getException());
                params.put("extParams", wxjsExceptionInfo.getExtParams());
                params.put("function", wxjsExceptionInfo.getFunction());
                params.put("instanceId", wxjsExceptionInfo.getInstanceId());
                params.put("jsFrameworkVersion", wxjsExceptionInfo.getJsFrameworkVersion());
                params.put("weexVersion", wxjsExceptionInfo.getWeexVersion());
            }
            instance.fireGlobalEventCallback("exception", params);
        }
        //WXLogUtils.d(wxjsExceptionInfo.toString());
    }

}
