/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.performance;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXPerformance;

import org.json.JSONObject;

import java.util.List;
import java.util.Map;

/**
 * @author zhongcang
 * @date 2018/2/28
 */

public class WXAnalyzerDataTransfer {

  private static final String GROUP = "WXAnalyzer";
  private static final String MODULE_PERFORMANCE = "WXPerformance";
  private static final String MODULE_ERROR = "WXError";


  public static void transferPerformance(WXPerformance performance, String instanceId) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    List<IWXAnalyzer> transferList = WXSDKManager.getInstance().getWXAnalyzerList();
    if (null == transferList || transferList.size() == 0) {
      return;
    }


    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (null == instance) {
      return;
    }
    String data = "";
    try {
      JSONObject dimensionMap = new JSONObject();
      JSONObject measureMap = new JSONObject();

      for (Map.Entry<String, String> entry : performance.getDimensionMap().entrySet()) {
        dimensionMap.put(entry.getKey(), entry.getValue());
      }
      for (Map.Entry<String, Double> entry : performance.getMeasureMap().entrySet()) {
        measureMap.put(entry.getKey(), entry.getValue());
      }

      data = new JSONObject()
              .put("instanceId", instanceId)
              .put("url", instance.getBundleUrl())
              .put("dimensionMap", dimensionMap)
              .put("measureMap", measureMap)
              .toString();

    } catch (Exception e) {
      e.printStackTrace();
    }
    for (IWXAnalyzer transfer : transferList) {
      transfer.transfer(GROUP, MODULE_PERFORMANCE, "instance", data);
    }
  }

  public static void transferError(WXJSExceptionInfo exceptionInfo, String instanceId) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    List<IWXAnalyzer> transferList = WXSDKManager.getInstance().getWXAnalyzerList();
    if (null == transferList || transferList.size() == 0) {
      return;
    }

    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (null == instance) {
      return;
    }
    WXErrorCode errorCode = exceptionInfo.getErrCode();
    String data = "";
    try {
      data = new JSONObject()
              .put("instanceId", instanceId)
              .put("url", instance.getBundleUrl())
              .put("errorCode", errorCode.getErrorCode())
              .put("errorMsg", errorCode.getErrorMsg())
              .put("errorGroup", errorCode.getErrorGroup())
              .toString();
    } catch (Exception e) {
      e.printStackTrace();
    }
    for (IWXAnalyzer transfer : transferList) {
      transfer.transfer(GROUP, MODULE_ERROR, errorCode.getErrorType().toString(), data);
    }
  }

  public static void transferFps(long fps) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    List<IWXAnalyzer> transferList = WXSDKManager.getInstance().getWXAnalyzerList();
    if (null == transferList || transferList.size() == 0) {
      return;
    }
    String data = "";
    try {
      data = new JSONObject().put("fps", fps).toString();
    } catch (Exception e) {
      e.printStackTrace();
    }
    for (IWXAnalyzer transfer : transferList) {
      transfer.transfer(GROUP, MODULE_PERFORMANCE, "fps", data);
    }
  }

//  @Override
//  public void transfer2(String tag, String module, String type, String data) {
//    WXLogUtils.d(tag, module + ":" + type + ":" + data);
//  }
}