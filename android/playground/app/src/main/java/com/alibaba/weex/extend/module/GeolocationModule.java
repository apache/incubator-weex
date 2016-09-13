package com.alibaba.weex.extend.module;

import com.alibaba.weex.extend.module.location.ILocatable;
import com.alibaba.weex.extend.module.location.LocationFactory;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.common.WXModuleAnno;

/**
 * Created by lixinke on 16/9/10.
 */
public class GeolocationModule extends WXModule {

  private ILocatable mILocatable;

  public GeolocationModule() {
    mILocatable = LocationFactory.getLocationProvider(mWXSDKInstance);
  }

  /**
   * 获取当前位置信息，只回调一次。
   *
   * @param successCallback 成功回调function id.
   * @param errorCallback   错误回调function id.(例如:没有权限)
   * @param params          JSON格式的参数(例如:准确度等).
   */
  @WXModuleAnno
  public void getCurrentPosition(String successCallback, String errorCallback, String params) {
    mILocatable.setWXSDKInstance(mWXSDKInstance);
    mILocatable.getCurrentPosition(successCallback, errorCallback, params);
  }

  /**
   * 注册监听全局定位
   *
   * @param successCallback 定位成功回调function id.
   * @param errorCallback   错误回调(例如:没有权限等).
   * @param params          SON格式的参数(例如:准确度等).
   */
  @WXModuleAnno
  public void watchPosition(String successCallback, String errorCallback, String params) {
    mILocatable.setWXSDKInstance(mWXSDKInstance);
    mILocatable.watchPosition(successCallback, errorCallback, params);
  }

  /**
   * 注销监听全局定位
   *
   * @param registerID 注册时返回的唯一ID。
   */
  @WXModuleAnno
  public void clearWatch(String registerID) {
    mILocatable.setWXSDKInstance(mWXSDKInstance);
    mILocatable.clearWatch(registerID);
  }
}
