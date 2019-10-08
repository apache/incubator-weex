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
package org.apache.weex.common;

import android.support.annotation.RestrictTo;

import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.utils.WXUtils;
import org.apache.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import org.apache.weex.bridge.WXBridgeManager;

@Deprecated
public class WXPerformance {

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public enum Dimension {
    JSLibVersion,
    WXSDKVersion,
    pageName,
    spm,
    scheme,
    cacheType,
    requestType,
    networkType,
    connectionType,
    zcacheInfo,
    wxContainerName,
    wxInstanceType,
    wxParentPage,
    wxdim1,
    wxdim2,
    wxdim3,
    wxdim4,
    wxdim5,
    bizType,
    templateUrl,
    useScroller
  }

  public enum Measure {
    /**
     * range : [min,max)
     */
    JSLibSize(0D, Double.MAX_VALUE),
    //normal 300ms.. first install apk 30s ?
    JSLibInitTime(0D, 80000D),
    SDKInitTime(0D, 120000D),
    SDKInitInvokeTime(0D, 5000D),
    SDKInitExecuteTime(0D, 5000D),
    JSTemplateSize(0D, 5000D),
    pureNetworkTime(0D, 15000D),
    networkTime(0D, 15000D),
    fsCreateInstanceTime(0D, 3000D),
    fsCallJsTotalTime(0D, 5000D),
    fsCallJsTotalNum(0D, Double.MAX_VALUE),
    fsCallNativeTotalTime(0D, 5000D),
    fsCallNativeTotalNum(0D, Double.MAX_VALUE),
    fsCallEventTotalNum(0D, Double.MAX_VALUE),
    fsComponentCount(0D,100000D),
    fsComponentCreateTime(0D,Double.MAX_VALUE),
    fsRenderTime(0D, 5000D),
    fsRequestNum(0D, 100D),
    callCreateFinishTime(0D, 10000D),
    cellExceedNum(0D, Double.MAX_VALUE),
    communicateTotalTime(0D, 5000D),
    maxDeepViewLayer(0D, Double.MAX_VALUE),
    maxDeepVDomLayer(0D, Double.MAX_VALUE),
    componentCount(0D, 1000000),
    componentCreateTime(0D,Double.MAX_VALUE),
    avgFps(0D, 61D),
    timerCount(0D, Double.MAX_VALUE),

    MaxImproveMemory(0D, Double.MAX_VALUE),
    BackImproveMemory(0D, Double.MAX_VALUE),
    PushImproveMemory(0D, Double.MAX_VALUE),
    measureTime1(0D, Double.MAX_VALUE),
    measureTime2(0D, Double.MAX_VALUE),
    measureTime3(0D, Double.MAX_VALUE),
    measureTime4(0D, Double.MAX_VALUE),
    measureTime5(0D, Double.MAX_VALUE),

    callBridgeTime(0D, Double.MAX_VALUE),
    cssLayoutTime(0D, Double.MAX_VALUE),
    parseJsonTime(0D, Double.MAX_VALUE),

    communicateTime(0D, 5000D),
    screenRenderTime(0D, 5000D),
    totalTime(0D, 5000D),
    localReadTime(0D, 5000D),
    templateLoadTime(0D, 5000D),
    packageSpendTime(0D, 5000D),
    syncTaskTime(0D, 5000D),
    actualNetworkTime(0D, 5000D),
    firstScreenJSFExecuteTime(0D, 5000D),
    //..

    fluency(0D, 101D),
    imgSizeCount(0D, 2000D),
    interactionTime(0D,10000D),
    interactionViewAddCount(0D, Double.MAX_VALUE),
    interactionViewAddLimitCount(0D, Double.MAX_VALUE),
    newFsRenderTime(0D, 10000D);

    private double mMinRange, mMaxRange;

    Measure(double min, double max) {
      this.mMinRange = min;
      this.mMaxRange = max;
    }

    public double getMinRange() {
      return mMinRange;
    }

