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

import android.text.TextUtils;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.ui.IFComponentHolder;
import com.taobao.weex.ui.WXComponentRegistry;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Component factory
 */
public class WXComponentFactory {

  public static WXComponent newInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    if (instance == null || TextUtils.isEmpty(basicComponentData.mComponentType)) {
      return null;
    }

    IFComponentHolder holder = WXComponentRegistry.getComponent(basicComponentData.mComponentType);
    if (holder == null) {
      if (WXEnvironment.isApkDebugable()) {
        String tag = "WXComponentFactory error type:[" +
                basicComponentData.mComponentType + "]" + " class not found";
        WXLogUtils.e(tag);
      }
      //For compatible reason of JS framework, unregistered type will be treated as container.
      holder = WXComponentRegistry.getComponent(WXBasicComponentType.CONTAINER);
      if (holder == null) {
        WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(),
                WXErrorCode.WX_RENDER_ERR_COMPONENT_NOT_REGISTER, "createComponent",
                basicComponentData.mComponentType + " not registered", null);
        return null;
      }
    }

    try {
      return holder.createInstance(instance, parent, basicComponentData);
    } catch (Throwable e) {
      WXLogUtils.e("WXComponentFactory Exception type:[" + basicComponentData.mComponentType + "] ", e);
    }

    return null;
  }
}
