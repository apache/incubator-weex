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
package com.taobao.weex.ui.module;

import static android.R.attr.end;
import static android.R.attr.start;
import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.assertThat;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Message;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.InitConfig;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXBridgeManagerTest;
import com.taobao.weex.common.WXThread;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.Shadows;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLooper;

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class)
@PowerMockIgnore({"org.mockito.*", "org.robolectric.*", "android.*"})
@PrepareForTest(WXBridgeManager.class)
public class WXTimerModuleTest {

  public final static int VALID_FUNC_ID = 20;
  public final static int NO_CACHING_FUNC_ID = 565654;
  public final static int INVALID_FUNC_ID = 0;
  public final static int DELAY = 50;
  public final static int IMMEDIATELY = 0;
  public final static int INVALID_DELAY = -50;
  public final static float FLOAT_DELAY = 20.6f;

  @Rule
  public PowerMockRule rule = new PowerMockRule();
  WXTimerModule module;
  ShadowLooper mLooper;

  @Before
  public void setup() throws Exception {
    WXSDKEngine.initialize(RuntimeEnvironment.application, new InitConfig.Builder().build());
    WXBridgeManager bridge = Mockito.mock(WXBridgeManager.class);
    when(bridge.getJSLooper()).thenReturn(new WXThread("js").getLooper());
    WXBridgeManagerTest.setBridgeManager(bridge);

    module = Mockito.spy(new WXTimerModule());
    module.mWXSDKInstance = WXSDKInstanceTest.createInstance();
    Handler handler = new Handler(WXBridgeManager.getInstance().getJSLooper(), module);
    mLooper = Shadows.shadowOf(handler.getLooper());
    module.setHandler(handler);
  }

  @Test
  public void testSetTimeoutDelay() throws Exception {
    module.setTimeout(VALID_FUNC_ID, DELAY);
    long start, end, duration;
    start = mLooper.getScheduler().getCurrentTime();
    mLooper.runOneTask();
    end = mLooper.getScheduler().getCurrentTime();
    duration = end - start;

    assertThat(duration, is((long) DELAY));
    Mockito.verify(module, times(1)).handleMessage(any(Message.class));
  }

  @Test
  public void testSetTimeoutDelay2() throws Exception {
    module.setTimeout(VALID_FUNC_ID, FLOAT_DELAY);
    long start, end, duration;
    start = mLooper.getScheduler().getCurrentTime();
    mLooper.runOneTask();
    end = mLooper.getScheduler().getCurrentTime();
    duration = end - start;

    assertThat(duration, is((long) FLOAT_DELAY));
    Mockito.verify(module, times(1)).handleMessage(any(Message.class));
  }

  @Test
  public void testSetTimeoutImmediately() throws Exception {
    module.setTimeout(VALID_FUNC_ID, IMMEDIATELY);
    mLooper.idle(IMMEDIATELY);
    Mockito.verify(module, times(1)).handleMessage(any(Message.class));
  }

  @SuppressWarnings("Range")
  @Test
  public void testSetTimeoutError1() throws Exception {
    module.setTimeout(INVALID_FUNC_ID, DELAY);
    mLooper.idle(DELAY);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @SuppressWarnings("Range")
  @Test
  public void testSetTimeoutError2() throws Exception {
    module.setTimeout(VALID_FUNC_ID, INVALID_DELAY);
    mLooper.runToEndOfTasks();
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @SuppressWarnings("Range")
  @Test
  public void testSetIntervalError1() throws Exception {
    module.setInterval(INVALID_FUNC_ID, DELAY);
    mLooper.idle(DELAY);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @SuppressWarnings("Range")
  @Test
  public void testSetIntervalError2() throws Exception {
    module.setInterval(VALID_FUNC_ID, INVALID_DELAY);
    mLooper.runToEndOfTasks();
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @Test
  public void testSetIntervalImmediately() throws Exception {
    long start, end, duration;
    module.setInterval(VALID_FUNC_ID, DELAY);

    start = mLooper.getScheduler().getCurrentTime();
    mLooper.runOneTask();
    end = mLooper.getScheduler().getCurrentTime();
    duration = end - start;

    assertThat(duration, is((long) DELAY));

    mLooper.runOneTask();
    mLooper.runOneTask();
    mLooper.runOneTask();
    mLooper.runOneTask();
    Mockito.verify(module, times(5)).handleMessage(any(Message.class));
  }

  @Test
  public void testSetIntervalDelay() {
    long start, end, duration;
    module.setInterval(VALID_FUNC_ID, DELAY);

    start = mLooper.getScheduler().getCurrentTime();
    mLooper.runOneTask();
    end = mLooper.getScheduler().getCurrentTime();
    duration = end - start;

    assertThat(duration, is((long) DELAY));

    mLooper.runOneTask();
    mLooper.runOneTask();
    Mockito.verify(module, times(3)).handleMessage(any(Message.class));
  }

  @Test
  public void testSetIntervalDelay2() {
    long start, end, duration;
    module.setInterval(VALID_FUNC_ID, FLOAT_DELAY);

    start = mLooper.getScheduler().getCurrentTime();
    mLooper.runOneTask();
    end = mLooper.getScheduler().getCurrentTime();
    duration = end - start;

    assertThat(duration, is((long) FLOAT_DELAY));

    mLooper.runOneTask();
    mLooper.runOneTask();
    Mockito.verify(module, times(3)).handleMessage(any(Message.class));
  }

  @Test
  public void testClearTimeout() throws Exception {
    module.setTimeout(VALID_FUNC_ID, DELAY);
    module.clearTimeout(VALID_FUNC_ID);
    mLooper.idle(DELAY, TimeUnit.MILLISECONDS);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @Test
  public void testClearInterval() throws Exception {
    module.setInterval(VALID_FUNC_ID, DELAY);
    module.clearInterval(VALID_FUNC_ID);
    mLooper.idle(DELAY, TimeUnit.MILLISECONDS);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @Test
  public void testClearTimeout2(){
    module.setTimeout(NO_CACHING_FUNC_ID, DELAY);
    module.clearTimeout(NO_CACHING_FUNC_ID);
    mLooper.idle(DELAY, TimeUnit.MILLISECONDS);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }

  @Test
  public void testClearInterval2(){
    module.setInterval(NO_CACHING_FUNC_ID, DELAY);
    module.clearInterval(NO_CACHING_FUNC_ID);
    mLooper.idle(DELAY, TimeUnit.MILLISECONDS);
    Mockito.verify(module, never()).handleMessage(any(Message.class));
  }
}
