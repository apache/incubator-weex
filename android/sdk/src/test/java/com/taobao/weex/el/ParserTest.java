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

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.parser.Feature;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.el.parse.Operators;
import com.taobao.weex.el.parse.Parser;
import com.taobao.weex.el.parse.Token;

import junit.framework.Assert;
import junit.framework.TestCase;


/**
 * Created by furture on 2017/8/28.
 */

public class ParserTest extends TestCase {



    public void  testParseString(){
        Assert.assertEquals("hello world", Parser.parse("\"hello world\"").execute(null));
        Assert.assertEquals("hello 'world",  Parser.parse("\"hello \'world\"").execute(null));
        Assert.assertEquals("hello \"world", Parser.parse("\"hello \\\"world\"").execute(null));
        Assert.assertEquals("hello world", Parser.parse("'hello world'").execute(null));
        Assert.assertEquals("hello \"world", Parser.parse("'hello \"world'").execute(null));
        Assert.assertEquals("hello 'world", Parser.parse("'hello \\'world'").execute(null));
    }

    public void  testMath(){
        Assert.assertEquals(5.0, Parser.parse("1+4").execute(null));
        Assert.assertEquals(4000001.0, Parser.parse("1+4e6").execute(null));
        Assert.assertEquals(400001.0, Parser.parse("1+.4e6").execute(null));
        Assert.assertEquals(1.4, Parser.parse("1+.4").execute(null));
        Assert.assertEquals(11.0, Parser.parse("1+e6").execute(createContext()));
        Assert.assertEquals("1e7hello", Parser.parse("1+e7").execute(createContext()));
        Assert.assertEquals(9.0, Parser.parse("1+4*2").execute(null));
        Assert.assertEquals(5.0, Parser.parse("1+4*2/2").execute(null));
        Assert.assertEquals(3.0, Parser.parse("1+4/2").execute(null));
        Assert.assertEquals(2.0, Parser.parse("1+4/4").execute(null));
        Assert.assertEquals(2.0, Parser.parse("1+4%3").execute(null));
        Assert.assertEquals(1.0, Parser.parse("1+4%4").execute(null));
        Assert.assertTrue(Double.isInfinite((double)Parser.parse("1+4/0").execute(null)));
        Assert.assertTrue(Double.isNaN((double)Parser.parse("4%0").execute(null)));

        Assert.assertEquals(Parser.parse("1").execute(null), Parser.parse("1++*").execute(null));
        Assert.assertEquals(3.0, Parser.parse("1+2+*").execute(null));
        Assert.assertEquals(3.0, Parser.parse("1+(2*+*)").execute(null));
    }

    public void testCondition(){
        Assert.assertEquals(1, Parser.parse("0 ? 2 : 1").execute(null));
        Assert.assertEquals(2, Parser.parse("1 ? 2 : 1").execute(null));
        Assert.assertEquals(3, Parser.parse("0 ? 1 : (2 ? 3 : 4)").execute(null));
        Assert.assertEquals(3, Parser.parse("0 ? 1 : 2 ? 3 : 4").execute(null));
        Assert.assertEquals(4, Parser.parse("0 ? 1 : 0 ? 3 : 4").execute(null));
        Assert.assertEquals(5, Parser.parse("1 ? 5 : (2 ? 3 : 4)").execute(null));
        Assert.assertEquals(1, Parser.parse("0?2:1").execute(null));
        Assert.assertEquals(1, Parser.parse("0?2:1").execute(null));
        Assert.assertEquals(4, Parser.parse("0?1:0?3:4").execute(null));
        Assert.assertEquals(3, Parser.parse("0?1:0+1?3:4").execute(null));
        Assert.assertEquals("hello world", Parser.parse("item.name ? item.name : false").execute(this.createContext()));
        Assert.assertEquals(10, Parser.parse("item.name ? item[1] : false").execute(this.createContext()));
        Assert.assertEquals("hello world", Parser.parse("item.name == null ? false : item.name").execute(this.createContext()));
        Assert.assertEquals(false, Parser.parse("true ? false : item.name").execute(this.createContext()));
        Assert.assertEquals(false, Parser.parse("true ? false : item.name.work").execute(this.createContext()));
        Assert.assertEquals("hello world", Parser.parse("item.name ? item.name : false").execute(this.createContext()));
        Assert.assertEquals(null, Parser.parse("item.name ? item.name.not : false").execute(this.createContext()));
        Assert.assertEquals("hello world", Parser.parse("item.name == null ? false : (item.name)").execute(this.createContext()));


    }

