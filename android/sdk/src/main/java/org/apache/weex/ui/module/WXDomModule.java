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
package org.apache.weex.ui.module;

import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.common.WXModule;
import org.apache.weex.dom.binding.JSONUtils;
import org.apache.weex.ui.action.ActionAddRule;
import org.apache.weex.ui.action.ActionGetComponentRect;
import org.apache.weex.ui.action.ActionGetLayoutDirection;
import org.apache.weex.ui.action.ActionInvokeMethod;
import org.apache.weex.ui.action.GraphicActionBatchBegin;
import org.apache.weex.ui.action.GraphicActionBatchEnd;
import org.apache.weex.ui.action.GraphicActionScrollToElement;
import org.apache.weex.ui.action.UpdateComponentDataAction;
import org.apache.weex.utils.WXLogUtils;

/**
 * <p>
 * Module class for dom operation.
 * </p>
 * <p>
 *   This module is work different with other regular module, method is invoked directly, without reflection.
 * </p>
 *
 * <p>
 *  This class is for internal purpose,
 *  please don't use it directly unless you know what you are doing.
 * </p>
 */
@RestrictTo(Scope.LIBRARY_GROUP)
public class WXDomModule extends WXModule {

  /** package **/
  public static final String SCROLL_TO_ELEMENT = "scrollToElement";
  public static final String ADD_RULE = "addRule";
  public static final String GET_COMPONENT_RECT = "getComponentRect";
  public static final String GET_COMPONENT_DIRECTION = "getLayoutDirection";
  public static final String WXDOM = "dom";
  public static final String INVOKE_METHOD = "invokeMethod";

  public static final String UPDATE_COMPONENT_DATA = "updateComponentData";

  public static final String BATCH_BEGIN = "beginBatchMark";
  public static final String BATCH_END = "endBatchMark";

  /**
   * Methods expose to js. Every method which will be called in js should add to this array.
   */
  public static final String[] METHODS = {SCROLL_TO_ELEMENT, ADD_RULE, GET_COMPONENT_RECT,
      INVOKE_METHOD, GET_COMPONENT_DIRECTION, BATCH_BEGIN, BATCH_END};

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
        case GET_COMPONENT_DIRECTION: {
          if(args == null){
            return null;
          }
          new ActionGetLayoutDirection(mWXSDKInstance, args.getString(0), args.getString(1))
                  .executeActionOnRender();
          break;
        }
        case SCROLL_TO_ELEMENT:{
          if (args == null) {
            return null;
          }
          String ref = args.size() >= 1 ? args.getString(0) : null;
          JSONObject options = args.size() >= 2 ? args.getJSONObject(1) : null;
          new GraphicActionScrollToElement(mWXSDKInstance, ref, options)
                  .executeActionOnRender();
          break;
        }
        case ADD_RULE:{
          if (args == null) {
            return null;
          }
          new ActionAddRule(mWXSDKInstance.getInstanceId(), args.getString(0), args.getJSONObject(1))
                  .executeAction();
          break;
        }
        case GET_COMPONENT_RECT:{
            if(args == null){
                return null;
            }
            new ActionGetComponentRect(mWXSDKInstance, args.getString(0), args.getString(1))
                    .executeActionOnRender();
            break;
        }
        case INVOKE_METHOD: {
          if(args == null){
            return null;
          }
          // todo：no sure where the request com from
          new ActionInvokeMethod(mWXSDKInstance.getInstanceId(), args.getString(0), args.getString(1), args.getJSONArray(2))
                  .executeAction();
          break;
        }
        case UPDATE_COMPONENT_DATA:
          if(args == null || args.size() < 3){
            return null;
          }
          new UpdateComponentDataAction(mWXSDKInstance, args.getString(0), JSONUtils.toJSON(args.get(1)), args.getString(2)).executeAction();
          break;
        case BATCH_BEGIN: {
          if(args == null){
            return null;
          }
          String ref = args.size() >= 1 ? args.getString(0) : null;
          new GraphicActionBatchBegin(mWXSDKInstance, ref).executeActionOnRender();
          break;
        }
        case BATCH_END: {
          String ref = args.size() >= 1 ? args.getString(0) : null;
          new GraphicActionBatchEnd(mWXSDKInstance, ref).executeActionOnRender();
          break;
        }
        default:
          WXLogUtils.e("Unknown dom action.");
          break;
      }

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

    new ActionInvokeMethod(mWXSDKInstance.getInstanceId(), ref, method, args)
            .executeAction();
  }
}
