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

import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.bridge.SimpleJSCallback;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.list.template.jni.NativeRenderObjectUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by listen on 18/09/12.
 */
public class ActionGetLayoutDirection extends BasicGraphicAction {

  private final String mCallback;

  public ActionGetLayoutDirection(WXSDKInstance instance, String ref, String callback) {
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
      WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
      if (component == null) {
        return;
      }

      String directionRet = "ltr";
      if (component != null) {
        int direction = NativeRenderObjectUtils.nativeRenderObjectGetLayoutDirectionFromPathNode(component.getRenderObjectPtr());
        switch (direction) {
          case 0: {
            directionRet = "inherit";
            break;
          }
          case 1: {
            directionRet = "ltr";
            break;
          }
          case 2: {
            directionRet = "rtl";
            break;
          }
          default: {
            directionRet = "ltr";
            break;
          }

        }
      }
      jsCallback.invoke(directionRet);
    }
  }

  private void callbackViewport(WXSDKInstance instance, JSCallback jsCallback) {
    View container;
    if ((container = instance.getContainerView()) != null) {
      Map<String, Object> options = new HashMap<>();
      options.put("direction", "ltr");
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
