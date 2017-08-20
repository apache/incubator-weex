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
package com.taobao.weex.ui.view;

import android.app.Activity;
import android.support.v4.view.ViewPager;
import android.view.View;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.TestActivity;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 9/7/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXCirclePageAdapterTest {

  WXCirclePageAdapter adapter;
  View child;

  @Before
  public void setUp() throws Exception {
    adapter = new WXCirclePageAdapter();
  }

  @After
  public void tearDown() throws Exception {
  }

  @Test
  public void testAddPageView() throws Exception {
    Activity activity = Robolectric.setupActivity(TestActivity.class);
    child = new View(activity);
    adapter.addPageView(child);
    child = new View(activity);
    adapter.addPageView(child);
    child = new View(activity);
    adapter.addPageView(child);

    assertEquals(adapter.getRealCount(),3);
  }

  @Test
  public void testRemovePageView() throws Exception {
    testAddPageView();
    adapter.removePageView(child);
    assertEquals(adapter.getRealCount(),2);
  }


  @Test
  public void testInstantiateItem() throws Exception {
    testAddPageView();
    ViewPager viewPager = new ViewPager(child.getContext());
    viewPager.setAdapter(adapter);
    Object obj = adapter.instantiateItem(viewPager,adapter.getRealCount());
    assertEquals(child,obj);
  }

  @Test
  public void testReplacePageView() throws Exception {
    testAddPageView();
    View relace = new View(child.getContext());

    adapter.replacePageView(child,relace);

    assertEquals(adapter.getRealCount(),3);
    
  }

  @Test
  public void testGetRealPosition() throws Exception {
    testAddPageView();
    assertEquals(adapter.getRealPosition(0), adapter.getRealCount() - 1);
    assertEquals(adapter.getRealPosition(1), 0);
    assertEquals(adapter.getRealPosition(adapter.getRealCount() + 1), 0);
    assertEquals(adapter.getRealPosition(100), -1);
    assertEquals(adapter.getRealPosition(-1), -1);
  }
}
