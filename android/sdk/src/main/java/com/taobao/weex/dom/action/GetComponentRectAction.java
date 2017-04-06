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
package com.taobao.weex.dom.action;

import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.bridge.SimpleJSCallback;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 02/03/2017.
 */
class GetComponentRectAction implements RenderAction {
  private final String mRef;
  private final String mCallback;


  GetComponentRectAction(String ref, String callback) {
    this.mRef = ref;
    this.mCallback = callback;
  }
  @Override
  public void executeRender(RenderActionContext context) {
    JSCallback jsCallback = new SimpleJSCallback(context.getInstance().getInstanceId(), mCallback);
    if (context.getInstance().isDestroy()) {
      //do nothing
    }else if (TextUtils.isEmpty(mRef)) {
      Map<String, Object> options = new HashMap<>();
      options.put("result", false);
      options.put("errMsg", "Illegal parameter");
      jsCallback.invoke(options);
    } else if ("viewport".equalsIgnoreCase(mRef)) {
      callbackViewport(context, jsCallback);
    } else {
      WXComponent component = context.getComponent(mRef);
      Map<String, Object> options = new HashMap<>();
      if (component != null) {
        Map<String, Float> size = new HashMap<>();
        Rect sizes = component.getComponentSize();
        size.put("width", getWebPxValue(sizes.width()));
        size.put("height", getWebPxValue(sizes.height()));
        size.put("bottom", getWebPxValue(sizes.bottom));
        size.put("left", getWebPxValue(sizes.left));
        size.put("right", getWebPxValue(sizes.right));
        size.put("top", getWebPxValue(sizes.top));
        options.put("size", size);
        options.put("result", true);
      } else {
        options.put("errMsg", "Component does not exist");
      }
      jsCallback.invoke(options);
    }
  }

  private void callbackViewport(RenderActionContext context, JSCallback jsCallback) {
    WXSDKInstance instance = context.getInstance();
    View container;
    if ((container = instance.getContainerView()) != null) {
      Map<String, Object> options = new HashMap<>();
      Map<String, Float> sizes = new HashMap<>();
      int[] location = new int[2];
      instance.getContainerView().getLocationOnScreen(location);
      sizes.put("left", 0f);
      sizes.put("top", 0f);
      sizes.put("right", getWebPxValue(container.getWidth()));
      sizes.put("bottom", getWebPxValue(container.getHeight()));
      sizes.put("width", getWebPxValue(container.getWidth()));
      sizes.put("height", getWebPxValue(container.getHeight()));
      options.put("size", sizes);
      options.put("result", true);
      jsCallback.invoke(options);
    } else {
      Map<String, Object> options = new HashMap<>();
      options.put("result", false);
      options.put("errMsg", "Component does not exist");
      jsCallback.invoke(options);
    }
  }

  @NonNull
  private float getWebPxValue(int value) {
    return WXViewUtils.getWebPxByWidth(value, WXSDKInstance.getViewPortWidth());
  }


}
