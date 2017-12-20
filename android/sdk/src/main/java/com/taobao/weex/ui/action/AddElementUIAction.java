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
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class AddElementUIAction extends AbstractAddElementUIAction {

  public AddElementUIAction(String pageId, String ref,
                            String componentType, String parentRef,
                            int index,
                            Map<String, String> style,
                            Map<String, String> attributes,
                            Set<String> events,
                            HashMap<String, String> paddings,
                            HashMap<String, String> margins,
                            HashMap<String, String> borders) {
    super(pageId, ref);
    this.mComponentType = componentType;
    this.mParentRef = parentRef;
    this.mIndex = index;
    this.mStyle = style;
    this.mAttributes = attributes;
    this.mEvents = events;
    this.mPaddings = paddings;
    this.mMargins = margins;
    this.mBorders = borders;
  }

  @Override
  public void executeAction() {
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (instance == null || instance.getContext() == null) {
      return;
    }

    final WXVContainer parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentRef);
    WXComponent child = createComponent(instance, parent);

    if (child == null || parent == null) {
      return;
    }

    WXSDKManager.getInstance().getSDKInstance(getPageId()).nativeBindComponentToWXCore(getPageId(), child, getRef());

    try {
      parent.addChild(child, mIndex);
      parent.createChildViewAt(mIndex);
    } catch (Exception e) {
      WXLogUtils.e("add component failed.", e);
    }
  }
}
