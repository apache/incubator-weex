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

import static junit.framework.Assert.assertNotNull;

import android.os.Handler;
import android.os.Message;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.InitConfig;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.WXSDKManagerTest;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXJSBridgeMsgType;
import com.taobao.weex.dom.WXDomModule;
import com.taobao.weex.ui.WXRenderManager;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.Shadows;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLooper;


/**
 * Created by lixinke on 16/2/24.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
@PrepareForTest(WXModuleManager.class)
public class WXBridgeManagerTest {

  WXSDKInstance instance;

  public static ShadowLooper getLooper() {
    WXBridgeManager bridgeManager = WXBridgeManager.getInstance();
    Handler handler = bridgeManager.mJSHandler;
    return Shadows.shadowOf(handler.getLooper());
  }

  public static void setBridgeManager(WXBridgeManager bridge) {
    WXBridgeManager.mBridgeManager = bridge;
  }

  private WXBridgeManager getInstance() {
    return WXBridgeManager.getInstance();
  }

  @Before
  public void setUp() throws Exception {
    WXSDKEngine.initialize(RuntimeEnvironment.application,new InitConfig.Builder().build());
    instance = WXSDKInstanceTest.createInstance();
    WXRenderManager rednerManager = new WXRenderManager();
    rednerManager.registerInstance(instance);//
    WXSDKManagerTest.setRenderManager(rednerManager);
  }

  @After
  public void tearDown() throws Exception {
    getLooper().idle();
    getInstance().destroy();
  }

  @Test
  public void testGetInstance() throws Exception {
    WXBridgeManager instance = WXBridgeManager.getInstance();
    assertNotNull(instance);
  }

  @Test
  public void testRestart() throws Exception {
    getInstance().restart();
  }

  @Test
  public void testSetStackTopInstance() throws Exception {
    getInstance().setStackTopInstance("");
  }

  @Test
  public void testCallNative() throws Exception {
    getInstance()
        .callNative(instance.getInstanceId(),
            "[{\"module\":\"testModule\",\"method\":\"test\"}]",
            null);

    getInstance()
        .callNative(instance.getInstanceId(),
            "[{\"module\":\""+WXDomModule.WXDOM+"\",\"method\":\"test\"}]",
            null);

  }

  @Test
  public void testInitScriptsFramework() throws Exception {
    getInstance().initScriptsFramework("");
  }

  @Test
  public void testFireEvent() throws Exception {
    getInstance().fireEvent(instance.getInstanceId(),"100", Constants.Event.CLICK,null,null);
  }

  @Test
  public void testCallback() throws Exception {
    getInstance().callbackJavascript(instance.getInstanceId(),"test",null,false);
  }

  @Test
  public void testRefreshInstance() throws Exception {
    getInstance().refreshInstance(instance.getInstanceId(),null);
  }

  @Test
  public void testCreateInstance() throws Exception {
    getInstance().createInstance(instance.getInstanceId(),"",null,null);
  }

  @Test
  public void testDestroyInstance() throws Exception {
    getInstance().destroyInstance(instance.getInstanceId());
  }

  @Test
  public void testHandleMessage() throws Exception {
    int[] msgs = {
        WXJSBridgeMsgType.INIT_FRAMEWORK,
        WXJSBridgeMsgType.CALL_JS_BATCH,
        WXJSBridgeMsgType.SET_TIMEOUT,
        WXJSBridgeMsgType.MODULE_INTERVAL,
        WXJSBridgeMsgType.MODULE_TIMEOUT
    };
    Message msg = new Message();
    for(int w:msgs) {
      msg.what = w;
      getInstance().handleMessage(msg);
    }
  }

  @Test
  public void testReportJSException() throws Exception {
    getInstance().reportJSException(instance.getInstanceId(),"test","test exception");
  }

}
