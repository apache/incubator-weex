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
import com.taobao.weex.ui.SimpleComponentHolder;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;

import static com.taobao.weex.common.Constants.Name;
import static org.junit.Assert.assertEquals;

/**
 * Created by sospartan on 8/9/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXSliderTest {

  WXSlider component;

  static final String[] PROPS = {
      Name.AUTO_PLAY,
      Name.VALUE,
      "unknown",
      Name.INTERVAL,
      Name.INDEX,
      Name.SHOW_INDICATORS
  };

  static final Object[][] VALUES = {
      {"","true","false",true,false,null},
      {"","1","0",1,-1,null},
      {null,"","test"},
      {"","100","0",100,-1,null},
      {"","1","0",1,2,3,-1,null},
      {"","true","false",true,false,null}
  };

  public static WXSlider create() throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXSlider) new SimpleComponentHolder(WXSlider.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), WXDivTest.create());
  }

  public static WXIndicator createIndicator() throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXIndicator) new SimpleComponentHolder(WXIndicator.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), WXDivTest.create());
  }

  public static WXIndicator createIndicator(WXVContainer container) throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXIndicator) new SimpleComponentHolder(WXIndicator.class).createInstance(WXSDKInstanceTest.createInstance(), new TestDomObject(), container);
  }

  @Before
  public void setup() throws Exception {
    component = create();
    ComponentTest.create(component);
  }

  @Test
  public void testSetProperties() throws Exception {
    ComponentTest.setProperty(component,PROPS,VALUES);
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

  static final String[] IPROPS = {
      Name.ITEM_SIZE,
      Name.ITEM_SELECTED_COLOR,
      Name.ITEM_COLOR,
      "unknown"
  };

  static final Object[][] IVALUES = {
      {"0",0,1,"test"},
      {"#ffffff",123,-1},
      {"#ffffff",123,-1},
      {"#ffffff",123,-1},
  };

  @Test
  public void testIndicator() throws Exception {
    WXIndicator indicator = createIndicator(component);
    ComponentTest.create(indicator);
    component.addChild(indicator);
    ComponentTest.setProperty(indicator,IPROPS,IVALUES);
  }

  @Test
  public void testOnScrollListener() throws Exception {
    component.mViewPager.addOnPageChangeListener(new WXSlider.SliderOnScrollListener(component));
    component.setOffsetXAccuracy(0.05f);
    component.mViewPager.setCurrentItem(0);
    for (int index=1;index<component.mViewPager.getRealCount();index++) {
      component.mViewPager.setCurrentItem(index,true);
    }
    for (int index=component.mViewPager.getRealCount() - 1;index>=0;index--) {
      component.mViewPager.setCurrentItem(index,true);
    }
    component.mViewPager.setCurrentItem(3,true);
    component.mViewPager.setCurrentItem(0,true);
  }

  @After
  public void tearDown() throws Exception {
    ComponentTest.destory(component);
  }

}
