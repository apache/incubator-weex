package com.alibaba.weex.extend.module.location;

import com.taobao.weex.WXSDKInstance;

/**
 * Created by lixinke on 16/9/10.
 */
public class LocationFactory {

  public  static ILocatable getLocationProvider(WXSDKInstance context){
    return new DefaultLocation(context);
  }
}
