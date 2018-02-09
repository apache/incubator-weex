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

import android.support.v4.util.ArrayMap;


import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.dom.binding.ELUtils;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * Store value of component event
 */
public class WXEvent extends ArrayList<String> implements Serializable, Cloneable {

  private static final long serialVersionUID = -8186587029452440107L;


  /**
   *  event data format
   *  {
   *  type: 'appear',
   *  params: [
   *  { '@binding': 'index' },
   *   'static',
   *   { '@binding': 'item.name' },
   *  { '@binding': '$event' }
   *  ]
   *  }
   * */
  public static final String EVENT_KEY_TYPE = "type";
  public static final String EVENT_KEY_ARGS = "params";


  /**
   * dynamic binding event args, can be null, only weex use
   * */
  private ArrayMap mEventBindingArgs;
  private ArrayMap<String, List<Object>> mEventBindingArgsValues;

  @Override
  public void clear() {
    if(mEventBindingArgs != null){
      mEventBindingArgs.clear();
    }
    if(mEventBindingArgsValues != null){
      mEventBindingArgsValues.clear();
    }
    super.clear();
  }


  public boolean remove(String o) {
    if(mEventBindingArgs != null){
      mEventBindingArgs.remove(o);
    }
    if(mEventBindingArgsValues != null){
      mEventBindingArgsValues.remove(o);
    }
    return super.remove(o);
  }

  /**
   * can by null
   * */
  public ArrayMap getEventBindingArgs() {
    return mEventBindingArgs;
  }


  public ArrayMap<String, List<Object>> getEventBindingArgsValues() {
    return mEventBindingArgsValues;
  }


  public void addEvent(Object event) {
    if(event instanceof CharSequence){
      String eventName = event.toString();
      if(!contains(eventName)){
        add(eventName);
      }
    }else if(event instanceof JSONObject){
      JSONObject bindings = (JSONObject) event;
      String eventName = bindings.getString(WXEvent.EVENT_KEY_TYPE);
      Object args = bindings.get(WXEvent.EVENT_KEY_ARGS);
      if (eventName != null) {
        putEventBindingArgs(eventName, args);
      }
    }
  }

  public static String getEventName(Object event){
    if(event instanceof CharSequence){
      return event.toString();
    }else if(event instanceof JSONObject){
      JSONObject bindings = (JSONObject) event;
      String eventName = bindings.getString(WXEvent.EVENT_KEY_TYPE);
      return  eventName;
    }
    if(event == null){
      return  null;
    }
    return  event.toString();
  }

  public void putEventBindingArgs(String event, Object args){
    if(!contains(event)){
      add(event);
    }
    if(args != null){
      if(mEventBindingArgs == null){
        mEventBindingArgs = new ArrayMap();
      }
      mEventBindingArgs.put(event, ELUtils.bindingBlock(args));
    }
  }

  public void putEventBindingArgsValue(String event, List<Object> value){
    if(mEventBindingArgsValues == null){
      mEventBindingArgsValues = new ArrayMap();
    }
    if(value == null){
      mEventBindingArgsValues.remove(event);
    }else{
      mEventBindingArgsValues.put(event, value);
    }
  }



  @Override
  public WXEvent clone() {
    WXEvent event = new WXEvent();
    event.addAll(this);
    if(mEventBindingArgs != null) {
      event.mEventBindingArgs = new ArrayMap(mEventBindingArgs);
    }
    event.mEventBindingArgsValues = null; //this should not be clone, it dynamic args
    return  event;
  }


}
