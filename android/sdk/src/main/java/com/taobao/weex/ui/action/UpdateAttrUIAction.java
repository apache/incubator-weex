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

public class UpdateAttrUIAction extends BasicUIAction {

  private String mPageId = null;
  private String mRef = null;
  private Map<String, String> mAttrs;

  public UpdateAttrUIAction(String pageId, String ref,
                            Map<String, String> attrs) {
    this.mPageId = pageId;
    this.mRef = ref;
    this.mAttrs = attrs;
  }

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(mPageId, mRef);
    if (component == null) {
      return;
    }
    if (mAttrs != null) {
      component.updateAttrs(mAttrs);
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

  public Map<String, String> getAttrs() {
    return mAttrs;
  }

  public void setAttrs(Map<String, String> attrs) {
    this.mAttrs = attrs;
  }
}
