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
package com.taobao.weex.common;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.bridge.MethodInvoker;
import com.taobao.weex.bridge.ModuleFactory;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Use class
 * Created by sospartan on 6/17/16.
 */
public class TypeModuleFactory<T extends WXModule> implements ModuleFactory<T> {
  public static final String TAG = "TypeModuleFactory";
  Class<T> mClazz;
  Map<String, Invoker> mMethodMap;

  public TypeModuleFactory(Class<T> clz) {
    mClazz = clz;
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


  @Override
  public T buildInstance() throws IllegalAccessException, InstantiationException {
    return mClazz.newInstance();
  }

  @Override
  public String[] getMethods() {
    if (mMethodMap == null) {
      generateMethodMap();
    }
    Set<String> keys = mMethodMap.keySet();
    return keys.toArray(new String[keys.size()]);
  }

  @Override
  public Invoker getMethodInvoker(String name) {
    if (mMethodMap == null) {
      generateMethodMap();
    }
    return mMethodMap.get(name);
  }
}
