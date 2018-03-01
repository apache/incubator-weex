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

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.List;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/2/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
public class WXReflectionUtilsTest {

  static class TestA{
    private boolean value = false;
  }

  @Test
  public void testParseArgument() throws Exception {


   Object value =  WXReflectionUtils.parseArgument(String.class,"dkdkdkdk");
    assertTrue(value instanceof String);

    value = WXReflectionUtils.parseArgument(int.class,123444);
    assertTrue(value instanceof Integer);

    value = WXReflectionUtils.parseArgument(long.class,"123444");
    assertTrue(value instanceof Long);

    value = WXReflectionUtils.parseArgument(int.class,"123444");
    assertTrue(value instanceof Integer);

    value = WXReflectionUtils.parseArgument(Integer.class,"123444");
    assertTrue(value instanceof Integer);

    value = WXReflectionUtils.parseArgument(int.class,123444);
    assertTrue(value instanceof Integer);

    value = WXReflectionUtils.parseArgument(double.class, Double.toString(123.444d));
    assertTrue(value instanceof Double);



    JSONObject j = new JSONObject();
    j.put("a","b");
    j.put("c",23);

   value = WXReflectionUtils.parseArgument(Map.class,j);
   assertTrue(value  == j);

    value = WXReflectionUtils.parseArgument(String.class,j);
    assertTrue(value instanceof String);

    value = WXReflectionUtils.parseArgument(Map.class,j);
    assertTrue(value instanceof Map);
    assertEquals(((Map)value).get("a"),"b");

    value = WXReflectionUtils.parseArgument(JSONObject.class,j);
    assertTrue(value instanceof JSONObject);
    assertEquals(((JSONObject)value).get("a"),"b");
    assertEquals(((JSONObject)value).get("c"),23);

    value = WXReflectionUtils.parseArgument(JSONObject.class, JSON.toJSONString(j));
    assertTrue(value instanceof JSONObject);
    assertEquals(((JSONObject)value).get("a"),"b");
    assertEquals(((JSONObject)value).get("c"),23);

    JSONArray k = new JSONArray();
    k.add("b");
    k.add(23);
    value = WXReflectionUtils.parseArgument(String[].class, k);
    assertTrue(value instanceof String[]);
    assertEquals(((String[])value)[0],"b");
    assertEquals(((String[])value)[1],"23");

    value = WXReflectionUtils.parseArgument(String[].class, JSON.toJSONString(k));
    assertTrue(value instanceof String[]);
    assertEquals(((String[])value)[0],"b");
    assertEquals(((String[])value)[1],"23");

    value = WXReflectionUtils.parseArgument(List.class, JSON.toJSONString(k));
    assertTrue(value instanceof List);
    assertEquals(((List)value).get(0),"b");
    assertEquals(((List)value).get(1),23);

    k = new JSONArray();
    k.add(1);
    k.add(23);
    value = WXReflectionUtils.parseArgument(int[].class, JSON.toJSONString(k));
    assertTrue(value instanceof int[]);
    assertEquals(((int[])value)[0],1);
    assertEquals(((int[])value)[1],23);
  }

  @Test
  public void testSetValue() throws Exception {
    TestA a = new TestA();

    WXReflectionUtils.setValue(a,"value","true");
    assertTrue(a.value);
  }


}
