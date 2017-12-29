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

import java.util.HashMap;
import java.util.Map;

public class WXPerformance {

  public static final String DEFAULT = "default";

  /**
   * Business unit, mandatory. If no business unit can be provided, set the field as default
   */
  public String bizType = "weex";

  /**
   * URL used for rendering view, optional
   */
  public String templateUrl;

  /**
   * Time spent for reading, time unit is ms.
   */
  public double localReadTime;

  /**
   * Name of the page
   */
  public String pageName = DEFAULT;

  /**
   * Size of JavaScript framework, the unit is KB
   */
  public double JSLibSize;

  /**
   * Time of initial JavaScript library
   */
  public long JSLibInitTime;

  /**
   * Size of JavaScript template
   */
  public double JSTemplateSize;

  public long templateLoadTime;

  /**
   * Time used for
   * {@link com.taobao.weex.bridge.WXBridgeManager#createInstance(String, String, Map, String)}
   */
  public long communicateTime;

  /**
   * Time spent when rendering first screen
   */
  public long screenRenderTime;

  /**
   * Call native Time spent when rendering first screen
   */
  public long callNativeTime;

  /**
   * Create Instance Time spent when rendering first screen
   */
  public long firstScreenJSFAndWeexCoreExecuteTime;

  /**
   * Call native Time spent when rendering first screen
   */
  public long batchTime;

  /**
   * Call native Time spent when rendering first screen
   */
  public long parseJsonTime;

  /**
   *  UpdateDomObj Time spent when rendering first screen
   */
  public long updateDomObjTime;

  /**
   *  ApplyUpdate Time spent when rendering first screen
   */
  public long applyUpdateTime;


  /**
   *  CssLayout Time spent when rendering first screen
   */
  public long cssLayoutTime;

  /**
   * Time spent, the unit is micro second
   */
  public double totalTime;

  /**
   * load bundle js time, unite ms
   */
  public long networkTime;

  /**
   * pure network time;
   */
  public long pureNetworkTime;

  public long actualNetworkTime;
  public long packageSpendTime;
  public long syncTaskTime;
  /**
   * view hierarchy
   */
  public int maxDeepViewLayer;
  /**
   * 1:true
   * 0:false
   */
  public int useScroller=0;

  /**
   * component Count
   */
  public long componentCount;

  /**
   * Version of JavaScript libraray
   */
  public String JSLibVersion = WXEnvironment.JS_LIB_SDK_VERSION;

  /**
   * Version of Weex SDK
   */
  public String WXSDKVersion = WXEnvironment.WXSDK_VERSION;

  /**
   * The detail message of render failure
   */
  public String renderFailedDetail;

  /**
   * Error code
   */
  public String errCode;

  /**
   * Error message
   */
  @Deprecated
  public String errMsg;
  private StringBuilder mErrMsgBuilder;

  public String args="";

  public String connectionType;
  public String requestType;


  public int mCallCreateBodySumTime = 0;
  public int mCallAddElementByWeexCoreSumTime = 0;

  public int mCallLayoutByWeexCoreSumTime = 0;
  public int mCallLayoutUpdateDemissionSumTime = 0;
  public int mCallLayoutApplyLayoutAndEventSumTime = 0;
  public int mCallLayoutBindDataSumTime = 0;

  public int mActionTotalSumTime = 0;
  public int mActionLayoutCount = 0;
  public int mActionLayoutSumTime = 0;

  public int mActionAddElementCount = 0;
  public int mActionAddElementSumTime = 0;

  public int mActionCreateBodyCount = 0;
  public int mActionCreateBodySumTime = 0;

  public int mActionOtherCount = 0;
  public int mActionOtherSumTime = 0;


  public WXPerformance(){
    mErrMsgBuilder=new StringBuilder();
  }

  public Map<String,Double> getMeasureMap(){
    Map<String,Double> quotas = new HashMap<>();
    quotas.put("JSTemplateSize", JSTemplateSize);
    quotas.put("JSLibSize", JSLibSize);
    quotas.put("communicateTime", (double)communicateTime);
    quotas.put("screenRenderTime", (double)screenRenderTime);
    quotas.put("totalTime", totalTime);
    quotas.put("localReadTime", localReadTime);
    quotas.put("JSLibInitTime", (double)JSLibInitTime);
    quotas.put("networkTime", (double)networkTime);
    quotas.put("templateLoadTime", (double)templateLoadTime);
    quotas.put("SDKInitInvokeTime",(double)WXEnvironment.sSDKInitInvokeTime);
    quotas.put("SDKInitExecuteTime",(double)WXEnvironment.sSDKInitExecuteTime);
    quotas.put("firstScreenJSFAndWeexCoreExecuteTime",(double) firstScreenJSFAndWeexCoreExecuteTime);
    quotas.put("componentCount",(double)componentCount);
    quotas.put("actualNetworkTime",(double)actualNetworkTime);
    quotas.put("pureNetworkTime",(double)pureNetworkTime);
    quotas.put("syncTaskTime",(double)syncTaskTime);
    quotas.put("packageSpendTime",(double)packageSpendTime);
    quotas.put("SDKInitTime",(double)WXEnvironment.sSDKInitTime);
    quotas.put("maxDeepViewLayer", (double) maxDeepViewLayer);
    quotas.put("useScroller", (double) useScroller);
    return quotas;
  }

