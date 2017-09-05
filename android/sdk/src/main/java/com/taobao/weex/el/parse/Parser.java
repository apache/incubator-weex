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

import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by furture on 2017/8/28.
 * simple expression parse, less node and save memory
 * reference https://github.com/jquery/esprima
 */
public class Parser {

    private String code;
    private int position;
    private ArrayStack<Token> stacks;
    private ArrayStack<Symbol> operators;

    public Parser(String code){
        this.code = code;
        this.position = 0;
        this.stacks = new ArrayStack<>();
        this.operators = new ArrayStack<>();
    }


    public Block parse(){
        while (hasNextToken()){
            scanNextToken();
        }
        while (!operators.isEmpty()){
            Symbol op = operators.pop();
            Token token = createOperator(op);
            if(token != null) {
                stacks.push(token);
            }
        }
        Block block = new Block(stacks.getList(), Token.TYPE_BLOCK);
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
            return new Block(null, Token.TYPE_BLOCK);
        }
    }


    char scanNextToken(){
        char ch = nextToken();
        System.out.println("token " + ch);
        if(ch == Operators.DOLLAR){
            position++;
            return ch;
        }else if(Character.isJavaIdentifierStart(ch)){
            scanIdentifier();
        }else if (ch == Operators.BRACKET_START || ch == Operators.BLOCK_START) {
            scanBracket();
        }else if (ch ==  Operators.QUOTE || ch == Operators.SINGLE_QUOTE) {
            scanString();
        }else if((ch == Operators.DOT && Character.isDigit(code.charAt(position + 1)))
                || Character.isDigit(ch)){ //number .00 .00e6
            scanNumber();
        }else if(ch ==  Operators.CONDITION_IF){
            scanIf();
        }else if(ch ==  Operators.CONDITION_IF_MIDDLE
                || ch ==  Operators.BRACKET_END
                || ch == Operators.BLOCK_END
                || ch == Operators.SPACE){
            position++;
            return ch;
        }else{
            scanOperator();
        }
        return ch;
    }


    void  scanBracket(){
        int stackSize = stacks.size();
        int opSize = operators.size();
        if(code.charAt(position) == Operators.BLOCK_START){
            operators.push(new Symbol(Operators.BLOCK_START_STR, stacks.size()));
            position++;
            while (hasNextToken()){
                if(scanNextToken() == Operators.BLOCK_END){
                    break;
                }
            }
        }else{
            operators.push(new Symbol(Operators.BRACKET_START_STR, stacks.size()));
            position++;
            while (hasNextToken()){
                if(scanNextToken() == Operators.BRACKET_END){
                    break;
                }
            }
        }
        if(stacks.size() <= stackSize){ // empty bracket, none need, save memory
            while (operators.size() > opSize){
                operators.pop();
            }
            return;
        }
        while (operators.size() > opSize){
            Symbol op = operators.pop();
            if(stacks.size() > stackSize){
                Token token = createOperator(op);
                if(token != null) {
                    stacks.push(token);
                }
            }
        }
        List<Token> tokens = new ArrayList<>();
        for(int i=stackSize; i<stacks.size(); i++){
            tokens.add(stacks.get(i));
        }
        while (stacks.size() > stackSize){
            stacks.pop();
        }
        if(tokens.size() == 1){
            stacks.push(tokens.get(0));
        }else{
            Block block = new Block(tokens, Token.TYPE_BLOCK);
            stacks.push(block);
        }
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
            if(!Operators.OPERATORS_PRIORITY.containsKey(operator)){
                operator = operator.substring(0, 2);
            }
        }
        if(operator.length() >= 2){
            if(!Operators.OPERATORS_PRIORITY.containsKey(operator)){
                operator = operator.substring(0, 1);
            }
        }
        if(!Operators.OPERATORS_PRIORITY.containsKey(operator)){
            //just skip illegal character
            int illegalChar = Math.min(start + 1, code.length());
            WXLogUtils.e("weex", new IllegalArgumentException(code.substring(0, illegalChar) + " illegal code operator" + operator));
            position += operator.length();
            return;
        }
        if((!operators.isEmpty() && operators.peek() != null)){
            String preOp = operators.peek().op;
            if(Operators.OPERATORS_PRIORITY.get(preOp) >= Operators.OPERATORS_PRIORITY.get(operator)){
                Symbol op = operators.pop();
                Token token = createOperator(op);
                if(token != null) {
                    stacks.push(token);
                }
            }
        }
        if(!Operators.isOpEnd(operator)){
            operators.push(new Symbol(operator, stacks.size()));
        }
        position += operator.length();
    }



    Token createOperator(Symbol symbol){
        String op = symbol.op;
        if(Operators.BRACKET_START_STR.equals(symbol.op)
                || Operators.BLOCK_START_STR.equals(symbol.op)
                || Operators.DOLLAR_STR.equals(symbol.op)){
            return null;
        }
        if(Operators.BLOCK_START_STR.equals(symbol.op)){
            return null;
        }
        int secondMin = symbol.pos;
        int firstMin = 0;
        if(!operators.empty()){
            firstMin = Math.max(operators.peek().pos, firstMin);
        }
        Operator operator = new Operator(op, Token.TYPE_OPERATOR);
        if(Operators.AND_NOT.equals(op)){
            if(stacks.size() > secondMin) {
                operator.self = stacks.pop();
                return operator;
            }
            return operator; //invalid
        }
        if(stacks.size() > secondMin) {
            operator.second = stacks.pop();
        }
        if(stacks.size() > firstMin) {
            operator.first = stacks.pop();
        }
        return operator;
    }

    /**
     * condition if
     * */
    void scanIf(){
        Operator operator = new Operator(Operators.CONDITION_IF_STRING, Token.TYPE_OPERATOR);
        int minSize = 0;
        if(operators.size() > 0){
            minSize = Math.max(operators.peek().pos, minSize);
        }
        if(stacks.size() > minSize){
            operator.self = stacks.pop();
        }
        position++;
        while (hasNextToken()){
            if(scanNextToken() == Operators.CONDITION_IF_MIDDLE){
                break;
            }
        }
        while (stacks.size() > minSize){
            operator.first = stacks.pop();
        }
        if(hasNextToken()){
            scanNextToken();
            while (stacks.size() > minSize){
                operator.second = stacks.pop();
            }
        }
        stacks.push(operator);
    }

    /**
     * 1+e6
     * .00
     * .00e6
     * 100
     * 199e5
     * */
    void scanNumber(){
        boolean isInt = true;
        int start = position;
        if(code.charAt(position) == 'e' || code.charAt(position) == '.'){
            isInt = false;
        }
        position++;
        while (hasNext()){
            char ch = code.charAt(position);
            if(Character.isDigit(ch)
                    || ch == '.'
                    || ch =='e'){
                if(ch == 'e'
                        || ch == '.'){
                    isInt = false;
                }
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
            stack = new Token(number, Token.TYPE_INT);
        }else{
            stack = new Token(number, Token.TYPE_DOUBLE);
        }
        stacks.push(stack);
    }


    void  scanString(){
        int start = position;
        ArrayStack operator = new ArrayStack();
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



    /**
     * scan el expression.
     * item.ddd
     * ${item.dd}
     * $item.dd
     * */
    void scanIdentifier(){
        int start = position;
        position++;
        while (hasNext()){
            char ch = code.charAt(position);
            if(Character.isJavaIdentifierPart(ch)){
               position++;
            }else{
                break;
            }
            //2.true
        }
        String el = code.substring(start, position);
        if(el.startsWith(Operators.DOLLAR_STR)){
            if(el.length() == Operators.DOLLAR_STR.length()){
                return;
            }
            el = el.substring(Operators.DOLLAR_STR.length());
        }
        int type = Token.TYPE_IDENTIFIER;
        if(Operators.KEYWORDS.contains(el)){
            if(!(!operators.isEmpty() && Operators.isDot(operators.peek().op))){
                type = Token.TYPE_KEYWORD;
            }
        }
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
