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
package com.taobao.weex.dom;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.ui.WXRenderManager;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.shadows.ShadowLooper;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/31/16.
 */
@RunWith(RobolectricTestRunner.class)
public class WXDomManagerTest {

  public static ShadowLooper getLooper() {
    return Shadows.shadowOf(WXSDKManager.getInstance().getWXDomManager().mDomHandler.getLooper());
  }

  WXDomManager dm;
  WXSDKInstance instance;

  @Before
  public void setUp() throws Exception {
    WXRenderManager rm = new WXRenderManager();
    instance =   WXSDKInstanceTest.createInstance();
    rm.registerInstance(instance);
    dm = new WXDomManager(rm);
  }

  @Test
  public void testRemoveDomStatement() throws Exception {
      dm.removeDomStatement(instance.getInstanceId());
  }

  @After
  public void tearDown() throws Exception {
    getLooper().idle();
    dm.destroy();
  }


}
