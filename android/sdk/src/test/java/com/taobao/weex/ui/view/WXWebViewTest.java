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
package com.taobao.weex.ui.view;

import static org.junit.Assert.assertNotNull;

import android.app.Activity;
import android.view.ViewGroup;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.TestActivity;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowWebView;

/**
 * Created by sospartan on 9/7/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXWebViewTest {

  IWebView view;
  WebView webView;
  ShadowWebView shadow;

  @Before
  public void setUp() throws Exception {
    Activity activity = Robolectric.setupActivity(TestActivity.class);
    view = new WXWebView(activity);
    webView = (WebView)((ViewGroup)view.getView()).getChildAt(0);//first child
    shadow = Shadows.shadowOf(webView);
  }

  @After
  public void tearDown() throws Exception {
    view.destroy();
  }

  @Test
  public void testDestory() throws Exception {

  }

  @Test
  public void testGetView() throws Exception {
    assertNotNull(view);
  }

  @Test
  public void testLoadUrl() throws Exception {
    String url = "http://www.taobao.com";
    view.loadUrl(url);
    WebViewClient client = shadow.getWebViewClient();
    client.onPageStarted(webView,url,null);
    client.onPageFinished(webView,url);

    WebChromeClient chromeClient = shadow.getWebChromeClient();
    chromeClient.onProgressChanged(webView,10);
    chromeClient.onProgressChanged(webView,100);
    chromeClient.onReceivedTitle(webView,"test");

  }

  @Test
  public void testReload() throws Exception {
    view.reload();
    testLoadUrl();
    view.reload();
  }

  @Test
  public void testGoBack() throws Exception {
    testLoadUrl();
    view.loadUrl("http://www.w3c.org");
    view.goBack();
  }

  @Test
  public void testGoForward() throws Exception {
    testGoBack();
    view.goForward();
  }
}
