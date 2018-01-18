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
package com.taobao.weex.ui.component;

import android.view.View;

import com.taobao.weex.WXSDKInstance;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 8/3/16.
 */
public class ComponentTest {
  public static void create(WXComponent comp){
    create(comp,null);
  }
  public static void create(WXComponent comp,View view){
    TestDomObject domObject = new TestDomObject();
    WXVContainer parent;

    if((parent = comp.getParent())!=null){
      if(view != null) {
        comp.mHost = view;
      }else{
        comp.createView();
      }
    }else{
      parent = WXDivTest.create();
      parent.addChild(comp);
      parent.createChildViewAt(-1);
    }

    comp.setLayout(comp);

//    domObject = new TestDomObject();
//    comp.updateDom(domObject);
    comp.applyLayoutAndEvent(comp);

    addEvent(comp);
  }


  public static void setProperty(WXComponent comp,String[] propNames,Object[][] valueGroups){
    Map<String, Object> props = new HashMap<>();
    int len = propNames.length;

    if(propNames.length != valueGroups.length){
      throw new RuntimeException("Property name and value group length not match");
    }
    for (int i=0;i<len;i++){
      for (Object obj:valueGroups[i]){
        props.put(propNames[i],obj);
        comp.updateProperties(props);
      }

    }
  }

  public static void addEvent(WXComponent comp){
    for (String event :
        TestConstants.Events) {
      comp.addEvent(event);
    }
  }

  public static void destory(WXComponent comp){
    comp.destroy();
  }

  public static <T> T createComponent(WXDomObject dom, WXVContainer parent, Class<T> type) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException, InstantiationException {
     return type
         .getConstructor(WXSDKInstance.class,WXDomObject.class,WXVContainer.class)
        .newInstance(parent.getInstance(),dom,parent);
  }
}
