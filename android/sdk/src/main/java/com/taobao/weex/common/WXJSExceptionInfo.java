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
package com.taobao.weex.common;


import com.taobao.weex.WXEnvironment;

import java.util.Map;

public class WXJSExceptionInfo {

  /**
   * instance id
   */
  private String mInstanceId;
  /**
   * The URL where the exception occurred
   */
  private String mBundleUrl;
  /**
   * error code
   */
  private WXErrorCode mErrCode;
  /**
   * The function name of the exception
   */
  private String mFunction;
  /**
   * Exception details
   */
  private String mException;

  /**
   * Extended field
   */
  private Map<String,String> mExtParams;

  /**
   * weex sdk version
   */
  private String mWeexVersion = WXEnvironment.WXSDK_VERSION;

  /**
   * js framework verison
   */
  private String mJsFrameworkVersion = WXEnvironment.JS_LIB_SDK_VERSION;

  public WXJSExceptionInfo(String instanceId, String bundleUrl, WXErrorCode errCode, String function, String exception, Map<String,String> extParams) {
    this.mInstanceId = instanceId;
    this.mBundleUrl = bundleUrl;
    this.mErrCode = errCode;
    this.mFunction = function;
    this.mException = exception;
    this.mExtParams = extParams;
  }

  public String getInstanceId() {
    return mInstanceId;
  }

  public void setInstanceId(String instanceId) {
    mInstanceId = instanceId;
  }

  public String getBundleUrl() {
    return mBundleUrl;
  }

  public void setBundleUrl(String bundleUrl) {
    mBundleUrl = bundleUrl;
  }

  public WXErrorCode getErrCode() {
    return mErrCode;
  }

  public void setErrCode(WXErrorCode errCode) {
    mErrCode = errCode;
  }

  public String getFunction() {
    return mFunction;
  }

  public void setFunction(String function) {
    mFunction = function;
  }

  public String getException() {
    return mException;
  }

  public void setException(String exception) {
    mException = exception;
  }

  public Map<String, String> getExtParams() {
    return mExtParams;
  }

  public void setExtParams(Map<String, String> extParams) {
    mExtParams = extParams;
  }

  public String getWeexVersion() {
    return mWeexVersion;
  }


  public String getJsFrameworkVersion() {
    return mJsFrameworkVersion;
  }


  @Override
  public String toString() {
    StringBuffer buffer = new StringBuffer();
    buffer.append("WeexSDKVersion:").append(mWeexVersion)
            .append(" JSFrameworkVersion:").append(mJsFrameworkVersion)
            .append(" instanceId:").append(mInstanceId)
            .append(" bundleUrl:").append(mBundleUrl)
            .append(" errCode:").append(mErrCode.getErrorCode())
            .append(" function:").append(mFunction)
            .append(" exception:").append(mException)
            .append(" extParams:").append(mExtParams);
    return buffer.toString();
  }
}
