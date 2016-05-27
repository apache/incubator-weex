package com.taobao.weex;

import android.app.Application;
import android.content.Context;

import com.taobao.prettyfish.core.comp.AbstractComponent;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;


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
    return "3D";
  }

  @Override
  public boolean onClick(Context context) {

    try {
      Class cls = Class.forName("com.taobao.weex.WXDebugTool");
      Method m = cls.getMethod("switchLayerInteractionEnabled", null);
      m.invoke(null, null);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (NoSuchMethodException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    } catch (InvocationTargetException e) {
      e.printStackTrace();
    }

//    WXDebugTool.switchLayerInteractionEnabled();
    return false;
  }

  @Override
  public void onClose() {
  }
}
