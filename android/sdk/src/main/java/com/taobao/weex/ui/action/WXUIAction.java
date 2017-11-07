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

import java.util.HashMap;
import java.util.List;
import java.util.Set;

public abstract class WXUIAction {

  public String mPageId = null;
  public String mComponentType = null;
  public String mParentRef = null;
  public String mRef = null;
  public int mIndex = -1;
  public WXUIPosition mLayoutPosition = new WXUIPosition(0, 0, 0, 0);
  public WXUISize mLayoutSize = new WXUISize(0, 0);
  public String mKey = null;
  public String mValue = null;
  public HashMap<String, String> mStyle;
  public HashMap<String, String> mAttributes;
  public Set<String> mEvents;
  public HashMap<String, String> mPaddings;
  public HashMap<String, String> mMargins;
  public HashMap<String, String> mBorders;

  public abstract void executeAction();

  protected WXComponent createComponent(WXSDKInstance instance, WXVContainer parent) {

    WXComponent component = WXComponentFactory.newInstanceByWeexCore(instance, parent, this);
    WXSDKManager.getInstance().getWXRenderManager().registerComponent(mPageId, mRef, component);

    if (component != null) {
      component.setStyle(mStyle);
      component.setAttr(mAttributes);
      component.setEvent(mEvents);
      component.setSpacing(mPaddings);
      component.setSpacing(mMargins);
      component.setSpacing(mBorders);
    }

    return component;
  }
}
