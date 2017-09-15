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
package com.taobao.weex.ui.component.list;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXListDomObject;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.component.ComponentTest;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXDivTest;
import com.taobao.weex.ui.component.WXHeaderTest;
import com.taobao.weex.ui.component.WXVContainer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 8/29/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXListComponentTest {

  WXListComponent component;

  public static WXListComponent create(WXVContainer parent) throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return create(parent,new WXListDomObject());
  }

  public static WXListComponent create(WXVContainer parent, WXDomObject dom) throws IllegalAccessException, InstantiationException, InvocationTargetException {
    return (WXListComponent) new SimpleComponentHolder(WXListComponent.class).createInstance(WXSDKInstanceTest.createInstance(), dom, parent);
  }

  @Before
  public void setUp() throws Exception {
    WXDiv div = WXDivTest.create();
    ComponentTest.create(div);
    component = create(div);
    ComponentTest.create(component);
  }

  @Test
  public void testAddChild() throws Exception {
    WXComponent child = WXDivTest.create(component);
    ComponentTest.create(child);
    component.addChild(child);

    child = WXHeaderTest.create(component);
    ComponentTest.create(child);
    component.addChild(child);

  }

  @Test
  public void testScrollTo() throws Exception {
    WXComponent child = WXDivTest.create(component);
    ComponentTest.create(child);
    component.addChild(child);

    child = WXHeaderTest.create(component);
    ComponentTest.create(child);
    component.addChild(child);

    Map<String, Object> options = new HashMap<>(2);
    options.put("offset", 10);
    options.put("animated", false);
    component.scrollTo(child,options);
  }

  @Test
  public void testAppear() throws Exception {
    WXComponent child = WXDivTest.create(component);
    ComponentTest.create(child);
    component.addChild(child);

    component.bindAppearEvent(child);

    component.notifyAppearStateChange(0,0,0,10);
  }

  @Test
  public void testParseTransforms() throws Exception {
    WXDiv div = WXDivTest.create();
    ComponentTest.create(div);

    WXDomObject dom = new WXListDomObject();
    dom.getAttrs().put(Constants.Name.TRANSFORM,"scale(0.9,0.8);translate(10,20);opacity(0.5);rotate(100)");
    component = create(div,dom);
    ComponentTest.create(component);
  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }
}
