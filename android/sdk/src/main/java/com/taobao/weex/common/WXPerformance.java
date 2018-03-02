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

import android.os.Debug;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.text.TextUtils;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.performance.FpsCollector;
import com.taobao.weex.performance.MemUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class WXPerformance {

  @RestrictTo(Scope.LIBRARY)
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
    JSLibInitTime(0D, 50000D),
    SDKInitTime(0D, 5000D),
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
    fsRenderTime(0D, 5000D),
    fsRequestNum(0D, 100D),
    callCreateFinishTime(0D, 10000D),
    cellExceedNum(0D, Double.MAX_VALUE),
    communicateTotalTime(0D, 5000D),
    maxDeepViewLayer(0D, Double.MAX_VALUE),
    maxDeepVDomLayer(0D, Double.MAX_VALUE),
    componentCount(0D, Double.MAX_VALUE),
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
    interactionTime(0D,10000D);


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

  @RestrictTo(Scope.LIBRARY_GROUP)
  public static final String CACHE_TYPE = "cacheType";

  public static final int VIEW_LIMIT_HEIGHT = WXViewUtils.getScreenHeight() / 2;
  public static final int VIEW_LIMIT_WIDTH = WXViewUtils.getScreenWidth() / 2;
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

  @RestrictTo(Scope.LIBRARY_GROUP)
  public String cacheType = "unknown";

  @RestrictTo(Scope.LIBRARY)
  public long renderTimeOrigin;

  public long fsRenderTime;

  public long callCreateFinishTime;

  public long interactionTime;

  /**
   * Time used for
   * {@link com.taobao.weex.bridge.WXBridgeManager#createInstance(String, String, Map, String)}
   */
  @RestrictTo(Scope.LIBRARY)
  public long callCreateInstanceTime;


  public long fsCallJsTotalTime;

  public int fsCallJsTotalNum;

  public long fsCallNativeTotalTime;

  public int fsCallNativeTotalNum;

  public int fsRequestNum;

  public int cellExceedNum;

  public int timerInvokeCount;

  public int fsCallEventTotalNum;

  public long avgFPS;
  public long frameSum;
  public long frameStartTime;
  public long frameEndTime;
  public double fluency = 100D;

  public long maxImproveMemory;

  public long backImproveMemory;

  public long pushImproveMemory;

  public long memTotalBeforeRender;


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
   * UpdateDomObj Time spent when rendering first screen
   */
  public long updateDomObjTime;

  /**
   * ApplyUpdate Time spent when rendering first screen
   */
  public long applyUpdateTime;


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

  public String args = "";

  public String connectionType;
  public String requestType;

  public String zCacheInfo;

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

  public WXPerformance() {
    mErrMsgBuilder = new StringBuilder();
  }

  public static void init() {
    if (WXPerformance.TRACE_DATA) {
      FpsCollector.getInstance().init();
    }
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
    quotas.put(Measure.fsCreateInstanceTime.toString(), (double) (callCreateInstanceTime - renderTimeOrigin));
    quotas.put(Measure.fsCallJsTotalTime.toString(), (double) fsCallJsTotalTime);
    quotas.put(Measure.fsCallJsTotalNum.toString(), (double) fsCallJsTotalNum);
    quotas.put(Measure.fsCallNativeTotalTime.toString(), (double) fsCallNativeTotalTime);
    quotas.put(Measure.fsCallNativeTotalNum.toString(), (double) fsCallNativeTotalNum);
    quotas.put(Measure.fsRenderTime.toString(), fsRenderTime);
    quotas.put(Measure.fsRequestNum.toString(), (double) fsRequestNum);
    quotas.put(Measure.communicateTotalTime.toString(), totalTime);
    quotas.put(Measure.maxDeepViewLayer.toString(), (double) maxDeepViewLayer);
    quotas.put(Measure.maxDeepVDomLayer.toString(), (double) maxDeepVDomLayer);
    quotas.put(Measure.componentCount.toString(), (double) componentCount);
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


  private FpsRecorder mFpsRecorder = new FpsRecorder();

  public void beforeInstanceRender(String instanceId) {
    renderTimeOrigin = System.currentTimeMillis();
    if (WXPerformance.TRACE_DATA) {
      if (TextUtils.isEmpty(instanceId)) {
        return;
      }
      Debug.MemoryInfo mem = MemUtils.getMemoryInfo(WXEnvironment.getApplication());
      if (null != mem) {
        memTotalBeforeRender = mem.getTotalPss();
      }
      FpsCollector.getInstance().registerListener(instanceId, mFpsRecorder);
    }
  }

  //  public void onInstanceEndRender(String instanceId,boolean isFirstScreen){
  //
  //  }


  public void afterInstanceDestroy(String instanceId) {
    if (WXPerformance.TRACE_DATA) {
      if (TextUtils.isEmpty(instanceId)) {
        return;
      }
      FpsCollector.getInstance().unRegister(instanceId);
      Debug.MemoryInfo mem = MemUtils.getMemoryInfo(WXEnvironment.getApplication());
      if (null != mem) {
        backImproveMemory = mem.getTotalPss() - memTotalBeforeRender;
      }
      if (mFpsRecorder.totalFpsCount > 0) {
        avgFPS = mFpsRecorder.totalFpsCount / mFpsRecorder.totalFpsCount;
        fluency = (double) mFpsRecorder.fluncyFpsPointCount / (double) mFpsRecorder.totalFpsCount;
      }
    }
  }

  private class FpsRecorder implements FpsCollector.IFPSCallBack {

    private final long FLUENCY_FPS_LIMIT = 35;

    private long totalFpsFrame;
    private long totalFpsCount;
    private long fluncyFpsPointCount;
    private final long LIMIT_LONG = Long.MAX_VALUE - 1000;

    @Override
    public void fps(int fps) {
      if (totalFpsFrame >= LIMIT_LONG || totalFpsCount >= Long.MAX_VALUE) {
        return;
      }
      totalFpsFrame += fps;
      totalFpsCount++;
      if (fps >= FLUENCY_FPS_LIMIT) {
        fluncyFpsPointCount++;
      }
    }
  }
}