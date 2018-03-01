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

import android.util.Pair;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.Set;

/**
 * Created by sospartan on 8/26/16.
 */
public class ExternalLoaderComponentHolder implements IFComponentHolder {
  public static final String TAG = "SimpleComponentHolder";
  private Map<String, Invoker> mPropertyInvokers;
  private Map<String, Invoker> mMethodInvokers;
  private final IExternalComponentGetter mClzGetter;
  private final String mType;
  private Class mClass;


  public ExternalLoaderComponentHolder(String type, IExternalComponentGetter clzGetter) {
    this.mClzGetter = clzGetter;
    mType = type;
  }

  @Override
  public void loadIfNonLazy() {
  }

  private synchronized boolean generate(){
    if(mClass==null){
      return false;
    }

    Pair<Map<String, Invoker>, Map<String, Invoker>> methodPair = SimpleComponentHolder.getMethods(mClass);
    mPropertyInvokers = methodPair.first;
    mMethodInvokers = methodPair.second;
    return true;
  }



  @Override
  public synchronized WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
    if(mClass == null){
      mClass = mClzGetter.getExternalComponentClass(mType,instance);
    }
    ComponentCreator creator = new SimpleComponentHolder.ClazzComponentCreator(mClass);
    WXComponent component = creator.createInstance(instance,node,parent);

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
    if(mMethodInvokers == null && !generate()){
      //generate failed
      return new String[0];
    }
    Set<String> keys = mMethodInvokers.keySet();
    return keys.toArray(new String[keys.size()]);
  }
}
