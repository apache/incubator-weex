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
package com.taobao.weex;

import com.taobao.weex.ui.WXRenderManager;
import org.junit.After;
import org.junit.Before;

import static org.junit.Assert.*;

/**
 * Created by sospartan on 8/30/16.
 */
public class WXSDKManagerTest {

  public static void setRenderManager(WXRenderManager rm){
    WXSDKManager.initInstance(rm);
  }

  public static void setInstance(WXSDKManager m){
    WXSDKManager.setInstance(m);
  }

  @Before
  public void setUp() throws Exception {

  }

  @After
  public void tearDown() throws Exception {

  }
}
