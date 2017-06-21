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

import com.facebook.yoga.YogaEdge;
import com.taobao.weex.dom.flex.CSSLayout;

import static com.taobao.weex.common.Constants.Event;

/**
 * Created by sospartan on 7/27/16.
 */
public class TestDomObject extends WXDomObject {
  public static void setRef(WXDomObject dom,String ref){
    dom.mRef = ref;
  }

  public static void setAttribute(WXDomObject dom,WXAttr attr){
    dom.mAttributes = attr;
  }

  public TestDomObject(){
    mStyles = new WXStyle();
    setHeight(100);
    setWidth(50);
    setPosition(YogaEdge.ALL,10);
    mAttributes = new WXAttr();

    mEvents = new WXEvent();
    mEvents.add(Event.DISAPPEAR);
    mEvents.add(Event.APPEAR);
    mEvents.add(Event.CHANGE);
    mEvents.add(Event.BLUR);
    mEvents.add(Event.INPUT);
    mEvents.add(Event.FOCUS);
  }
}
