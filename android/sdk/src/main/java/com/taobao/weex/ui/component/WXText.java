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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.annotation.NonNull;
import android.support.v4.content.LocalBroadcastManager;
import android.text.Layout;
import android.view.ViewGroup;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.view.WXTextView;
import com.taobao.weex.utils.FontDO;
import com.taobao.weex.utils.TypefaceUtil;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.reflect.InvocationTargetException;

/**
 * Text component
 */
@Component(lazyload = false)
public class WXText extends WXComponent<WXTextView> {

  /**
   * The default text size
   **/
  public static final int sDEFAULT_SIZE = 32;
  private BroadcastReceiver mTypefaceObserver;
  private String mFontFamily;

  public static class Creator implements ComponentCreator {

    public WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXText(instance, node, parent);
    }
  }

  @Deprecated
  public WXText(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance, dom, parent);
  }

  public WXText(WXSDKInstance instance, WXDomObject node,
                WXVContainer parent) {
    super(instance, node, parent);
  }

  @Override
  protected WXTextView initComponentHostView(@NonNull Context context) {
    WXTextView textView =new WXTextView(context);
    textView.holdComponent(this);
    return textView;
  }

  @Override
  public void updateExtra(Object extra) {
    if (extra instanceof Layout &&
        getHostView() != null && !extra.equals(getHostView().getTextLayout())) {
      final Layout layout = (Layout) extra;
      getHostView().setTextLayout(layout);
      getHostView().invalidate();
    }
  }

  @Override
  protected void setAriaLabel(String label) {
    WXTextView text = getHostView();
    if(text != null){
      text.setAriaLabel(label);
    }
  }

  @Override
  public void refreshData(WXComponent component) {
    super.refreshData(component);
    if (component instanceof WXText) {
      updateExtra(component.getDomObject().getExtra());
    }
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.LINES:
      case Constants.Name.FONT_SIZE:
      case Constants.Name.FONT_WEIGHT:
      case Constants.Name.FONT_STYLE:
      case Constants.Name.COLOR:
      case Constants.Name.TEXT_DECORATION:
      case Constants.Name.TEXT_ALIGN:
      case Constants.Name.TEXT_OVERFLOW:
      case Constants.Name.LINE_HEIGHT:
      case Constants.Name.VALUE:
        return true;
      case Constants.Name.FONT_FAMILY:
        if (param != null) {
          registerTypefaceObserver(param.toString());
        }
        return true;
      default:
        return super.setProperty(key, param);
    }
  }

  /**
   * Flush view no matter what height and width the {@link WXDomObject} specifies.
   * @param extra must be a {@link Layout} object, otherwise, nothing will happen.
   */
  private void flushView(Object extra) {
    if (extra instanceof Layout &&
        getHostView() != null && !extra.equals(getHostView().getTextLayout())) {
      final Layout layout = (Layout) extra;
      /**The following if block change the height of the width of the textView.
       * other part of the code is the same to updateExtra
       */
      ViewGroup.LayoutParams layoutParams = getHostView().getLayoutParams();
      if (layoutParams != null) {
        layoutParams.height = layout.getHeight();
        layoutParams.width = layout.getWidth();
        getHostView().setLayoutParams(layoutParams);
      }
      getHostView().setTextLayout(layout);
      getHostView().invalidate();
    }
  }

  @Override
  protected Object convertEmptyProperty(String propName, Object originalValue) {
    switch (propName) {
      case Constants.Name.FONT_SIZE:
        return WXText.sDEFAULT_SIZE;
      case Constants.Name.COLOR:
        return "black";
    }
    return super.convertEmptyProperty(propName, originalValue);
  }

  @Override
  public void destroy() {
    super.destroy();
    if (getContext() != null && mTypefaceObserver != null) {
      LocalBroadcastManager.getInstance(getContext()).unregisterReceiver(mTypefaceObserver);
    }
  }

  private void registerTypefaceObserver(String desiredFontFamily) {
    if (getContext() == null) {
      WXLogUtils.w("WXText", "Content is null on register typeface observer");
    }
    mFontFamily = desiredFontFamily;
    mTypefaceObserver = new BroadcastReceiver() {
      @Override
      public void onReceive(Context context, Intent intent) {
        String fontFamily = intent.getStringExtra("fontFamily");
        if (!mFontFamily.equals(fontFamily)) {
          return;
        }

        FontDO fontDO = TypefaceUtil.getFontDO(fontFamily);
        if (fontDO != null && fontDO.getTypeface() != null) {
          Layout layout = getHostView().getTextLayout();
          if (layout != null) {
            layout.getPaint().setTypeface(fontDO.getTypeface());
            WXLogUtils.d("WXText", "Apply font family " + fontFamily + " to paint");
          } else {
            WXLogUtils.w("WXText", "Layout not created");
          }
          getHostView().invalidate();
        }
        WXLogUtils.d("WXText", "Font family " + fontFamily + " is available");
      }
    };

    LocalBroadcastManager.getInstance(getContext()).registerReceiver(mTypefaceObserver, new IntentFilter(TypefaceUtil.ACTION_TYPE_FACE_AVAILABLE));
  }
}
