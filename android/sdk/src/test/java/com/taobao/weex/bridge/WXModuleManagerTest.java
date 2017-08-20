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

import com.alibaba.fastjson.JSONArray;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.InitConfig;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.common.TestModule;
import com.taobao.weex.common.TypeModuleFactory;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/31/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class)
public class WXModuleManagerTest {

  WXSDKInstance instance;

  @Before
  public void setUp() throws Exception {
    WXSDKEngine.initialize(RuntimeEnvironment.application,new InitConfig.Builder().build());
    instance = WXSDKInstanceTest.createInstance();
    WXModuleManager.registerModule("test",null,false);
    WXModuleManager.registerModule("test1",new TypeModuleFactory<>(TestModule.class),true);
    WXModuleManager.registerModule("test2",new TypeModuleFactory<>(TestModule.class),false);
    WXBridgeManagerTest.getLooper().idle();
  }

  @After
  public void tearDown() throws Exception {

  }

  @Test
  public void testCallModuleMethod() throws Exception {
    WXModuleManager.callModuleMethod(instance.getInstanceId(),"test1","testMethod",null);
    WXModuleManager.callModuleMethod(instance.getInstanceId(),"test2","testMethod",null);
  }

  @Test
  public void testCallModuleMethod2() throws Exception {
    JSONArray args = new JSONArray();
    args.add("testarg");
    WXModuleManager.callModuleMethod(instance.getInstanceId(),"test1","testMethod",args);
  }

  @Test
  public void testCallModuleMethod3() throws Exception {
    JSONArray args = new JSONArray();
    args.add("testarg");
    args.add(null);
    WXModuleManager.callModuleMethod(instance.getInstanceId(),"test1","testCallbackMethod",args);
  }

  @Test
  public void testCallModuleMethod4() throws Exception {
    JSONArray args = new JSONArray();
    args.add("testarg");
    args.add("testcallbackId");
    WXModuleManager.callModuleMethod(instance.getInstanceId(),"test1","testCallbackMethod",args);
  }

  @Test
  public void testDestroyInstanceModules() throws Exception {
    testCallModuleMethod();//module instance is lazy create.
    WXModuleManager.destroyInstanceModules(instance.getInstanceId());
  }

  @Test
  public void testReload() throws Exception {
    WXModuleManager.reload();
  }
}
