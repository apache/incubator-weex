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
package com.taobao.weex.utils;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 9/18/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest({WXEnvironment.class})
public class WXLogUtilsTest {
  static class Log extends WXLogUtils{};

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Before
  public void setUp() throws Exception {
    PowerMockito.mockStatic(WXEnvironment.class);
    PowerMockito.when(WXEnvironment.isApkDebugable()).thenReturn(true);
  }

  @Test
  public void testD() throws Exception {
    Log.d("test");
    Log.d(null);
    Log.d("tag","test");
    Log.d("tag","test | __ | __ __DEBUG | __ ");
    Log.d("tag",new Throwable("test"));
  }

  @Test
  public void testI() throws Exception {
    Log.i("test");
    Log.i(null);
    Log.i("tag","test");
    Log.i("tag",new Throwable("test"));
  }

  @Test
  public void testV() throws Exception {
    Log.v("test");
    Log.v(null);
    Log.v("tag","test");
    Log.v("tag",new Throwable("test"));
  }

  @Test
  public void testW() throws Exception {
    Log.w("test");
    Log.w(null);
    Log.w("tag","test");
    Log.w("tag",new Throwable("test"));
  }

  @Test
  public void testE() throws Exception {
    Log.e("test");
    Log.e(null);
    Log.e("tag","test");
    Log.e("tag",new Throwable("test"));
  }

  @Test
  public void testLogLevel() throws Exception {
    WXEnvironment.sLogLevel = LogLevel.DEBUG;
    Log.d("LogLevel.DEBUG", "test debug");
    Log.w("LogLevel.DEBUG", "test warning");
    Log.e("LogLevel.DEBUG", "test error");

    WXEnvironment.sLogLevel = LogLevel.WARN;

    Log.d("LogLevel.WARN", "test debug");
    Log.w("LogLevel.WARN", "test warning");
    Log.e("LogLevel.WARN", "test error");
  }
  }
