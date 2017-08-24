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

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.telephony.TelephonyManager;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.common.WXConfig;
import com.taobao.weex.utils.LogLevel;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXSoInstallMgrSdk;
import com.taobao.weex.utils.WXUtils;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

public class WXEnvironment {

  public static final String OS = "android";
  public static final String SYS_VERSION = android.os.Build.VERSION.RELEASE;
  public static final String SYS_MODEL = android.os.Build.MODEL;
  public static final String ENVIRONMENT = "environment";
  public static final String WEEX_CURRENT_KEY = "wx_current_url";
  /*********************
   * Global config
   ***************************/

  public static String JS_LIB_SDK_VERSION = BuildConfig.buildJavascriptFrameworkVersion;

  public static String WXSDK_VERSION = BuildConfig.buildVersion;
  public static Application sApplication;
  public static final String DEV_Id = getDevId();
  @Deprecated
  public static int sDefaultWidth = 750;
  public volatile static boolean JsFrameworkInit = false;

  public static final String SETTING_EXCLUDE_X86SUPPORT = "env_exclude_x86";

  public static boolean SETTING_FORCE_VERTICAL_SCREEN = false;
  /**
   * Debug model
   */
  public static boolean sDebugMode = false;
  public static String sDebugWsUrl = "";
  public static boolean sDebugServerConnectable = false;
  public static boolean sRemoteDebugMode = false;
  public static String sRemoteDebugProxyUrl = "";
  public static long sJSLibInitTime = 0;

  public static long sSDKInitStart = 0;// init start timestamp
  public static long sSDKInitInvokeTime = 0;//time cost to invoke init method
  public static long sSDKInitExecuteTime = 0;//time cost to execute init job
  /** from init to sdk-ready **/
  public static long sSDKInitTime =0;

  public static LogLevel sLogLevel = LogLevel.DEBUG;
  private static boolean isApkDebug = true;
  public static boolean isPerf = false;

  public static boolean sShow3DLayer=true;

  private static Map<String, String> options = new HashMap<>();
  static {
    options.put(WXConfig.os, OS);
    options.put(WXConfig.osName, OS);
  }

  /**
   * dynamic
   */
  public static boolean sDynamicMode = false;
  public static String sDynamicUrl = "";

  /**
   * Fetch system information.
   * @return map contains system information.
   */
  public static Map<String, String> getConfig() {
    Map<String, String> configs = new HashMap<>();
    configs.put(WXConfig.os, OS);
    configs.put(WXConfig.appVersion, getAppVersionName());
    configs.put(WXConfig.devId, DEV_Id);
    configs.put(WXConfig.sysVersion, SYS_VERSION);
    configs.put(WXConfig.sysModel, SYS_MODEL);
    configs.put(WXConfig.weexVersion, String.valueOf(WXSDK_VERSION));
    configs.put(WXConfig.logLevel,sLogLevel.getName());
    try {
      options.put(WXConfig.scale, Float.toString(sApplication.getResources().getDisplayMetrics().density));
    }catch (NullPointerException e){
      //There is little chance of NullPointerException as sApplication may be null.
      WXLogUtils.e("WXEnvironment scale Exception: ", e);
    }
    configs.putAll(options);
    if(configs!=null&&configs.get(WXConfig.appName)==null && sApplication!=null){
       configs.put(WXConfig.appName, sApplication.getPackageName());
    }
    return configs;
  }

  /**
   * Get the version of the current app.
   */
  private static String getAppVersionName() {
    String versionName = "";
    PackageManager manager;
    PackageInfo info = null;
    try {
      manager = sApplication.getPackageManager();
      info = manager.getPackageInfo(sApplication.getPackageName(), 0);
      versionName = info.versionName;
    } catch (Exception e) {
      WXLogUtils.e("WXEnvironment getAppVersionName Exception: ", e);
    }
    return versionName;
  }

  public static Map<String, String> getCustomOptions() {
    return options;
  }

  public static void addCustomOptions(String key, String value) {
    options.put(key, value);
  }

  @Deprecated
  /**
   * Use {@link #isHardwareSupport()} if you want to see whether current hardware support Weex.
   */
  public static boolean isSupport() {
    boolean isInitialized = WXSDKEngine.isInitialized();
    if(WXEnvironment.isApkDebugable()){
      WXLogUtils.d("WXSDKEngine.isInitialized():" + isInitialized);
    }
    return isHardwareSupport() && isInitialized;
  }

  /**
   * Tell whether Weex can run on current hardware.
   * @return true if weex can run on current hardware, otherwise false.
   * Weex has removed the restrictions on the tablet, please use {@link #isCPUSupport()}
   */
  @Deprecated
  public static boolean isHardwareSupport() {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("isTableDevice:" + WXUtils.isTabletDevice());
    }
    return isCPUSupport() && !WXUtils.isTabletDevice();
  }

  /**
   * Determine whether Weex supports the current CPU architecture
   * @return true when support
   */
  public static boolean isCPUSupport(){
    boolean excludeX86 = "true".equals(options.get(SETTING_EXCLUDE_X86SUPPORT));
    boolean isX86AndExcluded = WXSoInstallMgrSdk.isX86() && excludeX86;
    boolean isCPUSupport = WXSoInstallMgrSdk.isCPUSupport() && !isX86AndExcluded;
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("WXEnvironment.sSupport:" + isCPUSupport
                   + "isX86AndExclueded: "+ isX86AndExcluded);
    }
    return isCPUSupport;
  }

  public static boolean isApkDebugable() {
    if (sApplication == null) {
      return false;
    }

    if (isPerf) {
      return false;
    }

    if (!isApkDebug) {
      return false;
    }
    try {
      ApplicationInfo info = sApplication.getApplicationInfo();
      isApkDebug = (info.flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
      return isApkDebug;
    } catch (Exception e) {
      /**
       * Don't call WXLogUtils.e here,will cause stackoverflow
       */
      e.printStackTrace();
    }
    return false;
  }

  public static boolean isPerf() {
    return isPerf;
  }

  private static String getDevId() {
    return sApplication == null ? "" : ((TelephonyManager) sApplication
        .getSystemService(Context.TELEPHONY_SERVICE)).getDeviceId();
  }

  public static Application getApplication() {
    return sApplication;
  }

  public void initMetrics() {
    if (sApplication == null) {
      return;
    }
  }

  public static String getDiskCacheDir(Context context) {
    if (context == null) {
      return null;
    }
    String cachePath = null;
    try {
      if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())
              || !Environment.isExternalStorageRemovable()) {
        cachePath = context.getExternalCacheDir().getPath();
      } else {
        cachePath = context.getCacheDir().getPath();
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
    return cachePath;
  }

  public static String getFilesDir(Context context) {
    if (context == null) {
      return "";
    }
    File filesDir = context.getFilesDir();
    String path = "";
    if (filesDir != null) {
      path = filesDir.getPath();
    } else {
      path = WXEnvironment.getApplication().getApplicationInfo().dataDir;
      path += File.separator;
      path += "files";
    }

    return path;
  }

}
