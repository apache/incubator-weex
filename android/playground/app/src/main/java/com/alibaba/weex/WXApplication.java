package com.alibaba.weex;

import android.app.Application;

import com.alibaba.weex.commons.adapter.ImageAdapter;
import com.taobao.weex.InitConfig;
import com.alibaba.weex.extend.component.WTRichText;
import com.alibaba.weex.extend.module.RenderModule;
import com.alibaba.weex.extend.module.WXEventModule;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.adapter.DefautDebugAdapter;
import com.taobao.weex.common.WXException;

public class WXApplication extends Application {

  @Override
  public void onCreate() {
    super.onCreate();
    WXSDKEngine.addCustomOptions("appName", "WXSample");
    WXSDKEngine.addCustomOptions("appGroup", "WXApp");
    WXSDKEngine.initialize(this,
                           new InitConfig.Builder()
                               .setImgAdapter(new ImageAdapter()).setDebugAdapter(new DefautDebugAdapter())
                               .build()
                          );

    try {

      WXSDKEngine.registerComponent("wtRichText", WTRichText.class);
      WXSDKEngine.registerModule("render", RenderModule.class);
      WXSDKEngine.registerModule("event", WXEventModule.class);

    } catch (WXException e) {
      e.printStackTrace();
    }
  }
}