    public double getMaxRange() {
      return mMaxRange;
    }
  }

  public static final String DEFAULT = "default";

  @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
  public static final String CACHE_TYPE = "cacheType";

  public static final int VIEW_LIMIT_HEIGHT = WXViewUtils.getScreenHeight() /3*2;
  public static final int VIEW_LIMIT_WIDTH = WXViewUtils.getScreenWidth() /3*2;
  public static boolean TRACE_DATA = WXEnvironment.isApkDebugable();

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

  @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
  public String cacheType = "none";

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public long renderTimeOrigin;

  public long renderUnixTimeOrigin;

  public long fsRenderTime;

  public long callCreateFinishTime;

  public long interactionTime;

  public long interactionRealUnixTime;

  public int interactionViewAddCount;

  public int interactionViewAddLimitCount;

  public long newFsRenderTime;

  public int localInteractionViewAddCount;

  /**
   * Time used for
   * {@link WXBridgeManager#createInstance(String, String, Map, String)}
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public long callCreateInstanceTime;


  public long fsCallJsTotalTime;

  public int fsCallJsTotalNum;

  public long fsCallNativeTotalTime;

  public int fsCallNativeTotalNum;

  public int fsRequestNum;

  public int fsComponentCount;

  public int fsComponentCreateTime;

  public int cellExceedNum;

  public int timerInvokeCount;

  public int fsCallEventTotalNum;

  public long avgFPS;
  public double fluency = 100D;

  public long backImproveMemory;


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
   * Use {@link #callCreateInstanceTime} instead.
   */
  @Deprecated
  public long communicateTime;

  /**
   * Time spent when rendering first screen
   */
  public long screenRenderTime;

  /**
   * Call native Time spent when rendering first screen
   */
  public long callBridgeTime;

  /**
   * Create Instance Time spent when rendering first screen
   */
  public long firstScreenJSFExecuteTime;

  /**
   * Call native Time spent when rendering first screen
   */
  public long parseJsonTime;

  /**
   * CssLayout Time spent when rendering first screen
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

  public int maxDeepVDomLayer;

  public double wrongImgSizeCount;

  /**
   * 1:true
   * 0:false
   */
  public int useScroller = 0;

  /**
   * component Count
   */
  public long componentCount;

  public long componentCreateTime;
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
  public String requestType="other";

  public String zCacheInfo;

  /**
   *for network tracker
   */

  /**
   * TODO These dimensions will be moved to elsewhere
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Deprecated
  public String wxDims[] = new String[5];

  /**
   * TODO These dimensions will be moved to elsewhere
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Deprecated
  public long measureTimes[] = new long[5];


  /**
   * RenderAction
   */
  public int mActionAddElementCount = 0;
  public int mActionAddElementSumTime = 0;

  private String mInstanceId;

  public WXPerformance(String instanceId){
    mErrMsgBuilder=new StringBuilder();
    mInstanceId = instanceId;
  }

