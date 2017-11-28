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
package com.taobao.weex;

public class WXRenderErrorCode {

//  public static final String WX_CREATE_INSTANCE_ERROR = "wx_create_instance_error";
//
//  public static final String WX_NETWORK_ERROR = "wx_network_error";
//
//  public static final String WX_USER_INTERCEPT_ERROR = "wx_user_intercept_error";

  public enum DegradPassivityCode{

	/**
	 * degrade code.
	 */
	WX_DEGRAD_ERR("-1000", "degradeToH5|Weex DegradPassivity \n"),

	/**
	 * degrade for instance create failed, once this case occured,detail js stack and other specific
	 * cause need track into errmsg.
	 */
	WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED("-1001", "degradeToH5|createInstance fail|wx_create_instance_error"),

	/**
	 * degrade for network failed download js bundle.once this case occured,network requist response header
	 * and statuscode need track into errmsg.
	 */
	WX_DEGRAD_ERR_NETWORK_BUNDLE_DOWNLOAD_FAILED("-1002", "|wx_network_error|js bundle download failed"),

	/**
	 * degrade for network failed for bundlejs is unbroken , once this case occured,network requist response header
	 * and statuscode need track into errmsg.
	 */
	WX_DEGRAD_ERR_NETWORK_CHECK_CONTENT_LENGTH_FAILED("-1003", "degradeToH5|wx_network_error|js bundle content-length check failed"),

	/**
	 * degrade for Response header Content-Type is null or not "application/javascript".
	 * once this case occured,network requist response header and statuscode need track into errmsg.
	 */
	WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR("-1004", "degradeToH5|wx_user_intercept_error |Content-Type is not application/javascript, " +
			"Weex render template must be javascript, please check your request!"),

	/**
	 * degrade for other reason. such as white screen which block error for some unknown reason.
	 * once this case occured,detail msg need track.
	 */
	WX_DEGRAD_ERR_OTHER_CAUSE_DEGRADTOH5("-1005", "degradeToH5|for other reason|");

	private String degradCode;
	private String degradMsg;
	private String appendMsg = "";
	private String args;

	DegradPassivityCode(String degradCode, String degradMsg){
	  this.degradCode = degradCode;
	  this.degradMsg = degradMsg;
	}

	public String getDegradErrorCode() {
	  return degradCode;
	}

	public String getDegradErrorMsg() {
	  StringBuilder builder=new StringBuilder(degradMsg);
	  builder.append(appendMsg);
	  return builder.toString();
	}
  }
}
