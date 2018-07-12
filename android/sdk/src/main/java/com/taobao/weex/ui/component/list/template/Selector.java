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
package com.taobao.weex.ui.component.list.template;

import android.text.TextUtils;

import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.list.WXCell;

import java.util.List;

/**
 * Created by furture on 2018/7/24.
 */

public class Selector {

    /**
     * @param  selector  [att=xx]
     * query elements match selector, current only support attr selector
     * */
    public static void queryElementAll(WXComponent component, String selector, List<WXComponent> componentList){
        if(TextUtils.isEmpty(selector)){
            return;
        }
        selector = selector.replaceAll("\\[|]", "");
        String[] args = selector.split("=");
        if(args.length <= 0){
            return;
        }
        String key = args[0];
        String value = null;
        if(args.length > 1){
            value = args[1].trim();
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                queryElementAllByAttrs(container.getChild(i), key, value, componentList);
            }
        }
    }


    public static void closest(WXComponent component, String selector,List<WXComponent> componentList){
        if(TextUtils.isEmpty(selector)){
            return;
        }
        selector = selector.replaceAll("\\[|]", "");
        String[] args = selector.split("=");
        if(args.length <= 0){
            return;
        }
        String key = args[0];
        String value = null;
        if(args.length > 1){
            value = args[1].trim();
        }
        closestByAttrs(component, key, value, componentList);
    }

    private static void closestByAttrs(WXComponent component, String key, String value, List<WXComponent> componentList){
        if(matchAttrs(component, key, value)){
            componentList.add(component);
        }
        if(component instanceof  WXCell || component instanceof  WXRecyclerTemplateList){
            return;
        }
        queryElementAllByAttrs(component.getParent(), key, value, componentList);
    }


    private static void queryElementAllByAttrs(WXComponent component, String key, String value, List<WXComponent> componentList){
        if(matchAttrs(component, key, value)){
            componentList.add(component);
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                queryElementAllByAttrs(container.getChild(i), key, value, componentList);
            }
        }
    }


    private static boolean matchAttrs(WXComponent component, String key, String value){
        if(component.isWaste()){
            return false;
        }
        if(!component.getAttrs().containsKey(key)){
            return false;
        }
        if(TextUtils.isEmpty(value)){
            return true;
        }
        Object attrValue = component.getAttrs().get(key);
        if(attrValue == null){
            return false;
        }
        return value.equals(attrValue.toString());
    }
}
