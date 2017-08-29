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

import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import static com.taobao.weex.ui.component.binding.Statements.getString;
import static com.taobao.weex.ui.component.binding.Statements.isEmpty;
import static com.taobao.weex.ui.component.binding.Statements.isString;

/**
 * Created by furture on 2017/8/28.
 */
public class Code {


    public static Object execute(String code, Stack context){
        if(code.indexOf('?') >= 0 && code.indexOf(':') >= 0){
            int start = code.indexOf('?');
            int end = code.indexOf(':');
            if(start >= 0 && end > 0 && start < end){
                String[] parts = Statements.splitTwoByOperator(code, "?");
                String[] values = Statements.splitTwoByOperator(parts[1], ":");
                if(isExecuteTrue(execute(parts[0], context))){
                    return execute(values[0], context);
                }
                return execute(values[1], context);
            }
            return null;
        }else if(code.indexOf('(') >= 0){
            Stack operator = new Stack();
            int start = code.indexOf('(');
            int matchIndex = -1;
            for(int i= start; i<code.length(); i++){
                char ch = code.charAt(i);
                if(ch == '('){
                    operator.push(i);
                }
                if(ch == ')'){
                    operator.pop();
                    if(operator.size() == 0){
                        matchIndex = i;
                        break;
                    }
                }
            }
            if(matchIndex < 0){
                matchIndex = code.length();
            }
            StringBuilder builder = new StringBuilder();
            for(int i=0; i<start; i++){
                char ch = code.charAt(i);
                if(ch == ' '){
                    continue;
                }
                builder.append(ch);
            }
            boolean push = false;
            if(matchIndex > start +1){
                String   bracket = code.substring(start + 1, matchIndex);
                if(!isEmpty(bracket)){
                    Object value = execute(bracket, context);
                    String key = push(context, value);
                    if(value != null){
                        push = true;
                        builder.append(key);
                    }
                }
            }
            for(int i=matchIndex + 1; i<code.length(); i++){
                char ch = code.charAt(i);
                if(ch == ' '){
                    continue;
                }
                builder.append(ch);
            }
            Object value = execute(builder.toString(), context);
            if(push){
                context.pop();
            }
            return  value;
        }else if(code.indexOf("||") > 0){
            String[] values = Statements.splitTwoByOperator(code, "||");
            if(values == null || values.length == 0){
                return  false;
            }
            boolean left = isExecuteTrue(execute(values[0], context));
            if(left){
                return true;
            }
            return  isExecuteTrue(execute(values[1], context));
        }else if(code.indexOf("&&") > 0){
            String[] values = Statements.splitTwoByOperator(code, "&&");
            if(values == null
                    || values.length == 0){
                return  false;
            }
            boolean left = isExecuteTrue(execute(values[0], context));
            if(left){
                return  isExecuteTrue(execute(values[1], context));
            }
            return  false;
        }else if(code.indexOf(">=") > 0){
            String[] values = Statements.splitTwoByOperator(code, ">=");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = execute(values[0], context);
            Object right = execute(values[1], context);
            return  getNumber(left) >= getNumber(right);
        }else if(code.indexOf("<=") > 0){
            String[] values = Statements.splitTwoByOperator(code, "<=");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = execute(values[0], context);
            Object right = execute(values[1], context);
            return  getNumber(left) <= getNumber(right);
        }else if(code.indexOf(">") > 0){
            String[] values = Statements.splitTwoByOperator(code, ">");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = execute(values[0], context);
            Object right = execute(values[1], context);
            return  getNumber(left) > getNumber(right);
        }else if(code.indexOf("<") > 0){
            String[] values = Statements.splitTwoByOperator(code, "<");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = execute(values[0], context);
            Object right = execute(values[1], context);
            return  getNumber(left) < getNumber(right);
        }else if(code.indexOf("===") > 0){
            String[] values = Statements.splitTwoByOperator(code, "===");
            return isEquals(values, context);
        }else if(code.indexOf("!==") > 0){
            String[] values = Statements.splitTwoByOperator(code, "!==");
            return !isEquals(values, context);
        }else if(code.indexOf("==") > 0){
            String[] values = Statements.splitTwoByOperator(code, "==");
            return isEquals(values, context);
        }else if(code.indexOf("!=") > 0){
            String[] values = Statements.splitTwoByOperator(code, "!=");
            return !isEquals(values, context);
        }else if(code.indexOf('!') > 0){
            int index = code.indexOf('!');
            if(index + 1 >= code.length()){
                return  false;
            }
            String nextCode = code.substring(index + 1, code.length());
            if(isEmpty(nextCode)){
                return false;
            }
            return  !isExecuteTrue(execute(nextCode,context));
        }if(code.indexOf('+') > 0){
            String[] parts = Statements.splitTwoByOperator(code, "+");
            Object partone = execute(parts[0], context);
            Object parttwo = execute(parts[1], context);
            if(partone == null && parttwo == null){
                return  "";
            }
            if(isNumber(partone) && isNumber(parttwo)){
                return  getNumber(partone) + getNumber(parttwo);
            }
            StringBuilder builder = new StringBuilder();
            if(partone != null){
                builder.append(partone);
            }
            if(parttwo != null){
                builder.append(parttwo);
            }
            return builder.toString();
        }else if(code.indexOf('-') > 0){
            String[] parts = Statements.splitTwoByOperator(code, "-");
            return getNumber(execute(parts[0], context)) - getNumber(execute(parts[1], context));
        }else if(code.indexOf('*') > 0){
            String[] parts = Statements.splitTwoByOperator(code, "*");
            return getNumber(execute(parts[0], context))*getNumber(execute(parts[1], context));
        }else if(code.indexOf('/') > 0){
            String[] parts = Statements.splitTwoByOperator(code, "/");
            return getNumber(execute(parts[0], context))/getNumber(execute(parts[1], context));
        }else if(code.indexOf('%') > 0){
            String[] parts = Statements.splitTwoByOperator(code, "%");
            return ((int)getNumber(execute(parts[0], context)))%((int)getNumber(execute(parts[1], context)));
        }
        if(isNumber(code)){
            return code.trim();
        }
        if(isString(code)){
            return getString(code);
        }
        if("true".equals(code)
                || "false".equals(code)
                || "undefined".equals(code)){
            return  code;
        }
        return  ELUtils.getValue(context, code);
    }

