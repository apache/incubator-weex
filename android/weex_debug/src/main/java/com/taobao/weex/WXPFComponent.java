package com.taobao.weex;

import android.app.Application;
import android.content.Context;
import android.widget.Toast;

import com.taobao.prettyfish.core.comp.AbstractComponent;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.utils.WXFileUtils;


public class WXPFComponent extends AbstractComponent {

  public WXPFComponent(Application application) {
    super(application);
  }

  @Override
  public boolean isDeviceSupport() {
    return true;
  }

  @Override
  public int iconRes() {
    return R.drawable.ic_action_bug;
  }

  @Override
  public String title() {
    return "DEBUG";
  }

  @Override
  public boolean onClick(Context context) {
    Toast.makeText(context, "ONClick", Toast.LENGTH_SHORT).show();
    WXBridgeManager.getInstance().restart();
    WXBridgeManager.getInstance().initScriptsFramework(WXFileUtils.loadFileContent("main2.js",context));
    return false;
  }

  @Override
  public void onClose() {

  }
}
