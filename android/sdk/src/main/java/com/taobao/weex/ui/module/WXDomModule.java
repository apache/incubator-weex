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
package com.taobao.weex.ui.module;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.ui.action.BasicAction;
import com.taobao.weex.ui.action.GraphicActionScrollToElement;
import com.taobao.weex.utils.WXLogUtils;

/**
 * <p>
 * Module class for dom operation.
 * </p>
 * <p>
 *   This module is work different with other regular module, method is invoked directly, without reflection.
 * </p>
 */
public final class WXDomModule extends WXModule {

  /** package **/
  public static final String SCROLL_TO_ELEMENT = "scrollToElement";
  public static final String ADD_RULE = "addRule";
  public static final String GET_COMPONENT_RECT = "getComponentRect";
  public static final String WXDOM = "dom";
  public static final String INVOKE_METHOD = "invokeMethod";

  /**
   * Methods expose to js. Every method which will be called in js should add to this array.
   */
  public static final String[] METHODS = {SCROLL_TO_ELEMENT, ADD_RULE, GET_COMPONENT_RECT,
      INVOKE_METHOD};

  public WXDomModule(WXSDKInstance instance){
    mWXSDKInstance = instance;
  }

  public void callDomMethod(JSONObject task, long... parseNanos) {
    if (task == null) {
      return;
    }
    String method = (String) task.get(WXBridgeManager.METHOD);
    JSONArray args = (JSONArray) task.get(WXBridgeManager.ARGS);
    callDomMethod(method,args,parseNanos);
  }

  public Object callDomMethod(String method, JSONArray args, long... parseNanos) {

    if (method == null) {
      return null;
    }

    try {
      switch (method) {
        case SCROLL_TO_ELEMENT:
          if (args == null) {
            return null;
          }
          String ref = args.size() >= 1 ? args.getString(0) : null;
          JSONObject options = args.size() >= 2 ? args.getJSONObject(1) : null;
          BasicAction action = new GraphicActionScrollToElement(mWXSDKInstance.getInstanceId(), ref, options);
          postAction(action);
          break;
        case ADD_RULE:
          if (args == null) {
            return null;
          }
//        return new AddRuleAction(args.getString(0),args.getJSONObject(1));
          break;
        case GET_COMPONENT_RECT:
          if(args == null){
            return null;
          }
//        return new GetComponentRectAction(args.getString(0),args.getString(1));
          break;
        case INVOKE_METHOD:
          if(args == null){
            return null;
          }
//        return new InvokeMethodAction(args.getString(0),args.getString(1),args.getJSONArray(2));
          break;
        default:
          WXLogUtils.e("Unknown dom action.");
          break;
      }

//      if(action instanceof DOMAction){
//        postAction((DOMAction)action, CREATE_BODY.equals(method) || ADD_RULE.equals(method));
//      }else {
//      }

      // todo TraceableAction
//      if (WXTracing.isAvailable() && action instanceof TraceableAction) {
//        String ref = null;
//        String type = null;
//        if (args.size() > 0) {
//          if (args.size() >= 1) {
//            if (args.get(0) instanceof String) {
//              ref = args.getString(0);
//            } else if (args.get(0) instanceof JSONObject) {
//              ref = ((JSONObject) args.get(0)).getString("ref");
//              type = ((JSONObject) args.get(0)).getString("type");
//            }
//          }
//
//          if (args.size() >= 2) {
//            if (args.get(1) instanceof JSONObject) {
//              ref = ((JSONObject) args.get(1)).getString("ref");
//              type = ((JSONObject) args.get(1)).getString("type");
//            }
//          }
//        }
//        if (parseNanos != null && parseNanos.length == 1) {
//          ((TraceableAction) action).mParseJsonNanos = parseNanos[0];
//          ((TraceableAction) action).mStartMillis -= Stopwatch.nanosToMillis(parseNanos[0]);
//        }
//        ((TraceableAction) action).onStartDomExecute(mWXSDKInstance.getInstanceId(), method, ref, type, args.toJSONString());
//      }
    } catch (IndexOutOfBoundsException e) {
      // no enougn args
      e.printStackTrace();
      WXLogUtils.e("Dom module call miss arguments.");
    } catch (ClassCastException cce) {
      WXLogUtils.e("Dom module call arguments format error!!");
    }
    return null;
  }

  public void invokeMethod(String ref, String method, JSONArray args){
    if(ref == null || method == null){
      return;
    }

//    postAction(Actions.getInvokeMethod(ref,method,args),false);
  }

  public void postAction(BasicAction action){
    WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(mWXSDKInstance.getInstanceId(), action);
  }

}