    private static String push(Stack context, Object value){
        String key = key();
        Map<String, Object> map = new HashMap<>();
        map.put(key, value);
        context.push(map);
        return key;
    }

    public static boolean isExecuteTrue(Object value){
        if(value == null){
            return  false;
        }
        String bool = value.toString().trim();
        if("false".equals(bool) || "undefined".equals(bool)){
            return  false;
        }
        if(isEmpty(bool)){
            return  false;
        }
        return  true;
    }

    /**
     * get number
     * */
    private static double getNumber(Object value){
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


    /**
     * v-if check left and right values is equals in context
     * */
    private static boolean isEquals(String[] values, Stack context){
        if(values.length == 0){
            return  false;
        }
        String left = values[0];
        if(values.length == 1){
            return  false;
        }
        String right = values[1];
        Object leftValue = execute(left, context);
        Object rightValue = execute(right, context);
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
        if(leftValue instanceof CharSequence
                || leftValue instanceof  Number){
            String leftStr = leftValue.toString();
            if(isNumber(leftStr)){
                if(leftStr.indexOf('.') > 0){
                    return  getNumber(leftStr)  == getNumber(rightValue);
                }else{
                    return  ((int)getNumber(leftStr))  == ((int)getNumber(rightValue));
                }
            }
            return  leftValue.toString().trim().equals(rightValue.toString().trim());
        }
        if(rightValue instanceof CharSequence
                || rightValue instanceof  Number){
            String rightStr = leftValue.toString();
            if(isNumber(rightStr)){
                if(rightStr.indexOf('.') > 0){
                    return  getNumber(leftValue)  == getNumber(rightValue);
                }else{
                    return  ((int)getNumber(leftValue))  == ((int)getNumber(rightValue));
                }
            }
            return  leftValue.toString().trim().equals(rightValue.toString().trim());
        }
        return  leftValue.equals(rightValue);
    }


    /**
     * check if and value is number
     * */
    public static boolean isNumber(Object object){
        if(object == null){
            return  true;
        }
        if(object instanceof  CharSequence){
            CharSequence value = (CharSequence)object;
            for(int i=0; i<value.length(); i++){
                char ch = value.charAt(i);
                if(!(Character.isDigit(ch) || ch == '.')){
                    return  false;
                }
            }
            return true;
        }
        return  false;
    }



    public static String  key(){
        int index = varKey.length() - 1;
        if(varKey.charAt(index) == 'z'){
            varKey.append('a');
        }else{
            char ch = (char) (varKey.charAt(index) + 1);
            varKey.setCharAt(index, ch);
        }
        if(varKey.length() > 10){
            varKey.setLength(2);
            varKey.append("a");
        }
        return varKey.toString();
    }

    private static StringBuffer varKey = new StringBuffer("__a");
}
