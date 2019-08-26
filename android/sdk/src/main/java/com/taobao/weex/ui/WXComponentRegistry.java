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
import android.util.Log;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXException;
import com.taobao.weex.ui.config.AutoScanConfigRegister;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.cache.RegisterCache;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * All components must be registered within this class before used.
 */
public class WXComponentRegistry {

  private static Map<String, IFComponentHolder> sTypeComponentMap = new ConcurrentHashMap<>();
  private static ArrayList<Map<String, Object>> sComponentInfos=new ArrayList<>();
  private static final Map<String, Runnable> sEagleComponentTasks=new ConcurrentHashMap<>();

  public static synchronized boolean registerComponent(Map<String, RegisterCache.ComponentCache> componentCacheMap) {
    if (componentCacheMap.isEmpty())
      return true;
    final Iterator<Map.Entry<String, RegisterCache.ComponentCache>> iterator = componentCacheMap.entrySet().iterator();
    WXBridgeManager.getInstance().post(new Runnable() {
      @Override
      public void run() {
        ArrayList<Map<String, Object>> coms = new ArrayList<>();
        while (iterator.hasNext()) {
          Map.Entry<String, RegisterCache.ComponentCache> next = iterator.next();
          try {
            RegisterCache.ComponentCache value = next.getValue();
            Map<String, Object> registerInfo = value.componentInfo;
            if (registerInfo == null) {
              registerInfo = new HashMap<>();
            }
            registerInfo.put("type", value.type);
            registerInfo.put("methods", value.holder.getMethods());
            registerNativeComponent(value.type, value.holder);
            sComponentInfos.add(registerInfo);
            coms.add(registerInfo);
          } catch (WXException e) {
            e.printStackTrace();
          }
        }
        WXSDKManager.getInstance().registerComponents(coms);
      }
    });
    return true;
  }

  public static synchronized boolean registerComponent(final String type, final IFComponentHolder holder, final Map<String, Object> componentInfo) throws WXException {
    if (holder == null || TextUtils.isEmpty(type)) {
      return false;
    }

    if(RegisterCache.getInstance().cacheComponent(type,holder,componentInfo)) {
      return true;
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

  public static boolean registerEagleOnlyComponent(final String type, final IFComponentHolder holder, final Map<String, Object> componentInfo){
    if (holder == null || TextUtils.isEmpty(type)) {
      return false;
    }

    Runnable tasks = new Runnable() {
      @Override
      public void run() {
        try {
          Map<String, Object> registerInfo = componentInfo;
          if (registerInfo == null) {
            registerInfo = new HashMap<>();
          }

          registerInfo.put("type", type);
          registerInfo.put("methods", holder.getMethods());
          registerNativeComponent(type, holder);

          ArrayList<Map<String, Object>> coms = new ArrayList<>();
          coms.add(registerInfo);
          WXSDKManager.getInstance().registerEagleComponents(coms);
          sComponentInfos.add(registerInfo);
        } catch (WXException e) {
          WXLogUtils.e("register component error:", e);
        }

      }
    };
    sEagleComponentTasks.put(type, tasks);
    WXBridgeManager.getInstance()
        .post(new Runnable() {
          @Override
          public void run() {
            try {
              registerNativeComponent(type, holder);
            } catch (WXException e) {
              WXLogUtils.e("register component error:", e);
            }
          }
        });
    return true;
  }

  /**
   * will be called in js thread.
   */
  public static boolean requireEagleComponent(final String type){
    Runnable remove = sEagleComponentTasks.remove(type);
    if (remove == null){
      return false;
    }

    try {
      remove.run();
      return true;
    } catch (Exception e) {
      WXLogUtils.e("register eagle component error:", e);
      return false;
    }
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
