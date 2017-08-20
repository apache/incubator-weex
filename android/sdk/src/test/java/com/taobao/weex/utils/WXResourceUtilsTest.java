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
package com.taobao.weex.utils;

import android.graphics.Shader;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * Created by caolijie on 16/8/4.
 */
@RunWith(RobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class WXResourceUtilsTest {

  @Test
  public void testColor1() throws Exception {
    int color = WXResourceUtils.getColor("#ffffff");
    assertEquals(color, 0xffffffff);

    color = WXResourceUtils.getColor("#99ffffff");
    assertEquals(color, 0x99ffffff);

    color = WXResourceUtils.getColor("#aa123123");
    assertEquals(color, 0xaa123123);

    color = WXResourceUtils.getColor("#00000000");
    assertEquals(color, 0x00000000);

    //can't match
    color = WXResourceUtils.getColor("#ssssss");
    assertEquals(color, Integer.MIN_VALUE);
  }

  @Test
  public void testColor2() throws Exception {
    int color = WXResourceUtils.getColor("#123");
    assertEquals(color, 0xff112233);

    color = WXResourceUtils.getColor("#abc");
    assertEquals(color, 0xffaabbcc);

    //can't match
    color = WXResourceUtils.getColor("#12332");
    assertEquals(color, Integer.MIN_VALUE);

    color = WXResourceUtils.getColor("#sss");
    assertEquals(color, Integer.MIN_VALUE);
  }

  @Test
  public void testColor3() throws Exception {
    int color = WXResourceUtils.getColor("rgba(50%,50%,50%, 0.5)");
    assertEquals(color, 0x7f7f7f7f);

    color = WXResourceUtils.getColor("rgba( 50%, 50% , 50%   , 0.5 )");
    assertEquals(color, 0x7f7f7f7f);

    color = WXResourceUtils.getColor("rgba(255, 255, 255, 1.0)");
    assertEquals(color, 0xffffffff);

    color = WXResourceUtils.getColor("rgba(1%, 1% ,1%, 1)");
    assertEquals(color, 0xff020202);

    color = WXResourceUtils.getColor("rgba(0%, 0, 1%, 0.1)");
    assertEquals(color, 0x19000002);

    color = WXResourceUtils.getColor("rgba(100%, 100%, 100%, 0.1)");
    assertEquals(color, 0x19ffffff);

    color = WXResourceUtils.getColor("rgba(1, 1, 1, 1.0)");
    assertEquals(color, 0xff010101);

    //can't match
    color = WXResourceUtils.getColor("rgba(0.1, 0.1, 0.1, 1.0)");
    assertEquals(color, Integer.MIN_VALUE);

    color = WXResourceUtils.getColor("rgba(10.0%, 101%, -0.1, 50%)");
    assertEquals(color, Integer.MIN_VALUE);
  }

  @Test
  public void testColor4() throws Exception {
    int color = WXResourceUtils.getColor("rgb(255, 255, 255)");
    assertEquals(color, 0xffffffff);

    color = WXResourceUtils.getColor("rgb(000000, 255, 255)");
    assertEquals(color, 0xff00ffff);

    //can't match
    color = WXResourceUtils.getColor("rgb(256, 256, 256)");
    assertEquals(color, 0xffffffff);

    color = WXResourceUtils.getColor("rgb(-1, 255, 255)");
    assertEquals(color, 0xff00ffff);
  }

  @Test
  public void testColor5() throws Exception {
    int color = WXResourceUtils.getColor("aliceblue");
    assertEquals(color, 0XFFF0F8FF);

    color = WXResourceUtils.getColor("pink");
    assertEquals(color, 0XFFFFC0CB);

    //can't match
    color = WXResourceUtils.getColor("jahskdja");
    assertEquals(color, Integer.MIN_VALUE);
  }

  @Test
  public void testGetShader() throws Exception {
    Shader shader = WXResourceUtils.getShader("linear-gradient(to bottom,#a80077,blue)", 100, 100);
    assertNotNull(shader);
    shader = WXResourceUtils.getShader("linear-gradient(to bottom,#a80077,rgb(255,255,0))", 100, 100);
    assertNotNull(shader);
    shader = WXResourceUtils.getShader("linear-gradient(to bottom,#a80077,rgba(255,255,0,0.5))", 100, 100);
    assertNotNull(shader);
    shader = WXResourceUtils.getShader("gradient", 100, 100);
    assertNull(shader);
  }
}
