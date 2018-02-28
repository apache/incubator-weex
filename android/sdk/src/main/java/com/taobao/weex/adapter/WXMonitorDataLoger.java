package com.taobao.weex.adapter;

import android.os.Environment;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.performance.IWXMonitorDataTransfer;
import com.taobao.weex.utils.WXLogUtils;

import org.json.JSONObject;

import java.io.File;
import java.util.Map;

/**
 * @author zhongcang
 * @date 2018/2/28
 */

public class WXMonitorDataLoger implements IWXMonitorDataTransfer {

  private static final String MONITOR = "WXMonitor";
  private static final String MODULE_PERFORMANCE = "WXPerformance";
  private static final String MODULE_ERROR = "WXError";
  private static boolean transFpsData = false;

  static {
    judgeTransFerFps();
  }

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
    if (!transFpsData) {
      return;
    }
    IWXMonitorDataTransfer transfer = WXSDKManager.getInstance().getWXMonitorDataTransfer();
    if (null == transfer) {
      return;
    }
    String data = "";
    try {
      data = new JSONObject().put("fps", fps).toString();
    } catch (Exception e) {
      e.printStackTrace();
    }
    transfer.transfer(MONITOR, MODULE_PERFORMANCE, "fps", data);
  }


  private static void judgeTransFerFps() {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    try {
      transFpsData = new File(Environment.getExternalStorageDirectory(), "WXPerformance.dat")
          .exists();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  @Override
  public void transfer(String tag, String module, String type, String data) {
    WXLogUtils.d(tag, module + ":" + type + ":" + data);
  }
}