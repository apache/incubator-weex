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
package com.taobao.weex.ui.prerenderaction;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.ui.action.GraphicActionAnimation;

public class PrerenderGraphicActionAnimation extends GraphicActionAnimation {


  PrerenderGraphicActionAnimation(@NonNull WXSDKInstance instance, @NonNull String ref, @Nullable String animation,
                                         @Nullable final String callBack) {
    super(instance, ref, animation, callBack);

  }

  @Override
  public void executeAction() {

    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (instance == null || instance.getNeedInterceptRender()) {
      return;
    }

    super.executeAction();
  }
}