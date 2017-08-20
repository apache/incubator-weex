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

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.bridge.MethodInvoker;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Created by sospartan on 6/12/16.
 */
public class SimpleComponentHolder implements IFComponentHolder{
  public static final String TAG = "SimpleComponentHolder";
  private final Class<? extends WXComponent> mClz;
  private Map<String, Invoker> mPropertyInvokers;
  private Map<String, Invoker> mMethodInvokers;
  private ComponentCreator mCreator;

  static class ClazzComponentCreator implements ComponentCreator{

    private Constructor<? extends WXComponent> mConstructor;
    private final Class<? extends WXComponent> mCompClz;

    ClazzComponentCreator(Class<? extends WXComponent> c){
      mCompClz = c;
    }

    private void loadConstructor(){
      Class<? extends WXComponent> c = mCompClz;
      Constructor<? extends WXComponent> constructor;
      try {
        constructor = c.getConstructor(WXSDKInstance.class, WXDomObject.class, WXVContainer.class);
      } catch (NoSuchMethodException e) {
        WXLogUtils.d("ClazzComponentCreator","Use deprecated component constructor");
        try {
          //compatible deprecated constructor with 4 args
          constructor = c.getConstructor(WXSDKInstance.class, WXDomObject.class, WXVContainer.class, boolean.class);
        } catch (NoSuchMethodException e1) {
          try {
            //compatible deprecated constructor with 5 args
            constructor = c.getConstructor(WXSDKInstance.class, WXDomObject.class, WXVContainer.class,String.class, boolean.class);
          } catch (NoSuchMethodException e2) {
            throw new WXRuntimeException("Can't find constructor of component.");
          }
        }
      }
      mConstructor = constructor;
    }

    @Override
    public WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      if(mConstructor == null){
        loadConstructor();
      }
      int parameters = mConstructor.getParameterTypes().length;
      WXComponent component;

      if(parameters == 3){
        component =  mConstructor.newInstance(instance,node,parent);
      }else if(parameters == 4){
        component =  mConstructor.newInstance(instance,node,parent,false);
      }else{
        //compatible deprecated constructor
        component =  mConstructor.newInstance(instance,node,parent,instance.getInstanceId(),parent.isLazy());
      }
      return component;
    }
  }

  public SimpleComponentHolder(Class<? extends WXComponent> clz) {
    this(clz,new ClazzComponentCreator(clz));
  }

  public SimpleComponentHolder(Class<? extends WXComponent> clz,ComponentCreator customCreator) {
    this.mClz = clz;
    this.mCreator = customCreator;
  }

  @Override
  public void loadIfNonLazy() {
    Annotation[] annotations = mClz.getDeclaredAnnotations();
    for (Annotation annotation :
      annotations) {
      if (annotation instanceof Component){
        if(!((Component) annotation).lazyload() && mMethodInvokers == null){
          generate();
        }
        return;
      }
    }
  }

  private synchronized void generate(){
    if(WXEnvironment.isApkDebugable()) {
      WXLogUtils.d(TAG, "Generate Component:" + mClz.getSimpleName());
    }

    Pair<Map<String, Invoker>, Map<String, Invoker>> methodPair = getMethods(mClz);
    mPropertyInvokers = methodPair.first;
    mMethodInvokers = methodPair.second;
  }

  static Pair<Map<String,Invoker>,Map<String,Invoker>> getMethods(Class clz){
    Map<String, Invoker> methods = new HashMap<>();
    Map<String, Invoker> mInvokers = new HashMap<>();

    Annotation[] annotations;
    Annotation anno;
    try {
      for (Method method : clz.getMethods()) {
        try {
          annotations = method.getDeclaredAnnotations();
          for (int i = 0, annotationsCount = annotations.length;
               i < annotationsCount; ++i) {
            anno = annotations[i];
            if(anno == null){
              continue;
            }
            if (anno instanceof WXComponentProp) {
              String name = ((WXComponentProp) anno).name();
              methods.put(name, new MethodInvoker(method,true));
              break;
            }else if(anno instanceof JSMethod){
              JSMethod methodAnno = (JSMethod)anno;
              String name = methodAnno.alias();
              if(JSMethod.NOT_SET.equals(name)){
                name = method.getName();
              }
              mInvokers.put(name, new MethodInvoker(method,methodAnno.uiThread()));
              break;
            }
          }
        } catch (ArrayIndexOutOfBoundsException | IncompatibleClassChangeError e) {
          //ignore: getDeclaredAnnotations may throw this
        }
      }
    }catch (IndexOutOfBoundsException e){
      e.printStackTrace();
      //ignore: getMethods may throw this
    }
    return new Pair<>(methods,mInvokers);
  }



  @Override
  public synchronized WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
    WXComponent component = mCreator.createInstance(instance,node,parent);

    component.bindHolder(this);
    return component;
  }

  @Override
  public synchronized Invoker getPropertyInvoker(String name){
      if (mPropertyInvokers == null) {
        generate();
      }

    return mPropertyInvokers.get(name);
  }

  @Override
  public Invoker getMethodInvoker(String name) {
    if(mMethodInvokers == null){
      generate();
    }
    return mMethodInvokers.get(name);
  }

  @Override
  public String[] getMethods() {
    if(mMethodInvokers == null){
      generate();
    }
    Set<String> keys = mMethodInvokers.keySet();
    return keys.toArray(new String[keys.size()]);
  }

}
