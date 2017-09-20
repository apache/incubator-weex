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
package com.taobao.weex.el;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.el.parse.Operators;
import com.taobao.weex.el.parse.Parser;
import com.taobao.weex.el.parse.Token;

import junit.framework.Assert;
import junit.framework.TestCase;


/**
 * Created by furture on 2017/8/29.
 */

public class IfStatementTest extends TestCase {

    public void  testVIfFront(){
        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertTrue(isIfTrue("(3 == 3)  === (3 === 3)", createContext()));
        Assert.assertFalse(isIfTrue("1 ?  false : true", createContext()));
        Assert.assertTrue(isIfTrue("1 ?  true : false", createContext()));
        Assert.assertFalse(isIfTrue("!(source)", createContext()));
        Assert.assertTrue(isIfTrue("(source)", createContext()));
    }


    public void  testVIfMath(){
        Assert.assertFalse(isIfTrue("1 ?  false : true", createContext()));
        Assert.assertTrue(isIfTrue("1 ?  true : false", createContext()));
        Assert.assertTrue(isIfTrue("1 >= 1", createContext()));
        Assert.assertFalse(isIfTrue("1 >= 2", createContext()));
        Assert.assertTrue(isIfTrue("1 && 1 >= 0", createContext()));
        Assert.assertFalse(isIfTrue("false && 1 >= 0", createContext()));
        Assert.assertTrue(isIfTrue("1 >= '1'", createContext()));
        Assert.assertTrue(isIfTrue("0 >= '-1'", createContext()));
        Assert.assertTrue(isIfTrue("0 >= '-1", createContext()));
        Assert.assertFalse(isIfTrue("!(source)", createContext()));
        Assert.assertTrue(isIfTrue("(source)", createContext()));





        Assert.assertFalse(isIfTrue("1 > 1", createContext()));
        Assert.assertFalse(isIfTrue("-1 > 1", createContext()));
        Assert.assertTrue(isIfTrue("1 > -1", createContext()));


        Assert.assertTrue(isIfTrue("1 <= 1", createContext()));
        Assert.assertTrue(isIfTrue("1 <= '1'", createContext()));

        Assert.assertFalse(isIfTrue("1 < 1", createContext()));

        Assert.assertFalse(isIfTrue("1 <= -1", createContext()));
        Assert.assertFalse(isIfTrue("1 < -2 ", createContext()));
        Assert.assertTrue(isIfTrue("1 > 0", createContext()));
        Assert.assertTrue(isIfTrue("1 >= 0.1", createContext()));
        Assert.assertFalse(isIfTrue("0 >= 1", createContext()));
        Assert.assertFalse(isIfTrue(" -1 > 1", createContext()));
        Assert.assertTrue(isIfTrue("0 < 1", createContext()));
        Assert.assertTrue(isIfTrue(" -1 <= 1", createContext()));
    }



    public void  testVIfAndOr(){
        Assert.assertTrue(isIfTrue("1 && 1", createContext()));
        Assert.assertTrue(isIfTrue("1 && true", createContext()));
        Assert.assertFalse(isIfTrue("1 && false", createContext()));

        Assert.assertTrue(isIfTrue("true || false", createContext()));
        Assert.assertTrue(isIfTrue("1 || false", createContext()));
        Assert.assertFalse(isIfTrue("false && false", createContext()));
        Assert.assertFalse(isIfTrue("1 && false", createContext()));
        Assert.assertFalse(isIfTrue(" && 1", createContext()));
        Assert.assertTrue(isIfTrue("1 && 1 && 1 && true && true", createContext()));
        Assert.assertFalse(isIfTrue("false && 1 && 1 && true && true", createContext()));
        Assert.assertTrue(isIfTrue("1===1  && 1===1", createContext()));

    }

    public void  testVIfBracket(){
        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertFalse(isIfTrue("((true) && 2 < 1) && 1", createContext()));
        Assert.assertTrue(isIfTrue("((true) && 2 < 1) || 1", createContext()));
        Assert.assertTrue(isIfTrue("((true && 2 > 1 && 1", createContext()));
        Assert.assertTrue(isIfTrue("true && 2 > 1 && 1", createContext()));
        Assert.assertTrue(isIfTrue("(('true') && 2 > 1) && 1", createContext()));
        Assert.assertTrue(isIfTrue("(('true') && 2 > 1) && 1 ()()", createContext()));
        Assert.assertTrue(isIfTrue("(('true') && 2 > 1) && 1 (   )", createContext()));


        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && (1)", createContext()));
        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && (1) && (1)", createContext()));
        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && (1) || (3)", createContext()));

        Assert.assertTrue(isIfTrue("1", createContext()));
        Assert.assertTrue(isIfTrue("true && 2 > 1 && 1", createContext()));
    }

    public void  testDebug(){
        //System.out.println("isIfTrue" + isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertFalse(isIfTrue("1 <= 0", createContext()));
        Assert.assertTrue(isIfTrue("1 > -1 + 1", createContext()));
        Assert.assertFalse(isIfTrue("1 < -1", createContext()));
        //Assert.assertTrue(isIfTrue("-1", createContext()));
    }

    public void  testVIf(){
        Assert.assertTrue(isIfTrue("((true) && 2 > 1) && 1", createContext()));
        Assert.assertTrue(isIfTrue("'hello' === 'hello'", createContext()));
        Assert.assertTrue(isIfTrue("'3 ' === 3", createContext()));
        Assert.assertTrue(isIfTrue("'3 ' == 3", createContext()));
        Assert.assertFalse(isIfTrue("'3 ' != 3", createContext()));
        Assert.assertTrue(isIfTrue("(3 === 3)  === true", createContext()));
        Assert.assertTrue(isIfTrue("(3 == 3)  === (3 === 3)", createContext()));
    }

    private boolean isIfTrue(String code, Object context){
        Token block = Parser.parse(code);
        System.out.println( code + " ==> " + block);
        return Operators.isTrue(block.execute(context));
    }
    private ArrayStack createContext(){
        JSONObject data = new JSONObject();
        data.put("item", new JSONObject());
        data.put("index", 20);
        data.put("source", true);
        data.getJSONObject("item").put("name", "hello world");

        ArrayStack context = new ArrayStack();
        context.push(data);
        return context;
    }
}
