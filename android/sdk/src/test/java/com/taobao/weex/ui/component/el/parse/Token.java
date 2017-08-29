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
package com.taobao.weex.ui.component.el.parse;

import com.taobao.weex.ui.component.binding.ELUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Stack;

/**
 * Created by furture on 2017/8/28.
 */
public class Token {
    public static final List<String> keywords = new ArrayList<>();
    static {
        keywords.add("null");
        keywords.add("true");
        keywords.add("false");
        keywords.add("undefined");
    }
    /**
     * token's type
     * */
    public static final int TYPE_EL = 0;
    public static final int TYPE_INT = 1;
    public static final int TYPE_DOUBLE = 2;
    public static final int TYPE_STRING = 3;
    public static final int TYPE_KEYWORD = 4;
    public static final int TYPE_OPERATOR = 5;
    public static final int TYPE_BLOCK = 6;

    private String token;
    private int type;

    public Token(String token, int type){
        this.token = token;
        this.type = type;
    }


    /**
     * execute token in context, and return value.
     * */
    public Object execute(Object context){
        if(type == TYPE_EL){
            return ELUtils.getValue(context, token);
        }else if(type == TYPE_STRING){
            return  token;
        }else if(type == TYPE_INT){
            try{
                return  Integer.parseInt(token);
            }catch (Exception e){
                return 0;
            }
        }else if(type == TYPE_DOUBLE){
            try{
                return  Double.parseDouble(token);
            }catch (Exception e){
                return 0;
            }
        }else if(type == TYPE_KEYWORD){
            return  token;
        }
        throw new IllegalArgumentException("unhandled token type " + type);
    }

    @Override
    public String toString() {
        return "{" + token + "," + type + '}';
    }

    public String getToken() {
        return token;
    }

    public int getType() {
        return type;
    }
}
