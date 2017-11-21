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

import android.widget.ScrollView;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXScroller;
import com.taobao.weex.utils.WXLogUtils;

public class CreateBodyUIAction extends WXUIAction {

  @Override
  public void executeAction() {
    //Create component in dom thread
    final WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(mPageId);
    if (instance == null || instance.getContext() == null) {
      return;
    }

    final WXComponent component = createComponent(instance, null);
    if (component == null) {
      return;
    }

    WXSDKManager.getInstance().getSDKInstance(mPageId).nativeBindComponentToWXCore(mPageId, component, mRef);

    try {
      component.createView();

      if (component instanceof WXScroller) {
        WXScroller scroller = (WXScroller) component;
        if (scroller.getInnerView() instanceof ScrollView) {
          instance.setRootScrollView((ScrollView) scroller.getInnerView());
        }
      }

      instance.onRootCreated(component);

      if (instance.getRenderStrategy() != WXRenderStrategy.APPEND_ONCE) {
        instance.onCreateFinish();
      }
    } catch (Exception e) {
      WXLogUtils.e("create body failed.", e);
    }
  }
}
