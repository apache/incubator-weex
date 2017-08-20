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
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.bridge.WXBridgeManager;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static com.taobao.weex.dom.WXDomModule.*;

/**
 * Created by sospartan on 7/29/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*","org.json.*" })
@PrepareForTest()
public class WXDomModuleTest {

  static final String[] METHODS  = {
    CREATE_BODY,
    UPDATE_ATTRS ,
    UPDATE_STYLE ,
    REMOVE_ELEMENT,
    ADD_ELEMENT ,
    MOVE_ELEMENT,
    ADD_EVENT ,
    REMOVE_EVENT,
    CREATE_FINISH ,
    REFRESH_FINISH,
      UPDATE_FINISH,
    SCROLL_TO_ELEMENT,
    null,
    "unknown_method"
  };

  static JSONObject data;
  static{
    data = new JSONObject();
    data.put("a","b");
  }

  static final Object[][] ARGS_CASES = {
    null,
    {new JSONObject()},
    {"",new JSONObject()},
    {"test",data},
    {"test"},
    {"",new JSONObject(),1},
    {"test",new JSONObject(),1},
    {"","",1},
    {"test","test",1},
    {"","test"},
    {"test","test"},
  };



  WXDomModule module;

  @Before
  public void setUp() throws Exception {
    module = new WXDomModule(WXSDKInstanceTest.createInstance());
  }

  @After
  public void tearDown() throws Exception {
    WXDomManagerTest.getLooper().idle();
  }

  @Test
  public void testCallDomMethod() throws Exception {
    module.callDomMethod(null);

    JSONObject obj = new JSONObject();
    for (String m :
      METHODS) {
      obj.put(WXBridgeManager.METHOD,m);
      module.callDomMethod(obj);
    }


    obj = new JSONObject();
    for (Object[] args:ARGS_CASES
         ) {
      JSONArray ary = new JSONArray();
      if(args == null){
        ary = null;
      }else {
        for (Object arg : args
          ) {
          ary.add(arg);
        }
      }
      obj.put(WXBridgeManager.ARGS,ary);
      for (String m :
        METHODS) {
        obj.put(WXBridgeManager.METHOD,m);
        module.callDomMethod(obj);
      }
    }
  }


}
