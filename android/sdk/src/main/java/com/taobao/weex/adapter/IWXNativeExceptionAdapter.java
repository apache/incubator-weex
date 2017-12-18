package com.taobao.weex.adapter;

import com.taobao.weex.common.WXNativeExceptionInfo;

/**
 * @author chenpeihan
 * @date 2017/12/18
 */

public interface IWXNativeExceptionAdapter {

  /**
   * report sdk native exception
   *
   * @param exception {@link WXNativeExceptionInfo}
   */

  void onNativeException(WXNativeExceptionInfo exception);
}
