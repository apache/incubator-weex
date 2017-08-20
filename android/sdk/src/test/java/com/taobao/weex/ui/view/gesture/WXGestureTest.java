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
package com.taobao.weex.ui.view.gesture;

import android.provider.Settings;
import android.view.MotionEvent;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.TestActivity;
import com.taobao.weex.ui.component.ComponentTest;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXDivTest;
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
 * Created by sospartan on 27/09/2016.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXGestureTest {

  WXGesture mGesture;
  WXComponent component;
  @Before
  public void setUp() throws Exception {
    component = WXDivTest.create();
    ComponentTest.create(component);

    component.addEvent(WXGestureType.LowLevelGesture.ACTION_CANCEL.toString());
    component.addEvent(WXGestureType.LowLevelGesture.ACTION_DOWN.toString());
    component.addEvent(WXGestureType.LowLevelGesture.ACTION_MOVE.toString());
    component.addEvent(WXGestureType.LowLevelGesture.ACTION_UP.toString());

    TestActivity activity = Robolectric.setupActivity(TestActivity.class);
    mGesture = new WXGesture(component, activity);

  }

  @Test
  public void testOnTouch() throws Exception {
    MotionEvent event = MotionEvent.obtain(System.currentTimeMillis(), System.currentTimeMillis(),MotionEvent.ACTION_DOWN,0,0,0);
    mGesture.onTouch(component.getHostView(),event);

    event = MotionEvent.obtain(System.currentTimeMillis(), System.currentTimeMillis(),MotionEvent.ACTION_MOVE,0,0,0);
    mGesture.onTouch(component.getHostView(),event);

    event = MotionEvent.obtain(System.currentTimeMillis(), System.currentTimeMillis(),MotionEvent.ACTION_UP,0,0,0);
    mGesture.onTouch(component.getHostView(),event);

    event = MotionEvent.obtain(System.currentTimeMillis(), System.currentTimeMillis(),MotionEvent.ACTION_POINTER_UP,0,0,0);
    mGesture.onTouch(component.getHostView(),event);

    event = MotionEvent.obtain(System.currentTimeMillis(), System.currentTimeMillis(),MotionEvent.ACTION_CANCEL,0,0,0);
    mGesture.onTouch(component.getHostView(),event);
  }
}
