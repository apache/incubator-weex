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
package com.taobao.weex.dom.action;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.ui.animation.WXAnimationBean;

import static com.taobao.weex.dom.WXDomModule.ADD_ELEMENT;
import static com.taobao.weex.dom.WXDomModule.ADD_EVENT;
import static com.taobao.weex.dom.WXDomModule.ADD_RULE;
import static com.taobao.weex.dom.WXDomModule.CREATE_BODY;
import static com.taobao.weex.dom.WXDomModule.CREATE_FINISH;
import static com.taobao.weex.dom.WXDomModule.GET_COMPONENT_RECT;
import static com.taobao.weex.dom.WXDomModule.INVOKE_METHOD;
import static com.taobao.weex.dom.WXDomModule.MOVE_ELEMENT;
import static com.taobao.weex.dom.WXDomModule.REFRESH_FINISH;
import static com.taobao.weex.dom.WXDomModule.REMOVE_ELEMENT;
import static com.taobao.weex.dom.WXDomModule.REMOVE_EVENT;
import static com.taobao.weex.dom.WXDomModule.SCROLL_TO_ELEMENT;
import static com.taobao.weex.dom.WXDomModule.UPDATE_ATTRS;
import static com.taobao.weex.dom.WXDomModule.UPDATE_FINISH;
import static com.taobao.weex.dom.WXDomModule.UPDATE_STYLE;

/**
 * Created by sospartan on 01/03/2017.
 */

public class Actions {

  public static Action get(String actionName,JSONArray args){
    switch (actionName) {
      case CREATE_BODY:
        if (args == null) {
          return null;
        }
        return new CreateBodyAction(args.getJSONObject(0));
      case UPDATE_ATTRS:
        if (args == null) {
          return null;
        }
        return new UpdateAttributeAction(args.getString(0),args.getJSONObject(1));
      case UPDATE_STYLE:
        if (args == null) {
          return null;
        }
        return new UpdateStyleAction(args.getString(0),args.getJSONObject(1));
      case REMOVE_ELEMENT:
        if (args == null) {
          return null;
        }
        return new RemoveElementAction(args.getString(0));
      case ADD_ELEMENT:
        if (args == null) {
          return null;
        }
        return new AddElementAction(args.getJSONObject(1),args.getString(0),args.getInteger(2));
      case MOVE_ELEMENT:
        if (args == null) {
          return null;
        }
        return new MoveElementAction(args.getString(0),args.getString(1),args.getInteger(2));
      case ADD_EVENT:
        if (args == null) {
          return null;
        }
        return new AddEventAction(args.getString(0),args.getString(1));
      case REMOVE_EVENT:
        if (args == null) {
          return null;
        }
        return new RemoveEventAction(args.getString(0),args.getString(1));
      case CREATE_FINISH:
        return new CreateFinishAction();
      case REFRESH_FINISH:
        return new RefreshFinishAction();
      case UPDATE_FINISH:
        return new UpdateFinishAction();
      case SCROLL_TO_ELEMENT:
        if (args == null) {
          return null;
        }
        return new ScrollToElementAction(args.getString(0),args.getJSONObject(1));
      case ADD_RULE:
        if (args == null) {
          return null;
        }
        return new AddRuleAction(args.getString(0),args.getJSONObject(1));
      case GET_COMPONENT_RECT:
        if(args == null){
          return null;
        }
        return new GetComponentRectAction(args.getString(0),args.getString(1));
      case INVOKE_METHOD:
        if(args == null){
          return null;
        }
        return new InvokeMethodAction(args.getString(0),args.getString(1),args.getJSONArray(2));
    }

    return null;
  }


  public static DOMAction getInvokeMethod(String ref,String method,JSONArray args){
    return new InvokeMethodAction(ref,method,args);
  }

  /**
   * Bridge will get this action directly.
   * @param data
   * @param parentRef
   * @param index
   * @return
   */
  public static DOMAction getAddElement(JSONObject data, String parentRef, int index){
    return new AddElementAction(data,parentRef,index);
  }

  public static DOMAction getUpdateStyle(String ref, JSONObject data, boolean byPesudo){
    return new UpdateStyleAction(ref,data,byPesudo);
  }

  public static DOMAction getAddEvent(String ref, String type) {
    return new AddEventAction(ref,type);
  }

  public static DOMAction getAnimationAction(@NonNull final String ref, @NonNull String animation,
                                             @Nullable final String callBack){
    return new AnimationAction(ref, animation, callBack);
  }

  public static RenderAction getAnimationAction(@NonNull String ref,
                                                @NonNull final WXAnimationBean animationBean){
    return new AnimationAction(ref, animationBean);
  }

  public static RenderAction getAnimationAction(@NonNull String ref,
                                                @NonNull final WXAnimationBean animationBean,
                                                @Nullable String callback){
    return new AnimationAction(ref, animationBean, callback);
  }
}
