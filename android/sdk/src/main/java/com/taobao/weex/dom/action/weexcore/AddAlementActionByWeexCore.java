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
package com.taobao.weex.dom.action.weexcore;

import android.graphics.Color;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by miomin on 2017/8/15.
 */

public class AddAlementActionByWeexCore extends WeexCoreAction {

  static int count = 1;

  @Override
  public void excuteAction() {

    //Create component in dom thread
    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(mPageId);
    if (instance == null || instance.getContext() == null) {
      WXLogUtils.e("instance is null or instance is destroy!");
      return;
    }

    WXComponent component = createComponent(instance, null);
    if (component == null) {
      //stop redner, some fatal happened.
      return;
    }

    if (instance == null || instance.getContext() == null) {
      WXLogUtils.e("instance is null or instance is destroy!");
      return;
    }
    try {
      WXVContainer parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager().getWXComponent(mPageId, mParentRef);

      if (parent == null || component == null) {
        return;
      }

      parent.addChild(component, mIndex);
      parent.createChildViewAt(mIndex);
      component.applyLayoutAndEvent();
      component.bindData(component);
//            if (count == 0) {
//                component.getHostView().setBackgroundColor(Color.parseColor("#766556"));
//            } else if (count == 1) {
//                component.getHostView().setBackgroundColor(Color.parseColor("#898987"));
//            } else if (count == 2) {
//                component.getHostView().setBackgroundColor(Color.parseColor("#675645"));
//            } else if (count == 3) {
//                component.getHostView().setBackgroundColor(Color.parseColor("#755665"));
//                count = 0;
//            }
//            count++;
    } catch (Exception e) {
      WXLogUtils.e("add component failed.", e);
    }
  }
}