  public Map<String, Double> getMeasureMap() {
    double fsRenderTime;
    if (this.fsRenderTime != 0) {
      fsRenderTime = this.fsRenderTime - renderTimeOrigin;
    } else {
      if (totalTime != 0) {
        fsRenderTime = totalTime;
      } else {
        fsRenderTime = -1;
      }
    }
    Map<String, Double> quotas = new HashMap<>();
    quotas.put(Measure.JSLibSize.toString(), JSLibSize);
    quotas.put(Measure.JSLibInitTime.toString(), (double) JSLibInitTime);
    quotas.put(Measure.SDKInitTime.toString(), (double) WXEnvironment.sSDKInitTime);
    quotas.put(Measure.SDKInitInvokeTime.toString(), (double) WXEnvironment.sSDKInitInvokeTime);
    quotas.put(Measure.SDKInitExecuteTime.toString(), (double) WXEnvironment.sSDKInitExecuteTime);
    quotas.put(Measure.JSTemplateSize.toString(), JSTemplateSize);
    quotas.put(Measure.pureNetworkTime.toString(), (double) pureNetworkTime);
    quotas.put(Measure.networkTime.toString(), (double) networkTime);
    quotas.put(Measure.fsCreateInstanceTime.toString(), (double) callCreateInstanceTime);
    quotas.put(Measure.fsCallJsTotalTime.toString(), (double) fsCallJsTotalTime);
    quotas.put(Measure.fsCallJsTotalNum.toString(), (double) fsCallJsTotalNum);
    quotas.put(Measure.fsCallNativeTotalTime.toString(), (double) fsCallNativeTotalTime);
    quotas.put(Measure.fsCallNativeTotalNum.toString(), (double) fsCallNativeTotalNum);
    quotas.put(Measure.fsComponentCount.toString(),(double)fsComponentCount);
    quotas.put(Measure.fsComponentCreateTime.toString(),(double)fsComponentCreateTime);
    quotas.put(Measure.fsRenderTime.toString(), fsRenderTime);
    quotas.put(Measure.fsRequestNum.toString(), (double) fsRequestNum);
    quotas.put(Measure.communicateTotalTime.toString(), totalTime);
    quotas.put(Measure.maxDeepViewLayer.toString(), (double) maxDeepViewLayer);
    quotas.put(Measure.maxDeepVDomLayer.toString(), (double) maxDeepVDomLayer);
    quotas.put(Measure.componentCount.toString(), (double) componentCount);
    quotas.put(Measure.componentCreateTime.toString(),(double)componentCreateTime);
    quotas.put(Measure.cellExceedNum.toString(), (double) cellExceedNum);
    quotas.put(Measure.timerCount.toString(), (double) timerInvokeCount);
    quotas.put(Measure.avgFps.toString(), (double) avgFPS);
    quotas.put(Measure.fluency.toString(), fluency);
    quotas.put(Measure.MaxImproveMemory.toString(), 0D);
    quotas.put(Measure.BackImproveMemory.toString(), (double) backImproveMemory);
    quotas.put(Measure.PushImproveMemory.toString(), 0D);

    quotas.put(Measure.fsCallEventTotalNum.toString(), (double) fsCallEventTotalNum);
    quotas.put(Measure.callCreateFinishTime.toString(), (double) callCreateFinishTime);
    quotas.put(Measure.imgSizeCount.toString(), wrongImgSizeCount);
    quotas.put(Measure.interactionTime.toString(), (double) interactionTime);
    quotas.put(Measure.interactionViewAddCount.toString(), (double) interactionViewAddCount);
    quotas.put(Measure.interactionViewAddLimitCount.toString(), (double) interactionViewAddLimitCount);
    quotas.put(Measure.newFsRenderTime.toString(), (double) newFsRenderTime);

    quotas.put(Measure.callBridgeTime.toString(), (double) callBridgeTime);
    quotas.put(Measure.cssLayoutTime.toString(), (double) cssLayoutTime);
    quotas.put(Measure.parseJsonTime.toString(), (double) parseJsonTime);

    // TODO the following attribute is no longer needed and will be deleted soon.
    quotas.put(Measure.screenRenderTime.toString(), (double) screenRenderTime);
    quotas.put(Measure.communicateTime.toString(), (double) communicateTime);
    quotas.put(Measure.localReadTime.toString(), localReadTime);
    quotas.put(Measure.templateLoadTime.toString(), (double) templateLoadTime);
    quotas.put(Measure.firstScreenJSFExecuteTime.toString(), (double) firstScreenJSFExecuteTime);
    quotas.put(Measure.actualNetworkTime.toString(), (double) actualNetworkTime);
    quotas.put(Measure.syncTaskTime.toString(), (double) syncTaskTime);
    quotas.put(Measure.packageSpendTime.toString(), (double) packageSpendTime);

    // TODO These attribute will be moved to elsewhere
    quotas.put(Measure.measureTime1.toString(), (double) measureTimes[0]);
    quotas.put(Measure.measureTime2.toString(), (double) measureTimes[1]);
    quotas.put(Measure.measureTime3.toString(), (double) measureTimes[2]);
    quotas.put(Measure.measureTime4.toString(), (double) measureTimes[3]);
    quotas.put(Measure.measureTime5.toString(), (double) measureTimes[4]);
    return quotas;
  }

