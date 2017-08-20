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
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/9/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class TextareaTest  {

  Textarea component;

  @Before
  public void setUp() throws Exception {
    component = EditComponentTest.createTextarea();
    ComponentTest.create(component);
  }

  @Test
  public void testSetProperty() throws Exception {
    Map<String, Object> props = new HashMap<>();
    int len = EditComponentTest.PROPS.length;

    for (int i = 0; i < len; i++) {
      for (Object obj : EditComponentTest.TEST_VALUES[i]) {
        props.put(EditComponentTest.PROPS[i], obj);
        component.updateProperties(props);
      }

    }
  }

  @After
  public void tearDown() throws Exception {
    ComponentTest.destory(component);
  }
}
