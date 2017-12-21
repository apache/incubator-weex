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
package com.taobao.weex.ui.component;

import android.content.Context;
import android.support.annotation.NonNull;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.action.CommonCompData;
import com.taobao.weex.ui.view.WXFrameLayout;

import java.lang.reflect.InvocationTargetException;

/**
 * div component
 */
@Component(lazyload = false)
public class WXDiv extends WXVContainer<WXFrameLayout> {

  public static class Ceator implements ComponentCreator {
    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, CommonCompData commonCompData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXDiv(instance, parent, commonCompData);
    }
  }

  @Deprecated
  public WXDiv(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, CommonCompData commonCompData) {
    this(instance, parent, commonCompData);
  }

  public WXDiv(WXSDKInstance instance, WXVContainer parent, CommonCompData commonCompData) {
    super(instance, parent, commonCompData);
  }

  @Override
  protected WXFrameLayout initComponentHostView(@NonNull Context context) {
    WXFrameLayout frameLayout = new WXFrameLayout(context);
    frameLayout.holdComponent(this);
    return frameLayout;
  }

}
