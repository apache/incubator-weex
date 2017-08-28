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
 * Created by furture on 2017/8/21.
 */

public class IfStatementTest extends TestCase {

    public void  testVIfMath(){
        Assert.assertTrue(Statements.isIfTrue("1 ?  false : true", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 >= 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 >= 2", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 && 1 >= 0", createContext()));
        Assert.assertFalse(Statements.isIfTrue("false && 1 >= 0", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 >= '1'", createContext()));
        Assert.assertTrue(Statements.isIfTrue("0 >= '-1'", createContext()));
        Assert.assertTrue(Statements.isIfTrue("0 >= '-1", createContext()));


        Assert.assertFalse(Statements.isIfTrue("1 > 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue("-1 > 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 > -1", createContext()));


        Assert.assertTrue(Statements.isIfTrue("1 <= 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 <= '1'", createContext()));

        Assert.assertFalse(Statements.isIfTrue("1 < 1", createContext()));

        Assert.assertFalse(Statements.isIfTrue("1 <= ", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 < ", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 > ", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 >= ", createContext()));
        Assert.assertFalse(Statements.isIfTrue(" >= 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue(" > 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue(" < 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue(" <= 1", createContext()));
    }



    public void  testVIfAndOr(){
        Assert.assertTrue(Statements.isIfTrue("1 && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 && true", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 && false", createContext()));

        Assert.assertTrue(Statements.isIfTrue("true || false", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 || false", createContext()));
        Assert.assertFalse(Statements.isIfTrue("false && false", createContext()));
        Assert.assertFalse(Statements.isIfTrue("1 && ", createContext()));
        Assert.assertFalse(Statements.isIfTrue(" && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("1 && 1 && 1 && true && true", createContext()));
        Assert.assertFalse(Statements.isIfTrue("false && 1 && 1 && true && true", createContext()));

        Assert.assertTrue(Statements.isIfTrue("1===1  && 1===1", createContext()));

    }

    public void  testVIfBracket(){
        Assert.assertTrue(Statements.isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertFalse(Statements.isIfTrue("((true) && 2 < 1) && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("((true) && 2 < 1) || 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("((true && 2 > 1 && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("true && 2 > 1 && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("(('true') && 2 > 1) && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("(('true') && 2 > 1) && 1 ()()", createContext()));
        Assert.assertTrue(Statements.isIfTrue("(('true') && 2 > 1) && 1 (   )", createContext()));


        Assert.assertTrue(Statements.isIfTrue("((true) && 2 > 1) && (1)", createContext()));
        Assert.assertFalse(Statements.isIfTrue("((true) && 2 > 1) && (1) && ()", createContext()));
        Assert.assertTrue(Statements.isIfTrue("((true) && 2 > 1) && (1) || ()", createContext()));

        Assert.assertTrue(Statements.isIfTrue("1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("true && 2 > 1 && 1", createContext()));
    }

    public void  testDebug(){
        Assert.assertTrue(Statements.isIfTrue("1===2-1", createContext()));
    }

    public void  testVIf(){
        Assert.assertTrue(Statements.isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertTrue(Statements.isIfTrue("'hello' === 'hello'", createContext()));
        Assert.assertTrue(Statements.isIfTrue("'3 ' === 3", createContext()));
        Assert.assertTrue(Statements.isIfTrue("'3 ' == 3", createContext()));
        Assert.assertFalse(Statements.isIfTrue("'3 ' != 3", createContext()));
        Assert.assertTrue(Statements.isIfTrue("(3 === 3)  === true", createContext()));
        Assert.assertTrue(Statements.isIfTrue("(3 == 3)  === (3 === 3)", createContext()));
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
