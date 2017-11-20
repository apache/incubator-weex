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

import android.text.TextUtils;

import com.alibaba.fastjson.JSON;
import com.taobao.weex.scalpel.ScalpelFrameLayout;
import com.taobao.weex.utils.LogLevel;
import com.taobao.weex.websocket.WXWebSocketManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by lixinke on 16/5/13.
 */
public class WXDebugTool {

  public static ScalpelFrameLayout sScalpelFrameLayout;

  public static void sendLog(LogLevel level, String msg) {
    if (WXWebSocketManager.getInstance() != null && !TextUtils.isEmpty(msg)) {

      if (WXEnvironment.sLogLevel.compare(level) >= 0) {
        List<String> arguments = new ArrayList<>();
        arguments.add(level.getName());
        arguments.add(msg);
        Map<String, Object> msgObject = new HashMap<>();
        msgObject.put("method", "__logger");
        msgObject.put("arguments", arguments);
        WXWebSocketManager.getInstance().sendMessage(JSON.toJSONString(msgObject));
      }
    }
  }
  public static void connect(String debugUrl){
    WXWebSocketManager.getInstance().connect(debugUrl);
  }
  public static void close(){
    WXWebSocketManager.getInstance().closeQuietly();
  }

  public static void updateScapleView(Object scalpelFrameLayout){
    sScalpelFrameLayout= (ScalpelFrameLayout) scalpelFrameLayout;
  }

  public static void switchLayerInteractionEnabled(){
    if(sScalpelFrameLayout!=null){
      sScalpelFrameLayout.setLayerInteractionEnabled(!sScalpelFrameLayout.isLayerInteractionEnabled());
    }
  }
}
