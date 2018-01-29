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
package com.taobao.weex.bridge;

import android.content.Intent;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.Menu;

import com.alibaba.fastjson.JSONArray;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXException;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.WXDomModule;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.ui.module.WXTimerModule;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Manager class for weex module. There are two types of modules in weex, one is instance-level module,
 * the other is global-level module. Instance-level module will be created every time an instance
 * is created, while global-level module will be singleton in {@link com.taobao.weex.WXSDKEngine}.
 */
public class WXModuleManager {

  /**
   * module class object dictionary
   */
  private static Map<String, ModuleFactory> sModuleFactoryMap = new HashMap<>();
  private static Map<String, WXModule> sGlobalModuleMap = new HashMap<>();
  private static Map<String, WXDomModule> sDomModuleMap = new HashMap<>();

  /**
   * monitor keys
   */
  private static String MONITOR_ERROR_CODE = "errCode";
  private static String MONITOR_ARG = "arg";
  private static String MONITOR_ERROR_MSG = "errMsg";

  /**
   * module object dictionary
   * K : instanceId, V : Modules
   */
  private static Map<String, Map<String, WXModule>> sInstanceModuleMap = new ConcurrentHashMap<>();

  /**
   * Register module to JavaScript and Android
   */
  public static boolean registerModule(final String moduleName, final ModuleFactory factory, final boolean global) throws WXException {
    if (moduleName == null || factory == null) {
      return false;
    }

    if (TextUtils.equals(moduleName,WXDomModule.WXDOM)) {
      WXLogUtils.e("Cannot registered module with name 'dom'.");
      return false;
    }

    //execute task in js thread to make sure register order is same as the order invoke register method.
    WXBridgeManager.getInstance()
        .post(new Runnable() {
      @Override
      public void run() {
        if (sModuleFactoryMap.containsKey(moduleName)) {
          WXLogUtils.w("WXComponentRegistry Duplicate the Module name: " + moduleName);
        }

        if (global) {
          try {
            WXModule wxModule = factory.buildInstance();
            wxModule.setModuleName(moduleName);
            sGlobalModuleMap.put(moduleName, wxModule);
          } catch (Exception e) {
            WXLogUtils.e(moduleName + " class must have a default constructor without params. ", e);
          }
        }

        try {
          registerNativeModule(moduleName, factory);
        } catch (WXException e) {
          WXLogUtils.e("", e);
        }
        registerJSModule(moduleName, factory);
      }
    });
    return true;

  }

  static boolean registerNativeModule(String moduleName, ModuleFactory factory) throws WXException {
    if (factory == null) {
      return false;
    }

    try {
      sModuleFactoryMap.put(moduleName, factory);
    }catch (ArrayStoreException e){
      e.printStackTrace();
      //ignore:
      //may throw this exception:
      //java.lang.String cannot be stored in an array of type java.util.HashMap$HashMapEntry[]
    }
    return true;
  }

  static boolean registerJSModule(String moduleName, ModuleFactory factory) {
    Map<String, Object> modules = new HashMap<>();
    modules.put(moduleName, factory.getMethods());
    WXSDKManager.getInstance().registerModules(modules);
    return true;
  }

  static Object callModuleMethod(final String instanceId, String moduleStr, String methodStr, JSONArray args) {
    ModuleFactory factory = sModuleFactoryMap.get(moduleStr);
    if(factory == null){
      WXLogUtils.e("[WXModuleManager] module factory not found.");
      return null;
    }
    final WXModule wxModule = findModule(instanceId, moduleStr,factory);
    if (wxModule == null) {
      return null;
    }
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    wxModule.mWXSDKInstance = instance;

    final Invoker invoker = factory.getMethodInvoker(methodStr);
    try {
      if(instance != null) {
        IWXUserTrackAdapter userTrackAdapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
        if(userTrackAdapter != null) {
          HashMap<String, Serializable> data = new HashMap<String, Serializable>();
          data.put(MONITOR_ERROR_CODE, "101");
          data.put(MONITOR_ARG, moduleStr + "." + methodStr);
          data.put(MONITOR_ERROR_MSG, instance.getBundleUrl());
          userTrackAdapter.commit(instance.getContext(), null, IWXUserTrackAdapter.INVOKE_MODULE, null, data);
        }
        return dispatchCallModuleMethod(instance,wxModule,args,invoker);
      } else {
        WXLogUtils.e("callModuleMethod >>> instance is null");
        return null;
      }
    } catch (Exception e) {
	  WXExceptionUtils.commitCriticalExceptionRT(instanceId,
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED.getErrorCode(),
			  "callModuleMethod",
			  WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED.getErrorMsg()
			  + "callModuleMethod >>> invoke module:" + moduleStr + ", method:" + methodStr + " failed. "
			  + WXLogUtils.getStackTrace(e),
			  null);
	  WXLogUtils.e("callModuleMethod >>> invoke module:" + moduleStr + ", method:" + methodStr + " failed. ", e);
      return null;
    } finally {
      if (wxModule instanceof WXDomModule || wxModule instanceof WXTimerModule) {
        wxModule.mWXSDKInstance = null;
      }
    }
  }

