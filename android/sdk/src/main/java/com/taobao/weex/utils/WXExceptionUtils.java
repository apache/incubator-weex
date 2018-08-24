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

import java.util.HashMap;
import java.util.Map;

import android.support.annotation.Nullable;
import android.text.TextUtils;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXErrorCode.ErrorType;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.performance.WXAnalyzerDataTransfer;
import com.taobao.weex.performance.WXInstanceExceptionRecord;

/**
 * Created on 2017/10/13.
 */

public class WXExceptionUtils {

	/**
	 * degradeUrl for degrade case
	 */
	public static String degradeUrl = "BundleUrlDefaultDegradeUrl";


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
		IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
		WXSDKInstance instance = null;
		WXJSExceptionInfo exceptionCommit;
		String bundleUrlCommit = "BundleUrlDefault";
		String instanceIdCommit = "InstanceIdDefalut";
		String exceptionMsgCommit = exception;
		Map<String, String> commitMap = extParams;
		if (null == commitMap){
			commitMap = new HashMap<>();
		}
		if (!TextUtils.isEmpty(instanceId)) {
			instanceIdCommit = instanceId;
			instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);

			if (null != instance) {
				bundleUrlCommit = instance.getBundleUrl();
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
				commitMap.put(WXInstanceExceptionRecord.KEY_EXP_STAGE_LIST,instance.getExceptionRecorder().convertStageToStr());
				String bundleTemplate = instance.getTemplate();
				commitMap.put("wxTemplateOfBundle",null == bundleTemplate ?"has recycle by gc":bundleTemplate);
				commitMap.put("wxPageName",instance.getWXPerformance().pageName);
			}
		} else {//instance is null for instance id is null
			if (!TextUtils.isEmpty(WXSDKInstance.requestUrl)) {
				bundleUrlCommit = WXSDKInstance.requestUrl;
			}
			if (commitMap.size() > 0) {
				bundleUrlCommit = TextUtils.isEmpty(commitMap.get("weexUrl")) ? commitMap.get("weexUrl")
								: commitMap.get("bundleUrl");
			}
		}

		exceptionCommit = new WXJSExceptionInfo(instanceIdCommit, bundleUrlCommit, errCode, function, exceptionMsgCommit, commitMap);
		if (adapter != null) {
			adapter.onJSException(exceptionCommit);
		}

		if (null != instance && exceptionCommit.getErrCode().getErrorType() != ErrorType.RENDER_ERROR){
			instance.getExceptionRecorder().recordErrorMsg(exceptionCommit);
		}

		WXAnalyzerDataTransfer.transferError(exceptionCommit, instanceId);

	}
}