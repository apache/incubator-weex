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

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.dom.DOMAction;

/**
 * Created by sospartan on 01/03/2017.
 */

public class TestActions {
  public static DOMAction body(JSONObject data){
    return new CreateBodyAction(data);
  }


  public static DOMAction addDom(JSONObject obj, String parentRef, int index) {
    return new AddElementAction(obj,parentRef,index);
  }

  public static DOMAction remove(String ref) {
    return new RemoveElementAction(ref);
  }

  public static DOMAction updateAttr(String ref, JSONObject data) {
    return new UpdateAttributeAction(ref,data);
  }

  public static DOMAction updateStyle(String ref, JSONObject data, boolean byPesudo) {
    return new UpdateStyleAction(ref,data,byPesudo);
  }

  public static DOMAction moveDom(String ref, String parent, int index) {
    return new MoveElementAction(ref,parent,index);
  }

  public static DOMAction scrollTo(String ref,JSONObject data){
    return new ScrollToElementAction(ref,data);
  }

  public static DOMAction addEvent(String ref,String event){
    return new AddEventAction(ref,event);
  }

  public static DOMAction removeEvent(String ref,String event){
    return new RemoveEventAction(ref,event);
  }

  public static DOMAction createFinish(){
    return new CreateFinishAction();
  }

  public static DOMAction updateFinish(){
    return new UpdateFinishAction();
  }

  public static DOMAction refreshFinish(){
    return new RefreshFinishAction();
  }
}
