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

import com.taobao.weappplus_sdk.BuildConfig;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricGradleTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import static org.mockito.Mockito.anyMapOf;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

/**
 * Created by sospartan on 7/28/16.
 */
@RunWith(RobolectricGradleTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest(WXSQLiteOpenHelper.class)
public class DefaultWXStorageTest {

  WXSQLiteOpenHelper supplier;
  DefaultWXStorage storage;
  IWXStorageAdapter.OnResultReceivedListener listener;

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Before
  public void setup() throws Exception{
    supplier = Mockito.mock(WXSQLiteOpenHelper.class);
    listener = Mockito.mock(IWXStorageAdapter.OnResultReceivedListener.class);
    storage = new DefaultWXStorage(RuntimeEnvironment.application);

    mockStatic(WXSQLiteOpenHelper.class);
    PowerMockito.whenNew(WXSQLiteOpenHelper.class)
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
