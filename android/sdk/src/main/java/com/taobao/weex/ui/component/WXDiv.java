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
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.flat.FlatComponent;
import com.taobao.weex.ui.flat.WidgetContainer;
import com.taobao.weex.ui.flat.widget.WidgetGroup;
import com.taobao.weex.ui.view.WXFrameLayout;
import java.lang.reflect.InvocationTargetException;

/**
 * div component
 */
@Component(lazyload = false)
public class WXDiv extends WidgetContainer<WXFrameLayout> implements FlatComponent<WidgetGroup> {

  private WidgetGroup mWidgetGroup;

  public static class Ceator implements ComponentCreator {

    public WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent)
        throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXDiv(instance, node, parent);
    }
  }

  @Deprecated
  public WXDiv(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId,
      boolean isLazy) {
    this(instance, dom, parent);
  }

  public WXDiv(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
  }


  @Override
  protected WXFrameLayout initComponentHostView(@NonNull Context context) {
    WXFrameLayout frameLayout = new WXFrameLayout(context);
    frameLayout.holdComponent(this);
    return frameLayout;
  }

  @Override
  public boolean promoteToView(boolean checkAncestor) {
    return !intendToBeFlatContainer() ||
        getInstance().getFlatUIContext().promoteToView(this, checkAncestor, WXDiv.class);
  }

  /**
   * Create View tree there. Either this method or {@link #createViewImpl()} get called.
   * If this object will be promoted to view, then getOrCreateFlatWidget() should never be called.
   */
  @Override
  @NonNull
  public WidgetGroup getOrCreateFlatWidget() {
    if (mWidgetGroup == null) {
      mWidgetGroup = new WidgetGroup(getInstance().getFlatUIContext());
      for (int i = 0; i < getChildCount(); i++) {
        createChildViewAt(i);
      }
      mountFlatGUI();
    }
    return mWidgetGroup;
  }

  @Override
  protected void mountFlatGUI() {
    if (promoteToView(true)) {
      if(getHostView()!=null) {
        getHostView().mountFlatGUI(widgets);
      }
    } else {
      mWidgetGroup.replaceAll(widgets);
    }
  }

  @Override
  public void unmountFlatGUI() {
    if (getHostView() != null) {
      getHostView().unmountFlatGUI();
    }
  }

  @Override
  public boolean intendToBeFlatContainer() {
    return getInstance().getFlatUIContext().isFlatUIEnabled(this) && WXDiv.class.equals(getClass());
  }

  @Override
  public boolean isVirtualComponent() {
    return !promoteToView(true);
  }

}
