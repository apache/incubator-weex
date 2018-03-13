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

import android.content.Context;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;
import android.widget.FrameLayout;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.view.WXCircleIndicator;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

/**
 *
 * Slider indicator
 */
@Component(lazyload = false)
public class WXIndicator extends WXComponent<WXCircleIndicator> {

  @Deprecated
  public WXIndicator(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }

  public WXIndicator(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
    super(instance, parent, isLazy, basicComponentData);
  }

  @Override
  protected void setHostLayoutParams(WXCircleIndicator host, int width, int height, int left, int right, int top, int bottom) {
      FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(width, height);
      params.setMargins(left, top, right, bottom);
      host.setLayoutParams(params);
  }

  @Override
  protected WXCircleIndicator initComponentHostView(@NonNull Context context) {
    WXCircleIndicator view = new WXCircleIndicator(context);
    if (getParent() instanceof WXSlider) {
      return view;
    } else {
      if (WXEnvironment.isApkDebugable()) {
        throw new WXRuntimeException("WXIndicator initView error.");
      }
    }
    return null;
  }

  @Override
  protected void onHostViewInitialized(WXCircleIndicator host) {
    super.onHostViewInitialized(host);
    if (getParent() instanceof WXSlider) {
      ((WXSlider) getParent()).addIndicator(this);
    }
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.ITEM_COLOR:
        String item_color = WXUtils.getString(param,null);
        if (item_color != null)
          setItemColor(item_color);
        return true;
      case Constants.Name.ITEM_SELECTED_COLOR:
        String selected_color = WXUtils.getString(param,null);
        if (selected_color != null)
          setItemSelectedColor(selected_color);
        return true;
      case Constants.Name.ITEM_SIZE:
        Integer item_size = WXUtils.getInteger(param,null);
        if (item_size != null)
          setItemSize(item_size);
        return true;
    }
    return super.setProperty(key, param);
  }


  @WXComponentProp(name = Constants.Name.ITEM_COLOR)
  public void setItemColor(String itemColor) {
    if (!TextUtils.isEmpty(itemColor)) {
      int colorInt = WXResourceUtils.getColor(itemColor);
      if (colorInt != Integer.MIN_VALUE) {
        getHostView().setPageColor(colorInt);
        getHostView().forceLayout();
        getHostView().requestLayout();
      }
    }
  }

  @WXComponentProp(name = Constants.Name.ITEM_SELECTED_COLOR)
  public void setItemSelectedColor(String itemSelectedColor) {
    if (!TextUtils.isEmpty(itemSelectedColor)) {
      int colorInt = WXResourceUtils.getColor(itemSelectedColor);
      if (colorInt != Integer.MIN_VALUE) {
        getHostView().setFillColor(colorInt);
        getHostView().forceLayout();
        getHostView().requestLayout();
      }
    }
  }

  @WXComponentProp(name = Constants.Name.ITEM_SIZE)
  public void setItemSize(int itemSize) {
    if (itemSize < 0) {
      return;
    }
    getHostView().setRadius(WXViewUtils.getRealPxByWidth(itemSize,getInstance().getInstanceViewPortWidth()) / 2.0f);
    getHostView().forceLayout();
    getHostView().requestLayout();
  }

  public void setShowIndicators(boolean show) {
    if (getHostView() == null) {
      return;
    }
    if (show) {
      getHostView().setVisibility(View.VISIBLE);
    } else {
      getHostView().setVisibility(View.GONE);
    }
  }

  // TODO
//  public static class IndicatorDomNode extends WXDomObject{
//    public IndicatorDomNode(){
//      super();
//    }
//
//    @Override
//    protected Map<String, String> getDefaultStyle() {
//      WXStyle pendingStyles = getStyles();
//      Map<String,String> map = new HashMap<>();
//      if(!pendingStyles.containsKey(Constants.Name.RIGHT))
//        map.put(Constants.Name.LEFT,"0");
//      if(!pendingStyles.containsKey(Constants.Name.BOTTOM))
//        map.put(Constants.Name.TOP,"0");
//      return map;
//    }
//  }
}
