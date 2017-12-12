package com.alibaba.weex.extend.misc;

import android.os.Build;
import android.text.TextUtils;

import com.taobao.zcache.config.CommonConfigMirror;
import com.taobao.zcache.config.ZCacheConfigManager;
import com.taobao.zcache.config.ZCacheConfigUpdateCallback;
import com.taobao.zcache.config.managers.CDNConfigManagerV1;
import com.taobao.zcache.connect.ConnectManager;
import com.taobao.zcache.connect.HttpConnectListener;
import com.taobao.zcache.connect.HttpResponse;
import com.taobao.zcache.utils.ConfigStorage;
import com.taobao.zcache.utils.TaoLog;
import com.taobao.zcache.utils.WVConfigUtils;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;

public class WVCommonConfig {
  private static final String TAG = "WVCommonConfig";
  public static final WVCommonConfigData commonConfig = new WVCommonConfigData(); //更新前，使用默认配置
  private volatile static WVCommonConfig instance = null;

  public static WVCommonConfig getInstance() {
    if (instance == null) {
      synchronized (WVCommonConfig.class) {
        if (instance == null) {
          instance = new WVCommonConfig();
        }
      }
    }
    return instance;
  }

  private WVCommonConfig() {
    init();
  }

  /**
   * 首次加载，初始化配置规则
   */
  public void init() {
    String content = ConfigStorage.getStringVal(CDNConfigManagerV1.SPNAME_CONFIG, CDNConfigManagerV1.CONFIGNAME_COMMON + ConfigStorage.KEY_DATA);
    parseConfig(content);
  }

  /**
   * 更新common配置规则
   */
  public void updateCommonRule(final ZCacheConfigUpdateCallback callback, String defaultUrl, String snapshotN) {
    String commonConfigUrl = null;
    if (TextUtils.isEmpty(defaultUrl)) {
      commonConfigUrl = ZCacheConfigManager.getConfigUrl("1", commonConfig.v, WVConfigUtils.getTargetValue(), snapshotN);
    } else {
      commonConfigUrl = defaultUrl;
    }

    final String finalCommonConfigUrl = commonConfigUrl;
    ConnectManager.getInstance().connect(commonConfigUrl, new HttpConnectListener<HttpResponse>() {
      @Override
      public void onFinish(HttpResponse data, int token) {
        if (callback == null) {
          return;
        }
        if (data == null || data.getData() == null) {
          callback.updateStatus(ZCacheConfigUpdateCallback.CONFIG_UPDATE_STATUS.NULL_DATA, 0);
          return;
        }
        try {
          String content = new String(data.getData(), "utf-8");
          int updateCount = parseConfig(content);
          if (updateCount > 0) {
            ConfigStorage.putStringVal(CDNConfigManagerV1.SPNAME_CONFIG, CDNConfigManagerV1.CONFIGNAME_COMMON + ConfigStorage.KEY_DATA, content);
            callback.updateStatus(ZCacheConfigUpdateCallback.CONFIG_UPDATE_STATUS.SUCCESS, updateCount);
          } else {
            callback.updateStatus(ZCacheConfigUpdateCallback.CONFIG_UPDATE_STATUS.NO_VERSION, 0);
          }
        } catch (UnsupportedEncodingException e) {
          callback.updateStatus(ZCacheConfigUpdateCallback.CONFIG_UPDATE_STATUS.ENCODING_ERROR, 0);
          TaoLog.e(TAG, "config encoding error. " + e.getMessage());
        }
      }

      @Override
      public void onError(int code, String message) {
        if (null != callback) {
          callback.updateError(finalCommonConfigUrl, message);
          callback.updateStatus(ZCacheConfigUpdateCallback.CONFIG_UPDATE_STATUS.UNKNOWN_ERROR, 0);
        }
        TaoLog.d(TAG, "update common failed! : " + message);
        super.onError(code, message);
      }

    });
  }

