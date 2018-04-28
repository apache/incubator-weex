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

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.Constants;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 26/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
public class WXEmbedTest {

  WXEmbed component;

  @Before
  public void setUp() throws Exception {

    WXEnvironment.sApplication = RuntimeEnvironment.application;
    WXDiv div = WXDivTest.create();
    ComponentTest.create(div);
    component = new WXEmbed(div.getInstance(),new TestDomObject(),div);
    ComponentTest.create(component);
    component.getDomObject().getStyles().put(Constants.Name.VISIBILITY, Constants.Value.VISIBLE);
  }

  @Test
  public void testSetProperty() throws Exception {
    component.setProperty(Constants.Name.SRC,"http://www.taobao.com");
  }

  @Test
  public void testSetVisibility() throws Exception {
    component.setProperty(Constants.Name.VISIBILITY,Constants.Value.HIDDEN);
    component.setProperty(Constants.Name.SRC,"http://www.taobao.com");
    component.setProperty(Constants.Name.VISIBILITY,Constants.Value.VISIBLE);
  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }
}
