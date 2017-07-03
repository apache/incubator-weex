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
package com.taobao.weex.appfram.clipboard;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.bridge.JSCallback;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
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
public class WXClipboardModuleTest {

  WXClipboardModule module;

  @Before
  public void setUp() throws Exception {
    module = new WXClipboardModule();
    module.mWXSDKInstance = WXSDKInstanceTest.createInstance();
  }

  @After
  public void tearDown() throws Exception {

  }

  @Test
  public void testSetString() throws Exception {
    module.setString("test");
  }

  @Test
  public void testGetString() throws Exception {

    JSCallback mock = Mockito.mock(JSCallback.class);
    module.getString(mock);
    Mockito.verify(mock,Mockito.times(1)).invoke(Mockito.anyObject());

    testSetString();

    mock = Mockito.mock(JSCallback.class);
    module.getString(mock);
    Mockito.verify(mock,Mockito.times(1)).invoke(Mockito.anyObject());
  }
}
