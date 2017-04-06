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

import android.text.TextUtils;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXException;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * All components must be registered within this class before used.
 */
public class WXComponentRegistry {

  private static Map<String, IFComponentHolder> sTypeComponentMap = new HashMap<>();
  private static ArrayList<Map<String, Object>> sComponentInfos=new ArrayList<>();

  public static boolean registerComponent(final String type, final IFComponentHolder holder, final Map<String, Object> componentInfo) throws WXException {
    if (holder == null || TextUtils.isEmpty(type)) {
      return false;
    }

    //execute task in js thread to make sure register order is same as the order invoke register method.
    WXBridgeManager.getInstance()
        .post(new Runnable() {
      @Override
      public void run() {
        try {
          Map<String, Object> registerInfo = componentInfo;
          if (registerInfo == null){
            registerInfo = new HashMap<>();
          }

          registerInfo.put("type",type);
          registerInfo.put("methods",holder.getMethods());
          registerNativeComponent(type, holder);
          registerJSComponent(registerInfo);
          sComponentInfos.add(registerInfo);
        } catch (WXException e) {
          WXLogUtils.e("register component error:", e);
        }

      }
    });
    return true;
  }

  private static boolean registerNativeComponent(String type, IFComponentHolder holder) throws WXException {
    try {
      holder.loadIfNonLazy();
      sTypeComponentMap.put(type, holder);
    }catch (ArrayStoreException e){
      e.printStackTrace();
      //ignore: ArrayStoreException: java.lang.String cannot be stored in an array of type java.util.HashMap$HashMapEntry[]
    }
    return true;
  }

  private static boolean registerJSComponent(Map<String, Object> componentInfo) throws WXException {
    ArrayList<Map<String, Object>> coms = new ArrayList<>();
    coms.add(componentInfo);
    WXSDKManager.getInstance().registerComponents(coms);
    return true;
  }

  public static IFComponentHolder getComponent(String type) {
    return sTypeComponentMap.get(type);
  }

  public static void reload(){
    WXBridgeManager.getInstance().post(new Runnable() {
      @Override
      public void run() {
        try {
          for(Map<String,Object> com:sComponentInfos){
            registerJSComponent(com);
          }
        } catch (WXException e) {
          WXLogUtils.e("", e);
        }
      }
    });
  }

}
