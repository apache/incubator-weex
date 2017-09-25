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

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.el.parse.Parser;

import junit.framework.Assert;
import junit.framework.TestCase;

import java.util.Stack;

/**
 * Created by furture on 2017/8/22.
 */

public class BindingValueTest extends TestCase {

    public void testBindingValue(){
        System.out.println(Parser.parse("true ? item.name : false").execute(createContext()));
        Assert.assertEquals(getValue("true ? true : false", createContext()), true);
        Assert.assertEquals(getValue("true ? item.name : false", createContext()), "hello world");
        Assert.assertEquals(getValue("false ? item.name : false", createContext()), false);
        Assert.assertEquals(getValue("item.name ? item.name : false", createContext()), "hello world");
        Assert.assertEquals(getValue("item.name > 0 ? item.name : false", createContext()), false);
        Assert.assertEquals(getValue("? item.name : false", createContext()), false);
    }


    public void testDebug(){
        System.out.println(Parser.parse("true ? item.name : false"));
        Assert.assertTrue(getValue("false ? item.name : false", createContext()).toString().equals("false"));
    }


    private Object getValue(String expression, Stack context){
        return Parser.parse(expression).execute(context);
    }

    private Stack createContext(){
        JSONObject data = new JSONObject();
        data.put("item", new JSONObject());
        data.getJSONObject("item").put("name", "hello world");

        Stack context = new Stack();
        context.push(data);
        return context;
    }
}
