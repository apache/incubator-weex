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

/**
 * util's for binding and statment
 * Created by furture on 2017/8/17.
 */
public class BindingUtils {

    public static final String BINDING = "@binding";



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
     * @param name value check object is statement
     * */

    public static boolean isStatement(String name){
        if(WXStatement.WX_IF.equals(name) || WXStatement.WX_FOR.equals(name)){
            return  true;
        }
        return  false;
    }


}
