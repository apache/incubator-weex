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
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.ui.SimpleComponentHolder;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 27/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXSliderNeighborTest {

  public static WXSliderNeighbor create() throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXSliderNeighbor) new SimpleComponentHolder(WXSliderNeighbor.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), WXDivTest.create());
  }

  WXSliderNeighbor component;

  @Before
  public void setUp() throws Exception {
    component = create();
    ComponentTest.create(component);
  }

  @Test
  public void testPages() throws Exception {
    component = create();
    component.addChild(ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class));
    component.addChild(ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class));
    component.addChild(ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class));
    component.addChild(ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class));
    component.addChild(ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class));

    WXIndicator indicator = new WXIndicator(component.getInstance(),new TestDomObject(),component,false);
    ComponentTest.create(indicator);
    component.addChild(indicator);
    ComponentTest.create(component);

    assertEquals(5,component.mViewPager.getCirclePageAdapter().getRealCount());
    assertEquals(6,component.getChildCount());

    component.mViewPager.setCurrentItem(0);
  }

  @Test
  public void testSetProperties() throws Exception {
    component.setProperty(WXSliderNeighbor.NEIGHBOR_ALPHA,0.4f);
    component.setProperty(WXSliderNeighbor.NEIGHBOR_SCALE,0.9f);
  }

  @Test
  public void testZoomTransformer() throws Exception {
    component = create();
    TestComponent page = ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class);
    TestComponent pageChild = ComponentTest.createComponent(new TestDomObject(),component,TestComponent.class);
    page.addChild(pageChild);
    component.addChild(page);

    ComponentTest.create(component);
//    ComponentTest.create(pageChild);
//    ComponentTest.create(page);
    WXSliderNeighbor.ZoomTransformer transformer = component.createTransformer();
    transformer.transformPage(page.getHostView(),0.2f);
  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }
}
