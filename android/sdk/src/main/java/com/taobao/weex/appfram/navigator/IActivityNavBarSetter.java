package com.taobao.weex.appfram.navigator;

/**
 *  Define the  NavBarSetter api of the Activity
 */
public interface IActivityNavBarSetter {

  boolean setNavBarRightItem(String param);

  boolean clearNavBarRightItem(String param);

  boolean setNavBarLeftItem(String param);

  boolean clearNavBarLeftItem(String param);

  boolean setNavBarMoreItem(String param);

  boolean clearNavBarMoreItem(String param);

  boolean setNavBarTitle(String param);

  boolean clearNavBarTitle(String param);

}
