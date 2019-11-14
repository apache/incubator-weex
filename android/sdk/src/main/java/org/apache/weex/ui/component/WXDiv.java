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
package org.apache.weex.ui.component;

import android.content.Context;
import android.support.annotation.NonNull;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.ui.ComponentCreator;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.flat.FlatComponent;
import org.apache.weex.ui.flat.WidgetContainer;
import org.apache.weex.ui.flat.widget.WidgetGroup;
import org.apache.weex.ui.view.WXFrameLayout;

import java.lang.reflect.InvocationTargetException;
import java.util.LinkedList;

/**
 * div component
 */
@Component(lazyload = false)
public class WXDiv extends WidgetContainer<WXFrameLayout> implements FlatComponent<WidgetGroup> {

  private WidgetGroup mWidgetGroup;

  public static class Ceator implements ComponentCreator {
    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXDiv(instance, parent, basicComponentData);
    }
  }

  @Deprecated
  public WXDiv(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXDiv(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
  }

  @Override
  protected WXFrameLayout initComponentHostView(@NonNull Context context) {
    WXFrameLayout frameLayout = new WXFrameLayout(context);
    frameLayout.holdComponent(this);
    return frameLayout;
  }

  @Override
  public boolean promoteToView(boolean checkAncestor) {
    if (null != getInstance().getFlatUIContext()) {
      return !intendToBeFlatContainer() ||
            getInstance().getFlatUIContext().promoteToView(this, checkAncestor, WXDiv.class);
    }
    return false;
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
    if(widgets == null){
      widgets = new LinkedList<>();
    }
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
