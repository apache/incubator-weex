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
package com.taobao.weex.dom;

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Factory class for creating {@link WXDomObject}
 */
public class WXDomObjectFactory {

  public static @Nullable WXDomObject newInstance(String type) {
    if (TextUtils.isEmpty(type)) {
      return null;
    }

    Class<? extends WXDomObject> clazz = WXDomRegistry.getDomObjectClass(type);
    if (clazz == null) {
      if (WXEnvironment.isApkDebugable()) {
        String tag = "WXDomObjectFactory error type:[" +
                type + "]" + " class not found";
        WXLogUtils.e(tag);
      }
    }

    try {
      if (WXDomObject.class.isAssignableFrom(clazz)) {
        WXDomObject domObject = clazz.getConstructor()
            .newInstance();
        return domObject;
      }
    } catch (Exception e) {
      WXLogUtils.e("WXDomObjectFactory Exception type:[" + type + "] ", e);
    }

    return null;
  }
}
