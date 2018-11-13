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
import android.text.TextUtils;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

public class GraphicActionMoveElement extends BasicGraphicAction {

  private String mParentref;
  private int mIndex;

  public GraphicActionMoveElement(WXSDKInstance instance, String ref, String parentRef, int index) {
    super(instance, ref);
    this.mParentref = parentRef;
    this.mIndex = index;
  }

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if(component == null) {
      return;
    }
    WXVContainer oldParent = component.getParent();
    WXComponent newParent = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentref);
    if (oldParent == null
            || newParent == null || !(newParent instanceof WXVContainer)) {
      return;
    }

    if (component.getHostView() != null && !TextUtils.equals(component.getComponentType(), "video") && !TextUtils.equals(component.getComponentType(), "videoplus")) {
      int[] location = new  int[2] ;
      component.getHostView().getLocationInWindow(location);
    }

    oldParent.remove(component, false);

    ((WXVContainer) newParent).addChild(component, mIndex);

    if (component.getHostView() != null && !TextUtils.equals(component.getComponentType(), "video") && !TextUtils.equals(component.getComponentType(), "videoplus")) {
      int[] location = new  int[2] ;
      component.getHostView().getLocationInWindow(location);
    }

    if (!component.isVirtualComponent()) {
      ((WXVContainer) newParent).addSubView(component.getHostView(), mIndex);
    }
  }
}
