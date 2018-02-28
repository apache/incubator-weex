package com.taobao.weex.adapter;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.performance.IWXMonitorDataTransfer;
import com.taobao.weex.utils.WXLogUtils;

import org.json.JSONObject;

/**
 * @author zhongcang
 * @date 2018/2/28
 */

public class WXMonitorDataLoger implements IWXMonitorDataTransfer {

  private static final String MONITOR = "WXMonitor";
  private static final String MODULE_PERFORMANCE = "WXPerformance";
  private static final String MODULE_ERROR = "WXError";

  public static void transferPerformance(WXPerformance performance, String instanceId) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    IWXMonitorDataTransfer transfer = WXSDKManager.getInstance().getWXMonitorDataTransfer();
    if (null == transfer) {
      return;
    }

    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (null == instance) {
      return;
    }
    String data = "";
    try {
      JSONObject dimen = new JSONObject();

      data = new JSONObject()
          .put("instanceId", instanceId)
          .put("url", instance.getBundleUrl())
          // .put()
          .toString();

    } catch (Exception e) {
      e.printStackTrace();
    }
    transfer.transfer(MONITOR, MODULE_PERFORMANCE, "instance", data);


  }

  public static void transferError(WXJSExceptionInfo exceptionInfo, String instanceId) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    IWXMonitorDataTransfer transfer = WXSDKManager.getInstance().getWXMonitorDataTransfer();
    if (null == transfer) {
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
    transfer.transfer(MONITOR, MODULE_ERROR, errorCode.getErrorType().toString(), data);
  }

  public static void transferFps(long fps) {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    IWXMonitorDataTransfer transfer = WXSDKManager.getInstance().getWXMonitorDataTransfer();
    if (null == transfer) {
      return;
    }
    transfer.transfer(MONITOR, MODULE_PERFORMANCE, "fps", String.valueOf(fps));
    //
    //    //for dev-tool
    //    try {
    //      JSONObject data = new JSONObject();
    //      data.put("fps", fps);
    //      WXJSObject[] args = new WXJSObject[]{
    //          new WXJSObject(WXJSObject.String, module),
    //          new WXJSObject(WXJSObject.String, type),
    //          new WXJSObject(WXJSObject.JSON, data.toString())
    //      };
    //      WXBridgeManager.getInstance().invokeExecJS(MONITOR, null, MONITOR, args, false);
    //    } catch (Exception e) {
    //      e.printStackTrace();
    //    }
  }

  private void logData(String module, String type, String data) {

  }

  @Override
  public void transfer(String tag, String module, String type, String data) {
    WXLogUtils.d(tag, module + ":" + type + ":" + data);
  }
}