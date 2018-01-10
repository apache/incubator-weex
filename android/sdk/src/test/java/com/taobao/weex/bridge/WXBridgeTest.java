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

import com.alibaba.fastjson.JSON;
import com.taobao.weappplus_sdk.BuildConfig;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 27/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXBridgeTest {

  WXBridge bridge;

  @Before
  public void setUp() throws Exception {
    bridge = new WXBridge();
  }

  @Test
  public void testCallNative() throws Exception {
    bridge.callNative("1", JSON.parseArray("[]"),"100");
  }

  @Test
  public void testCallAddElement() throws Exception {
    bridge.callAddElement("1","1", JSON.parseObject("{}"),"0","100");
  }


  @After
  public void tearDown() throws Exception {

  }

  @Test
  public void testReportJSException() throws Exception {
    bridge.reportJSException("1","test","some exception");
  }

  @Test
  public void testSetTimeoutNative() throws Exception {
    bridge.setTimeoutNative("100","1024");
  }

  @Test
  public void testSetJSFrmVersion() throws Exception {
    bridge.setJSFrmVersion("v0.1");
  }
}
