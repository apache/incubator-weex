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

import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.bridge.ModuleFactory;

import java.util.ArrayList;
import java.util.Map;

/**
 * Created by sospartan on 7/27/16.
 */
public class TestModuleFactory extends WXSDKEngine.DestroyableModuleFactory<TestModule> {


  public TestModuleFactory(Class<TestModule> clz) {
    super(clz);
  }
}
