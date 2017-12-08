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
package com.alibaba.weex.extend.adapter;

import java.util.HashMap;
import java.util.Map;

import com.alibaba.fastjson.JSON;
import com.alibaba.ha.adapter.AliHaAdapter;
import com.alibaba.ha.adapter.service.bizError.BizErrorInfo;
import com.alibaba.ha.bizerrorreporter.module.AggregationType;

import android.content.Context;
import android.util.Log;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.common.WXJSExceptionInfo;

/**
 */

public class JSExceptionAdapter implements IWXJSExceptionAdapter {

    private final String INSTANCE_ID = "instanceId";
    private final String FRAMEWORK_VERSION = "frameWorkVersion";
    private final String ERROR_CODE = "errorCode";
    private Context mAppContext;

    public JSExceptionAdapter(Context appContext) {
        mAppContext = appContext;
    }

    @Override
    public void onJSException(WXJSExceptionInfo exception) {
        if (null == exception) {
            return;
        }
        try {
            Log.i("weex js err", "js err start");

            BizErrorInfo exceptionModule = new BizErrorInfo();
            exceptionModule.businessType = "WEEX_ERROR";
            exceptionModule.aggregationType = AggregationType.CONTENT;

            //统计维度, weex不要按errorcode聚合，没关系，我把url设置到errcode里去
            String bundleUrl = exception.getBundleUrl();
            if (bundleUrl != null) {
                String code = exceptionUrl(bundleUrl);
                if (code.length() > 1024) {
                    code = code.substring(0, 1024);
                }
                exceptionModule.exceptionCode = code;
                exceptionModule.exceptionDetail = bundleUrl;
            } else {
                Log.i("weex js err", "bundle url is null");
            }
            String weexVersion = exception.getWeexVersion();
            if (weexVersion != null) {
                exceptionModule.exceptionVersion = weexVersion;
            }
            String exceptionContent = exception.getException();
            if (exceptionContent != null) {
                exceptionModule.exceptionArg1 = exceptionContent;
            }
            String function = exception.getFunction();
            if (function != null) {
                exceptionModule.exceptionArg2 = function;
            }

            //不统计维度
            Map<String, Object> exceptionArgs = new HashMap<String, Object>();
            String errorCode = exception.getErrCode();
            if (errorCode != null) {
                exceptionArgs.put(ERROR_CODE, errorCode);
            }
            String instanceId = exception.getInstanceId();
            if (instanceId != null) {
                exceptionArgs.put(INSTANCE_ID, instanceId);
            } else {
                exceptionArgs.put(INSTANCE_ID, "no instanceId");
            }
            String frameWorkVersion = exception.getJsFrameworkVersion();
            if (frameWorkVersion != null) {
                exceptionArgs.put(FRAMEWORK_VERSION, frameWorkVersion);
            } else {
                exceptionArgs.put(FRAMEWORK_VERSION, "no framework version");
            }
            Map<String, String> extParames = exception.getExtParams();
            if (extParames != null && extParames.size() > 0) {
                exceptionArgs.putAll(extParames);
            }
            exceptionModule.exceptionArgs = exceptionArgs;

            //当前线程
            exceptionModule.thread = Thread.currentThread();

            //发送
            AliHaAdapter.getInstance().bizErrorService.sendBizError(mAppContext, exceptionModule);
            //  MotuReportAdapteHandler reportAdapteHandler = new MotuReportAdapteHandler();
            //reportAdapteHandler.adapter(context,exceptionModule);

            Log.i("weex js err", "js err end");

            WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(exception.getInstanceId());
            if (null != instance) {
                Map<String, Object> params;
                try {
                    params = JSON.parseObject(JSON.toJSONString(exception));
                } catch (Exception e) {
                    params = new HashMap<>();
                    params.put("bundleUrl", exception.getBundleUrl());
                    params.put("errorCode", exception.getErrCode());
                    params.put("exception", exception.getException());
                    params.put("extParams", exception.getExtParams());
                    params.put("function", exception.getFunction());
                    params.put("instanceId", exception.getInstanceId());
                    params.put("jsFrameworkVersion", exception.getJsFrameworkVersion());
                    params.put("weexVersion", exception.getWeexVersion());
                }
                instance.fireGlobalEventCallback("exception", params);
            }

        } catch (Exception e) {
            Log.e("weex js err", "build weex callback data err", e);
        }

    }

    /**
     * bundle url
     *
     * @param bundleUrl
     * @return
     */
    private String exceptionUrl(String bundleUrl) {
        if (bundleUrl.startsWith("https:")) {
            return bundleUrl.substring(8);
        } else if (bundleUrl.startsWith("http:")) {
            return bundleUrl.substring(7);
        }
        return bundleUrl;
    }

}
