package com.alibaba.weex.extend.adapter;

import android.text.TextUtils;

import com.taobao.weex.adapter.IWXAccessibilityRoleAdapter;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * Created by moxun on 2017/11/13.
 */

public class DefaultAccessibilityRoleAdapter implements IWXAccessibilityRoleAdapter {
  private static Map<String, String> CNRoles = new HashMap<>();

  static {
    CNRoles.put("button", "按钮");
    CNRoles.put("link", "链接");
    CNRoles.put("img", "图像");
    CNRoles.put("search", "搜索");
    CNRoles.put("header", "标题");
  }

  @Override
  public String getRole(String key) {
    if (isCN()) {
      String cnRole = CNRoles.get(key);
      if (!TextUtils.isEmpty(cnRole)) {
        return cnRole;
      }
    }
    return key;
  }

  private static boolean isCN() {
    try {
      return "CN".equals(Locale.getDefault().getCountry());
    } catch (Throwable t) {
    }
    return false;
  }
}
