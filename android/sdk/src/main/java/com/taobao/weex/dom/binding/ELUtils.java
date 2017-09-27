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
package com.taobao.weex.dom.binding;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.el.parse.Parser;
import com.taobao.weex.el.parse.Token;
import com.taobao.weex.utils.WXLogUtils;

import java.util.Set;

/**
 * util's for binding and statment
 * Created by jianbai.gbj on 2017/8/17.
 */
public class ELUtils {

    public static final String BINDING = "@binding";
    /**
     * sub template
     * */
    public static final String IS_COMPONENT_ROOT = "@isComponentRoot";

    public static final String COMPONENT_PROPS = "@componentProps";

    public static final  String[] EXCLUDES_BINDING = {"clickEventParams"};

    /**
     * @param value check object is binding expression
     * */
    public static boolean isBinding(Object value){
        if(value instanceof JSONObject){
            JSONObject  object = (JSONObject) value;
            if(object.containsKey(BINDING)){
                return  true;
            }
        }else if(value instanceof JSONArray){
            JSONArray array = (JSONArray) value;
            for(int i=0; i<array.size(); i++){
                if(isBinding(array.get(i))){
                    return  true;
                }
            }
        }
        return  false;
    }

    /**
     * parse binding code to block, enable fast execute
     * */
    public static Object bindingBlock(Object value){
        if(value instanceof JSONObject){
            JSONObject  object = (JSONObject) value;
            if(object.containsKey(BINDING)){
                Object binding = object.get(BINDING);
                if(!(binding instanceof Token)){
                    object.put(BINDING, Parser.parse(binding.toString()));
                }
            }
            Set<String> keys = object.keySet();
            for(Object propsKey : keys){
                if(object.get(propsKey) instanceof  JSONObject
                        && ((JSONObject)object.get(propsKey)).containsKey(BINDING)){
                    JSONObject propsValue = (JSONObject) object.get(propsKey);
                    Object binding = propsValue.get(BINDING);
                    if(!(binding instanceof Token)){
                        propsValue.put(BINDING, Parser.parse(binding.toString()));
                    }
                }
            }
        }else if(value instanceof JSONArray){
            JSONArray array = (JSONArray) value;
            for(int i=0; i<array.size(); i++){
                bindingBlock(array.get(i));
            }
        }
        return  value;
    }

    public static Object vforBlock(Object vfor){
        if(vfor instanceof  JSONObject){
            if(((JSONObject) vfor).containsKey(WXStatement.WX_FOR_LIST)){
                Object list = ((JSONObject) vfor).get(WXStatement.WX_FOR_LIST);
                if(!(list instanceof Token)){
                    ((JSONObject) vfor).put(WXStatement.WX_FOR_LIST, Parser.parse(list.toString()));
                }
            }
        }else{
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.e("weex", "weex vfor is illegal " + vfor);
            }
        }
        return vfor;
    }


}
