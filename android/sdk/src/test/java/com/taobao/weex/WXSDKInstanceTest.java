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
package com.taobao.weex;

import static org.junit.Assert.assertEquals;
import static org.powermock.api.mockito.PowerMockito.mock;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.when;

import android.view.View;
import android.widget.FrameLayout;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.bridge.WXBridgeManagerTest;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.WXDomManagerTest;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXDivTest;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXFileUtils;
import com.taobao.weex.utils.WXSoInstallMgrSdk;
import java.util.HashMap;
import java.util.Map;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLooper;


/**
 * Created by sospartan on 7/27/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19,manifest = Config.NONE)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest({WXFileUtils.class,WXSoInstallMgrSdk.class})
public class WXSDKInstanceTest {
  @Rule
  public PowerMockRule rule = new PowerMockRule();

  public static WXSDKInstance createInstance(){
    WXSDKInstance instance =  new WXSDKInstance(Robolectric.setupActivity(TestActivity.class),"1");
    final FrameLayout container = new FrameLayout(instance.getContext());
    instance.registerRenderListener(new IWXRenderListener() {
      @Override
      public void onViewCreated(WXSDKInstance instance, View view) {
        container.addView(view);
      }

      @Override
      public void onRenderSuccess(WXSDKInstance instance, int width, int height) {

      }

      @Override
      public void onRefreshSuccess(WXSDKInstance instance, int width, int height) {

      }

      @Override
      public void onException(WXSDKInstance instance, String errCode, String msg) {

      }
    });
    instance.mContext = Robolectric.setupActivity(TestActivity.class);

    return instance;
  }

  public static void setupRoot(WXSDKInstance instance){

    WXDomObject domObject = new WXDomObject();
    WXVContainer comp = (WXVContainer) WXComponentFactory.newInstance(instance, domObject, null);

    WXComponent root = WXDivTest.create(comp);
    comp.addChild(root);
    comp.createView();

    instance.onCreateFinish();
    ShadowLooper.idleMainLooper();
  }

  WXSDKInstance mInstance;

  @Before
  public void setup() throws Exception {
    mockStatic(WXSoInstallMgrSdk.class);
    when(WXSoInstallMgrSdk.initSo("weexv8", 1, null)).thenReturn(true);
    WXSDKEngine.initialize(RuntimeEnvironment.application,new InitConfig.Builder().build());
    mInstance = createInstance();
    WXBridgeManagerTest.getLooper().idle();

    mockStatic(WXFileUtils.class);
    when(WXFileUtils.loadAsset(null,null)).thenReturn("{}");
  }

  @After
  public void tearDown() throws Exception {
    WXBridgeManagerTest.getLooper().idle();
    WXDomManagerTest.getLooper().idle();
    mInstance.destroy();
  }


  @Test
  public void testRender() throws Exception {
    assertEquals(WXFileUtils.loadAsset(null,null),"{}");

    mInstance.render("{}",null,null,null);

  }

  @Test
  public void testSetSize() throws Exception {
    setupRoot(mInstance);
    mInstance.setSize(10,10);
  }

  @Test
  public void testRenderEvent() throws Exception {
    mInstance.onRenderError("test","test");
    mInstance.onRenderSuccess(10,10);
  }

  @Test
  public void testRenderByUrl() throws Exception {
    mInstance.renderByUrl(WXPerformance.DEFAULT,"file:///test",null,null,100,100, WXRenderStrategy.APPEND_ASYNC);
    mInstance.renderByUrl(WXPerformance.DEFAULT,"http://taobao.com",null,null,100,100, WXRenderStrategy.APPEND_ASYNC);
  }

  @Test
  public void testGlobalEvent() throws Exception {
    mInstance.addEventListener(null,null);
    mInstance.addEventListener(null,"");
    mInstance.addEventListener("",null);

    mInstance.addEventListener("test","123");
    mInstance.fireGlobalEventCallback("test",null);
    mInstance.removeEventListener("test");
    mInstance.removeEventListener("test","123");
  }

  @Test
  public void testFireEvent() throws Exception {
    mInstance.fireEvent("1","test");
    Map<String,Object> params = new HashMap<>();
    params.put("arg1",null);
    params.put("arg2",123);
    mInstance.fireEvent("1","test",params);

    Map<String,Object> domChange = new HashMap<>();
    domChange.put("attr1","123");
    mInstance.fireEvent("1","test",params,domChange);
  }


  @Test
  public void testOnActivityLifecycle() throws Exception {
    mInstance.registerActivityStateListener(mock(IWXActivityStateListener.class));
    mInstance.onActivityCreate();
    mInstance.onActivityStart();
    mInstance.onActivityResume();
    mInstance.onActivityPause();
    mInstance.onActivityStop();
  }

  @Test
  public void testOnJSException() throws Exception {
    mInstance.onJSException(null,null,null);
    mInstance.onJSException("100","test","some error");
  }
}
