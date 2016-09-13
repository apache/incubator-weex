package com.alibaba.weex.extend.module.location;

import com.taobao.weex.WXSDKInstance;

/**
 */
public interface ILocatable {

  int REQUEST_CUR_PERMISSION_CODE = 0x12;
  int REQUEST_WATCH_PERMISSION_CODE = 0x13;


  String REQUEST_PERMISSION_CODE = "REQUEST_PERMISSION_CODE";
  String ERROR_CODE = "errorCode";
  String ERROR_MSG = "errorMsg";
  String COORDS = "coords";
  String ADDRESS = "address";
  String REGISTER_ID = "registerId";

  interface ErrorCode {

    int SUCCESS = 90000;
    int NO_PERMISSION_ERROR = 90001;
    int PARAMS_ERROR = 90002;
    int LOCATION_ERROR = 9003;
    int LOCATION_TIME_OUT = 9004;
  }

  interface ErrorMsg {

    String NO_PERMISSION_ERROR = "NO PERMISSION";
    String PARAMS_ERROR = "PARAMS_ERROR";
    String LOCATION_ERROR = "LOCATION_FAIL";
    String LOCATION_TIME_OUT = "LOCATION_TIME_OUT";
    String SUCCESS = "SUCCESS";

  }

  /**
   * 获取当前位置信息，只回调一次。
   *
   * @param successCallback 成功回调function id.
   * @param errorCallback   错误回调function id.(例如:没有权限)
   * @param params          JSON格式的参数(例如:准确度等).
   */
  void getCurrentPosition(String successCallback, String errorCallback, String params);

  /**
   * 注册监听全局定位
   *
   * @param successCallback 定位成功回调function id.
   * @param errorCallback   错误回调(例如:没有权限等).
   * @param params          JSON格式的参数(例如:准确度等).
   */
  void watchPosition(String successCallback, String errorCallback, String params);

  /**
   * 注销监听全局定位
   *
   * @param registerID 注册时返回的唯一ID。
   */
  void clearWatch(String registerID);

  void setWXSDKInstance(WXSDKInstance instance);

  void destroy();

}
