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

import static com.taobao.weex.ui.component.el.parse.Token.TYPE_BLOCK;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_DOUBLE;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_INT;
import static com.taobao.weex.ui.component.el.parse.Token.TYPE_OPERATOR;
import static com.taobao.weex.ui.component.el.parse.Operators.*;

/**
 * Created by furture on 2017/8/28.
 * simple expression parse, less node and save memory
 * reference https://github.com/jquery/esprima
 */
public class Parser {

    private String code;
    private int position;
    private Stack<Token> stacks;
    private Stack<Symbol> operators;

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
            Symbol op = operators.pop();
            Token token = createOperator(op);
            if(token != null) {
                stacks.push(token);
            }
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
        System.out.println("token " + ch);
        if(Character.isJavaIdentifierStart(ch)){
            scanIdentifier();
        }else if (ch == BRACKET_START) {
            scanBracket();
        }else if (ch ==  QUOTE || ch == SINGLE_QUOTE) {
            scanString();
        }else if((ch == DOT && Character.isDigit(code.charAt(position + 1)))
                || Character.isDigit(ch)){ //number .00 .00e6
            scanNumber();
        }else if(ch ==  CONDITION_IF){
            scanIf();
        }else if(ch ==  CONDITION_IF_MIDDLE
                || ch ==  BRACKET_END
                || ch == SPACE){
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
        int start = position;
        position++;
        operators.push(new Symbol(BRACKET_START_STR, stacks.size()));
        while (hasNextToken()){
            if(scanNextToken() == BRACKET_END){
                break;
            }
        }
        if(operators.size() <= opSize){ // empty bracket, none need, save memory
            //opetion

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
        Block block = new Block(tokens, TYPE_BLOCK);
        stacks.push(block);
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
            if(!OPERATORS_PRIORITY.containsKey(operator)){
                operator = operator.substring(0, 2);
            }
        }
        if(operator.length() >= 2){
            if(!OPERATORS_PRIORITY.containsKey(operator)){
                operator = operator.substring(0, 1);
            }
        }
        if(!OPERATORS_PRIORITY.containsKey(operator)){
            //just skip illegal character
            int illegalChar = Math.min(start + 1, code.length());
            WXLogUtils.e("weex", new IllegalArgumentException(code.substring(0, illegalChar) + " illegal code operator" + operator));
            position += operator.length();
            return;
        }
        if((!operators.isEmpty() && operators.peek() != null)){
            String preOp = operators.peek().op;
            if(OPERATORS_PRIORITY.get(preOp) >= OPERATORS_PRIORITY.get(operator)){
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
        if(BRACKET_START_STR.equals(symbol.op)){
            return null;
        }
        int secondMin = symbol.pos;
        int firstMin = 0;
        if(!operators.empty()){
            firstMin = Math.max(operators.peek().pos, firstMin);
        }
        Operator operator = new Operator(op, TYPE_OPERATOR);
        if(AND_NOT.equals(op)){
            if(stacks.size() > secondMin) {
                operator.self = stacks.pop();
                return operator;
            }
            return null; //invalid
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
        Operator operator = new Operator(CONDITION_IF_STRING, TYPE_OPERATOR);
        if(stacks.peek() != null){
            operator.self = stacks.pop();
        }
        stacks.push(operator);
        position++;
        while (hasNextToken()){
            if(scanNextToken() == CONDITION_IF_MIDDLE){
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
                if(ch == 'e' || ch == '.'){
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



    /**
     * scan el expression.
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
        int type = Token.TYPE_IDENTIFIER;
        if(KEYWORDS.contains(el)){
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
