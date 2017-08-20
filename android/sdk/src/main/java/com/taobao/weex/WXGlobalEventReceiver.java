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
package com.taobao.weex;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;

public class WXGlobalEventReceiver extends BroadcastReceiver {

  public static final String EVENT_NAME = "eventName";
  public static final String EVENT_PARAMS = "eventParams";
  public static final String EVENT_ACTION = "wx_global_action";
  public static final String EVENT_WX_INSTANCEID = "wx_instanceid";

  private WXSDKInstance mWXSDKInstance;

  public WXGlobalEventReceiver() {
  }

  public WXGlobalEventReceiver(WXSDKInstance instance) {
    mWXSDKInstance = instance;
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    String eventName = intent.getStringExtra(EVENT_NAME);
    String params = intent.getStringExtra(EVENT_PARAMS);
    HashMap<String, Object> maps = null;
    try {
      maps = com.alibaba.fastjson.JSON.parseObject(params, HashMap.class);
      mWXSDKInstance.fireGlobalEventCallback(eventName, maps);
    } catch (Exception e) {
      WXLogUtils.e("global-receive",e);
    }
  }
}
