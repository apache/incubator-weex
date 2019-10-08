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
package org.apache.weex.performance;

import java.util.Iterator;
import java.util.List;

import android.util.Log;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXJSExceptionInfo;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.utils.WXUtils;
import org.json.JSONObject;

/**
 * @author zhongcang
 * @date 2018/2/28
 */

public class WXAnalyzerDataTransfer {

  private static final String GROUP = "WXAnalyzer";
  private static final String MODULE_ERROR = "WXError";
  private static final String MODULE_WX_APM = "wxapm";
  public static boolean isOpenPerformance = false;
  public static final String INTERACTION_TAG = "wxInteractionAnalyzer";
  private static boolean sOpenInteractionLog;


  public static void transferPerformance(String instanceId,String type,String key,Object value)
  {
      if (!isOpenPerformance){
          return;
      }
      if (sOpenInteractionLog && "stage".equals(type)){
          Log.d(INTERACTION_TAG, "[client][stage]"+instanceId+","+key+","+value);
      }
      List<IWXAnalyzer> transferList = WXSDKManager.getInstance().getWXAnalyzerList();
      if (null == transferList || transferList.size() == 0) {
          return;
      }

      WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(instanceId);
      if (null == instance){
          return;
      }

      String data;
      try {
        data = new JSONObject().put(key,value).toString();
      }catch (Exception e){
        e.printStackTrace();
        return;
      }

      Iterator<IWXAnalyzer> itr = transferList.iterator();
      while (itr.hasNext()){
          IWXAnalyzer item = itr.next();
          item.transfer(MODULE_WX_APM, instance.getInstanceId(), type, data);
      }
  }

  public static void transferInteractionInfo(WXComponent targetComponent){
    if (!isOpenPerformance){
      return;
    }

    List<IWXAnalyzer> transferList = WXSDKManager.getInstance().getWXAnalyzerList();
    if (null == transferList || transferList.size() == 0) {
      return;
    }
    long renderOriginDiffTime = WXUtils.getFixUnixTime() - targetComponent.getInstance().getWXPerformance().renderUnixTimeOrigin;
    String data;
    try{
      data = new JSONObject()
          .put("renderOriginDiffTime",renderOriginDiffTime)
          .put("type",targetComponent.getComponentType())
          .put("ref",targetComponent.getRef())
          .put("style",targetComponent.getStyles())
          .put("attrs",targetComponent.getAttrs())
          .toString();
    }catch (Exception e){
      e.printStackTrace();
      return;
    }
    for (IWXAnalyzer transfer : transferList) {
      transfer.transfer(MODULE_WX_APM, targetComponent.getInstanceId(), "wxinteraction", data);
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

  public static void switchInteractionLog(final boolean isOpen){
      if ( sOpenInteractionLog == isOpen || !WXEnvironment.JsFrameworkInit){
          return;
      }
      sOpenInteractionLog = isOpen;
      //for jsfm && jsengin
      //TODO wait for JSFramework
//      WXBridgeManager.getInstance().post(new Runnable() {
//          @Override
//          public void run() {
//              WXJSObject[] args = {new WXJSObject(isOpen?1:0)};
//              WXBridgeManager.getInstance().invokeExecJS(
//                  "",
//                  null,
//                  "switchInteractionLog",
//                  args,
//                  false);
//          }
//      });
      //for weex_core
      WXBridgeManager.getInstance().registerCoreEnv("switchInteractionLog",String.valueOf(isOpen));
  }

  public static boolean isInteractionLogOpen(){
     return sOpenInteractionLog;
  }
}