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

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

public class AddElementAction extends AbstractAddElementUIAction {

  private JSONObject mData;

  public AddElementAction(String pageId, String parentRef,
                          JSONObject data, int index) {
    super(pageId, null);
    this.mParentRef = parentRef;
    this.mData = data;
    this.mIndex = index;
  }

  @Override
  public void executeAction() {
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (instance == null || instance.getContext() == null) {
      return;
    }

    final WXVContainer parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentRef);

    if (this.mData == null || this.mData.size() <= 0) {
      return;
    }

    this.mComponentType = (String) this.mData.get("type");
    setRef((String) mData.get("ref"));
    CommonCompData commonCompData = new CommonCompData(getPageId(), null, null, getParentRef());
    WXComponent child = createComponent(instance, parent, commonCompData);

    if (child == null || parent == null) {
      return;
    }

    Object style = mData.get("style");
    if (style != null && style instanceof JSONObject) {
      WXStyle styles = new WXStyle();
      styles.putAll((JSONObject) style, false);
      child.addStyle(styles);
    }
    Object attr = mData.get("attr");
    if (attr != null && attr instanceof JSONObject) {
      WXAttr attrs = new WXAttr((JSONObject) attr);
      child.addAttr(attrs);
    }
    Object event = mData.get("event");
    if (event != null && event instanceof JSONArray) {
      WXEvent events = new WXEvent();
      JSONArray eventArray = (JSONArray) event;
      int count = eventArray.size();
      for (int i = 0; i < count; i++) {
        Object value = eventArray.get(i);
        events.add(value.toString());
      }
      child.addEvent(events);
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