  private static Object dispatchCallModuleMethod(@NonNull WXSDKInstance instance, @NonNull WXModule wxModule,
                                                 @NonNull JSONArray args, @NonNull Invoker invoker) throws Exception{
    if(!instance.isPreRenderMode()) {
      return instance.getNativeInvokeHelper().invoke(wxModule,invoker,args);
    }
    // we are in preRender mode
    if(invoker.isRunOnUIThread()) {/*ASYNC CALL*/
      DOMAction moduleInvocationAction = Actions.getModuleInvocationAction(wxModule,args,invoker);
      WXSDKManager.getInstance().getWXDomManager().postAction(instance.getInstanceId(), moduleInvocationAction,false);
      return null;
    } else {/*SYNC CALL*/
      return instance.getNativeInvokeHelper().invoke(wxModule,invoker,args);
    }
  }



  private static WXModule findModule(String instanceId, String moduleStr,ModuleFactory factory) {
    // find WXModule
    WXModule wxModule = sGlobalModuleMap.get(moduleStr);

    //not global module
    if (wxModule == null) {
      Map<String, WXModule> moduleMap = sInstanceModuleMap.get(instanceId);
      if (moduleMap == null) {
        moduleMap = new ConcurrentHashMap<>();
        sInstanceModuleMap.put(instanceId, moduleMap);
      }
      // if cannot find the Module, create a new Module and save it
      wxModule = moduleMap.get(moduleStr);
      if (wxModule == null) {
        try {
          wxModule = factory.buildInstance();
          wxModule.setModuleName(moduleStr);
        } catch (Exception e) {
          WXLogUtils.e(moduleStr + " module build instace failed.", e);
          return null;
        }
        moduleMap.put(moduleStr, wxModule);
      }
    }

    return wxModule;
  }

  /**Hook Activity life cycle callback begin***/


  public static void onActivityCreate(String instanceId){

    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityCreate();
        } else {
          WXLogUtils.w("onActivityCreate can not find the " + key + " module");
        }
      }
    }

  }

  public static void onActivityStart(String instanceId){

    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityStart();
        } else {
          WXLogUtils.w("onActivityStart can not find the " + key + " module");
        }
      }
    }
  }

  public static void onActivityPause(String instanceId){
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityPause();
        } else {
          WXLogUtils.w("onActivityPause can not find the " + key + " module");
        }
      }
    }
  }

  public static void onActivityResume(String instanceId){
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityResume();
        } else {
          WXLogUtils.w("onActivityResume can not find the " + key + " module");
        }
      }
    }
  }

  public static void onActivityStop(String instanceId){
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityStop();
        } else {
          WXLogUtils.w("onActivityStop can not find the " + key + " module");
        }
      }
    }
  }

  public static void onActivityDestroy(String instanceId){
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityDestroy();
        } else {
          WXLogUtils.w("onActivityDestroy can not find the " + key + " module");
        }
      }
    }
  }

  public static boolean onActivityBack(String instanceId){
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          return module.onActivityBack();
        } else {
          WXLogUtils.w("onActivityCreate can not find the " + key + " module");
        }
      }
    }
    return false;
  }

  public static void onActivityResult(String instanceId,int requestCode, int resultCode, Intent data){

    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onActivityResult(requestCode, resultCode, data);
        } else {
          WXLogUtils.w("onActivityResult can not find the " + key + " module");
        }
      }
    }
  }

  public static boolean onCreateOptionsMenu(String instanceId,Menu menu) {
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onCreateOptionsMenu(menu);
        } else {
          WXLogUtils.w("onActivityResult can not find the " + key + " module");
        }
      }
    }
    return false;
  }

  public static void onRequestPermissionsResult(String instanceId ,int requestCode, String[] permissions, int[] grantResults) {
    Map<String, WXModule> modules = sInstanceModuleMap.get(instanceId);
    if(modules!=null) {
      for (String key : modules.keySet()) {
        WXModule module = modules.get(key);
        if (module != null) {
          module.onRequestPermissionsResult(requestCode, permissions, grantResults);
        } else {
          WXLogUtils.w("onActivityResult can not find the " + key + " module");
        }
      }
    }
  }

  public static void destroyInstanceModules(String instanceId) {
    sDomModuleMap.remove(instanceId);
    Map<String, WXModule> moduleMap = sInstanceModuleMap.remove(instanceId);
    if (moduleMap == null || moduleMap.size() < 1) {
      return;
    }
    Iterator<Entry<String, WXModule>> iterator = moduleMap.entrySet().iterator();
    Entry<String, WXModule> entry;
    while (iterator.hasNext()) {
      entry = iterator.next();
      WXModule module = entry.getValue();
      if(module instanceof Destroyable){
        ((Destroyable)module).destroy();
      }

    }
  }

  public static void createDomModule(WXSDKInstance instance){
    if(instance != null) {
      sDomModuleMap.put(instance.getInstanceId(), new WXDomModule(instance));
    }
  }

  public static void destoryDomModule(String instanceID){
    sDomModuleMap.remove(instanceID);
  }

  public static WXDomModule getDomModule(String instanceId){
    return sDomModuleMap.get(instanceId);
  }

  public static void reload(){
    if (sModuleFactoryMap != null && sModuleFactoryMap.size() > 0) {
      for (Map.Entry<String, ModuleFactory> entry : sModuleFactoryMap.entrySet()) {
        registerJSModule(entry.getKey(), entry.getValue());
      }
    }
  }

}
