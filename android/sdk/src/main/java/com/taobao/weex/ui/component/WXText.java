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
import android.text.Layout;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.flat.FlatComponent;
import com.taobao.weex.ui.flat.widget.TextWidget;
import com.taobao.weex.ui.view.WXTextView;
import com.taobao.weex.utils.WXUtils;

import java.lang.reflect.InvocationTargetException;

/**
 * Text component
 */
@Component(lazyload = false)
public class WXText extends WXComponent<WXTextView> implements FlatComponent<TextWidget> {

  private TextWidget mTextWidget;

  /**
   * The default text size
   **/
  public static final int sDEFAULT_SIZE = 32;

  @Override
  public boolean promoteToView(boolean checkAncestor) {
    return getInstance().getFlatUIContext().promoteToView(this, checkAncestor, WXText.class);
  }

  @Override
  @NonNull
  public TextWidget getOrCreateFlatWidget() {
    if (mTextWidget == null) {
      mTextWidget = new TextWidget(getInstance().getFlatUIContext());
    }
    return mTextWidget;
  }

  @Override
  public boolean isVirtualComponent() {
    return !promoteToView(true);
  }

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
    if(extra instanceof Layout) {
      final Layout layout = (Layout) extra;
      if (!promoteToView(true)) {
        getOrCreateFlatWidget().updateTextDrawable(layout);
      } else if (getHostView() != null && !extra.equals(getHostView().getTextLayout())) {
        getHostView().setTextLayout(layout);
        getHostView().invalidate();
      }
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
        return true;
      case Constants.Name.ENABLE_COPY:
        boolean enabled = WXUtils.getBoolean(param, false);
        if (getHostView() != null) {
          getHostView().enableCopy(enabled);
        }
        return true;
      default:
        return super.setProperty(key, param);
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
  protected void createViewImpl() {
    if(promoteToView(true)) {
      super.createViewImpl();
    }
  }

  @Override
  public void destroy() {
    super.destroy();
  }

}
