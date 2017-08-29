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



import static com.taobao.weex.ui.component.el.parse.Operators.*;

/**
 * Created by furture on 2017/8/28.
 * one operator, two operator, and three operator
 */
public class Operator extends Token {
    public Token self;
    public Token first;
    public Token second;

    public Operator(String operator, int type) {
        super(operator, type);
    }


    @Override
    public Object execute(Object context){
        String op = getToken();
        switch (op){
            case  DOT_STR:
            case  ARRAY_START_STR:{
                return  Operators.dot(first, second, context);
            }
            case  EQUAL:
            case  EQUAL2:{
                return Operators.isEquals(first, second, context);
            }
            case  NOT_EQUAL:
            case  NOT_EQUAL2:{
                return !Operators.isEquals(first, second, context);
            }
            case  CONDITION_IF_STRING:{
                return Operators.condition(self, first, second, context);
            }
            case  AND_NOT:{
                return  !Operators.tokenTrue(self, context);
            }
            case  AND:{
                return  Operators.tokenTrue(first, context) && Operators.tokenTrue(second, context);
            }
            case  OR:{
                return  Operators.tokenTrue(first, context) || Operators.tokenTrue(second, context);
            }
            case  G:{
                return  Operators.tokenNumber(first, context) > Operators.tokenNumber(second, context);
            }
            case  GE:{
                return  Operators.tokenNumber(first, context) >= Operators.tokenNumber(second, context);
            }
            case  L:{
                return  Operators.tokenNumber(first, context) < Operators.tokenNumber(second, context);
            }
            case  LE:{
                return  Operators.tokenNumber(first, context) <= Operators.tokenNumber(second, context);
            }
            case  PLUS:{
                return  Operators.plus(first, second, context);
            }
            case  SUB:{
                return  Operators.sub(first, second, context);
            }
            case  MUL:{
                return  Operators.mul(first, second, context);
            }
            case  DIV:{
                return  Operators.div(first, second, context);
            }
            case  MOD:{
                return  Operators.mod(first, second, context);
            }
            default:
                throw new IllegalArgumentException(op + " operator is not supported");
        }
    }

    @Override
    public String toString() {
        if(self == null){
            return "{" + first +  getToken() + second + "}";
        }
        return "{" + self + getToken() + first + ":" + second + "}";
    }
}
