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

import junit.framework.Assert;
import junit.framework.TestCase;

import java.util.Stack;

/**
 * Created by furture on 2017/8/22.
 */

public class BindingValueTest extends TestCase {

    public void testBindingValue(){
        Assert.assertTrue(Statements.getBindingValue("true ? true : false", createContext()).equals("true"));
        Assert.assertTrue(Statements.getBindingValue("true ? item.name : false", createContext()).equals("hello world"));
        Assert.assertTrue(Statements.getBindingValue("false ? item.name : false", createContext()).equals("false"));
        Assert.assertTrue(Statements.getBindingValue("item.name ? item.name : false", createContext()).equals("hello world"));
        Assert.assertTrue(Statements.getBindingValue("item.name > 0 ? item.name : false", createContext()).equals("false"));
        Assert.assertTrue(Statements.getBindingValue("? item.name : false", createContext()).equals("false"));
    }


    public void testDebug(){
        Assert.assertTrue(Statements.getBindingValue("? item.name : false", createContext()).equals("false"));
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
