/*
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
package com.taobao.weex.ui.animation;

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.ui.component.WXComponent;


import static com.taobao.weex.dom.action.Actions.getAnimationAction;

public class WXAnimationModule extends WXModule {

  @JSMethod
  public void transition(@Nullable String ref, @Nullable JSONObject animation, @Nullable String callBack) {
    if (!TextUtils.isEmpty(ref) && animation != null && mWXSDKInstance != null) {
      DOMAction animationActions = getAnimationAction(ref, animation, callBack);
      //Due to animation module rely on the result of the css-layout and the batch mechanism of
      //css-layout, the animation.transition must be delayed the batch time.
      WXSDKManager.getInstance().getWXDomManager().postActionDelay(mWXSDKInstance.getInstanceId(),
                                                                   animationActions,
                                                                   false, WXDomHandler.DELAY_TIME);
    }
  }

  //add by moxun on 12/26/2016
  public static class AnimationHolder {

    private WXAnimationBean wxAnimationBean;
    private String callback;

    public void execute(WXSDKInstance mInstance, WXComponent component) {
      RenderAction action = Actions.getAnimationAction(component.getRef(), wxAnimationBean, callback);
      WXSDKManager.getInstance().getWXRenderManager().runOnThread(mInstance.getInstanceId(), action);
    }

    public AnimationHolder(WXAnimationBean wxAnimationBean, String callback) {
      this.wxAnimationBean = wxAnimationBean;
      this.callback = callback;
    }
  }

}
