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
package org.apache.weex.ui.action;

import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.bridge.JSCallback;
import org.apache.weex.bridge.SimpleJSCallback;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by listen on 18/01/10.
 */
public class ActionGetComponentRect extends BasicGraphicAction {

  private final String mCallback;

  public ActionGetComponentRect(WXSDKInstance instance, String ref, String callback) {
    super(instance, ref);
    this.mCallback = callback;
  }

  @Override
  public void executeAction() {
    WXSDKInstance instance = getWXSDKIntance();
    if (instance == null || instance.isDestroy()) {
      return;
    }

    JSCallback jsCallback = new SimpleJSCallback(instance.getInstanceId(), mCallback);

    if (TextUtils.isEmpty(getRef())) {
      Map<String, Object> options = new HashMap<>();
      options.put("result", false);
      options.put("errMsg", "Illegal parameter");
      jsCallback.invoke(options);
    } else if ("viewport".equalsIgnoreCase(getRef())) {
      callbackViewport(instance, jsCallback);
    } else {
      WXComponent component = WXSDKManager
          .getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
      if (component == null) {
        return;
      }

      Map<String, Object> options = new HashMap<>();
      if (component != null) {
        int viewPort = instance.getInstanceViewPortWidth();
        Map<String, Float> size = new HashMap<>();
        Rect sizes = component.getComponentSize();
        size.put("width", getWebPxValue(sizes.width(),viewPort));
        size.put("height", getWebPxValue(sizes.height(),viewPort));
        size.put("bottom", getWebPxValue(sizes.bottom,viewPort));
        size.put("left", getWebPxValue(sizes.left,viewPort));
        size.put("right", getWebPxValue(sizes.right,viewPort));
        size.put("top", getWebPxValue(sizes.top,viewPort));
        options.put("size", size);
        options.put("result", true);
      } else {
        options.put("errMsg", "Component does not exist");
      }
      jsCallback.invoke(options);
    }
  }

  private void callbackViewport(WXSDKInstance instance, JSCallback jsCallback) {
    View container;
    if ((container = instance.getContainerView()) != null) {
      Map<String, Object> options = new HashMap<>();
      Map<String, Float> sizes = new HashMap<>();
      int[] location = new int[2];
      instance.getContainerView().getLocationOnScreen(location);
      int viewport = instance.getInstanceViewPortWidth();
      sizes.put("left", 0f);
      sizes.put("top", 0f);
      sizes.put("right", getWebPxValue(container.getWidth(),viewport));
      sizes.put("bottom", getWebPxValue(container.getHeight(),viewport));
      sizes.put("width", getWebPxValue(container.getWidth(),viewport));
      sizes.put("height", getWebPxValue(container.getHeight(),viewport));
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
  private float getWebPxValue(int value,int viewport) {
    return WXViewUtils.getWebPxByWidth(value, viewport);
  }

}
