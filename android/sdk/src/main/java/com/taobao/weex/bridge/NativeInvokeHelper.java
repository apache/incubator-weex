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

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.performance.WXAnalyzerDataTransfer;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXReflectionUtils;

import java.lang.reflect.Type;

/**
 * Created by sospartan on 10/11/2016.
 */

public class NativeInvokeHelper {
  private String mInstanceId;

  public NativeInvokeHelper(String instanceId){
    mInstanceId = instanceId;
  }

  public Object invoke(final Object target,final Invoker invoker,JSONArray args) throws Exception {
    final Object[] params = prepareArguments(
            invoker.getParameterTypes(),
            args);

    if (WXAnalyzerDataTransfer.isInteractionLogOpen() && invoker instanceof MethodInvoker) {
      for (int i = 0; i < params.length; i++) {
        if (params[i] instanceof SimpleJSCallback) {
          final String callBackId = ((SimpleJSCallback)params[i]).getCallbackId();
          Log.d(WXAnalyzerDataTransfer.INTERACTION_TAG, "[client][callNativeModuleStart]," + mInstanceId + "," + ((MethodInvoker) invoker).mMethod.getDeclaringClass() + "," + ((MethodInvoker) invoker).mMethod.getName() + "," + callBackId);
          ((SimpleJSCallback) params[i]).setInvokerCallback(new SimpleJSCallback.InvokerCallback() {
            @Override
            public void onInvokeSuccess() {
              Log.d(WXAnalyzerDataTransfer.INTERACTION_TAG, "[client][callNativeModuleEnd]," + mInstanceId + "," + ((MethodInvoker) invoker).mMethod.getDeclaringClass() + "," + ((MethodInvoker) invoker).mMethod.getName() + "," + callBackId);
            }
          });
          break;
        }
      }
    }

    if (invoker.isRunOnUIThread()) {
      WXSDKManager.getInstance().postOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (invoker != null) {
            try {
              WXSDKInstance targetInstance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
              if (null == targetInstance || targetInstance.isDestroy()){
                return;
              }
              invoker.invoke(target, params);
            } catch (Exception e) {
              WXLogUtils.e("NativeInvokeHelper",target + " Invoker " + invoker.toString()+" exception:"+e);
            }
          }
        }
      }, 0);
    } else {
      return invoker.invoke(target, params);
    }
    return null;
  }

  protected Object[] prepareArguments(Type[] paramClazzs, JSONArray args) throws Exception {
    Object[] params = new Object[paramClazzs.length];
    Object value;
    Type paramClazz;
    for (int i = 0; i < paramClazzs.length; i++) {
      paramClazz = paramClazzs[i];
      if(i>=args.size()){
        if(!paramClazz.getClass().isPrimitive()) {
          params[i] = null;
          continue;
        }else {
          throw new Exception("[prepareArguments] method argument list not match.");
        }
      }
      value = args.get(i);

      if (paramClazz == JSONObject.class) {
        if(value instanceof  JSONObject || value == null) {
          params[i] = value;
        }else if (value instanceof String){
          params[i] = JSON.parseObject(value.toString());
        }
      } else if(JSCallback.class == paramClazz){
        if(value instanceof String){
          params[i] = new SimpleJSCallback(mInstanceId,(String)value);
        }else{
          throw new Exception("Parameter type not match.");
        }
      } else {
        params[i] = WXReflectionUtils.parseArgument(paramClazz,value);
      }
    }
    return params;
  }
}
