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

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/29/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXDomObjectTest {

  WXDomObject dom;

  @Before
  public void setUp() throws Exception {
    dom = new TestDomObject();
  }

  @After
  public void tearDown() throws Exception {
    dom.destroy();
  }

  @Test
  public void testParseFromJson() throws Exception {
    dom.parseFromJson(JSONObject.parseObject("{\"ref\":\"100\",\"type\":\"div\",\"attr\":{},\"style\":{\"backgroundColor\":\"rgb(40,96,144)\",\"fontSize\":40,\"color\":\"#ffffff\",\"paddingRight\":30,\"paddingLeft\":30,\"paddingBottom\":20,\"paddingTop\":20}}"));
    assertEquals(dom.getRef(),"100");
    assertEquals(dom.getType(),"div");

    dom.applyStyleToNode();
  }

  @Test
  public void testAdd() throws Exception {
    JSONObject obj = new JSONObject();
    obj.put("ref","100");
    obj.put("type","div");
    dom.parseFromJson(obj);

    JSONObject child = new JSONObject();
    child.put("ref","101");
    child.put("type","test");
    WXDomObject childDom = new WXDomObject();
    childDom.parseFromJson(child);

    dom.add(childDom,0);
    assertEquals(dom.getChildCount(),1);
    assertEquals(dom.getChild(0),childDom);

    dom.removeChildAt(0);
    assertEquals(dom.getChildCount(),0);

    dom.add(childDom,0);
    assertEquals(dom.getChildCount(),1);
    assertEquals(dom.getChild(0),childDom);

    dom.remove(childDom);

  }

  @Test
  public void testClone() throws Exception {
    JSONObject.parseObject("{\"ref\":\"100\",\"type\":\"div\",\"attr\":{},\"style\":{\"backgroundColor\":\"rgb(40,96,144)\",\"fontSize\":40,\"color\":\"#ffffff\",\"paddingRight\":30,\"paddingLeft\":30,\"paddingBottom\":20,\"paddingTop\":20}}");
    JSONObject obj = new JSONObject();
    obj.put("ref","101");
    obj.put("type","test");

    JSONArray event = new JSONArray();
    event.add("click");
    obj.put("event",event);
    dom.parseFromJson(obj);

    WXDomObject clone = dom.clone();
    assertEquals(clone.getRef(),"101");
    assertEquals(clone.getType(),"test");

  }
}
