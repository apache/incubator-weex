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
import com.taobao.weex.common.Constants;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 7/27/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXComponentTest {

  WXComponent component;

  @Before
  public void setUp() throws Exception {
    WXVContainer root = WXDivTest.create();
    ComponentTest.create(root);
    component = WXDivTest.create(root);
    ComponentTest.create(component);
  }

  @Test
  public void testSetProperty() throws Exception {

    assertTrue(component.setProperty(Constants.Name.VISIBILITY,null));
    assertTrue(component.setProperty(Constants.Name.VISIBILITY, Constants.Value.VISIBLE));

    assertTrue(component.setProperty(Constants.Name.DISABLED,true));
    assertTrue(component.setProperty(Constants.Name.POSITION, Constants.Value.FIXED));
    assertTrue(component.setProperty(Constants.Name.BACKGROUND_COLOR, "#ffffff"));
    assertTrue(component.setProperty(Constants.Name.OPACITY, 0.5f));
    assertTrue(component.setProperty(Constants.Name.BORDER_RADIUS,0.5f));
    assertTrue(component.setProperty(Constants.Name.BORDER_RADIUS,null));
    assertTrue(component.setProperty(Constants.Name.BORDER_WIDTH,null));
    assertTrue(component.setProperty(Constants.Name.BORDER_WIDTH,10));
    assertTrue(component.setProperty(Constants.Name.BORDER_STYLE,null));
    assertTrue(component.setProperty(Constants.Name.BORDER_STYLE, "SOLID"));
    assertTrue(component.setProperty(Constants.Name.BORDER_COLOR,null));
    assertTrue(component.setProperty(Constants.Name.BORDER_COLOR, "#ff0000"));

    assertTrue(component.setProperty(Constants.Name.BORDER_TOP_LEFT_RADIUS, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_TOP_RIGHT_RADIUS, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_BOTTOM_LEFT_RADIUS, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_TOP_WIDTH, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_LEFT_WIDTH, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_BOTTOM_WIDTH, 1));
    assertTrue(component.setProperty(Constants.Name.BORDER_RIGHT_WIDTH,1));

    assertTrue(component.setProperty(Constants.Name.BORDER_TOP_COLOR, "#ff0000"));
    assertTrue(component.setProperty(Constants.Name.BORDER_BOTTOM_COLOR, "#ff0000"));
    assertTrue(component.setProperty(Constants.Name.BORDER_LEFT_COLOR, "#ff0000"));
    assertTrue(component.setProperty(Constants.Name.BORDER_RIGHT_COLOR, "#ff0000"));

    assertTrue(component.setProperty(Constants.Name.WIDTH, null));
    assertTrue(component.setProperty(Constants.Name.MIN_WIDTH, null));
    assertTrue(component.setProperty(Constants.Name.MAX_WIDTH, null));
    assertTrue(component.setProperty(Constants.Name.HEIGHT, null));
    assertTrue(component.setProperty(Constants.Name.MIN_HEIGHT, null));
    assertTrue(component.setProperty(Constants.Name.MAX_HEIGHT, null));
    assertTrue(component.setProperty(Constants.Name.ALIGN_ITEMS, null));
    assertTrue(component.setProperty(Constants.Name.ALIGN_SELF, null));
    assertTrue(component.setProperty(Constants.Name.FLEX, null));
    assertTrue(component.setProperty(Constants.Name.FLEX_DIRECTION, null));
    assertTrue(component.setProperty(Constants.Name.JUSTIFY_CONTENT, null));
    assertTrue(component.setProperty(Constants.Name.FLEX_WRAP, null));
    assertTrue(component.setProperty(Constants.Name.MARGIN, null));
    assertTrue(component.setProperty(Constants.Name.MARGIN_TOP, null));
    assertTrue(component.setProperty(Constants.Name.MARGIN_LEFT, null));
    assertTrue(component.setProperty(Constants.Name.MARGIN_RIGHT, null));
    assertTrue(component.setProperty(Constants.Name.MARGIN_BOTTOM, null));
    assertTrue(component.setProperty(Constants.Name.PADDING, null));
    assertTrue(component.setProperty(Constants.Name.PADDING_TOP, null));
    assertTrue(component.setProperty(Constants.Name.PADDING_LEFT, null));
    assertTrue(component.setProperty(Constants.Name.PADDING_RIGHT, null));
    assertTrue(component.setProperty(Constants.Name.PADDING_BOTTOM, null));

  }


  @Test
  public void testAddEvent() throws Exception {
    component.addEvent(Constants.Event.FOCUS);
  }
}
