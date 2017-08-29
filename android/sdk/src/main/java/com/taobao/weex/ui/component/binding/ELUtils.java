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
package com.taobao.weex.ui.component.binding;

import java.lang.reflect.Array;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.StringTokenizer;

/**
 * Created by furture on 2017/8/18.
 * el expression
 */
public class ELUtils {

    private static final String DELIMITER = " .[]";
    /**
     * @param  context stack,
     * @param  el     expression
     * get item.key value in context. only supported map list array
     * */
    public static Object getValue(Object context, String el){
        StringTokenizer tokenizer = new StringTokenizer(el, DELIMITER, true);
        Object value = context;
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            if (token.length() == 1) {
                char ch = token.charAt(0);
                if (' ' == ch
                        || '.' == ch
                        || '[' == ch
                        || ']' == ch){
                    continue;
                }
            }
            value = doGetValue(value, token);
            //FIXME length
            if(value == null && "length".equals(token)){

            }
        }
        if(value == context){
            value = null;
        }
        return  value;
    }




    private static  Object doGetValue(Object context, String key){
        if(context == null){
            return  null;
        }
        if(context instanceof  Stack){
            Stack<Map> stack = (Stack) context;
            for(Map map : stack){
                if(map.containsKey(key)){
                    return map.get(key);
                }
            }
        }

        if(context instanceof  Map){
            return ((Map) context).get(key);
        }

        if(context instanceof List){
            List list = (List) context;
            try{
                return list.get(Integer.parseInt(key));
            }catch (Exception e){}
        }
        if(context.getClass().isArray()){
            try{
                return Array.get(context, Integer.parseInt(key));
            }catch (Exception e){}
        }
        return  null;
    }




}
