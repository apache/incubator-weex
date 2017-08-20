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

import static com.taobao.weex.common.Constants.Name.IMAGE_QUALITY;
import static com.taobao.weex.common.Constants.Name.SRC;
import static com.taobao.weex.common.Constants.Name.VALUE;
import static com.taobao.weex.common.Constants.Value.HIGH;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.junit.Assert.assertEquals;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXImageSharpen;
import com.taobao.weex.utils.WXViewUtils;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class)
public class WXAttrTest {

  WXAttr attr;

  @Before
  public void setUp() throws Exception {
    attr = new WXAttr();
  }

  @After
  public void tearDown() throws Exception {
    attr.clear();
  }

  @Test
  public void testGetValue() throws Exception {
    assertEquals(null,WXAttr.getValue(attr));

    attr.put(VALUE,"test");

    assertEquals("test",WXAttr.getValue(attr));
  }

  @Test
  public void testGetImageQuality() throws Exception {
    assertEquals(WXImageQuality.AUTO,attr.getImageQuality());

    attr.put(IMAGE_QUALITY,HIGH);
    assertEquals(WXImageQuality.HIGH,attr.getImageQuality());
  }

  @Test
  public void testGetImageSharpen() throws Exception {
    assertEquals(WXImageSharpen.UNSHARPEN,attr.getImageSharpen());

  }

  @Test
  public void testGetImageSrc() throws Exception {
    assertEquals(null,attr.getImageSrc());

    attr.put(SRC,"test");
    assertEquals("test",attr.getImageSrc());
  }

  @Test
  public void testShowIndicators() throws Exception {
    assertEquals(true,attr.showIndicators());
  }

  @Test
  public void testAutoPlay() throws Exception {
    assertEquals(false,attr.autoPlay());
  }

  @Test
  public void testGetScope() throws Exception {
    assertEquals(null,attr.getScope());
  }

  @Test
  public void testGetLoadMoreRetry() throws Exception {
    assertEquals(null,attr.getLoadMoreRetry());
  }

  @Test
  public void testGetLoadMoreOffset() throws Exception {
    assertEquals(null,attr.getLoadMoreOffset());
  }

  @Test
  public void testGetIsRecycleImage() throws Exception {
    assertEquals(true,attr.getIsRecycleImage());
  }

  @Test
  public void testGetScrollDirection() throws Exception {
    assertEquals("vertical",attr.getScrollDirection());
  }

  @Test
  public void testGetElevation() {
    int elevation = 100, viewPortW = 750;

    attr.put(Constants.Name.ELEVATION, elevation);
    assertThat(attr.getElevation(viewPortW),
               is(WXViewUtils.getRealSubPxByWidth(elevation, viewPortW)));

    attr.put(Constants.Name.ELEVATION, "");
    assertThat(attr.getElevation(viewPortW), is(0f));

    attr.put(Constants.Name.ELEVATION, "give me a NAN");
    assertThat(attr.getElevation(viewPortW), is(Float.NaN));
  }

}
