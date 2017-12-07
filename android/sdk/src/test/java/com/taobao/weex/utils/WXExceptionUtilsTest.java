/**
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

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Created by atomtong on 2017/10/19.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest({WXEnvironment.class})

public class WXExceptionUtilsTest {
  static class Exp extends WXExceptionUtils{};

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Before
  public void setUp() throws Exception {
	PowerMockito.mockStatic(WXEnvironment.class);
	PowerMockito.when(WXEnvironment.isApkDebugable()).thenReturn(true);
  }

  @Test
  public void testExpCommitNull(){
	Exp.commitCriticalExceptionRT(null,null,null,null,null);
  }

  @Test
  public void testExpCommitString(){
	Map testMap =  new HashMap<String, String>();
	testMap.put("weexUrl","weexurltest");
	testMap.put("bundleUrl","bundleUrltest");
	Exp.commitCriticalExceptionRT("1","test","test","test",testMap);
	Exp.commitCriticalExceptionRT("1","test","test","test",null);
  }
}