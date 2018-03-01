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
package com.taobao.weex.ui.config;

import android.content.Context;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.bridge.MethodInvoker;
import com.taobao.weex.bridge.ModuleFactory;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.common.WXModuleAnno;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/2/7.
 */

public class ConfigModuleFactory<T extends WXModule> implements ModuleFactory<T> {

    public static final String TAG = AutoScanConfigRegister.TAG;

    private String mName;
    private String mClassName;
    private String[] methods;
    private Class<T> mClazz;
    private Map<String, Invoker> mMethodMap;

    private ClassLoader mClassLoader;

    public ConfigModuleFactory(String mName, String mClassName, String[] methods) {
        this.mName = mName;
        this.mClassName = mClassName;
        this.methods = methods;
    }

    @Override
    public String[] getMethods() {
        if(methods == null){
            return new String[0];
        }
        return methods;
    }

    @Override
    public Invoker getMethodInvoker(String name) {
        if (mMethodMap == null) {
            generateMethodMap();
        }
        return mMethodMap.get(name);
    }

    @Override
    public T buildInstance() throws IllegalAccessException, InstantiationException {
        if(mClazz == null){
            mClazz = (Class<T>) WXSDKManager.getInstance().getClassLoaderAdapter().getModuleClass(mName, mClassName, WXEnvironment.getApplication().getApplicationContext());
        }
        return mClazz.newInstance();
    }

    public T buildInstance(WXSDKInstance instance) throws IllegalAccessException, InstantiationException {
        if(instance == null){
            return buildInstance();
        }
        if(mClazz == null || mClassLoader != instance.getContext().getClassLoader()){
            mClazz = (Class<T>) WXSDKManager.getInstance().getClassLoaderAdapter().getModuleClass(mName, mClassName, instance.getContext());
            mClassLoader = instance.getContext().getClassLoader();
        }
        return mClazz.newInstance();
    }


    private void generateMethodMap() {
        if(WXEnvironment.isApkDebugable()) {
             WXLogUtils.d(TAG, "extractMethodNames:" + mClazz.getSimpleName());
        }
        HashMap<String, Invoker> methodMap = new HashMap<>();
        try {
            for (Method method : mClazz.getMethods()) {
                // iterates all the annotations available in the method
                for (Annotation anno : method.getDeclaredAnnotations()) {
                    if (anno != null) {
                        if(anno instanceof JSMethod) {
                            JSMethod methodAnnotation = (JSMethod) anno;
                            String name = JSMethod.NOT_SET.equals(methodAnnotation.alias())? method.getName():methodAnnotation.alias();
                            methodMap.put(name, new MethodInvoker(method, methodAnnotation.uiThread()));
                            break;
                        }else if(anno instanceof WXModuleAnno) {
                            WXModuleAnno methodAnnotation = (WXModuleAnno)anno;
                            methodMap.put(method.getName(), new MethodInvoker(method,methodAnnotation.runOnUIThread()));
                            break;
                        }
                    }
                }
            }
        } catch (Throwable e) {
            WXLogUtils.e("[WXModuleManager] extractMethodNames:", e);
        }
        mMethodMap = methodMap;
    }


    public static ConfigModuleFactory fromConfig(JSONObject config){
        try{
            if(config == null){
                return null;
            }

            String name = config.getString("name");
            String className = config.getString("className");
            JSONArray methods = config.getJSONArray("methods");
            if(TextUtils.isEmpty(name) || TextUtils.isEmpty(className)){
                return null;
            }
            String[]  arrays = new String[methods.size()];
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.d(TAG, " resolve module " + name + " className " + className +  " methods " + methods);
            }
            return new ConfigModuleFactory(name, className, methods.toArray(arrays));
        }catch (Exception e){
             WXLogUtils.e(TAG, e);
             return null;
        }
    }

    public String getName() {
        return mName;
    }
}
