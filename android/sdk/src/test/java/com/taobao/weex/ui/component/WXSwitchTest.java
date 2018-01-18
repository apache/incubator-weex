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
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.common.Constants;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.view.WXSwitchView;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;

/**
 * Created by sospartan on 28/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXSwitchTest {

  public static WXSwitch create() throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXSwitch) new SimpleComponentHolder(WXSwitch.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), WXDivTest.create());
  }

  WXSwitch component;

  @Before
  public void setUp() throws Exception {
    component = create();
    ComponentTest.create(component, Mockito.mock(WXSwitchView.class));
  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }

  @Test
  public void testAddEvent() throws Exception {
    component.addEvent(Constants.Event.CHANGE);
  }

  @Test
  public void testSetProperty() throws Exception {
    component.setProperty(Constants.Name.CHECKED,true);
  }
}
