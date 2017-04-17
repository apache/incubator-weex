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
package com.taobao.weex.ui;

import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.ui.animation.WXAnimationBean;
import com.taobao.weex.ui.animation.WXAnimationModule;
import com.taobao.weex.ui.component.Scrollable;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Class for rendering view. Method in this class should be run in main thread.
 * This class is also <strong>not</storng> thread safe.
 * This class is very similar to {@link com.taobao.weex.dom.WXDomStatement}
 * @see com.taobao.weex.dom.WXDomStatement
 */
class RenderActionContextImpl implements RenderActionContext {

  private Map<String, WXComponent> mRegistry;
  private WXSDKInstance mWXSDKInstance;
  /**
   * The container for weex root view.
   */

  public RenderActionContextImpl(WXSDKInstance instance) {
    mWXSDKInstance = instance;
    mRegistry = new HashMap<>();
  }

  /**
   * @see com.taobao.weex.dom.WXDomStatement#destroy()
   */
  public void destroy() {
    mWXSDKInstance = null;
    mRegistry.clear();
  }

  public WXSDKInstance getWXSDKInstance() {
    return mWXSDKInstance;
  }

  /**
   * set layout information of View
   */
  void setLayout(String ref, WXDomObject domObject) {
    WXComponent component = mRegistry.get(ref);
    if (component == null) {
      return;
    }
    component.setLayout(domObject);
  }

  /**
   * set extra information of View
   */
  void setExtra(String ref, Object extra) {
    WXComponent component = mRegistry.get(ref);
    if (component == null) {
      return;
    }
    component.updateExtra(extra);
  }

  @Override
  public WXSDKInstance getInstance() {
    return mWXSDKInstance;
  }

  @Override
  public WXComponent getComponent(String ref) {
    return mRegistry.get(ref);
  }

  public void registerComponent(String ref, WXComponent comp) {
    mRegistry.put(ref,comp);
  }

  public WXComponent unregisterComponent(String ref) {
    return mRegistry.remove(ref);
  }
}
