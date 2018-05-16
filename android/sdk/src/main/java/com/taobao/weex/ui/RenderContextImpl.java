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

import android.support.v4.util.ArrayMap;
import android.util.Log;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.dom.RenderContext;
import com.taobao.weex.ui.component.WXComponent;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Class for rendering view. Method in this class should be run in main thread.
 * This class is also <strong>not</storng> thread safe.
 */
class RenderContextImpl implements RenderContext {

  private Map<String, WXComponent> mRegistry;
  private WXSDKInstance mWXSDKInstance;

  public RenderContextImpl(WXSDKInstance instance) {
    mWXSDKInstance = instance;
    mRegistry = new ArrayMap<>();
  }

  public void destroy() {
    mWXSDKInstance = null;
    try {
      mRegistry.clear();
    } catch (Throwable e) {
      e.printStackTrace();
    }
  }

  public WXSDKInstance getWXSDKInstance() {
    return mWXSDKInstance;
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
    mRegistry.put(ref, comp);
  }

  @Override
  public WXComponent unregisterComponent(String ref) {
    return mRegistry.remove(ref);
  }
}
