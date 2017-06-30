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

import android.view.ViewGroup;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXScrollerDomObject;
import com.taobao.weex.ui.view.WXScrollView;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/25/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXScrollerTest {

  public static WXScroller create(){
    WXDiv div = WXDivTest.create();
    ComponentTest.create(div);
    WXScroller component = new WXScroller(WXSDKInstanceTest.createInstance(),new WXScrollerDomObject(),div);
    div.addChild(component);
    return component;
  }


  WXScroller component;

  @Before
  public void setUp() throws Exception {
    component = create();
    ComponentTest.create(component);
  }

  @Test
  public void testAddChild() throws Exception{
    WXDiv div = WXDivTest.create(component);
    component.addChild(div);
    ComponentTest.create(div);

  }

  @Test
  public void testScroll() throws Exception {
    WXScroller comp = create();
    WXDiv div = WXDivTest.create(comp);
    ComponentTest.create(div);
    comp.addChild(div);
    ComponentTest.create(comp);
    WXScrollView view = (WXScrollView) comp.getInnerView();
    view.scrollTo(100,100);
  }

  @After
  public void tearDown() throws Exception {
    component.destroy();
  }
}