  public Map<String,String> getDimensionMap(){
    Map<String,String> quotas = new HashMap<>();
    quotas.put("bizType", bizType);
    quotas.put("templateUrl", templateUrl);
    quotas.put("pageName", pageName);
    quotas.put("JSLibVersion", JSLibVersion);
    quotas.put("WXSDKVersion", WXSDKVersion);
    quotas.put("connectionType",connectionType);
    quotas.put("requestType",requestType);

    return quotas;
  }

  public static String[] getDimensions(){
    return new String[]{"bizType","templateUrl","pageName","JSLibVersion","WXSDKVersion","connectionType","requestType"};
  }

  public static String[] getMeasures(){
    return new String[]{"JSTemplateSize",
        "JSLibSize",
        "communicateTime",
        "screenRenderTime",
        "totalTime",
        "localReadTime",
        "JSLibInitTime",
        "networkTime",
        "componentCount",
        "templateLoadTime",
        "SDKInitInvokeTime",
        "SDKInitExecuteTime",
        "SDKInitTime",
        "packageSpendTime",
        "syncTaskTime",
        "pureNetworkTime",
        "actualNetworkTime",
        "firstScreenJSFAndWeexCoreExecuteTime",
        "maxDeepViewLayer",
        "useScroller"};
  }

  @Override
  public String toString() {
    if (WXEnvironment.isApkDebugable()) {
      return "bizType:" + bizType + ",pageName:" + pageName + ",templateLoadTime" + templateLoadTime
             + ",localReadTime:" + localReadTime + ",JSLibInitTime:" + JSLibInitTime
             + ",JSLibSize:" + JSLibSize + ",templateUrl" + templateUrl
             + ",JSTemplateSize:" + JSTemplateSize + ",communicateTime:" + communicateTime
             + ",screenRenderTime:" + screenRenderTime
             + ",firstScreenJSFAndWeexCoreExecuteTime:" + firstScreenJSFAndWeexCoreExecuteTime
             + ",componentCount:" + componentCount
             + ",syncTaskTime:" + syncTaskTime
             + ",pureNetworkTime:" + pureNetworkTime
             + ",networkTime:" + networkTime
             + ",actualNetworkTime:" + actualNetworkTime
             + ",packageSpendTime:" + packageSpendTime
             + ",connectionType:" + connectionType
             + ",requestType:" + requestType
             + ",initInvokeTime:"+WXEnvironment.sSDKInitInvokeTime+",initExecuteTime:"+WXEnvironment.sSDKInitExecuteTime
             + ",SDKInitTime:"+ WXEnvironment.sSDKInitTime
             + ",totalTime:" + totalTime + ",JSLibVersion:" + JSLibVersion + ",WXSDKVersion:" + WXSDKVersion
             + ",errCode:" + errCode + ",renderFailedDetail:" + renderFailedDetail
             + ",arg:" + args
             + ",errMsg:" + getErrMsg();
    }
    return super.toString();
  }
  public String getPerfData(){
      return  "networkTime:" + networkTime
              + " actualNetworkTime:" + actualNetworkTime
              + " connectionType:" + connectionType
              + " requestType:" + requestType
              + " firstScreenRenderTime:" + screenRenderTime
              + " firstScreenJSFAndWeexCoreExecuteTime:" + firstScreenJSFAndWeexCoreExecuteTime
              + " componentCount:" + componentCount
              + " JSTemplateSize:" + JSTemplateSize
              + " SDKInitTime:" + WXEnvironment.sSDKInitTime
              + " totalTime:" + totalTime
              + " JSLibVersion:" + JSLibVersion
              + " WXSDKVersion:" + WXSDKVersion
              + " pageName:" + pageName
              + " useScroller:" + useScroller;

  }

  public String getErrMsg() {
    return mErrMsgBuilder.toString();
  }

  public void appendErrMsg(CharSequence msg) {
    mErrMsgBuilder.append(msg);
  }

  public int renderSumTime() {
    return mActionAddElementSumTime + mActionCreateBodySumTime + mActionLayoutSumTime + mActionOtherSumTime;
  }

  public int renderSumCount() {
    return mActionAddElementCount + mActionCreateBodyCount + mActionLayoutCount + mActionOtherCount;
  }
}
