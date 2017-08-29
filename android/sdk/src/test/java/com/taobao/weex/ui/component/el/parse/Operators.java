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

/**
 * Created by furture on 2017/8/28.
 */

public class Operators {



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
        return ((int)getNumber(leftValue))%((int)getNumber(rightValue));
    }

    /**
     * is token value is true
     * */
    public static boolean isTokenTrue(Token selfs,  Object context) {
        if(selfs == null){
            return false;
        }
        Object value = selfs.execute(context);
        return  isTrue(value);
    }

    public static double tokenNumber(Token self,  Object context) {
        if(self == null){
            return  0;
        }
        Object value = self.execute(context);
        return  getNumber(value);
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
            right.execute(context);
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
}
