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
package com.taobao.weex.ui.component;

import android.view.View;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.common.Constants;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.view.IWebView;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 28/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXWebTest {

  public static WXWeb create() throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXWeb) new SimpleComponentHolder(WXWeb.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), WXDivTest.create());
  }

  WXWeb component;
  ProxyWebView mWebView;

  static class ProxyWebView implements IWebView {
    IWebView mIWebView;
    OnPageListener mOnPageListener;
    OnErrorListener mOnErrorListener;
    OnMessageListener mOnMessageListener;

    ProxyWebView(IWebView proxy){
      mIWebView = proxy;
    }

    @Override
    public View getView() {
      return mIWebView.getView();
    }

    @Override
    public void destroy() {
      mIWebView.destroy();
    }

    @Override
    public void loadUrl(String url) {
      mIWebView.loadUrl(url);
    }

    @Override
    public void loadDataWithBaseURL(String source) {
      mIWebView.loadDataWithBaseURL(source);
    }

    @Override
    public void reload() {
      mIWebView.reload();
    }

    @Override
    public void goBack() {
      mIWebView.goBack();
    }

    @Override
    public void goForward() {
      mIWebView.goForward();
    }

    @Override
    public void postMessage(Object msg) {}

    @Override
    public void setShowLoading(boolean shown) {
      mIWebView.setShowLoading(shown);
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
      mIWebView.setOnErrorListener(listener);
      mOnErrorListener = listener;
    }

    @Override
    public void setOnPageListener(OnPageListener listener) {
      mIWebView.setOnPageListener(listener);
      mOnPageListener = listener;
    }

    @Override
    public void setOnMessageListener(OnMessageListener listener) {
      mIWebView.setOnMessageListener(listener);
      mOnMessageListener = listener;
    }
  }

  @Before
  public void setUp() throws Exception {
    component = create();
    mWebView = new ProxyWebView(component.mWebView);
    component.mWebView = mWebView;
    ComponentTest.create(component);

  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }

  @Test
  public void testSetProperty() throws Exception {
    component.setProperty(Constants.Name.SHOW_LOADING,true);
    component.setProperty(Constants.Name.SRC,"http://taobao.com");
    component.setProperty(Constants.Name.SOURCE, "<p><span>hello weex</span></p>");
  }

  @Test
  public void testSetAction() throws Exception {
    Map<String, Object> msg = new HashMap<>();
    msg.put("test1", 1);
    msg.put("test2", "2");
    component.setAction(WXWeb.GO_BACK, null);
    component.setAction(WXWeb.GO_FORWARD, null);
    component.setAction(WXWeb.RELOAD, null);
    component.setAction(WXWeb.POST_MESSAGE, msg);
  }

  @Test
  public void testListener() throws Exception {
    Map<String, Object> msg = new HashMap<>();
    msg.put("test1", 1);
    msg.put("test2", "2");
    component.addEvent(Constants.Event.RECEIVEDTITLE);
    component.addEvent(Constants.Event.PAGESTART);
    component.addEvent(Constants.Event.PAGEFINISH);
    component.addEvent(Constants.Event.ERROR);
    component.addEvent(Constants.Event.ONMESSAGE);
    mWebView.mOnPageListener.onPageFinish("http://taobao.com",true,true);
    mWebView.mOnPageListener.onReceivedTitle("test");
    mWebView.mOnPageListener.onPageStart("http://taobao.com");
    mWebView.mOnErrorListener.onError("test","error occurred");
    mWebView.mOnMessageListener.onMessage(msg);
  }
}
