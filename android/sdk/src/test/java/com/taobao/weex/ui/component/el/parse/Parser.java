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

import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import static com.taobao.weex.ui.component.el.parse.Operator.AND_NOT;
import static com.taobao.weex.ui.component.el.parse.Operator.CONDITION_IF_STRING;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_BLOCK;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_DOUBLE;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_INT;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_OPERATOR;
import static com.taobao.weex.ui.component.el.parse.Token.keywords;

/**
 * Created by furture on 2017/8/28.
 * simple expression parse
 * reference https://github.com/jquery/esprima
 */
public class Parser {

    private String code;
    private int position;
    private Stack<Token> stacks;
    private Stack<String> operators;

    public Parser(String code){
        this.code = code;
        this.position = 0;
        this.stacks = new Stack<>();
        this.operators = new Stack<>();
    }


    public Block parse(){
        while (hasNextToken()){
            scanNextToken();
        }
        while (!operators.isEmpty()){
            stacks.push(createOperator(operators.pop()));
        }
        Block block = new Block(stacks, TYPE_BLOCK);
        return block;
    }


    /**
     * item.length > 10
     * parse expression to code.
     * */
    public static  Block parse(String code){
        try{
            Parser parser = new Parser(code);
            return parser.parse();
        }catch (Exception e){
            return new Block(null, TYPE_BLOCK);
        }
    }


    char scanNextToken(){
        char ch = nextToken();
        if(Character.isJavaIdentifierStart(ch)){
            scanEl();
        }else if (ch == EL.BRACKET_START) {
            scanBracket();
        }else if (ch ==  EL.QUOTE || ch == EL.SINGLE_QUOTE) {
            scanString();
        }else if((ch == EL.DOT && Character.isDigit(code.charAt(position + 1))) || Character.isDigit(ch)){ //number .00 .00e6
            scanNumber();
        }else if(ch ==  Operator.CONDITION_IF){
            scanIf();
        }else if(ch ==  Operator.CONDITION_IF_MIDDLE){
            return ch;
        }else if(ch ==  EL.BRACKET_END){
            return ch;
        }else if(ch ==  ' '){
            position++;
            return ch;
        }else{
            scanOperator();
        }
        return ch;
    }

    /**
     * 1  === (1 + 3) &&  1
     * scan operator
     * */
    void scanOperator(){
        int start = position;
        int length = Math.min(position + 3, code.length());
        String operator = code.substring(position, length);
        if(operator.length() >= 3){
            if(!Operator.PRIORITY_MAP.containsKey(operator)){
                operator = operator.substring(0, 2);
            }
        }
        if(operator.length() >= 2){
            if(!Operator.PRIORITY_MAP.containsKey(operator)){
                operator = operator.substring(0, 1);
            }
        }
        if(!Operator.PRIORITY_MAP.containsKey(operator)){
            //just skip illegal character
            int illegalChar = Math.min(start + 1, code.length());
            WXLogUtils.e("weex", new IllegalArgumentException(code.substring(0, illegalChar) + " illegal code operator" + operator));
            position += operator.length();
            return;
        }
        if((!operators.isEmpty() && operators.peek() != null)){
            if(Operator.PRIORITY_MAP.get(operators.peek()) >= Operator.PRIORITY_MAP.get(operator)){
                stacks.push(createOperator(operators.pop()));
            }
        }
        operators.push(operator);
        position += operator.length();
    }

    Operator createOperator(String op){
        Operator operator = new Operator(op, TYPE_OPERATOR);
        if(AND_NOT.equals(op)){
            if(!stacks.isEmpty()) {
                operator.self = stacks.pop();
            }
            return operator;
        }
        if(!stacks.isEmpty()) {
            operator.second = stacks.pop();
        }
        if(!stacks.isEmpty()) {
            operator.first = stacks.pop();
        }
        return operator;
    }

    /**
     * condition if
     * */
    void scanIf(){
        Operator operator = new Operator(CONDITION_IF_STRING, TYPE_OPERATOR);
        if(stacks.peek() != null){
            operator.self = stacks.pop();
        }
        stacks.push(operator);
        position++;
        while (hasNextToken()){
            if(scanNextToken() == Operator.CONDITION_IF_MIDDLE){
                break;
            }
            position++;
        }
        if(stacks.peek() != null && stacks.peek() != operator){
            operator.first = stacks.pop();
        }
        position++;
        if(hasNextToken()){
            scanNextToken();
            if(stacks.peek() != null && stacks.peek() != operator){
                operator.second = stacks.pop();
            }
        }
    }

    /**
     * .00
     * .00e6
     * 100
     * 199e5
     * */
    void scanNumber(){
        int start = position;
        position++;
        boolean isInt = true;
        while (hasNext()){
            char ch = code.charAt(position);
            if(ch == 'e' || ch == '.'){
                isInt = false;
            }
            if(Character.isDigit(ch)
                    || ch == '.'
                    || ch =='e'){
                position++;
            }else{
                break;
            }
        }
        String number = code.substring(start, position);
        if(".".equals(number)){
            return;
        }
        Token stack = null;
        if(isInt){
            stack = new Token(number, TYPE_INT);
        }else{
            stack = new Token(number, TYPE_DOUBLE);
        }
        stacks.push(stack);
    }


    void  scanString(){
        int start = position;
        Stack operator = new Stack();
        char quote = code.charAt(start);
        operator.push(quote);
        StringBuilder builder = new StringBuilder();
        for(position= start + 1; position<code.length(); position++){
            char ch = code.charAt(position);
            if(ch == quote){
                if(code.charAt(position -1) != '\\'){
                    operator.pop();
                    if(operator.size() == 0){
                       position++;
                       break;
                    }
                }else{
                    builder.deleteCharAt(builder.length()-1);
                    builder.append(ch);
                }
            }else{
                builder.append(ch);
            }
        }
        String string =  builder.toString();
        Token token = new Token(string, Token.TYPE_STRING);
        stacks.push(token);
    }

    void  scanBracket(){
        int stateSize = stacks.size();
        position++;
        while (hasNextToken()){
            if(scanNextToken() == EL.BRACKET_END){
                position++;
                break;
            }
            position++;
        }
        if(stacks.size() <= stateSize){ // empty bracket, none need, save memory
            return;
        }
        List<Token> tokens = new ArrayList<>();
        while (stacks.size() > stateSize){
            tokens.add(stacks.pop());
        }
        Block block = new Block(tokens, TYPE_BLOCK);
        stacks.push(block);
    }

    /**
     * scan el expression.
     * */
    void scanEl(){
        int start = position;
        position++;
        while (hasNext()){
            char ch = code.charAt(position);
            System.out.println("el " + ch);
            if(Character.isJavaIdentifierPart(ch)
                    || ch == '.'
                    || ch =='['
                    || ch ==']'){
               position++;
            }else{
                break;
            }
        }
        String el = code.substring(start, position);
        System.out.println("el" + el);
        int type = Token.TYPE_EL;
        if(keywords.contains(el)){
            type = Token.TYPE_KEYWORD;
        }
        //handle el to o
        Token token = new Token(el, type);
        stacks.push(token);
    }


    boolean hasNext(){
        return position < code.length();
    }



    boolean hasNextToken(){
        while (hasNext()){
            char ch = code.charAt(position);
            if(ch == ' '){
                position++;
                continue;
            }
            return  true;
        }
        return false;
    }

    char nextToken(){
        char ch = code.charAt(position);
        while (ch == ' '){
            position ++;
            if(code.length() >= position){
                break;
            }
            code.charAt(position);
        }
        return ch;
    }
}
