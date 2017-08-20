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
package com.taobao.weex.appfram.navigator;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.bridge.WXBridgeManager;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.*;
import static org.mockito.Mockito.*;

/**
 * Created by sospartan on 7/28/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*","org.json.*" })
@PrepareForTest(WXSDKEngine.class)
public class WXNavigatorModuleTest {

  WXNavigatorModule module;
  JSCallback callback;

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Before
  public void setUp() throws Exception {
    mockStatic(WXSDKEngine.class);
    callback = Mockito.mock(JSCallback.class);


    module = new WXNavigatorModule();
    module.mWXSDKInstance = WXSDKInstanceTest.createInstance();
  }

  private void mockSetter(){
    when(WXSDKEngine.getActivityNavBarSetter()).thenReturn(new IActivityNavBarSetter() {
      @Override
      public boolean push(String param) {
        return true;
      }

      @Override
      public boolean pop(String param) {
        return true;
      }

      @Override
      public boolean setNavBarRightItem(String param) {
        return true;
      }

      @Override
      public boolean clearNavBarRightItem(String param) {
        return true;
      }

      @Override
      public boolean setNavBarLeftItem(String param) {
        return true;
      }

      @Override
      public boolean clearNavBarLeftItem(String param) {
        return true;
      }

      @Override
      public boolean setNavBarMoreItem(String param) {
        return true;
      }

      @Override
      public boolean clearNavBarMoreItem(String param) {
        return true;
      }

      @Override
      public boolean setNavBarTitle(String param) {
        return true;
      }
    });
  }

  @Test
  public void testPush() throws Exception {
    module.push("{}",callback);
    module.push("{'url':'kdkdkdkdkd'}",callback);
    mockSetter();
    module.push("{}",callback);
  }


  @Test
  public void testPop() throws Exception {
    mockSetter();
    module.pop("{}",callback);
  }

  @Test
  public void testSetNavBarRightItem() throws Exception {
    mockSetter();
    module.setNavBarRightItem("{}",callback);
  }

  @Test
  public void testClearNavBarRightItem() throws Exception {
    mockSetter();
    module.clearNavBarRightItem("{}",callback);
  }

  @Test
  public void testSetNavBarLeftItem() throws Exception {
    mockSetter();
    module.setNavBarLeftItem("{}",callback);
  }

  @Test
  public void testClearNavBarLeftItem() throws Exception {
    mockSetter();
    module.clearNavBarLeftItem("{}",callback);
  }

  @Test
  public void testSetNavBarMoreItem() throws Exception {
    mockSetter();
    module.setNavBarMoreItem("{}",callback);
  }

  @Test
  public void testClearNavBarMoreItem() throws Exception {
    mockSetter();
    module.clearNavBarMoreItem("{}",callback);
  }

  @Test
  public void testSetNavBarTitle() throws Exception {
    mockSetter();
    module.setNavBarTitle("{}",callback);
  }
}
