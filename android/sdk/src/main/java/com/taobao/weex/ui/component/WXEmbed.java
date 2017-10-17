/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.ui.component;

import android.annotation.SuppressLint;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.taobao.weappplus_sdk.R;
import com.taobao.weex.IWXRenderListener;
import com.taobao.weex.WXRenderErrorCode;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
@Component(lazyload = false)
public class WXEmbed extends WXDiv implements WXSDKInstance.OnInstanceVisibleListener,NestedContainer {

  public static final String ITEM_ID = "itemId";

  private String src;
  private WXSDKInstance mNestedInstance;
  private static int ERROR_IMG_WIDTH = (int) WXViewUtils.getRealPxByWidth(270,750);
  private static int ERROR_IMG_HEIGHT = (int) WXViewUtils.getRealPxByWidth(260,750);

  private boolean mIsVisible = true;
  private EmbedRenderListener mListener;

  public interface EmbedManager {
    WXEmbed getEmbed(String itemId);
    void putEmbed(String itemId,WXEmbed comp);
  }

  public static class FailToH5Listener extends ClickToReloadListener {
    @SuppressLint("SetJavaScriptEnabled")
    @Override
    public void onException(NestedContainer comp, String errCode, String msg) {
      //downgrade embed
      if( errCode != null && comp instanceof WXEmbed && errCode.startsWith("1|")) {
        ViewGroup container = comp.getViewContainer();
        WebView webView = new WebView(container.getContext());
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        webView.setLayoutParams(params);
        webView.getSettings().setJavaScriptEnabled(true);

        //WebView Remote Code Execution Vulnerability
        webView.removeJavascriptInterface("searchBoxJavaBridge_");
        webView.removeJavascriptInterface("accessibility");
        webView.removeJavascriptInterface("accessibilityTraversal");
        webView.getSettings().setSavePassword(false);

        container.removeAllViews();
        container.addView(webView);
        webView.loadUrl(((WXEmbed) comp).src);
      }else{
        super.onException(comp,errCode,msg);
      }
    }
  }

