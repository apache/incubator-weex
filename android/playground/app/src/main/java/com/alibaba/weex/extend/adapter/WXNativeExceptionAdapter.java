package com.alibaba.weex.extend.adapter;

import com.taobao.weex.adapter.IWXNativeExceptionAdapter;
import com.taobao.weex.common.WXNativeExceptionInfo;
import com.taobao.weex.utils.WXLogUtils;

/**
 * @author chenpeihan
 * @date 2017/12/18
 */

public class WXNativeExceptionAdapter implements IWXNativeExceptionAdapter {

  @Override
  public void onNativeException(WXNativeExceptionInfo exception) {
    if (exception != null) {
      WXLogUtils.d("[weex_native_exception] :"+exception.toString());
    }
  }
}
