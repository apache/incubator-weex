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
package com.taobao.weex.el.parse;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

/**
 * Created by furture on 2017/8/28.
 */
public class Operators {


    public static Object dot(Token left, Token right, Object context){
        if(left == null || right == null){
            return null;
        }
        Object leftValue = left.execute(context);
        if(leftValue == null){
            return null;
        }
        Object value =  null;
        if(right.getType() != Token.TYPE_IDENTIFIER){
            Object identifter = right.execute(context);
            if(identifter instanceof Double){
                identifter = ((Double) identifter).intValue();
            }
            String key = identifter == null ? "" : identifter.toString().trim();
            value = Operators.el(leftValue, key);
        }else{
            value = right.execute(leftValue);
        }
        if(value != null){
            return value;
        }
        return  specialKey(leftValue, right.getToken());
    }

    /**
     * get key's value on object
     * */
    public static  Object el(Object context, String key){
        if(context == null){
            return  null;
        }
        if(context instanceof ArrayStack){
            ArrayStack stack = (ArrayStack) context;
            for(int index=stack.size()-1; index >= 0; index--){
                Object value = stack.get(index);
                if(value instanceof  Map){
                    Map map = (Map) value;
                    if(map.containsKey(key)){
                        return map.get(key);
                    }
                }
            }
        }
        if(context instanceof Stack){
            Stack stack = (Stack) context;
            for(int index=stack.size()-1; index >= 0; index--){
                Object value = stack.get(index);
                if(value instanceof  Map){
                    Map map = (Map) value;
                    if(map.containsKey(key)){
                        return map.get(key);
                    }
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

    public static  Object specialKey(Object leftValue, String key){
        if("length".equals(key)){
            if(leftValue instanceof  CharSequence){
                return ((CharSequence) leftValue).length();
            }
            if(leftValue instanceof  Map){
                return  ((Map) leftValue).size();
            }
            if(leftValue instanceof  Map){
                return  ((Map) leftValue).size();
            }
            if(leftValue instanceof  List){
                return  ((List) leftValue).size();
            }
            if(leftValue.getClass().isArray()){
                return Array.getLength(leftValue);
            }
        }
        return null;
    }

    public static Object plus(Token left, Token right, Object context){
        Object leftValue = null;
        Object rightValue = null;
        if(left != null){
            leftValue = left.execute(context);
        }
        if(right  != null){
            rightValue = right.execute(context);
        }
        if(leftValue instanceof  CharSequence || rightValue instanceof  CharSequence){
            if(leftValue == null){
                return  rightValue;
            }
            return leftValue.toString() + (rightValue == null ? "" : rightValue.toString());
        }
        if(leftValue instanceof  Number || rightValue instanceof Number){
            return getNumber(leftValue) + getNumber(rightValue);
        }
        if(leftValue == null && rightValue == null){
            return  null;
        }
        if(leftValue == null){
            return  rightValue.toString();
        }
        return leftValue.toString() + (rightValue == null ? "" : rightValue.toString());
    }


    public static Object sub(Token left, Token right, Object context){
        Object leftValue = null;
        Object rightValue = null;
        if(left != null){
            leftValue = left.execute(context);
        }
        if(right  != null){
            rightValue = right.execute(context);
        }
        return getNumber(leftValue) - getNumber(rightValue);
    }

    public static Object div(Token left, Token right, Object context){
        Object leftValue = null;
        Object rightValue = null;
        if(left != null){
            leftValue = left.execute(context);
        }
        if(right  != null){
            rightValue = right.execute(context);
        }
        return getNumber(leftValue)/getNumber(rightValue);
    }

    public static Object mul(Token left, Token right, Object context){
        Object leftValue = null;
        Object rightValue = null;
        if(left != null){
            leftValue = left.execute(context);
        }
        if(right  != null){
            rightValue = right.execute(context);
        }
        return getNumber(leftValue)*getNumber(rightValue);
    }

    public static Object mod(Token left, Token right, Object context){
        Object leftValue = null;
        Object rightValue = null;
        if(left != null){
            leftValue = left.execute(context);
        }
        if(right  != null){
            rightValue = right.execute(context);
        }
        return (getNumber(leftValue))%(getNumber(rightValue));
    }

    /**
     * condition expression
     * */
    public static Object condition(Token selfs, Token first, Token second, Object context){
        boolean value = false;
        if(selfs != null){
            value = isTrue(selfs.execute(context));
        }
        if(value){
            if(first != null){
                return  first.execute(context);
            }
        }else{
            if(second != null){
                return  second.execute(context);
            }
        }
        return null;
    }


    /**
     * is token value is true
     * */
    public static boolean tokenTrue(Token selfs, Object context) {
        if(selfs == null){
            return false;
        }
        Object value = selfs.execute(context);
        return  isTrue(value);
    }

    /**
     * get token number
     * */
    public static double tokenNumber(Token self,  Object context) {
        if(self == null){
            return  0;
        }
        Object value = self.execute(context);
        return  getNumber(value);
    }



    /**
     * isEquls operation
     * */
    public static boolean isEquals(Token left, Token right, Object context){
        if(left == null && right == null){
            return  true;
        }
        Object leftValue =  null;
        if(left != null){
            leftValue  = left.execute(context);;
        }
        Object rightValue = null;
        if(right != null) {
            rightValue = right.execute(context);
        }
        if(leftValue == null){
            if(rightValue == null){
                return  true;
            }
            if(rightValue instanceof  CharSequence){
                if(isEmpty(rightValue.toString())){
                    return  true;
                }
            }
            return  false;
        }
        if(rightValue == null){
            if(isEmpty(leftValue.toString())){
                return  true;
            }
            return  false;
        }
        if(leftValue instanceof  Number){
            if(rightValue instanceof  Number){
                return  ((Number) leftValue).doubleValue() == ((Number) rightValue).doubleValue();
            }
            return ((Number) leftValue).doubleValue() == getNumber(rightValue);
        }
        if(rightValue instanceof  Number){
            return getNumber(leftValue) == ((Number) rightValue).doubleValue();
        }
        if(leftValue instanceof CharSequence
                ||  rightValue instanceof CharSequence){
            return  leftValue.toString().trim().equals(rightValue.toString().trim());
        }
        return  leftValue.equals(rightValue);
    }


    /**
     * check whether value is true
     * */
    public static boolean isTrue(Object value){
        if(value == null){
            return  false;
        }
        if(value instanceof  Number){
            return ((Number) value).doubleValue() != 0;
        }
        String bool = value.toString().trim();
        if("false".equals(bool)
                || "undefined".equals(bool)
                || "null".equals(bool)){
            return  false;
        }
        if(isEmpty(bool)){
            return  false;
        }
        return  true;
    }

    /**
     * check String value is empty
     * */
    public static boolean isEmpty(String value){
        if (value == null){
            return  true;
        }
        for(int i=0; i<value.length(); i++){
            if(value.charAt(i) != ' '){
                return  false;
            }
        }
        return true;
    }


    /**
     * get number
     * */
    public static double getNumber(Object value){
        if(value == null){
            return  0;
        }
        if(value instanceof  Number){
            return  ((Number) value).doubleValue();
        }
        try{
            return Double.parseDouble(value.toString());
        }catch (Exception e){return  0;}
    }

    public static boolean isOpEnd(String op){
        return isOpEnd(op.charAt(0));
    }
    /**
     * op end, has none operation, should not enter operator stack.
     * */
    public static boolean isOpEnd(char op){
        if(op == BRACKET_END
                || op == ARRAY_END
                || op == SPACE
                || op == ARRAY_SEPRATOR){
            return true;
        }
        return  false;
    }

    /**
     * is not
     * */
    public static boolean isDot(String opStr){
        char op = opStr.charAt(0);
        return  op == DOT || op == ARRAY_START;
    }




    public static final char BRACKET_END = ')';
    public static final String BRACKET_END_STR = ")";
    public static final char BRACKET_START = '(';
    public static final String BRACKET_START_STR = "(";
    public static final char QUOTE = '"';
    public static final char SINGLE_QUOTE = '\'';
    public static final char DOT = '.';
    public static final String DOT_STR = ".";
    public static final char ARRAY_START = '[';
    public static final String ARRAY_START_STR = "[";
    public static final char ARRAY_SEPRATOR = ',';
    public static final String ARRAY_SEPRATOR_STR = ",";
    public static final char ARRAY_END = ']';
    public static final String ARRAY_END_STR = "]";
    public static final String SPACE_STR = " ";
    public static final char SPACE = ' ';
    public static final char BLOCK_START = '{';
    public static final String BLOCK_START_STR = "{";
    public static final char BLOCK_END = '}';
    public static final String BLOCK_END_STR = "}";
    public static final char DOLLAR = '$';
    public static final String DOLLAR_STR = "$";

    /**
     * condition
     * */
    public static final char  CONDITION_IF = '?';
    public static final String  CONDITION_IF_STRING = "?";
    public static final char  CONDITION_IF_MIDDLE = ':';



    /**
     * match
     * */
    public static final String PLUS ="+";
    public static final String SUB = "-";
    public static final String MUL = "*";
    public static final String DIV = "/";
    public static final String MOD = "%";



    public static final String AND  = "&&";
    public static final String OR  = "||";
    /**
     * and operator
     * */
    public static final String EQUAL = "===";
    public static final String EQUAL2 = "==";
    public static final String NOT_EQUAL = "!==";
    public static final String NOT_EQUAL2 = "!=";
    public static final String AND_NOT = "!";


    public static final String G = ">";
    public static final String GE = ">=";
    public static final String LE = "<=";
    public static final String L = "<";





    /**
     * https://github.com/jquery/esprima/blob/master/src/parser.ts
     * */
    public static Map<String, Integer> OPERATORS_PRIORITY = new HashMap<>();
    static {
        OPERATORS_PRIORITY.put(BLOCK_END_STR, 0);
        OPERATORS_PRIORITY.put(BRACKET_END_STR, 0);
        OPERATORS_PRIORITY.put(SPACE_STR, 0);
        OPERATORS_PRIORITY.put(ARRAY_SEPRATOR_STR, 0);
        OPERATORS_PRIORITY.put(ARRAY_END_STR, 0);


        OPERATORS_PRIORITY.put(OR, 1);
        OPERATORS_PRIORITY.put(AND, 1);

        OPERATORS_PRIORITY.put(EQUAL, 2);
        OPERATORS_PRIORITY.put(EQUAL2, 2);
        OPERATORS_PRIORITY.put(NOT_EQUAL, 2);
        OPERATORS_PRIORITY.put(NOT_EQUAL2, 2);

        OPERATORS_PRIORITY.put(G, 7);
        OPERATORS_PRIORITY.put(GE, 7);
        OPERATORS_PRIORITY.put(L, 7);
        OPERATORS_PRIORITY.put(LE, 8);

        OPERATORS_PRIORITY.put(PLUS, 9);
        OPERATORS_PRIORITY.put(SUB, 9);
        OPERATORS_PRIORITY.put(MUL, 10);
        OPERATORS_PRIORITY.put(DIV, 10);
        OPERATORS_PRIORITY.put(MOD, 10);
        OPERATORS_PRIORITY.put(AND_NOT, 11);

        OPERATORS_PRIORITY.put(DOT_STR, 15);
        
        OPERATORS_PRIORITY.put(ARRAY_START_STR, 16);


        OPERATORS_PRIORITY.put(BRACKET_START_STR, 17);
        OPERATORS_PRIORITY.put(BLOCK_START_STR, 17);


    }

    public static final Object keywordValue(String keyword){
        return KEYWORDS.get(keyword);
    }

    public static final Map<String,Object> KEYWORDS = new HashMap<>();
    static {
        KEYWORDS.put("null", null);
        KEYWORDS.put("true", Boolean.TRUE);
        KEYWORDS.put("false", Boolean.FALSE);
        KEYWORDS.put("undefined", null);
    }

}
