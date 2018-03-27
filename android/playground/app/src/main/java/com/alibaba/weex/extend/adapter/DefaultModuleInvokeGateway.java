package com.alibaba.weex.extend.adapter;

import android.net.Uri;
import android.text.TextUtils;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IModuleInvokeGateway;
import com.taobao.weex.utils.WXModuleUtil;

/**
 * Created by moxun on 2018/3/27.
 */

public class DefaultModuleInvokeGateway implements IModuleInvokeGateway {
  @Override
  public boolean allowAccess(String instanceId, String moduleName, String methodName) {
    if (WXModuleUtil.isPublicAccess(instanceId, moduleName, methodName)) {
      return true;
    } else {
      String bundleUrl = WXSDKManager.getInstance().getSDKInstance(instanceId).getBundleUrl();
      if (TextUtils.isEmpty(bundleUrl)) {
        return true;
      }
      Uri uri = Uri.parse(bundleUrl);
      if ("assets".equals(uri.getScheme())) {
        return true;
      }
    }
    return false;
  }
}
