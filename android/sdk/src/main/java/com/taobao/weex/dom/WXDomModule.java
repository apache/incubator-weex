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

import android.os.Message;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.utils.WXLogUtils;



/**
 * <p>
 * Module class for dom operation. Methods in this class will run in dom thread by default.
 * Actually, methods in this class are wrapper classes, they just wrap method call info, and hand
 * the wrapped info to the {@link WXDomHandler} for further process. This class is also singleton
 * in the {@link com.taobao.weex.WXSDKInstance}
 * </p>
 * <p>
 *   This module is work different with other regular module, method is invoked directly, without reflection.
 * </p>
 */
public final class WXDomModule extends WXModule {

  /** package **/
  // method
  public static final String CREATE_BODY = "createBody";
  public static final String UPDATE_ATTRS = "updateAttrs";
  public static final String UPDATE_STYLE = "applyStyle";
  public static final String REMOVE_ELEMENT = "removeElement";
  public static final String ADD_ELEMENT = "addElement";
  public static final String MOVE_ELEMENT = "moveElement";
  public static final String ADD_EVENT = "applyEvent";
  public static final String REMOVE_EVENT = "removeEvent";
  public static final String CREATE_FINISH = "createFinish";
  public static final String REFRESH_FINISH = "refreshFinish";
  public static final String UPDATE_FINISH = "updateFinish";
  public static final String SCROLL_TO_ELEMENT = "scrollToElement";
  public static final String ADD_RULE = "addRule";
  public static final String GET_COMPONENT_RECT = "getComponentRect";

  public static final String WXDOM = "dom";


  public static final String INVOKE_METHOD = "invokeMethod";
  /**
   * Methods expose to js. Every method which will be called in js should add to this array.
   */
  public static final String[] METHODS = {CREATE_BODY, UPDATE_ATTRS, UPDATE_STYLE,
      REMOVE_ELEMENT, ADD_ELEMENT, MOVE_ELEMENT, ADD_EVENT, REMOVE_EVENT, CREATE_FINISH,
      REFRESH_FINISH, UPDATE_FINISH, SCROLL_TO_ELEMENT, ADD_RULE,GET_COMPONENT_RECT,
      INVOKE_METHOD};

  public WXDomModule(WXSDKInstance instance){
    mWXSDKInstance = instance;
  }

}