    public void testDebug(){
        System.out.println("execute " + Parser.parse("true ? false : item.name").execute(this.createContext()));
        show("true ? false : true xxxx");
        show("true ? false : ((item.name))");
        Parser.parse("item[1]").execute(this.createContext());
        Token block = Parser.parse("{{{item.name}}}");
        show("true ? item.name : false");
        show("((true) && 2 > 1) && (1) && (1)");
        System.out.println(block.execute(createContext())
        + "  " + Double.parseDouble(".0e6"));
        show("1 > -1");

    }


    public void  testBracket(){
        Assert.assertEquals(Parser.parse("(item.name)").toString(), Parser.parse("(((item.name)))").toString());
        Assert.assertEquals(Parser.parse("((item.name))").toString(), Parser.parse("(((item.name)))").toString());
        Assert.assertEquals("hello world", Parser.parse("(((item.name)))").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("((item.name))").execute(createContext()));
    }

    public void  testEl(){
        Assert.assertEquals(true, Parser.parse("${item.name.length == 11}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("${{{item.name}}}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("${{item.name}}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("${{{item.name}}}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("{{item.name}}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("{{{item.name}}}").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("{{{{item.name}}}}").execute(createContext()));
        Assert.assertEquals(1, Parser.parse("1").execute(createContext()));
        Assert.assertEquals(30.0, Parser.parse("item[1] + index").execute(createContext()));
        Assert.assertEquals("hello world20", Parser.parse("item.name + index").execute(createContext()));
        Assert.assertEquals("hello world20", Parser.parse("item[name] + index").execute(createContext()));
        Assert.assertEquals(21.0, Parser.parse("1 + index").execute(createContext()));
        Assert.assertEquals(11.0, Parser.parse("1 + index/2").execute(createContext()));
        Assert.assertEquals(20.0, Parser.parse("item.name/10 + index").execute(createContext()));

        Assert.assertEquals(31.0, Parser.parse("item.name.length + index").execute(createContext()));
        Assert.assertEquals(22.0, Parser.parse("item.length + index").execute(createContext()));
        Assert.assertEquals(36.0, Parser.parse("count * ${ratio}").execute(createContext()));
        Assert.assertEquals(36.0, Parser.parse("count * ${ratio}").execute(createContext()));
        Assert.assertEquals(36.0, Parser.parse("count * ${{ratio}}").execute(createContext()));
        Assert.assertEquals(36.0, Parser.parse("count * ${{{ratio}}}").execute(createContext()));
        Assert.assertEquals(36.0, Parser.parse("count * $ratio").execute(createContext()));
    }

    public void testIf(){
        Assert.assertTrue(Operators.isTrue(Parser.parse("1 ?  true : false").execute(createContext())));
        Assert.assertFalse(Operators.isTrue(Parser.parse("1 ?  false : true").execute(createContext())));
        Assert.assertFalse(Operators.isTrue(Parser.parse("1 ?  null : true").execute(createContext())));
        Assert.assertFalse(Operators.isTrue(Parser.parse("1 ?  undefined : true").execute(createContext())));
        Assert.assertFalse(Operators.isTrue(Parser.parse("1 ?  \"\" : true").execute(createContext())));
        Assert.assertEquals("hello world", Parser.parse("true ? item.name : false").execute(createContext()));
        Assert.assertEquals("hello world", Parser.parse("item.name ? item.name : false").execute(createContext()));
        Assert.assertEquals(true, Parser.parse("true").execute(createContext()));
        Assert.assertEquals(false, Parser.parse("false").execute(createContext()));
        Assert.assertEquals(null, Parser.parse("null").execute(createContext()));
        Assert.assertEquals(null, Parser.parse("undefined").execute(createContext()));

    }


    public void testParse(){
        Assert.assertEquals(1, Parser.parse("0?2:1").execute(null));
        Assert.assertEquals(4, Parser.parse("0?1:0?3:4").execute(null));
        Assert.assertEquals(3, Parser.parse("0?1:0+1?3:4").execute(null));
        Parser.parse("item.code  \"string test \" ( item.ddd)  .item  1000  ccc ? ddd : 0");
        Parser.parse("1+e6");
        show(null);

        show("()++++");

        show("item");
    }


    private void show(String code){
        Token block = Parser.parse(code);
        System.out.println( code + " ==> " + block);
    }

    public void  testArray(){
        int disableDecimalFeature = JSON.DEFAULT_PARSER_FEATURE &= ~Feature.UseBigDecimal.getMask();
        Assert.assertEquals(JSONArray.parse("[2, 3, 3]", disableDecimalFeature), Parser.parse("[2, 3, 3]").execute(null));
        Assert.assertEquals(JSONArray.parse("[2.0, 3, 3]", disableDecimalFeature), Parser.parse("[1+1, 3, 3]").execute(null));
        Assert.assertEquals(JSONArray.parse("[2.0, 3,[3, 3], 3]", disableDecimalFeature), Parser.parse("[1+1, 3, [3, 3], 3]").execute(null));
        Assert.assertEquals(JSONArray.parse("['hello world', 3,[3, 3], 3]", disableDecimalFeature), Parser.parse("[item.name, 3, [3, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[10, 3,[3, 3], 3]", disableDecimalFeature), Parser.parse("[item[1], 3, [3, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[1, 3,[3, 3], 3]", disableDecimalFeature), Parser.parse("[1, 3, [3, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[1, 3,['hello world', 3], 3]", disableDecimalFeature), Parser.parse("[1, 3, [item.name, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[1, [1, 2],[3, 3], 3]", disableDecimalFeature), Parser.parse("[1, [1, 2], [3, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(null,  Parser.parse("item[name + index]").execute(this.createContext()));
        Assert.assertEquals("hello world", Parser.parse("item[name]").execute(this.createContext()));
        Assert.assertEquals("hello world", Parser.parse("item['name']").execute(this.createContext()));
        Assert.assertEquals(10, Parser.parse("item[1]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[false, [1, 2],[3, 3], 3]", disableDecimalFeature), Parser.parse("[!true, [1, 2], [3, 3], 3]").execute(this.createContext()));
        Assert.assertEquals(JSONArray.parse("[2, 3, 3]", disableDecimalFeature), Parser.parse("[(2), 3, 3]").execute(null));
        Assert.assertEquals(JSONArray.parse("[2, 3, 3]", disableDecimalFeature), Parser.parse("([2, 3, 3])").execute(null));
        Assert.assertEquals(JSONArray.parse("[2, 3, 3]", disableDecimalFeature), Parser.parse("([2, 3, (3)])").execute(null));

    }


    public void testOperator(){
        Parser parser = new Parser("(1 + 3)/3*3");
        Token block = parser.parse();

        System.out.println(block.execute(null));

        System.out.println(block);

        Parser parser2 = new Parser("1 + +++");
        block = parser2.parse();


        System.out.println(block);


        String[] values = {null, null};
        for (Object value : values) {
            System.out.println(value + "ddd");
        }

    }

    private ArrayStack createContext(){
        JSONObject data = new JSONObject();
        data.put("item", new JSONObject());
        data.put("index", 20);
        data.put("1", 10);
        data.put("source", true);
        data.put("e6", 10);
        data.put("e7", "e7hello");
        data.put("ratio", 6);
        data.put("count", 6);
        data.getJSONObject("item").put("name", "hello world");
        data.getJSONObject("item").put("1", 10);

        ArrayStack context = new ArrayStack();
        context.push(data);
        return context;
    }






}
