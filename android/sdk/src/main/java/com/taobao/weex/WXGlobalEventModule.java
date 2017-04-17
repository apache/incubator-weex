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
package com.taobao.weex;

import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;

import java.util.Map;

/**
 * Created by lixinke on 16/8/25.
 */
public class WXGlobalEventModule extends WXModule {


  @JSMethod
  public void addEventListener(String eventName, String callback) {
    mWXSDKInstance.addEventListener(eventName,callback);
  }

  public void removeEventListener(String eventName, String callback) {
    mWXSDKInstance.removeEventListener(eventName,callback);
  }

  @JSMethod
  public void removeEventListener(String eventName){
    mWXSDKInstance.removeEventListener(eventName);
  }


  @Override
  public void addEventListener(String eventName, String callback, Map<String, Object> options) {
    super.addEventListener(eventName, callback, options);
    addEventListener(eventName,callback);
  }
}
