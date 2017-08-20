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
import android.view.MotionEvent;
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
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 9/7/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXScrollViewTest {

  WXScrollView view;

  static final int[] EVENTS = {MotionEvent.ACTION_DOWN,MotionEvent.ACTION_MOVE,MotionEvent.ACTION_UP};

  @Before
  public void setUp() throws Exception {
    Activity activity = Robolectric.setupActivity(TestActivity.class);
    view = new WXScrollView(activity);
    View child = new View(activity);
    view.addView(child);
  }

  @After
  public void tearDown() throws Exception {
    view.destroy();
  }

  @Test
  public void testDispatchTouchEvent() throws Exception {
    for(int action:EVENTS) {
      MotionEvent event = MotionEvent.obtain(100, 100, action,
          10, 10, 0);
      view.dispatchTouchEvent(event);
      event.recycle();
    }
  }

  @Test
  public void testOnTouchEvent() throws Exception {
    for(int action:EVENTS) {
      MotionEvent event = MotionEvent.obtain(100, 100, action,
          10, 10, 0);
      view.onTouchEvent(event);
      event.recycle();
    }
  }

  @Test
  public void testOnScrollChanged() throws Exception {
    view.onScrollChanged(0,10,0,20);
  }
}
