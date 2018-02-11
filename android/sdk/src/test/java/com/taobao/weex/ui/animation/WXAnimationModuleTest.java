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
package com.taobao.weex.ui.animation;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 7/29/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*","org.json.*" })
@PrepareForTest()
public class WXAnimationModuleTest {


  WXAnimationModule module;
  @Before
  public void setUp() throws Exception {
    module = new WXAnimationModule();
    module.mWXSDKInstance = WXSDKInstanceTest.createInstance();


  }

  @Test
  public void testTransition() throws Exception {
    module.transition("", JSONObject.parseObject("{}"),"");
    module.transition("test", JSONObject.parseObject("{\"test\":\"test\"}"),"");
  }

  @Test
  public void testCreateAnimatorListener() throws Exception {

  }
}