  /**
   * Default event listener.
   */
  public static class ClickToReloadListener implements OnNestedInstanceEventListener {
    @Override
    public void onException(NestedContainer container, String errCode, String msg) {
      if (TextUtils.equals(errCode, WXRenderErrorCode.DegradPassivityCode.
			  WX_DEGRAD_ERR_NETWORK_BUNDLE_DOWNLOAD_FAILED.getDegradErrorCode()) && container instanceof WXEmbed) {
        final WXEmbed comp = ((WXEmbed)container);
        final ImageView imageView = new ImageView(comp.getContext());
        imageView.setImageResource(R.drawable.error);
        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(ERROR_IMG_WIDTH, ERROR_IMG_HEIGHT);
        layoutParams.gravity = Gravity.CENTER;
        imageView.setLayoutParams(layoutParams);
        imageView.setScaleType(ImageView.ScaleType.FIT_XY);
        imageView.setAdjustViewBounds(true);
        imageView.setOnClickListener(new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            imageView.setOnClickListener(null);
            imageView.setEnabled(false);
            comp.loadContent();
          }
        });
        FrameLayout hostView = comp.getHostView();
        hostView.removeAllViews();
        hostView.addView(imageView);
        WXLogUtils.e("WXEmbed", "NetWork failure :" + errCode + ",\n error message :" + msg);
      }
    }

    @Override
    public boolean onPreCreate(NestedContainer comp, String src) {
      return true;
    }

    @Override
    public String transformUrl(String origin) {
      return origin;
    }

    @Override
    public void onCreated(NestedContainer comp, WXSDKInstance nestedInstance) {

    }
  }

  static class EmbedRenderListener implements IWXRenderListener {
    WXEmbed mComponent;
    OnNestedInstanceEventListener mEventListener;

    EmbedRenderListener(WXEmbed comp) {
      mComponent = comp;
      mEventListener = new ClickToReloadListener();
    }

    @Override
    public void onViewCreated(WXSDKInstance instance, View view) {
      FrameLayout hostView = mComponent.getHostView();
      hostView.removeAllViews();
      hostView.addView(view);
    }

    @Override
    public void onRenderSuccess(WXSDKInstance instance, int width, int height) {

    }

    @Override
    public void onRefreshSuccess(WXSDKInstance instance, int width, int height) {

    }

    @Override
    public void onException(WXSDKInstance instance, String errCode, String msg) {
      if (mEventListener != null) {
        mEventListener.onException(mComponent, errCode, msg);
      }
    }
  }

  @Deprecated
  public WXEmbed(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance,dom,parent);
  }

  public WXEmbed(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
    mListener = new EmbedRenderListener(this);

    ERROR_IMG_WIDTH = (int) WXViewUtils.getRealPxByWidth(270,instance.getInstanceViewPortWidth());
    ERROR_IMG_HEIGHT = (int) WXViewUtils.getRealPxByWidth(260,instance.getInstanceViewPortWidth());
    if(instance instanceof EmbedManager) {
      Object itemId = node.getAttrs().get(ITEM_ID);
      if (itemId != null) {
        ((EmbedManager) instance).putEmbed(itemId.toString(), this);
      }
    }
  }

  @Override
  public void setOnNestEventListener(OnNestedInstanceEventListener listener){
    mListener.mEventListener = listener;
  }

  @Override
  public ViewGroup getViewContainer() {
    return getHostView();
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.SRC:
        String src = WXUtils.getString(param,null);
        if (src != null)
          setSrc(src);
        return true;
    }
    return super.setProperty(key, param);
  }

  @Override
  public void renderNewURL(String url) {
    src = url;
    loadContent();
  }

  @Override
  public void reload() {
    if (!TextUtils.isEmpty(src)) {
      loadContent();
    }
  }

  public String getOriginUrl() {
    return originUrl;
  }

  public void setOriginUrl(String originUrl) {
    this.originUrl = originUrl;
  }

  private String originUrl;

  @WXComponentProp(name = Constants.Name.SRC)
  public void setSrc(String src) {
    originUrl=src;
    this.src = src;
    if (mNestedInstance != null) {
      mNestedInstance.destroy();
      mNestedInstance = null;
    }
    if (mIsVisible && !TextUtils.isEmpty(this.src)) {
      loadContent();
    }
  }
  public String getSrc() {
    return src;
  }

  /**
   * Load embed content, default behavior is create a nested instance.
   */
  protected void loadContent(){
    mNestedInstance = createInstance();
    if(mListener != null && mListener.mEventListener != null){
      if(!mListener.mEventListener.onPreCreate(this,src)){
        //cancel render
        mListener.mEventListener.onCreated(this, mNestedInstance);
      }
    }
  }

  private WXSDKInstance createInstance() {
    WXSDKInstance sdkInstance = getInstance().createNestedInstance(this);
    getInstance().addOnInstanceVisibleListener(this);
    sdkInstance.registerRenderListener(mListener);

    String url=src;
    if(mListener != null && mListener.mEventListener != null){
      url=mListener.mEventListener.transformUrl(src);
      if(!mListener.mEventListener.onPreCreate(this,src)){
        //cancel render
        return null;
      }
    }

    if(TextUtils.isEmpty(url)){
      mListener.mEventListener.onException(this,
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getDegradErrorCode(),
			  WXRenderErrorCode.DegradPassivityCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getDegradErrorMsg() + "!!wx embed src url is null"
	  );
      return sdkInstance;
    }

    ViewGroup.LayoutParams layoutParams = getHostView().getLayoutParams();
    sdkInstance.renderByUrl(WXPerformance.DEFAULT,
                            url,
                            null, null, layoutParams.width,
                            layoutParams.height,
                            WXRenderStrategy.APPEND_ASYNC);
    return sdkInstance;
  }

  @Override
  public void setVisibility(String visibility) {
    super.setVisibility(visibility);
    boolean visible = TextUtils.equals(visibility, Constants.Value.VISIBLE);
    if (!TextUtils.isEmpty(src) && visible) {
      if (mNestedInstance == null) {
        loadContent();
      } else {
        mNestedInstance.onViewAppear();
      }
    }

    if (!visible) {
      if (mNestedInstance != null) {
        mNestedInstance.onViewDisappear();
      }
    }
    mIsVisible = visible;
  }

  @Override
  public void destroy() {
    super.destroy();
    if (mNestedInstance != null) {
      mNestedInstance.destroy();
      mNestedInstance = null;
    }
    src = null;
    if (getInstance() != null) {
      getInstance().removeOnInstanceVisibleListener(this);
    }
  }

  @Override
  public void onAppear() {
    //appear event from root instance will not trigger visibility change
    if(mIsVisible && mNestedInstance != null){
      WXComponent comp = mNestedInstance.getRootComponent();
      if(comp != null)
        comp.fireEvent(Constants.Event.VIEWAPPEAR);
    }
  }

  @Override
  public void onDisappear() {
    //appear event from root instance will not trigger visibility change
    if(mIsVisible && mNestedInstance != null){
      WXComponent comp = mNestedInstance.getRootComponent();
      if(comp != null)
        comp.fireEvent(Constants.Event.VIEWDISAPPEAR);
    }
  }

  @Override
  public void onActivityStart() {
    super.onActivityStart();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityStart();
    }
  }

  @Override
  public void onActivityResume() {
    super.onActivityResume();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityResume();
    }
  }

  @Override
  public void onActivityPause() {
    super.onActivityPause();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityPause();
    }
  }

  @Override
  public void onActivityStop() {
    super.onActivityStop();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityStop();
    }
  }

  @Override
  public void onActivityDestroy() {
    super.onActivityDestroy();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityDestroy();
    }
  }
}
