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
package com.taobao.weex.dom;

import com.taobao.weappplus_sdk.BuildConfig;

import static com.taobao.weex.common.Constants.Name.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 26/09/2016.
 */

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
public class WXTextDomObjectTest {

  WXTextDomObject dom;

  @Before
  public void setUp() throws Exception {
    dom = new WXTextDomObject();
    WXStyle styles = dom.getStyles();
    styles.put(LINES,10);
    styles.put(FONT_SIZE,10);

    dom.getAttrs().put(VALUE,"test");
  }

  @Test
  public void testLayoutBefore() throws Exception {
    dom.layoutBefore();
  }

  @Test
  public void testMeasure() throws Exception {
    testLayoutBefore();
    MeasureOutput output = new MeasureOutput();
    WXTextDomObject mock = PowerMockito.spy(dom);
    PowerMockito.when(mock,"getTextWidth",dom.getTextPaint(),100f,false).thenReturn(10f);
    WXTextDomObject.TEXT_MEASURE_FUNCTION.measure(mock,100,output);

    assertEquals(output.width,10f,0.1f);
  }

  @Test
  public void testLayoutAfter() throws Exception {
    dom.layoutAfter();
  }

  @Test
  public void testClone() throws Exception {
    WXTextDomObject cloneDom = dom.clone();

    assertFalse(cloneDom == dom);
  }

  @After
  public void tearDown() throws Exception {

  }
}
