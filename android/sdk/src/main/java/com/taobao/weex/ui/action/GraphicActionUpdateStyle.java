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

import java.util.Map;

public class GraphicActionUpdateStyle extends BasicGraphicAction {

  private Map<String, String> mStyle;
  private Map<String, String> mPaddings;
  private Map<String, String> mMargins;
  private Map<String, String> mBorders;
  private WXComponent component;

  public GraphicActionUpdateStyle(String pageId, String ref,
                                  Map<String, String> style,
                                  Map<String, String> paddings,
                                  Map<String, String> margins,
                                  Map<String, String> borders) {
    super(pageId, ref);
    this.mStyle = style;
    this.mPaddings = paddings;
    this.mMargins = margins;
    this.mBorders = borders;

    component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if (component == null) {
      return;
    }
    if (mStyle != null) {
      component.updateStyles(mStyle);
      component.updateCSSShorthand(mPaddings);
      component.updateCSSShorthand(mMargins);
      component.updateCSSShorthand(mBorders);
    }
  }

  @Override
  public void executeAction() {
    if (component == null) {
      return;
    }
    component.setLayout(component);
  }
}
