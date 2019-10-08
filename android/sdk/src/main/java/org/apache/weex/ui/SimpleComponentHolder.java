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
package org.apache.weex.ui;

import android.util.Pair;

import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.bridge.Invoker;
import org.apache.weex.bridge.MethodInvoker;
import org.apache.weex.annotation.Component;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXRuntimeException;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXComponentProp;
import org.apache.weex.ui.component.WXVContainer;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import org.apache.weex.bridge.WXBridgeManager;

/**
 * Created by sospartan on 6/12/16.
 */
public class SimpleComponentHolder implements IFComponentHolder{
  public static final String TAG = "SimpleComponentHolder";
  private final Class<? extends WXComponent> mClz;
  private Map<String, Invoker> mPropertyInvokers;
  private Map<String, Invoker> mMethodInvokers;
  private ComponentCreator mCreator;

  public static class ClazzComponentCreator implements ComponentCreator{

    private Constructor<? extends WXComponent> mConstructor;
    private final Class<? extends WXComponent> mCompClz;

    public ClazzComponentCreator(Class<? extends WXComponent> c){
      mCompClz = c;
    }

    private void loadConstructor() {
      Class<? extends WXComponent> c = mCompClz;
      Constructor<? extends WXComponent> constructor;
      try {
        constructor = c.getConstructor(WXSDKInstance.class, WXVContainer.class, BasicComponentData.class);
      } catch (NoSuchMethodException e) {
        WXLogUtils.d("ClazzComponentCreator", "Use deprecated component constructor");
        try {
          //compatible deprecated constructor with 4 args
          constructor = c.getConstructor(WXSDKInstance.class, WXVContainer.class, boolean.class, BasicComponentData.class);
        } catch (NoSuchMethodException e1) {
          try {
            //compatible deprecated constructor with 5 args
            constructor = c.getConstructor(WXSDKInstance.class, WXVContainer.class, String.class, boolean.class, BasicComponentData.class);
          } catch (NoSuchMethodException e2) {
            throw new WXRuntimeException("Can't find constructor of component.");
          }
        }
      }
      mConstructor = constructor;
    }

    @Override
    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      if(mConstructor == null){
        loadConstructor();
      }
      int parameters = mConstructor.getParameterTypes().length;
      WXComponent component;

      if(parameters == 3){
        component =  mConstructor.newInstance(instance,parent, basicComponentData);
      }else if(parameters == 4){
        component =  mConstructor.newInstance(instance,parent,false, basicComponentData);
      }else{
        //compatible deprecated constructor
        component =  mConstructor.newInstance(instance,parent,instance.getInstanceId(),parent.isLazy());
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

  public static Pair<Map<String,Invoker>,Map<String,Invoker>> getMethods(Class clz){
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
    }catch (Exception e){ // in meizhu mobile, throw class not found exception in getMethods
      WXLogUtils.e(TAG, e);
    }
    return new Pair<>(methods,mInvokers);
  }



  @Override
  public synchronized WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
    WXComponent component = mCreator.createInstance(instance, parent, basicComponentData);

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
  public synchronized String[] getMethods() {
    if(mMethodInvokers == null){
      generate();
    }
    Set<String> keys = mMethodInvokers.keySet();
    try {
      return keys.toArray(new String[keys.size()]);
    } catch (Throwable throwable) {
      if(mClz != null) {
        String name = mClz.getName();
        String errorMsg = name + ": gen methods failed";
        WXExceptionUtils.commitCriticalExceptionRT(null,
                WXErrorCode.WX_KEY_EXCEPTION_INVOKE_REGISTER_COMPONENT,
                WXBridgeManager.METHOD_REGISTER_COMPONENTS, errorMsg,
                null);
      }

      return new String[1];
    }

  }

}
