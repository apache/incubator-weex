package com.alibaba.weex.extend.module;

import android.widget.Toast;

import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;

import java.util.HashMap;
import java.util.Map;

public class MyModule extends WXModule {

  @JSMethod(uiThread = true)
  public Object printLog(String msg) {
    Toast.makeText(mWXSDKInstance.getContext(),msg,Toast.LENGTH_SHORT).show();
    Map map = new HashMap();
    map.put("11","weex");
    map.put("22","RN");
    return map;
  }
}
