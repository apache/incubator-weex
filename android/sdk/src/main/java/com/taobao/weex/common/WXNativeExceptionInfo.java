package com.taobao.weex.common;

import java.util.Map;

/**
 * @author chenpeihan
 * @date 2017/12/18
 */

public class WXNativeExceptionInfo extends WXJSExceptionInfo {

  public WXNativeExceptionInfo(String instanceId, String bundleUrl, String errCode, String function, String exception, Map<String, String> extParams) {
    super(instanceId, bundleUrl, errCode, function, exception, extParams);
  }
}
