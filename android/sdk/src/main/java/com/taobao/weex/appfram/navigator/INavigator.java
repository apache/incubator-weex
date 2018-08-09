package com.taobao.weex.appfram.navigator;
import android.app.Activity;

/**
 * Created by zhengshihan on 2018/8/9.
 */

public interface INavigator {

  boolean push(Activity activity,String param);

  boolean pop(Activity activity,String param);
}
