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
package org.apache.weex.bridge;

import org.apache.weex.WXEnvironment;
import org.apache.weex.base.CalledByNative;
import org.apache.weex.utils.WXLogUtils;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

public class WXParams implements Serializable {

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
  private String cacheDir;
  private String useSingleProcess;
  private String crashFilePath;
  private String libJssPath;
  private String layoutDirection;

  private String libJscPath;
  private String libIcuPath;
  private String libLdPath;
  private String libJsbPath;

  private Map<String, String> options;

  @CalledByNative
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

  @CalledByNative
  public String getPlatform() {
    return platform;
  }

  public void setPlatform(String platform) {
    this.platform = platform;
  }

  public void setCacheDir(String cache) {
    this.cacheDir = cache;
  }

  @CalledByNative
  public String getCacheDir() {
    return this.cacheDir;
  }

  @CalledByNative
  public String getOsVersion() {
    return osVersion;
  }

  public void setOsVersion(String osVersion) {
    this.osVersion = osVersion;
  }

  @CalledByNative
  public String getAppVersion() {
    return appVersion;
  }

  public void setAppVersion(String appVersion) {
    this.appVersion = appVersion;
  }

  @CalledByNative
  public String getWeexVersion() {
    return weexVersion;
  }

  public void setWeexVersion(String weexVersion) {
    this.weexVersion = weexVersion;
  }

  @CalledByNative
  public String getDeviceModel() {
    return deviceModel;
  }

  public void setDeviceModel(String deviceModel) {
    this.deviceModel = deviceModel;
  }

  @CalledByNative
  public String getLayoutDirection() {return layoutDirection;}

  public void setLayoutDirection(String direction) { this.layoutDirection = direction; }

  @CalledByNative
  public String getAppName() {
    return appName;
  }

  public void setAppName(String appName) {
    this.appName = appName;
  }

  @CalledByNative
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

  @CalledByNative
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

  @CalledByNative
  public String getUseSingleProcess() {
    WXLogUtils.e("getUseSingleProcess is running " + useSingleProcess);
    return useSingleProcess;
  }

  public void setUseSingleProcess(String useSingleProcess) {
    this.useSingleProcess = useSingleProcess;
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

  public void setCrashFilePath(String crashFilePath) {
    WXLogUtils.e("WXParams","setCrashFilePath: " + crashFilePath);
    this.crashFilePath = crashFilePath;
  }

  @CalledByNative
  public String getCrashFilePath() {
    WXLogUtils.e("WXParams", "getCrashFilePath:" + crashFilePath);
    return this.crashFilePath;
  }

  @CalledByNative
  public String getLibJssPath() {
    WXLogUtils.e("getLibJssPath is running " + libJssPath);
    return libJssPath;
  }

  @CalledByNative
  public String getLibJsbPath() {
    WXLogUtils.e("getLibJsbPath is running " + libJsbPath);
    return libJsbPath;
  }

  public void setLibJsbPath(String libJsbPath) {
    this.libJsbPath = libJsbPath;
  }

  @CalledByNative
  public String getLibJscPath() {
    WXLogUtils.e("getLibJscPath is running " + libJscPath);
    return libJscPath;
  }
  public void setLibJscPath(String libJscPath) {
    this.libJscPath = libJscPath;
  }
  public void setLibJssPath(String libJssPath) {
    this.libJssPath = libJssPath;
  }
  @CalledByNative
  public String getLibIcuPath() {
    WXLogUtils.e("getLibIcuPath is running " + libIcuPath);
    return libIcuPath;
  }

  public void setLibIcuPath(String libIcuPath) {
    this.libIcuPath = libIcuPath;
  }

  @CalledByNative
  public String getLibLdPath() {
    WXLogUtils.e("getLibLdPath is running " + libLdPath);
    return libLdPath;
  }

  public void setLibLdPath(String libLdPath) {
    this.libLdPath = libLdPath;
  }

  @CalledByNative
  public String getUseRunTimeApi() {
    return String.valueOf(WXEnvironment.sUseRunTimeApi);
  }

  public Map<String, Object> toMap() {
    HashMap<String, Object> map  = new HashMap<>();
    map.put("appName", appName);
    map.put("appVersion", appVersion);
    map.put("cacheDir", cacheDir);
    map.put("deviceHeight", deviceHeight);
    map.put("deviceModel", deviceModel);
    map.put("deviceWidth", deviceWidth);
    map.put("layoutDirection", layoutDirection);
    map.put("libJssPath", libJssPath);
    map.put("logLevel", logLevel);
    map.put("needInitV8", needInitV8);
    map.put("osVersion", osVersion);
    map.put("platform", platform);
    map.put("useSingleProcess", useSingleProcess);
    map.put("shouldInfoCollect", shouldInfoCollect);
    map.put("weexVersion", weexVersion);
    map.put("crashFilePath", crashFilePath);
    map.put("libJscPath", libJscPath);
    map.put("libIcuPath", libIcuPath);
    map.put("libLdPath", libLdPath);
    map.put("options", options);
    map.put("useRunTimeApi",WXEnvironment.sUseRunTimeApi);
    map.put("__enable_native_promise__",!WXEnvironment.sUseRunTimeApi);
    return map;
  }
}
