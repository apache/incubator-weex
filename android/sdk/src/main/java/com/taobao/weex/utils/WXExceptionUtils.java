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

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.common.WXJSExceptionInfo;

import java.util.Map;

/**
 * Created on 2017/10/13.
 */

public class WXExceptionUtils {
  /**
   * commitCriticalExceptionRT eg:JsRuntime Exception or JsFramework Init Exception
   * @param instanceId
   * @param errCode
   * @param function
   * @param exception
   * @param extParams
   */
  public static void commitCriticalExceptionRT(@Nullable String instanceId, @Nullable String errCode, @Nullable String function, @Nullable String exception, @Nullable Map<String,String> extParams ){
	IWXJSExceptionAdapter adapter = WXSDKManager.getInstance().getIWXJSExceptionAdapter();
	WXSDKInstance instance ;
	WXJSExceptionInfo exceptionCommit  = null ;
	String bundleULR = "Weex_SDK_Default_BundleUrl";

	if(!TextUtils.isEmpty(instanceId)){
	  instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
	  if(instance.getContext() != null && instance.getBundleUrl() != null){
		bundleULR = instance.getBundleUrl();
		if(adapter != null){
		  exceptionCommit = new WXJSExceptionInfo(instanceId, bundleULR, errCode, function, exception, extParams);
		  adapter.onJSException(exceptionCommit);
		}
	  }
	} else {
	  WXLogUtils.e("Weex instance is Null");
	  if(adapter != null ){
		exceptionCommit = new WXJSExceptionInfo("instanceIdIsNull", bundleULR, errCode, function, exception, extParams);
		adapter.onJSException(exceptionCommit);
	  }
	}
	WXLogUtils.e(exceptionCommit.toString());
  }
}