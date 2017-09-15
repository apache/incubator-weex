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

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.bridge.WXBridgeManager;
import java.util.Map;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 7/28/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest(WXBridgeManager.class)
public class WXStorageModuleTest {


  WXStorageModule module;
  JSCallback callback;

  @Before
  public void setUp() throws Exception {
    module = new WXStorageModule();
    module.mWXSDKInstance = WXSDKInstanceTest.createInstance();
    module.mStorageAdapter = new IWXStorageAdapter() {
      @Override
      public void setItem(String key, String value, OnResultReceivedListener listener) {
        Map<String, Object> data = StorageResultHandler.setItemResult(true);
        listener.onReceived(data);
      }

      @Override
      public void getItem(String key, OnResultReceivedListener listener) {
        Map<String, Object> data = StorageResultHandler.setItemResult(true);
        listener.onReceived(data);
      }

      @Override
      public void removeItem(String key, OnResultReceivedListener listener) {
        Map<String, Object> data = StorageResultHandler.setItemResult(true);
        listener.onReceived(data);
      }

      @Override
      public void length(OnResultReceivedListener listener) {
        Map<String, Object> data = StorageResultHandler.setItemResult(true);
        listener.onReceived(data);
      }

      @Override
      public void getAllKeys(OnResultReceivedListener listener) {
        Map<String, Object> data = StorageResultHandler.setItemResult(true);
        listener.onReceived(data);
      }

      @Override
      public void setItemPersistent(String key, String value, OnResultReceivedListener listener) {

      }

      @Override
      public void close() {

      }
    };
    callback = Mockito.mock(JSCallback.class);
  }

  @Test
  public void testSetItem() throws Exception {
    module.setItem("","",callback);
    verify(callback,times(1)).invoke(any());

  }

  @Test
  public void testGetItem() throws Exception {
    module.getItem("",callback);
    verify(callback,times(1)).invoke(any());
  }

  @Test
  public void testRemoveItem() throws Exception {
    module.removeItem("",callback);
    verify(callback,times(1)).invoke(any());
  }

  @Test
  public void testLength() throws Exception {
    module.length(callback);
    verify(callback,times(1)).invoke(any());
  }

  @Test
  public void testGetAllKeys() throws Exception {
    module.getAllKeys(callback);
    verify(callback,times(1)).invoke(any());
  }
}
