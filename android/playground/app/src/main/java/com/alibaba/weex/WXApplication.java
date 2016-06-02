package com.alibaba.weex;

import android.app.Application;

import com.alibaba.weex.extend.adapter.ImageAdapter;
import com.alibaba.weex.extend.component.WTRichText;
import com.alibaba.weex.extend.module.RenderModule;
import com.alibaba.weex.extend.module.WXEventModule;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.common.WXException;

public class WXApplication extends Application {

  @Override
  public void onCreate() {
    super.onCreate();
    WXSDKEngine.addCustomOptions("appName", "WXSample");
    WXSDKEngine.addCustomOptions("appGroup", "WXApp");
    WXSDKEngine.init(this,null,null,new ImageAdapter(),null);

    try {

      WXSDKEngine.registerComponent("wtRichText", WTRichText.class);
      WXSDKEngine.registerModule("render", RenderModule.class);
      WXSDKEngine.registerModule("event", WXEventModule.class);

    } catch (WXException e) {
      e.printStackTrace();
    }
  }
}
