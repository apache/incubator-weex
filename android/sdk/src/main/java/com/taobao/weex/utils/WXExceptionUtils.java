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
package com.taobao.weex.utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

import android.support.annotation.Nullable;
import android.text.TextUtils;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXConfigAdapter;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXErrorCode.ErrorGroup;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.performance.WXAnalyzerDataTransfer;
import com.taobao.weex.performance.WXInstanceApm;

/**
 * Created on 2017/10/13.
 */

public class WXExceptionUtils {

    private static Set<String> sGlobalExceptionRecord = new CopyOnWriteArraySet<String>();

	/**
	 * degradeUrl for degrade case
	 */
	public static String degradeUrl = "BundleUrlDefaultDegradeUrl";

	private static boolean checkNeedReportCauseRepeat(String instanceId,WXErrorCode errCode,String exception){

	    if (TextUtils.isEmpty(exception)){
	        return true;
        }

        if (null != errCode && errCode.getErrorGroup() != ErrorGroup.JS){
	        return true;
        }

        if (TextUtils.isEmpty(instanceId)){
            instanceId = "instanceIdNull";
        }

        String targetException = exception.length() > 200 ?exception.substring(0,200) : exception;
        Set<String> recordExceptionHistory= null;

        WXSDKInstance instance =  WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
        if (null == instance){
            recordExceptionHistory = sGlobalExceptionRecord;
        }else {
            recordExceptionHistory =  instance.getApmForInstance().exceptionRecord;
        }

        if (null == recordExceptionHistory){
            return true;
        }

        if (recordExceptionHistory.contains(targetException)){
            return false;
        }
        recordExceptionHistory.add(targetException);
        return true;
    }




	/**
	 * commitCriticalExceptionRT eg:JsRuntime Exception or JsFramework Init Exception
	 * @param instanceId
	 * @param errCode
	 * @param function
	 * @param exception
	 * @param extParams
	 */
	public static void commitCriticalExceptionRT(@Nullable final String instanceId,
												 @Nullable final WXErrorCode errCode,
												 @Nullable final String function,
												 @Nullable final String exception,
												 @Nullable final Map<String,String> extParams ) {

        try {
            IWXConfigAdapter configAdapter = WXSDKManager.getInstance().getWxConfigAdapter();
            boolean doCheck = true;
            if (null != configAdapter){
                String value = configAdapter.getConfig("wxapm","check_repeat_report","true");
                doCheck = "true".equalsIgnoreCase(value);
            }
            boolean doReport =true;
            if (doCheck){
                doReport = checkNeedReportCauseRepeat(instanceId,errCode,exception);
            }
            if (!doReport){
                return;
            }
        }catch (Throwable e){
            e.printStackTrace();
        }


		commitCriticalExceptionWithDefaultUrl(
		    "BundleUrlDefault",
            instanceId,
            errCode,
            function,
            exception,
            extParams
        );
	}

    public static void commitCriticalExceptionWithDefaultUrl(
        @Nullable final String defaultUrl,
        @Nullable final String instanceId,
        @Nullable final WXErrorCode errCode,
        @Nullable final String function,
        @Nullable final String exception,
        @Nullable final Map<String,String> extParams
    ){
        IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
        WXSDKInstance instance = null;
        WXJSExceptionInfo exceptionCommit;
        String bundleUrlCommit = TextUtils.isEmpty(defaultUrl)?"BundleUrlDefault":defaultUrl;
        String instanceIdCommit = "InstanceIdDefalut";
        String exceptionMsgCommit = exception;
        Map<String, String> commitMap = extParams;
        if (null == commitMap){
            commitMap = new HashMap<>();
            commitMap.put("wxSdkInitStartTime", String.valueOf(WXEnvironment.sSDKInitStart));
            commitMap.put("wxSDKInitCostTime", String.valueOf(WXEnvironment.sSDKInitTime));
            commitMap.put("wxSDKCurExceptionTime", String.valueOf(System.currentTimeMillis()));
        }
        if (!TextUtils.isEmpty(instanceId)) {
            instanceIdCommit = instanceId;
            instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);

            if (null != instance) {
                bundleUrlCommit = instance.getApmForInstance().reportPageName;
                Object loadLength = instance.getApmForInstance().extInfo.get(WXInstanceApm.VALUE_BUNDLE_LOAD_LENGTH);
                String loadLengthStr = (loadLength instanceof Integer)?String.valueOf(loadLength):"unknownLength";
                commitMap.put(WXInstanceApm.VALUE_BUNDLE_LOAD_LENGTH,loadLengthStr);
                commitMap.put("templateInfo",instance.getTemplateInfo());
                if (TextUtils.isEmpty(bundleUrlCommit) || bundleUrlCommit.equals(WXPerformance.DEFAULT)) {
                    if (!TextUtils.equals(degradeUrl, "BundleUrlDefaultDegradeUrl")) {
                        bundleUrlCommit = degradeUrl;
                    } else
                        bundleUrlCommit = WXSDKInstance.requestUrl;
                }
                for (Map.Entry<String,String> entry: instance.getContainerInfo().entrySet()){
                    commitMap.put(entry.getKey(),entry.getValue());
                }
                commitMap.put("wxStageList",convertStageToStr(instance));
                String bundleTemplate = instance.getTemplate();
                if (null == bundleTemplate){
                    bundleTemplate = "has recycle by gc";
                }else {
                    int length = bundleTemplate.length();
                    bundleTemplate = bundleTemplate.substring(0,Math.min(length,300));
                }
                commitMap.put("wxTemplateOfBundle",bundleTemplate);

                Long pageStartTime = instance.getApmForInstance().stageMap.get(WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_START);
                if (null == pageStartTime){
                    pageStartTime = instance.getApmForInstance().stageMap.get(WXInstanceApm.KEY_PAGE_STAGES_RENDER_ORGIGIN);
                }
                if (null != pageStartTime){
                    commitMap.put("wxUseTime", String.valueOf(WXUtils.getFixUnixTime() - pageStartTime));
                }
            }
        } else {//instance is null for instance id is null
            if (commitMap.size() > 0) {
                bundleUrlCommit = TextUtils.isEmpty(commitMap.get("weexUrl")) ? commitMap.get("weexUrl")
                    : commitMap.get("bundleUrl");
            }
        }

        String illegalValue = commitMap.get("errorCode");
        if (null != illegalValue && illegalValue.length()>200){
            commitMap.remove("errorCode");
        }

        exceptionCommit = new WXJSExceptionInfo(instanceIdCommit, bundleUrlCommit, errCode, function, exceptionMsgCommit, commitMap);
        if (adapter != null) {
            adapter.onJSException(exceptionCommit);
        }

        WXAnalyzerDataTransfer.transferError(exceptionCommit, instanceId);
    }

    private static String convertStageToStr(WXSDKInstance instance) {
        if (null == instance || null == instance.getApmForInstance() || instance.getApmForInstance().stageMap.isEmpty()) {
            return "noStageRecord";
        }
        List<Entry<String, Long>> list = new ArrayList<>(instance.getApmForInstance().stageMap.entrySet());
        Collections.sort(list, new Comparator<Entry<String, Long>>() {
            @Override
            public int compare(Entry<String, Long> o1, Entry<String, Long> o2) {
                return (int)(o1.getValue() - o2.getValue());
            }
        });

        StringBuilder builder = new StringBuilder();
        for (Map.Entry<String, Long> entry : list) {
            builder.append(entry.getKey()).append(':').append(entry.getValue()).append("->");
        }
        return builder.toString();
    }

}