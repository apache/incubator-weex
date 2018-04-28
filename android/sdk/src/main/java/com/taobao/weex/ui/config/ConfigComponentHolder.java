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

import android.text.TextUtils;
import android.util.Pair;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.IFComponentHolder;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.reflect.InvocationTargetException;
import java.util.Map;

/**
 * Created by furture on 2018/2/7.
 */
public class ConfigComponentHolder implements IFComponentHolder {


    public static final String TAG = AutoScanConfigRegister.TAG;
    private Map<String, Invoker> mPropertyInvokers;
    private Map<String, Invoker> mMethodInvokers;

    private ClassLoader mClassLoader;

    private String mType;
    private boolean mAppendTree;
    private String mClassName;
    private String[] methods;
    private Class mClass;

    public ConfigComponentHolder(String mType, boolean mAppendTree, String mClassName, String[] methods) {
        this.mType = mType;
        this.mAppendTree = mAppendTree;
        this.mClassName = mClassName;
        this.methods = methods;
    }

    @Override
    public void loadIfNonLazy() {
    }

    private synchronized boolean generate(){
        if(mClass == null){
            return false;
        }

        Pair<Map<String, Invoker>, Map<String, Invoker>> methodPair = SimpleComponentHolder.getMethods(mClass);
        mPropertyInvokers = methodPair.first;
        mMethodInvokers = methodPair.second;
        return true;
    }



    @Override
    public synchronized WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
        if(mClass == null || mClassLoader != instance.getContext().getClassLoader()){
            mClass = WXSDKManager.getInstance().getClassLoaderAdapter().getComponentClass(mType, mClassName, instance);
            mClassLoader = instance.getContext().getClassLoader();
        }
        ComponentCreator creator = new SimpleComponentHolder.ClazzComponentCreator(mClass);
        WXComponent component = creator.createInstance(instance, parent, basicComponentData);

        component.bindHolder(this);
        return component;
    }

    @Override
    public synchronized Invoker getPropertyInvoker(String name){
        if (mPropertyInvokers == null && !generate()) {
            return null;
        }

        return mPropertyInvokers.get(name);
    }

    @Override
    public Invoker getMethodInvoker(String name) {
        if(mMethodInvokers == null && !generate()){
            return null;
        }
        return mMethodInvokers.get(name);
    }

    @Override
    public String[] getMethods() {
        if(methods == null){
            //generate failed
            return new String[0];
        }
        return methods;
    }


    public static final ConfigComponentHolder fromConfig(JSONObject config){
        if(config == null){
            return null;
        }
        try{
            String type = config.getString("name");
            boolean appendTree  = config.getBooleanValue("appendTree");
            String className = config.getString("className");
            JSONArray methods =  null;
            if(config.containsKey("methods")) {
                methods = config.getJSONArray("methods");
            }
            if(TextUtils.isEmpty(type) || TextUtils.isEmpty(className)){
                return null;
            }
            String[]  arrays = new String[0];
            if(methods != null){
                arrays = new String[methods.size()];
                methods.toArray(arrays);
            }
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.d(TAG, "resolve component " + type + " className " + className +  " methods " + methods);
            }
            return new ConfigComponentHolder(type, appendTree, className, arrays);
        }catch (Exception e){
            WXLogUtils.e(TAG, e);
            return null;
        }

    }

    public boolean isAppendTree() {
        return mAppendTree;
    }

    public String getType() {
        return mType;
    }
}
