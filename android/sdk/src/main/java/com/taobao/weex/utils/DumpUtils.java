/**
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
package com.taobao.weex.utils;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

import java.util.ArrayList;

/**
 * Created by furture on 2018/8/31.
 */

public class DumpUtils {

    public static JSONObject dump(WXComponent component){
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("ref", component.getRef());
        jsonObject.put("type", component.getComponentType());
        if(component.getStyles().size() > 0){
            jsonObject.put("style", component.getStyles());
        }
        if(component.getAttrs().size() > 0){
            jsonObject.put("attr", component.getAttrs());
        }
        if(component.getEvents().size() > 0){
            jsonObject.put("event", component.getEvents());
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            ArrayList<JSONObject> list = new ArrayList<>();
            for(int i=0; i<container.getChildCount(); i++){
                list.add(dump(container.getChild(i)));
            }
            if(list.size() > 0) {
                jsonObject.put("children", list);
            }
        }
        return jsonObject;
    }
}
