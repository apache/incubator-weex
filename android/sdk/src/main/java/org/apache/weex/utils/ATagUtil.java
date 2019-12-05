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

package org.apache.weex.utils;

import android.net.Uri;
import android.view.View;

import com.alibaba.fastjson.JSONArray;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.URIAdapter;

public class ATagUtil {
  public static void onClick(View widget, String instanceId, String url) {
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (instance == null) {
      return;
    }
    String href = instance.rewriteUri(Uri.parse(url), URIAdapter.LINK).toString();
    JSONArray array = new JSONArray();
    array.add(href);
    WXSDKManager.getInstance().getWXBridgeManager().
        callModuleMethod(instanceId, "event", "openURL", array);
  }
}
