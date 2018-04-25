package com.alibaba.weex.extend.module;

import com.taobao.weex.annotation.AccessRestriction;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by moxun on 2018/3/27.
 */

@AccessRestriction
public class PrivateModule extends WXModule {
  @JSMethod
  public void testAccess() {
    WXLogUtils.e("allow method access");
  }
}