  private int parseConfig(String config) {
    CommonConfigMirror.getInstance().parseConfig(config);
    if (TextUtils.isEmpty(config)) {
      return 0;
    }
    JSONObject jsonObj = null;
    try {
      jsonObj = new JSONObject(config);
    } catch (JSONException e) {
    }
    if (jsonObj != null) {
      String v = jsonObj.optString("v", "");
      if (TextUtils.isEmpty(v)) {
        return 0;
      }
      commonConfig.v = v;
      long configUpdateInterval = jsonObj.optLong("configUpdateInterval", 0L);
      //更新间隔必须大于0
      if (configUpdateInterval >= 0L) {
        commonConfig.updateInterval = configUpdateInterval;
        CDNConfigManagerV1.getInstance().setUpdateInterval(configUpdateInterval);
      }
      commonConfig.packagePriorityWeight = jsonObj.optDouble("packagePriorityWeight", 0.1);
      int packageAppStatus = jsonObj.optInt("packageAppStatus", 2);//类型 int 0-2，0 表示关闭 PackageApp 和 ZCache，1 表示打开但不更新配置，2 表示打开且更新配置
      commonConfig.packageAppStatus = packageAppStatus;
      int monitorStatus = jsonObj.optInt("monitorStatus", 2);//类型 int 0-2，0 表示关闭 Monitor，1 表示打开但不更新配置，2 表示打开且更新配置
      commonConfig.monitorStatus = monitorStatus;
      int urlRuleStatus = jsonObj.optInt("urlRuleStatus", 2);//类型 int 0-2，0 表示关闭 urlRuleStatus，1 表示打开但不更新配置，2 表示打开且更新配置
      commonConfig.urlRuleStatus = urlRuleStatus;
      int packageMaxAppCount = jsonObj.optInt("packageMaxAppCount", 100);//预加载最大安装数
      commonConfig.packageMaxAppCount = packageMaxAppCount;
      String urlScheme = jsonObj.optString("urlScheme", "http");//默认补全scheme时使用 http
      //兼容老版本配置，去掉多余的 :
      commonConfig.urlScheme = urlScheme.replace(":", "");
      JSONObject verifySampleRate = jsonObj.optJSONObject("verifySampleRate");//安全校验采样
      if (null != verifySampleRate) {
        commonConfig.verifySampleRate = verifySampleRate.toString();
      }
      String ucParam = jsonObj.optString("ucParam", "");
      if (TextUtils.isEmpty(ucParam)) {
        commonConfig.ucParam = "skip_old_extra_kernel=true";
      } else {
        commonConfig.ucParam = ucParam;
      }
      commonConfig.useSystemWebView = jsonObj.optBoolean("useSystemWebView", false);
      commonConfig.ucsdk_alinetwork_rate = jsonObj.optDouble("ucsdk_alinetwork_rate", 1.0);
      commonConfig.ucsdk_image_strategy_rate = jsonObj.optDouble("ucsdk_image_strategy_rate", 1.0);
      commonConfig.cookieUrlRule = jsonObj.optString("cookieUrlRule", "");

      commonConfig.ucCoreUrl = jsonObj.optString("ucCoreUrl", "");

      commonConfig.shareBlankList = jsonObj.optString("shareBlankList", "");

      commonConfig.excludeUCVersions = jsonObj.optString("excludeUCVersions", "1.12.11.0, 1.15.15.0, 1.14.13.0, 1.13.12.0");

      commonConfig.isOpenCombo = jsonObj.optBoolean("isOpenCombo", false);
      commonConfig.isCheckCleanup = jsonObj.optBoolean("isCheckCleanup", true);
      commonConfig.isAutoRegisterApp = jsonObj.optBoolean("isAutoRegisterApp", false);
      commonConfig.isUseTBDownloader = jsonObj.optBoolean("isUseTBDownloader", true);
      commonConfig.isUseAliNetworkDelegate = jsonObj.optBoolean("isUseAliNetworkDelegate", true);
      commonConfig.packageDownloadLimit = jsonObj.optInt("packageDownloadLimit", 30);
      commonConfig.packageAccessInterval = jsonObj.optInt("packageAccessInterval", 3000);
      commonConfig.packageRemoveInterval = jsonObj.optInt("packageRemoveInterval", 432000000);
      commonConfig.recoveryInterval = jsonObj.optInt("recoveryInterval", 432000000);
      commonConfig.customsComboLimit = jsonObj.optInt("customsComboLimit", 3);
      commonConfig.customsDirectQueryLimit = jsonObj.optInt("customsDirectQueryLimit", 10);
      commonConfig.packageZipPrefix = jsonObj.optString("packageZipPrefix", "");
      commonConfig.packageZipPreviewPrefix = jsonObj.optString("packageZipPreviewPrefix", "");
      commonConfig.ucSkipOldKernel = jsonObj.optBoolean("ucSkipOldKernel", true);

      parseUnzipDegradeConfig(jsonObj);

      return jsonObj.length();
    }
    return 0;
  }

  private void parseUnzipDegradeConfig(JSONObject config) {
    try {
      commonConfig.zipDegradeMode = config.optInt("zipDegradeMode", 0);
      commonConfig.zipDegradeList = config.optString("zipDegradeList", "");

      String identifier = Build.BRAND + "@" + Build.VERSION.RELEASE;
      String zipDegradeList = commonConfig.zipDegradeList;
      if (!TextUtils.isEmpty(zipDegradeList)) {
        String[] ids = zipDegradeList.split(",");
        if (ids != null) {
          for (String s : ids) {
            if (identifier.equalsIgnoreCase(s)) {
              TaoLog.e(TAG, "Degrade unzip: " + identifier);
              commonConfig.needZipDegrade = true;
              if (commonConfig.zipDegradeMode == 2) {
                // 关闭当前设备上的预加载
                commonConfig.packageAppStatus = 0;
                TaoLog.w(TAG, "Disable package app");
              }
              break;
            }
          }
        }
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }
}
