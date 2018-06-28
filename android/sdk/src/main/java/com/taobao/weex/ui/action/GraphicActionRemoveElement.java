/**
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
package com.taobao.weex.ui.action;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

public class GraphicActionRemoveElement extends BasicGraphicAction {

  private boolean isJSCreateFinish = false;

  public GraphicActionRemoveElement(String pageId, String ref) {
    super(pageId, ref);
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (null != instance){
      isJSCreateFinish = instance.isJSCreateFinish;
    }

  }

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if (component == null || component.getParent() == null) {
      return;
    }
    clearRegistryForComponent(component);
    WXVContainer parent = component.getParent();
    parent.remove(component, true);
  }

  private void clearRegistryForComponent(WXComponent component) {
    WXComponent removedComponent = WXSDKManager.getInstance().getWXRenderManager().unregisterComponent(getPageId(), getRef());
    if (removedComponent != null) {
      removedComponent.removeAllEvent();
      removedComponent.removeStickyStyle();
    }
    if (component instanceof WXVContainer) {
      WXVContainer container = (WXVContainer) component;
      int count = container.childCount();
      for (int i = count - 1; i >= 0; --i) {
        clearRegistryForComponent(container.getChild(i));
      }
    }
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (null!=instance){
      instance.onElementChange(isJSCreateFinish);
    }
  }
}
