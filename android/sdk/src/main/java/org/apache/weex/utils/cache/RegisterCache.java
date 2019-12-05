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
package org.apache.weex.utils.cache;

import org.apache.weex.bridge.ModuleFactory;
import org.apache.weex.bridge.WXModuleManager;
import org.apache.weex.ui.IFComponentHolder;
import org.apache.weex.ui.WXComponentRegistry;
import org.apache.weex.ui.config.AutoScanConfigRegister;
import org.apache.weex.utils.WXLogUtils;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class RegisterCache {
    private static Map<String, ModuleCache> moduleCacheMap = new ConcurrentHashMap<>();
    private static Map<String, ComponentCache> componentCacheMap = new ConcurrentHashMap<>();
    private static RegisterCache registerCache;

    private boolean enable = false;

    private boolean enableAutoScan = true;

    public static RegisterCache getInstance() {
        if (registerCache == null) {
            synchronized (RegisterCache.class) {
                if (registerCache == null) {
                    registerCache = new RegisterCache();
                }
            }
        }
        return registerCache;
    }

    private RegisterCache() {

    }

    private volatile boolean finished = false;

    private volatile int doNotCacheSize = Integer.MAX_VALUE;

    public void setEnable(boolean enable) {
        this.enable = enable;
    }

    private boolean enableCache() {
        return enable;
    }

    private boolean canCache() {
        return enableCache()
                && !finished
                && getDoNotCacheSize() < 1;
    }

    public boolean enableAutoScan() {
        return enableAutoScan;
    }

    public void setEnableAutoScan(boolean enableAutoScan) {
        if(this.enableAutoScan != enableAutoScan) {
            if(enableAutoScan) {
                AutoScanConfigRegister.doScanConfig();
            }
            this.enableAutoScan = enableAutoScan;
        }

    }

    private int getDoNotCacheSize() {
        return doNotCacheSize--;
    }

    public void setDoNotCacheSize(int doNotCacheSize) {
        this.doNotCacheSize = doNotCacheSize;
    }

    public boolean idle(boolean atCreateInstance) {
        if (finished)
            return true;

        String message;
        if(atCreateInstance) {
            message = "idle from create instance";
        } else {
            message = "idle from external";
        }

        WXLogUtils.e(message + " cache size is " + (moduleCacheMap.size() + componentCacheMap.size()));

        finished = true;
        CacheComponentRegister();
        CacheModuleRegister();
        return true;
    }

    public boolean cacheModule(final String moduleName, final ModuleFactory factory, final boolean global) {
        if (!canCache()) {
            return false;
        }
        try {
            moduleCacheMap.put(moduleName, new ModuleCache(moduleName, factory, global));
        } catch (Exception e) {
            return false;
        }
        return true;
    }


    public boolean cacheComponent(final String type, final IFComponentHolder holder, final Map<String, Object> componentInfo) {
        if (!canCache()) {
            return false;
        }
        try {
            componentCacheMap.put(type, new ComponentCache(type, holder, componentInfo));
        } catch (Exception e) {
            return false;
        }
        return true;
    }


    private void CacheComponentRegister() {
        if (componentCacheMap.isEmpty())
            return;
        WXComponentRegistry.registerComponent(componentCacheMap);
    }


    private void CacheModuleRegister() {
        if (moduleCacheMap.isEmpty())
            return;

        WXModuleManager.registerModule(moduleCacheMap);
    }

    public class ModuleCache {
        public final String name;
        public final ModuleFactory factory;
        public final boolean global;

        ModuleCache(String moduleName, ModuleFactory factory, boolean global) {
            this.name = moduleName;
            this.factory = factory;
            this.global = global;
        }
    }


    public class ComponentCache {
        public final String type;
        public final IFComponentHolder holder;
        public final Map<String, Object> componentInfo;

        ComponentCache(String type, IFComponentHolder holder, Map<String, Object> componentInfo) {
            this.type = type;
            this.componentInfo = componentInfo;
            this.holder = holder;
        }
    }



}
