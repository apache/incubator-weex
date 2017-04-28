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
package com.taobao.weex.bridge;

import java.util.Map;

public class WXParams {

  private String platform;
  private String osVersion;
  private String appVersion;
  private String weexVersion;
  private String deviceModel;
  private String appName;
  private String deviceWidth;
  private String deviceHeight;
  private String shouldInfoCollect;
  private String logLevel;
  private String needInitV8;

  private Map<String, String> options;

  public Object getOptions() {
    return options;
  }

  public void setOptions(Map<String, String> options) {
    this.options = options;
  }

  public String getShouldInfoCollect() {
    return shouldInfoCollect;
  }

  public void setShouldInfoCollect(String shouldInfoCollect) {
    this.shouldInfoCollect = shouldInfoCollect;
  }

  public String getPlatform() {
    return platform;
  }

  public void setPlatform(String platform) {
    this.platform = platform;
  }

  public String getOsVersion() {
    return osVersion;
  }

  public void setOsVersion(String osVersion) {
    this.osVersion = osVersion;
  }

  public String getAppVersion() {
    return appVersion;
  }

  public void setAppVersion(String appVersion) {
    this.appVersion = appVersion;
  }

  public String getWeexVersion() {
    return weexVersion;
  }

  public void setWeexVersion(String weexVersion) {
    this.weexVersion = weexVersion;
  }

  public String getDeviceModel() {
    return deviceModel;
  }

  public void setDeviceModel(String deviceModel) {
    this.deviceModel = deviceModel;
  }

  public String getAppName() {
    return appName;
  }

  public void setAppName(String appName) {
    this.appName = appName;
  }

  public String getDeviceWidth() {
    return deviceWidth;
  }


  /**
   * Device should not be set manually, instead it suppose to represent the width of device and
   * initialized automatically.
   * @param deviceWidth
   */
  @Deprecated
  public void setDeviceWidth(String deviceWidth) {
    this.deviceWidth = deviceWidth;
  }

  public String getDeviceHeight() {
    return deviceHeight;
  }

  public void setDeviceHeight(String deviceHeight) {
    this.deviceHeight = deviceHeight;
  }

  public String getLogLevel() {
    if(logLevel == null){
      return "";
    }
    return logLevel;
  }

  public void setLogLevel(String logLevel) {
    this.logLevel = logLevel;
  }

  public String getNeedInitV8() {
    if(needInitV8 ==null){
      return "";
    }
    return this.needInitV8;
  }

  public void setNeedInitV8(boolean need) {
    if (need) {
      this.needInitV8 = "1";
    } else {
      this.needInitV8 = "0";
    }
  }
}
