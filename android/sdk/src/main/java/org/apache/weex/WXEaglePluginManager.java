/**
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
package org.apache.weex;
import android.util.Pair;
import android.text.TextUtils;
import com.alibaba.fastjson.JSONObject;
import org.apache.weex.adapter.IWXUserTrackAdapter;
import org.apache.weex.bridge.ModuleFactory;
import org.apache.weex.bridge.WXEaglePlugin;
import org.apache.weex.common.WXRenderStrategy;
import org.apache.weex.ui.IFComponentHolder;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXSoInstallMgrSdk;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class WXEaglePluginManager {
  private static final String LOG_TAG = "WXEaglePluginManager";

  private static class InstHolder {

    private static final WXEaglePluginManager INST = new WXEaglePluginManager();
  }

  private Map<String, WXEaglePlugin> mPluginMap = new ConcurrentHashMap<>();

  private Map<String, Pair<IFComponentHolder, Map<String, Object>>> mCompCache = new ConcurrentHashMap<>();
  private Map<String, Pair<ModuleFactory, Boolean>> mModuleCache = new ConcurrentHashMap<>();

  public static WXEaglePluginManager getInstance() {
    return InstHolder.INST;
  }

  private WXEaglePluginManager() {
  }

  public void register(WXEaglePlugin plugin) {
    if (plugin == null || TextUtils.isEmpty(plugin.getPluginName())) {
      return;
    }
    WXEaglePlugin old = mPluginMap.put(plugin.getPluginName(), plugin);
    //If plugin register is delayed.
    for (Map.Entry<String, Pair<IFComponentHolder, Map<String, Object>>> entry : mCompCache.entrySet()) {
      plugin.registerComponent(entry.getKey(),entry.getValue().first, entry.getValue().second);
    }
    for (Map.Entry<String, Pair<ModuleFactory, Boolean>> entry : mModuleCache.entrySet()) {
      plugin.registerModules(entry.getKey(),entry.getValue().first, entry.getValue().second);
    }
    if (old != null) {
      WXLogUtils.w(LOG_TAG, "Register plugin already exist: " + plugin.getPluginName());
    }
  }

  void initSo(@SuppressWarnings("SameParameterValue") int version, IWXUserTrackAdapter userTrackAdapter) {
    for (WXEaglePlugin plugin : mPluginMap.values()) {
      WXSoInstallMgrSdk.initSo(plugin.getSoLibName(), version, userTrackAdapter);
    }
  }

  public WXEaglePlugin getPlugin(String type) {
    if (type == null) {
      return null;
    }
    return mPluginMap.get(type);
  }

  public Pair<String, WXEaglePlugin> filterUrl(String url) {
    for (Map.Entry<String, WXEaglePlugin> entry : mPluginMap.entrySet()) {
      String result = entry.getValue().isSupportedUrl(url);
      if (result != null) {
        //support by this plugin
        return Pair.create(result, entry.getValue());
      }
    }
    return null;
  }

  public void registerComponent(String type, IFComponentHolder holder, Map<String, Object> componentInfo) {
    mCompCache.put(type, Pair.create(holder, componentInfo));
    for (WXEaglePlugin plugin : mPluginMap.values()) {
      plugin.registerComponent(type, holder, componentInfo);
    }
  }

  public void registerModule(String moduleName, ModuleFactory factory, boolean global) {
    mModuleCache.put(moduleName, Pair.create(factory, global));
    for (WXEaglePlugin plugin : mPluginMap.values()) {
      plugin.registerModules(moduleName, factory, global);
    }
  }

  public boolean callEagleTaskFromWeex(String type, String task, JSONObject option) {
    boolean support = false;
    if (TextUtils.isEmpty(type)) {
      for (WXEaglePlugin plugin : mPluginMap.values()) {
        support = support || plugin.callEagleTaskFromWeex(task, option);
      }
    } else {
      WXEaglePlugin plugin = getPlugin(type);
      if (plugin != null) {
        support = plugin.callEagleTaskFromWeex(task, option);
      }
    }
    return support;
  }

  //TODO(Eagle) remove later
  static String getPluginName(WXRenderStrategy strategy) {
    if (strategy == WXRenderStrategy.DATA_RENDER) {
      return "EagleVue";
    } else if (strategy == WXRenderStrategy.DATA_RENDER_BINARY) {
      return "EagleRax";
    } else {
      return null;
    }
  }

  //TODO(Eagle) remove later
  static WXRenderStrategy getRenderStrategyByPlugin(String name) {
    switch (name) {
      case "EagleVue":
        return WXRenderStrategy.DATA_RENDER;
      case "EagleRax":
        return WXRenderStrategy.DATA_RENDER_BINARY;
    }
    return null;
  }

}
