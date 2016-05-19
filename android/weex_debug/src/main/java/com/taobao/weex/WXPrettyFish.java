package com.taobao.weex;

import android.app.Application;

import com.jakewharton.scalpel.ScalpelFrameLayout;
import com.taobao.prettyfish.PrettyFishCommon;
import com.taobao.prettyfish.core.comp.AbstractBaseComp;
import com.taobao.prettyfish.core.fab.FloatWndManager;

/**
 * Created by lixinke on 16/5/4.
 */
public class WXPrettyFish {
  public static ScalpelFrameLayout sScalpelFrameLayout;
  public static void init(Application application) {
    PrettyFishCommon.init(application);
    AbstractBaseComp wxBaseComp=new AbstractBaseComp() {
      @Override
      public int iconRes() {
        return R.drawable.ic_action_weex;
      }

      @Override
      public String title() {
        return "WEEX";
      }
    };
    wxBaseComp.registerComp(new WXPFComponent(application));
    FloatWndManager.registerBaseComp(wxBaseComp);
  }

  public static void updateScapleView(ScalpelFrameLayout scalpelFrameLayout){
    sScalpelFrameLayout=scalpelFrameLayout;
  }
}
