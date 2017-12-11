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

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.ui.component.WXComponent;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class UpdateStyleUIAction extends BasicUIAction {

  public String mPageId = null;
  public String mRef = null;
  public Map<String, String> mStyle;
  public Map<String, String> mAttributes;
  public Set<String> mEvents;
  public HashMap<String, String> mPaddings;
  public HashMap<String, String> mMargins;
  public HashMap<String, String> mBorders;

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(mPageId, mRef);
    if (component == null) {
      return;
    }
    if (mStyle != null) {
      component.updateStyles(mStyle);
      component.updateAttrs(mAttributes);
      component.updateCSSShorthand(mPaddings);
      component.updateCSSShorthand(mMargins);
      component.updateCSSShorthand(mBorders);
      component.setLayout(component);
    }
  }
}
