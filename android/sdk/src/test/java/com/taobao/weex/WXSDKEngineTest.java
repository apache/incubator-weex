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

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.test.mock.MockApplication;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.bridge.WXBridgeManagerTest;
import com.taobao.weex.common.TestModule;
import com.taobao.weex.common.TestModuleFactory;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.http.WXStreamModule;
import com.taobao.weex.ui.component.TestComponent;
import com.taobao.weex.ui.component.WXComponent;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 7/20/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(manifest = Config.NONE)
@PrepareForTest({})
public class WXSDKEngineTest {

  @Before
  public void setUp() throws Exception {

  }

  @After
  public void tearDown() throws Exception {
    WXBridgeManagerTest.getLooper().idle();
  }

  @Test
  public void testInit() throws Exception {
    assertFalse(WXSDKEngine.isInitialized());
    WXSDKEngine.initialize(RuntimeEnvironment.application,null);

    new Thread(){
      public void run(){
        try {
          Thread.sleep(60000);
          assertTrue(WXSDKEngine.isInitialized());
        } catch (InterruptedException e) { }
      }
    }.start();


    //keep compatible
    WXSDKEngine.init(RuntimeEnvironment.application);
    WXSDKEngine.init(RuntimeEnvironment.application,null);
    WXSDKEngine.init(RuntimeEnvironment.application,null,null);
    WXSDKEngine.init(RuntimeEnvironment.application,null,null,null,null);
  }

  @Test
  public void testRegisterComponent() throws Exception {
    assertFalse(WXSDKEngine.registerComponent(null,(Class<? extends WXComponent>) null,true));
    assertTrue(WXSDKEngine.registerComponent("test", TestComponent.class,true));
    assertTrue(WXSDKEngine.registerComponent("test1",TestComponent.class));
    assertTrue(WXSDKEngine.registerComponent(TestComponent.class,false,"testA","testB","testC"));
    Map<String,Object> compInfo = new HashMap<>();
    assertFalse(WXSDKEngine.registerComponent(compInfo,TestComponent.class));
  }

  @Test
  public void testRegisterModule() throws Exception {
    assertTrue(WXSDKEngine.registerModule("test", TestModule.class));
    assertFalse(WXSDKEngine.registerModule(null,TestModule.class));
    assertFalse(WXSDKEngine.registerModule("test",null));

    assertFalse(WXSDKEngine.registerModuleWithFactory(null,new TestModuleFactory(TestModule.class),true));
    assertTrue(WXSDKEngine.registerModuleWithFactory("test1",new TestModuleFactory(TestModule.class),true));
  }


  @Test
  public void testRegisterDomObject() throws Exception {
    assertFalse(WXSDKEngine.registerDomObject("test",null));
    assertFalse(WXSDKEngine.registerDomObject("", TestDomObject.class));
    assertTrue(WXSDKEngine.registerDomObject("test",TestDomObject.class));
  }
}
