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
package org.apache.weex.ui.animation;

import android.support.annotation.Nullable;
import android.text.TextUtils;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.common.WXModule;
import org.apache.weex.ui.action.GraphicActionAnimation;
import org.apache.weex.ui.component.WXComponent;

public class WXAnimationModule extends WXModule {

  @JSMethod
  public void transition(@Nullable String ref, @Nullable String animation, @Nullable String callBack) {
    if (!TextUtils.isEmpty(ref) && !TextUtils.isEmpty(animation) && mWXSDKInstance != null) {
      //Due to animation module rely on the result of the css-layout and the batch mechanism of
      //css-layout, the animation.transition must be delayed the batch time.
      GraphicActionAnimation action = new GraphicActionAnimation(mWXSDKInstance, ref, animation, callBack);
      WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
    }
  }


 

  //add by moxun on 12/26/2016
  public static class AnimationHolder {

    private WXAnimationBean wxAnimationBean;
    private String callback;

    public void execute(WXSDKInstance instance, WXComponent component) {
      if (null != instance && null != component) {
        GraphicActionAnimation action = new GraphicActionAnimation(instance, component.getRef(), wxAnimationBean, callback);
        WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(action.getPageId(), action);
      }
    }

    public AnimationHolder(WXAnimationBean wxAnimationBean, String callback) {
      this.wxAnimationBean = wxAnimationBean;
      this.callback = callback;
    }
  }

}
