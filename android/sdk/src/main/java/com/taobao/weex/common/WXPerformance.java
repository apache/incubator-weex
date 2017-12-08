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

import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import com.taobao.weex.WXEnvironment;
import java.util.HashMap;
import java.util.Map;

public class WXPerformance {

  public static final String DEFAULT = "default";

  /**
   * No longer needed.
   */
  @Deprecated
  public String bizType = "weex";

  /**
   * Use {@link #pageName} instead.
   */
  @Deprecated
  public String templateUrl;

  @RestrictTo(Scope.LIBRARY_GROUP)
  public String cacheType="unknown";

  @RestrictTo(Scope.LIBRARY)
  public long renderTimeOrigin;

  public long fsRenderTime;
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
  public long firstScreenJSFExecuteTime;

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

  /**
   *for network tracker
   */

  /**
   * TODO These dimensions will be moved to elsewhere
   */
  @RestrictTo(Scope.LIBRARY)
  @Deprecated
  public String wxDims[] = new String[5];

  /**
   * TODO These dimensions will be moved to elsewhere
   */
  @RestrictTo(Scope.LIBRARY)
  @Deprecated
  public long measureTimes[] = new long[5];

  public WXPerformance(){
    mErrMsgBuilder=new StringBuilder();
  }

  public Map<String,Double> getMeasureMap(){
    Map<String,Double> quotas = new HashMap<>();
    quotas.put("JSLibSize", JSLibSize);
    quotas.put("JSLibInitTime", (double)JSLibInitTime);
    quotas.put("SDKInitTime",(double)WXEnvironment.sSDKInitTime);
    quotas.put("SDKInitInvokeTime",(double)WXEnvironment.sSDKInitInvokeTime);
    quotas.put("SDKInitExecuteTime",(double)WXEnvironment.sSDKInitExecuteTime);
    quotas.put("JSTemplateSize", JSTemplateSize);
    quotas.put("pureNetworkTime",(double)pureNetworkTime);
    quotas.put("networkTime", (double)networkTime);

    double fsRenderTime;
    if (this.fsRenderTime == 0) {
      fsRenderTime = totalTime;
    } else {
      fsRenderTime = this.fsRenderTime - renderTimeOrigin;
    }

    quotas.put("fsRenderTime", fsRenderTime);
    quotas.put("screenRenderTime", (double)screenRenderTime);
    quotas.put("communicateTime", (double)communicateTime);
    quotas.put("communicateTotalTime", totalTime);
    quotas.put("localReadTime", localReadTime);
    quotas.put("templateLoadTime", (double)templateLoadTime);
    quotas.put("firstScreenJSFExecuteTime",(double) firstScreenJSFExecuteTime);
    quotas.put("componentCount",(double)componentCount);
    quotas.put("actualNetworkTime",(double)actualNetworkTime);
    quotas.put("syncTaskTime",(double)syncTaskTime);
    quotas.put("packageSpendTime",(double)packageSpendTime);
    quotas.put("maxDeepViewLayer", (double) maxDeepViewLayer);
    quotas.put("useScroller", (double) useScroller);

    /**
     * TODO These attribute will be moved to elsewhere
     * Extra Value for 3rd developers.
     */
    quotas.put("measureTime1", (double) measureTimes[0]);
    quotas.put("measureTime2", (double) measureTimes[1]);
    quotas.put("measureTime3", (double) measureTimes[2]);
    quotas.put("measureTime4", (double) measureTimes[3]);
    quotas.put("measureTime5", (double) measureTimes[4]);
    return quotas;
  }

  public Map<String,String> getDimensionMap(){
    Map<String,String> quotas = new HashMap<>();
    quotas.put("JSLibVersion", JSLibVersion);
    quotas.put("WXSDKVersion", WXSDKVersion);
    quotas.put("pageName", pageName);
    quotas.put("requestType",requestType);
    quotas.put("connectionType",connectionType);

    /**
     * TODO These attribute will be moved to elsewhere
     * Extra Dimension for 3rd developers.
     */
	  quotas.put("wxdim1", wxDims[0]);
	  quotas.put("wxdim2", wxDims[1]);
	  quotas.put("wxdim3", wxDims[2]);
	  quotas.put("wxdim4", wxDims[3]);
	  quotas.put("wxdim5", wxDims[4]);

    /**
     * TODO the following attribute is no longer needed and will be deleted soon.
     */
    quotas.put("bizType", bizType);
    quotas.put("templateUrl", templateUrl);
	  return quotas;
  }

  public static String[] getDimensions(){
    return new String[]{"bizType","templateUrl","pageName","JSLibVersion","WXSDKVersion","connectionType","requestType"
    ,"wxdim1","wxdim2","wxdim3","wxdim4","wxdim5"};
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
        "firstScreenJSFExecuteTime",
        "maxDeepViewLayer",
        "useScroller",
		"measureTime1",
		"measureTime2",
		"measureTime3",
		"measureTime4",
		"measureTime5"
	};
  }

  @Override
  public String toString() {
    if (WXEnvironment.isApkDebugable()) {
      return "bizType:" + bizType + ",pageName:" + pageName + ",templateLoadTime" + templateLoadTime
             + ",localReadTime:" + localReadTime + ",JSLibInitTime:" + JSLibInitTime
             + ",JSLibSize:" + JSLibSize + ",templateUrl" + templateUrl
             + ",JSTemplateSize:" + JSTemplateSize + ",communicateTime:" + communicateTime
             + ",screenRenderTime:" + screenRenderTime
             + ",firstScreenJSFExecuteTime:" + firstScreenJSFExecuteTime
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
              + " firstScreenJSFExecuteTime:" + firstScreenJSFExecuteTime
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
}
