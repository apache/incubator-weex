package com.alibaba.weex;

import android.app.Application;

import com.alibaba.weex.extend.Components.WTRichText;
import com.alibaba.weex.extend.ImageAdapter;
import com.alibaba.weex.extend.Modules.RenderModule;
import com.alibaba.weex.extend.Modules.WXEventModule;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.common.WXException;

public class WXApplication extends Application {
  public static String CURRENT_IP =  "30.30.29.246"; // "your_current_IP"

  @Override
  public void onCreate() {
    super.onCreate();
    initDebugEnv(false, CURRENT_IP);
    WXSDKEngine.addCustomOptions("appName", "WXSample");
    WXSDKEngine.addCustomOptions("appGroup", "WXApp");
//    WXSDKEngine.addCustomOptions("infoCollect", "false");
    WXSDKEngine.init(this,null,null,new ImageAdapter(),null);

    try {

      WXSDKEngine.registerComponent("wtRichText", WTRichText.class);
      WXSDKEngine.registerModule("render", RenderModule.class);
      WXSDKEngine.registerModule("event", WXEventModule.class);

    } catch (WXException e) {
      e.printStackTrace();
    }

  }

  private void initDebugEnv(boolean enable, String host) {
    WXEnvironment.sDebugMode = enable;
    WXEnvironment.sDebugWsUrl = "ws://" + host + ":8088/debugProxy/native";
  }
}
