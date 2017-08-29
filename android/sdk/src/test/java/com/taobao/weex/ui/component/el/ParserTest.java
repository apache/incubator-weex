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
package com.taobao.weex.ui.component.el;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.ui.component.el.parse.Block;
import com.taobao.weex.ui.component.el.parse.Parser;

import junit.framework.Assert;
import junit.framework.TestCase;
import java.util.Stack;

/**
 * Created by furture on 2017/8/28.
 */

public class ParserTest extends TestCase {


    public void testParse(){
        Parser parser = new Parser("item.code  \"string test \" ( item.ddd)  .item  1000  ccc ? ddd : 0");
        parser.parse();

        System.out.println(Double.parseDouble("122.01e6"));

        System.out.println(Double.parseDouble("199e1"));


        System.out.println("ddd".split(" ").length);
    }

    public void testCondition(){
        Block block = Parser.parse("0 ? 1 : (2 ? 2 : 1)");
        Assert.assertEquals(2, block.execute(null));


        block = Parser.parse("1 ? 1 : (2 ? 2 : 1)");
        Assert.assertEquals(1, block.execute(null));

        block = Parser.parse("item.name + index");
        System.out.println(block);
        Assert.assertEquals("hello world20", block.execute(createContext()));


        System.out.println(Parser.parse(" ? ????"));

    }

    public void testOperator(){
        Parser parser = new Parser("(1 + 3)/3*3");
        Block block = parser.parse();

        System.out.println(block.execute(null));

        System.out.println(block);

        System.out.println(block.getTokens().size());

        Parser parser2 = new Parser("1 + +++");
        block = parser2.parse();


        System.out.println(block);

        System.out.println(block.getTokens().size());

    }

    private Stack createContext(){
        JSONObject data = new JSONObject();
        data.put("item", new JSONObject());
        data.put("index", 20);
        data.put("source", true);
        data.getJSONObject("item").put("name", "hello world");

        Stack context = new Stack();
        context.push(data);
        return context;
    }

}
