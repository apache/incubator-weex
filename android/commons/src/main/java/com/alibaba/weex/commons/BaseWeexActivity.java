package com.alibaba.weex.commons;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.ViewGroup;
import com.alibaba.weex.commons.util.ScreenUtil;
import com.taobao.weex.IWXRenderListener;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXRenderStrategy;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 5/30/16.
 */
public abstract class BaseWeexActivity extends AppCompatActivity implements IWXRenderListener {
  private static final String TAG = "BaseWeexActivity";

  private ViewGroup mContainer;
  private WXSDKInstance mInstance;

  protected ViewGroup initLayout(){
    return (ViewGroup) getWindow().findViewById(android.R.id.content);
  }

  @Override
  protected void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    mContainer = initLayout();
    rebuildWeexInstance();
  }

  protected void rebuildWeexInstance(){
    if (mInstance != null) {
      mInstance.destroy();
      mInstance = null;
    }

    mInstance = new WXSDKInstance(this);
    mInstance.registerRenderListener(this);

    String url = getBundleUrl();
    Map<String, Object> options = new HashMap<>();
    options.put("bundleUrl", url);
    mInstance.render(
      getPageName(),
      url,
      options,
      getJsonInitData(),
      ScreenUtil.getDisplayWidth(this),
      ScreenUtil.getDisplayHeight(this),
      WXRenderStrategy.APPEND_ASYNC);
  }

  protected String getPageName(){
    return TAG;
  }

  /**
   * Get bundle file url, http or file.
   * @return
   */
  protected abstract String getBundleUrl();

  protected String getJsonInitData(){
    return null;
  }

  @Override
  public void onStart() {
    super.onStart();
    if(mInstance!=null){
      mInstance.onActivityStart();
    }
  }

  @Override
  public void onResume() {
    super.onResume();
    if(mInstance!=null){
      mInstance.onActivityResume();
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    if(mInstance!=null){
      mInstance.onActivityPause();
    }
  }

  @Override
  public void onStop() {
    super.onStop();
    if(mInstance!=null){
      mInstance.onActivityStop();
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    if(mInstance!=null){
      mInstance.onActivityDestroy();
    }
  }

  @Override
  public void onViewCreated(WXSDKInstance wxsdkInstance, View view) {
    if (mContainer != null) {
      mContainer.addView(view);
    }
  }



  @Override
  public void onRefreshSuccess(WXSDKInstance wxsdkInstance, int i, int i1) {

  }
}
