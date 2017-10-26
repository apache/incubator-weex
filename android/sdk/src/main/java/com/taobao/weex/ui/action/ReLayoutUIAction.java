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
import com.taobao.weex.utils.WXLogUtils;

public class ReLayoutUIAction extends WXUIAction {

  @Override
  public void executeAction() {
    //Create component in dom thread
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(mPageId);
    if (instance == null || instance.getContext() == null) {
      WXLogUtils.e("instance is null or instance is destroy!");
      return;
    }

    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(mPageId, mRef);
    if (component == null) {
      return;
    }

    component.requestLayout(this);
  }
}
