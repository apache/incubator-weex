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

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.List;


/**
 * Created by furture on 2017/8/28.
 * simple expression parse, less ast node and save memory
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


    public final Token parse(){
        while (hasNextToken()){
            scanNextToken();
        }
        while (!operators.isEmpty()){
            Symbol op = operators.pop();
            doOperator(op);
        }
        if(stacks.size() == 1){
            return  stacks.pop();
        }
        Block block = new Block(stacks.getList(), Token.TYPE_BLOCK);
        return block;
    }


    /**
     * parse code to ast block.
     * */
    public static Token parse(String code){
        try{
            Parser parser = new Parser(code);
            return parser.parse();
        }catch (Exception e){
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.e("code " + code, e);
            }
            return new Block(null, Token.TYPE_BLOCK);
        }
    }


    final char scanNextToken(){
        char ch = nextToken();
        if(ch == Operators.DOLLAR){
            position++;
            return ch;
        }else if(Character.isJavaIdentifierStart(ch)){
            scanIdentifier();
        }else if (ch == Operators.BRACKET_START || ch == Operators.BLOCK_START) {
            scanBracket();
        }else if (ch == Operators.ARRAY_START) {
            scanArray();
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
                || ch == Operators.SPACE
                || ch == Operators.ARRAY_END){
            position++;
            return ch;
        }else{
            scanOperator();
        }
        return ch;
    }


    final void  scanArray(){
        int stackSize = stacks.size();
        int opSize = operators.size();
        int type = Token.TYPE_IDENTIFIER;
        if(position - 1 < 0 || !Character.isJavaIdentifierPart(code.charAt(position - 1))){
            type = Token.TYPE_ARRAY;
        }
        operators.push(new Symbol(Operators.ARRAY_START_STR, stacks.size()));
        position++;
        while (hasNextToken()){
            char token = scanNextToken();
            if(token == Operators.ARRAY_END){
                break;
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
                doOperator(op);
            }
        }
        List<Token> tokens = new ArrayList<>(4);
        for(int i=stackSize; i<stacks.size(); i++){
            tokens.add(stacks.get(i));
        }
        while (stacks.size() > stackSize){
            stacks.pop();
        }
        if(type == Token.TYPE_ARRAY || stacks.size() == 0){
            Block block = new Block(tokens, Token.TYPE_ARRAY);
            stacks.push(block);
            return;
        }

        Token identifer = stacks.pop();
        Token second = null;
        if(tokens.size() == 1){
            second = tokens.get(0);
        }else{
            second = new Block(tokens, Token.TYPE_BLOCK);
        }
        Operator operator = new Operator(Operators.DOT_STR, type);
        operator.first = identifer;
        operator.second = second;
        stacks.push(operator);
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
                doOperator(op);
            }
        }
        List<Token> tokens = new ArrayList<>(4);
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
                doOperator(op);
            }
        }
        if(!Operators.isOpEnd(operator)){
            operators.push(new Symbol(operator, stacks.size()));
        }
        position += operator.length();
    }



    void doOperator(Symbol symbol){
        String op = symbol.op;
        if(Operators.BRACKET_START_STR.equals(symbol.op)
                || Operators.BLOCK_START_STR.equals(symbol.op)
                || Operators.ARRAY_START_STR.equals(symbol.op)
                || Operators.DOLLAR_STR.equals(symbol.op)){
            return;
        }
        if(Operators.BLOCK_START_STR.equals(symbol.op)){
            return;
        }
        int second = symbol.pos;
        int first  = Math.max(symbol.pos - 1, 0);
        if(!operators.isEmpty()){
            first = Math.max(first, operators.peek().pos);
        }

        Operator operator = new Operator(op, Token.TYPE_OPERATOR);
        if(Operators.AND_NOT.equals(op)){
            if(stacks.size() > second) {
                Token token = stacks.remove(second);
                operator.self = token;
                stacks.add(second, operator);
                return;
            }
            return; //invalid
        }
        if(stacks.size() > second) {
            operator.second = stacks.remove(second);
        }else{
            return;
        }
        if(stacks.size() > first) {
            operator.first = stacks.remove(first);
        }else{
            if(operator.second == null){
                return;
            }
        }
        stacks.add(first, operator);
    }

    /**
     * condition if
     * */
    void scanIf(){
        Operator operator = new Operator(Operators.CONDITION_IF_STRING, Token.TYPE_OPERATOR);
        int selfIndex = 0;
        doStackOperators(0);
        if(operators.size() > 0){
            selfIndex = Math.max(operators.peek().pos, selfIndex);
        }
        if(stacks.size() > selfIndex){
            operator.self = stacks.pop();
        }

        int stackSize = stacks.size();
        int leftOperatorSize = operators.size();
        position++;
        while (hasNextToken()){
            if(scanNextToken() == Operators.CONDITION_IF_MIDDLE){
                break;
            }
        }
        while (operators.size() > leftOperatorSize){
            Symbol symbol = operators.pop();
            doOperator(symbol);
        }

        while (stacks.size() > stackSize){
            operator.first = stacks.pop();
        }
        int rightOperatorsSize = operators.size();
        while (hasNextToken()){
            scanNextToken();
            if(hasNextToken()){
                scanNextToken();
            }
            if(operators.size() <= rightOperatorsSize){
                break;
            }
        }
        doStackOperators(rightOperatorsSize);
        while (stacks.size() > stackSize){
            operator.second = stacks.pop();
        }
        stacks.push(operator);
    }

    private final void doStackOperators(int operatorSize){
        while (operators.size() > operatorSize){
            Symbol symbol = operators.pop();
            doOperator(symbol);
        }
    }

    /**
     * 1+e6
     * .00
     * .00e6
     * 100
     * 199e5
     * */
    final void scanNumber(){
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


    final void  scanString(){
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
    final void scanIdentifier(){
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
        if(Operators.KEYWORDS.containsKey(el)){
            if(!(!operators.isEmpty() && Operators.isDot(operators.peek().op))){
                type = Token.TYPE_KEYWORD;
            }
        }
        Token token = new Token(el, type);
        stacks.push(token);
    }


    final boolean hasNext(){
        return position < code.length();
    }



    final boolean hasNextToken(){
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

    final char nextToken(){
        char ch = code.charAt(position);
        while (ch == ' '){
            position ++;
            if(code.length() <= position){
                break;
            }
            ch = code.charAt(position);
        }
        return ch;
    }
}
