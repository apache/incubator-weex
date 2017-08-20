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
package com.taobao.weex.appfram.storage;

import static org.mockito.Mockito.anyMapOf;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import com.taobao.weappplus_sdk.BuildConfig;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 7/28/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
public class DefaultWXStorageTest {

  @Mock
  WXSQLiteOpenHelper supplier;
  @Mock
  IWXStorageAdapter.OnResultReceivedListener listener;
  DefaultWXStorage storage;

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Before
  public void setup() throws Exception{
    MockitoAnnotations.initMocks(this);
    storage = new DefaultWXStorage(RuntimeEnvironment.application);
    mockStatic(WXSQLiteOpenHelper.class);
    whenNew(WXSQLiteOpenHelper.class)
            .withArguments(RuntimeEnvironment.application)
            .thenReturn(supplier);
  }


  @Test
  public void testSetItem() throws Exception {
    storage.setItem("","",listener);

    verify(listener,timeout(3000).times(1)).onReceived(anyMapOf(String.class,Object.class));
    storage.close();
  }

  @Test
  public void testGetItem() throws Exception {
    storage.getItem("",listener);

    verify(listener,timeout(3000).times(1)).onReceived(anyMapOf(String.class,Object.class));
    storage.close();
  }

  @Test
  public void testRemoveItem() throws Exception {
    storage.removeItem("",listener);

    verify(listener,timeout(3000).times(1)).onReceived(anyMapOf(String.class,Object.class));
    storage.close();
  }

  @Test
  public void testLength() throws Exception {
    storage.length(listener);

    verify(listener,timeout(3000).times(1)).onReceived(anyMapOf(String.class,Object.class));
    storage.close();
  }

  @Test
  public void testGetAllKeys() throws Exception {
    storage.getAllKeys(listener);

    verify(listener,timeout(3000).times(1)).onReceived(anyMapOf(String.class,Object.class));
    storage.close();
  }
}