  public Map<String, String> getDimensionMap() {
    Map<String, String> quotas = new HashMap<>();
    quotas.put(Dimension.JSLibVersion.toString(), JSLibVersion);
    quotas.put(Dimension.WXSDKVersion.toString(), WXSDKVersion);
    quotas.put(Dimension.pageName.toString(), pageName);
    quotas.put(Dimension.requestType.toString(), requestType);
    quotas.put(Dimension.networkType.toString(), "unknown");
    quotas.put(Dimension.connectionType.toString(), connectionType);
    quotas.put(Dimension.zcacheInfo.toString(), zCacheInfo);
    quotas.put(Dimension.cacheType.toString(), cacheType);
    quotas.put(Dimension.useScroller.toString(), String.valueOf(useScroller));

    WXSDKInstance sdkInstance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
    String keyActivity = WXInstanceApm.KEY_PAGE_PROPERTIES_CONTAINER_NAME;
    quotas.put(keyActivity, null == sdkInstance? "unKnow" : sdkInstance.getContainerInfo().get(keyActivity));
    String keyType = WXInstanceApm.KEY_PAGE_PROPERTIES_INSTANCE_TYPE;
    quotas.put(keyType,sdkInstance == null ?"unKnow": sdkInstance.getContainerInfo().get(keyType));
    String keyParentPae = WXInstanceApm.KEY_PAGE_PROPERTIES_PARENT_PAGE;
    quotas.put(keyParentPae,null == sdkInstance ?"unKnow":sdkInstance.getContainerInfo().get(keyParentPae));

    // TODO These attribute will be moved to elsewhere
    // Extra Dimension for 3rd developers.
    quotas.put(Dimension.wxdim1.toString(), wxDims[0]);
    quotas.put(Dimension.wxdim2.toString(), wxDims[1]);
    quotas.put(Dimension.wxdim3.toString(), wxDims[2]);
    quotas.put(Dimension.wxdim4.toString(), wxDims[3]);
    quotas.put(Dimension.wxdim5.toString(), wxDims[4]);

    // TODO the following attribute is no longer needed and will be deleted soon.
    quotas.put(Dimension.bizType.toString(), bizType);
    quotas.put(Dimension.templateUrl.toString(), templateUrl);
    return quotas;
  }

  public static String[] getDimensions() {
    List<String> ret = new LinkedList<>();
    for (Dimension dimension : Dimension.values()) {
      ret.add(dimension.toString());
    }
    return ret.toArray(new String[ret.size()]);
  }

  public static String[] getMeasures() {
    List<String> ret = new LinkedList<>();
    for (Measure measure : Measure.values()) {
      ret.add(measure.toString());
    }
    return ret.toArray(new String[ret.size()]);
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
              + ",initInvokeTime:" + WXEnvironment.sSDKInitInvokeTime + ",initExecuteTime:" + WXEnvironment.sSDKInitExecuteTime
              + ",SDKInitTime:" + WXEnvironment.sSDKInitTime
              + ",totalTime:" + totalTime + ",JSLibVersion:" + JSLibVersion + ",WXSDKVersion:" + WXSDKVersion
              + ",errCode:" + errCode + ",renderFailedDetail:" + renderFailedDetail
              + ",arg:" + args
              + ",errMsg:" + getErrMsg();
    }
    return super.toString();
  }

  public String getPerfData() {
    return "networkTime:" + networkTime
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

  public void beforeInstanceRender(String instanceId) {
    renderTimeOrigin = System.currentTimeMillis();
    renderUnixTimeOrigin = WXUtils.getFixUnixTime();
  }

  public void afterInstanceDestroy(String instanceId) {
  }
}
