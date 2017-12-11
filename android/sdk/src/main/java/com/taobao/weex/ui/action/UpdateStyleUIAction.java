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

  private String mPageId = null;
  private String mRef = null;
  private Map<String, String> mStyle;
  private HashMap<String, String> mPaddings;
  private HashMap<String, String> mMargins;
  private HashMap<String, String> mBorders;

  public UpdateStyleUIAction(String pageId, String ref,
                             Map<String, String> style,
                             HashMap<String, String> paddings,
                             HashMap<String, String> margins,
                             HashMap<String, String> borders) {
    this.mPageId = pageId;
    this.mRef = ref;
    this.mStyle = style;
    this.mPaddings = paddings;
    this.mMargins = margins;
    this.mBorders = borders;
  }

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(mPageId, mRef);
    if (component == null) {
      return;
    }
    if (mStyle != null) {
      component.updateStyles(mStyle);
      component.updateCSSShorthand(mPaddings);
      component.updateCSSShorthand(mMargins);
      component.updateCSSShorthand(mBorders);
      component.setLayout(component);
    }
  }

  public String getmPageId() {
    return mPageId;
  }

  public void setmPageId(String mPageId) {
    this.mPageId = mPageId;
  }

  public String getmRef() {
    return mRef;
  }

  public void setmRef(String mRef) {
    this.mRef = mRef;
  }

  public Map<String, String> getmStyle() {
    return mStyle;
  }

  public void setmStyle(Map<String, String> mStyle) {
    this.mStyle = mStyle;
  }

  public HashMap<String, String> getmPaddings() {
    return mPaddings;
  }

  public void setmPaddings(HashMap<String, String> mPaddings) {
    this.mPaddings = mPaddings;
  }

  public HashMap<String, String> getmMargins() {
    return mMargins;
  }

  public void setmMargins(HashMap<String, String> mMargins) {
    this.mMargins = mMargins;
  }

  public HashMap<String, String> getmBorders() {
    return mBorders;
  }

  public void setmBorders(HashMap<String, String> mBorders) {
    this.mBorders = mBorders;
  }
}
