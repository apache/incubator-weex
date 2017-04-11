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

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Type;

/**
 * Created by sospartan on 6/16/16.
 */
public class MethodInvoker implements Invoker {

  final Method mMethod;
  final boolean mRunOnUIThread;
  Type[] mParam;

  public MethodInvoker(Method method){
    this(method,false);
  }

  public MethodInvoker(Method method,boolean runInUIThread){
    mMethod = method;
    mParam = mMethod.getGenericParameterTypes();
    mRunOnUIThread = runInUIThread;
  }

  @Override
  public Object invoke(Object receiver, Object... params) throws InvocationTargetException, IllegalAccessException {
    return mMethod.invoke(receiver,params);
  }

  @Override
  public Type[] getParameterTypes() {
    if(mParam ==null){
      mParam = mMethod.getGenericParameterTypes();
    }
    return mParam;
  }

  @Override
  public boolean isRunOnUIThread() {
    return mRunOnUIThread;
  }

  @Override
  public String toString() {
    return mMethod.getName();
  }
}
