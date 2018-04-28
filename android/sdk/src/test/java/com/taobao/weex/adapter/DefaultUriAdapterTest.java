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
package com.taobao.weex.adapter;

import android.net.Uri;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.WXSDKManagerTest;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.common.WXRequest;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.internal.util.MockUtil;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import static org.junit.Assert.assertEquals;


/**
 * Created by sospartan on 21/11/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({"org.mockito.*", "org.robolectric.*", "android.*"})
public class DefaultUriAdapterTest {
  URIAdapter adapter;
  WXSDKInstance instance;

  @Before
  public void setup() {
    WXEnvironment.sApplication = RuntimeEnvironment.application;
    WXSDKManager wxsdkManager = WXSDKManager.getInstance();
    if (!new MockUtil().isSpy(wxsdkManager)) {
      WXSDKManager spy = Mockito.spy(wxsdkManager);
      WXSDKManagerTest.setInstance(spy);
      Mockito.when(spy.getIWXHttpAdapter()).thenReturn(new IWXHttpAdapter() {
        @Override
        public void sendRequest(WXRequest request, OnHttpListener listener) {
          //do nothing.
        }
      });
    }

    adapter = new DefaultUriAdapter();
    instance = WXSDKInstanceTest.createInstance();
  }

  @Test
  public void testRewrite() throws Exception {

    final String host = "http://127.0.0.1";
    final String base = host + "/test/123/";
    final String bundleWithSlash = base + "?arg=value";
    final String bundle = base + "bundle.js?arg=value";


    instance.renderByUrl("", bundle, null, null, 0, 0, WXRenderStrategy.APPEND_ONCE);
    testRelative(host, base, bundle);
    instance.renderByUrl("", bundleWithSlash, null, null, 0, 0, WXRenderStrategy.APPEND_ONCE);
    testRelative(host, base, bundleWithSlash);
  }

  private void testRelative(String host, String baseWithSlash, String bundleUrl) {
    Uri uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("./456"));
    assertEquals(Uri.parse(baseWithSlash + "./456"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("../456"));
    assertEquals(Uri.parse(baseWithSlash + "../456"), uri);


    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("./456?k=v"));
    assertEquals(Uri.parse(baseWithSlash + "./456?k=v"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("./456?k=v&k2=v2"));
    assertEquals(Uri.parse(baseWithSlash + "./456?k=v&k2=v2"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("./456?k=v#flagment"));
    assertEquals(Uri.parse(baseWithSlash + "./456?k=v#flagment"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("//test.com/test/test2"));
    assertEquals(Uri.parse("http://test.com/test/test2"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse("/test2"));
    assertEquals(Uri.parse(host + "/test2"), uri);

    uri = adapter.rewrite(instance, URIAdapter.IMAGE, Uri.parse(""));
    assertEquals(Uri.parse(bundleUrl), uri);
  }

}
