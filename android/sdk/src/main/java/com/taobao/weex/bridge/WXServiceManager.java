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

import android.text.TextUtils;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.WXJSService;

import java.util.HashMap;
import java.util.Map;

public class WXServiceManager {

    private static Map<String, WXJSService> sInstanceJSServiceMap = new HashMap<>();

    public static boolean registerService(String name, String serviceScript, Map<String, Object> options) {
        if (TextUtils.isEmpty(name) || TextUtils.isEmpty(serviceScript)) return false;

        String param1 = "register: global.registerService, unregister: global.unregisterService";
        String param2 = "serviceName: \"" + name + "\"";
        for (String key: options.keySet()) {
            // TODO - why always string?
            Object value = options.get(key);
            if (value instanceof  String) {
                param2 += ", \'" + key + "\': \'" + value + "\'";
            } else {
                param2 += ", \'" + key + "\': " + value;
            }
        }
        String serviceJs = String.format(";(function(service, options){ ;%s; })({ %s }, { %s });", serviceScript, param1, param2);

        WXJSService service = new WXJSService();
        service.setName(name);
        service.setScript(serviceScript);
        service.setOptions(options);
        sInstanceJSServiceMap.put(name, service);

        WXBridgeManager.getInstance().execJSService(serviceJs);
        return true;
    }

    public static boolean unRegisterService(String name) {
        if (TextUtils.isEmpty(name)) return false;

        if(WXEnvironment.isApkDebugable()) {
            sInstanceJSServiceMap.remove(name);
        }

        String js = String.format("global.unregisterService( \"%s\" );", name);
        WXBridgeManager.getInstance().execJSService(js);
        return true;
    }


    public static void execAllCacheJsService() {
        for (String serviceName: sInstanceJSServiceMap.keySet()) {
            WXJSService service = sInstanceJSServiceMap.get(serviceName);
            registerService(service.getName(), service.getScript(), service.getOptions());
        }
    }

    public static WXJSService getService(String serviceName) {
        if (sInstanceJSServiceMap != null) {
            return sInstanceJSServiceMap.get(serviceName);
        }
        return null;
    }

    public static void reload() {
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                for (Map.Entry<String, WXJSService> entry : sInstanceJSServiceMap.entrySet()) {
                    WXJSService service = entry.getValue();
                    registerService(service.getName(), service.getScript(), service.getOptions());
                }
            }
        });
    }
}
