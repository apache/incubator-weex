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
import com.taobao.weex.el.parse.Parser;
import com.taobao.weex.el.parse.Token;

import junit.framework.TestCase;

/**
 * Created by furture on 2018/1/25.
 */

public class FailedCaseTest extends TestCase {


    public void testVElseIf0(){
        JSONObject data = new JSONObject();
        JSONObject item = new JSONObject();
        item.put("number", 0.0);
        data.put("item", item);
        ArrayStack stack = new ArrayStack();
        stack.push(data);

        Token token = Parser.parse("!(item.number%3 === 0) && (item.number%3 === 1)");

        System.out.println(token.toString() + "  " + token.execute(stack));

        Token if2 = Parser.parse("!(!(item.number%3 === 0) && (item.number%3 === 1))");
        System.out.println(if2 + "   " + if2.execute(stack));
    }
}
