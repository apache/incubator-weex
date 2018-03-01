/*
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
package com.taobao.weex.utils;

import android.support.annotation.NonNull;

import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Parser for function like "rotate(30 ) transform(50 , 20)".
 * This class will translate the raw string presentation of a group of function(s) to give type
 * according to the {@link com.taobao.weex.utils.FunctionParser.Mapper}
 */
public class FunctionParser<K, V> {

  public static final char SPACE = ' ';

  private Mapper<K, V> mapper;
  private Lexer lexer;

  /**
   * Construct a function parser
   * @param source the raw string representation of a group of function(s)
   * @param mapper the mapping rule between string and corresponding type of object.
   */
  public FunctionParser(@NonNull String source, @NonNull Mapper<K, V> mapper) {
    this.lexer = new Lexer(source);
    this.mapper = mapper;
  }

  /**
   * Start to parse the raw string. The result will be stored in a sorted map where the order
   * is defined by the function order in the raw string.
   * @return
   */
  public LinkedHashMap<K, V> parse() {
    lexer.moveOn();
    return definition();
  }

  private LinkedHashMap<K, V> definition() {
    LinkedHashMap<K, V> result = new LinkedHashMap<>();
    do {
      result.putAll(function());
    } while (lexer.getCurrentToken() == Token.FUNC_NAME);
    return result;
  }

  private Map<K, V> function() {
    List<String> list = new LinkedList<>();
    String functionName = match(Token.FUNC_NAME);
    match(Token.LEFT_PARENT);
    list.add(match(Token.PARAM_VALUE));
    while (lexer.getCurrentToken() == Token.COMMA) {
      match(Token.COMMA);
      list.add(match(Token.PARAM_VALUE));
    }
    match(Token.RIGHT_PARENT);
    return mapper.map(functionName, list);
  }

  private String match(Token token) {
    if (token == lexer.getCurrentToken()) {
      String value = lexer.getCurrentTokenValue();
      lexer.moveOn();
      return value;
    }
    throw new WXInterpretationException(token + "Token doesn't match" + lexer.source);
  }

  private enum Token {
    FUNC_NAME, PARAM_VALUE, LEFT_PARENT, RIGHT_PARENT, COMMA
  }

  public interface Mapper<K, V> {

    /**
     * Map one function to a specified type of object
     * @param functionName the name of the raw function.
     * @param raw the list of parameter of the raw function
     * @return the expected mapping relationship, where the key in the map is the same as the
     * key in the return value of {{@link #parse()}},
     * and the value in the map is the type of object that expected by user.
     */
    Map<K, V> map(String functionName, List<String> raw);
  }

  private static class WXInterpretationException extends RuntimeException {

    private WXInterpretationException(String msg) {
      super(msg);
    }
  }

  /**
   * Lexer for the parser. For now,  digit, alphabet, '(', ')', '.', ',', '+', '-' and '%' is
   * valid character, and '(', ')', ',', parameter value and function name is valid token.
   * Parameter value is defined as "(?i)[\+-]?[0-9]+(\.[0-9]+)?(%||deg||px)?" while function
   * name is defined as "[a-zA-Z]+".
   *
   * The Lexer can also be expressed using the following EBNF format.
   * <ul>
   *   <li>definition = {function};</li>
   *   <li>function = name, "(", value, { ",", value } , ")";</li>
   *   <li>name = character, {character};</li>
   *   <li>value = identifier, {identifier};</li>
   *   <li>identifier = character | "." | "%" | "+" | "-";</li>
   *   <li>character = digit | letter;</li>
   *   <li>digit =  "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;</li>
   *   <li>letter = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" |
   *   "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z" | "a" | "b" |
   *   "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" |
   *   "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;</li>
   * </ul>
   */
  private static class Lexer {

    private static final String LEFT_PARENT = "(";
    private static final String RIGHT_PARENT = ")";
    private static final String COMMA = ",";
    private static final char A_LOWER = 'a';
    private static final char Z_LOWER = 'z';
    private static final char A_UPPER = 'A';
    private static final char Z_UPPER = 'Z';
    private static final char ZERO = '0';
    private static final char NINE = '9';
    private static final char DOT = '.';
    private static final char MINUS = '-';
    private static final char PLUS = '+';
    private String source;
    private Token current;
    private String value;
    private int pointer = 0;

    private Lexer(String source) {
      this.source = source;
    }

    private Token getCurrentToken() {
      return current;
    }

    private String getCurrentTokenValue() {
      return value;
    }

    private boolean moveOn() {
      int start = pointer;
      char curChar;
      while (pointer < source.length()) {
        curChar = source.charAt(pointer);
        if (curChar == SPACE) {
          if (start == pointer++) {
            start++;
          } else {
            break;
          }
        } else if (isCharacterOrDigit(curChar) || curChar == DOT
                   || curChar == WXUtils.PERCENT || curChar == MINUS || curChar == PLUS) {
          pointer++;
        } else {
          if (start == pointer) {
            pointer++;
          }
          break;
        }
      }
      if (start != pointer) {
        String symbol = source.substring(start, pointer);
        moveOn(symbol);
        return true;
      } else {
        current = null;
        value = null;
        return false;
      }
    }

    private void moveOn(String token) {
      if (LEFT_PARENT.equals(token)) {
        current = Token.LEFT_PARENT;
        value = LEFT_PARENT;
      } else if (RIGHT_PARENT.equals(token)) {
        current = Token.RIGHT_PARENT;
        value = RIGHT_PARENT;
      } else if (COMMA.equals(token)) {
        current = Token.COMMA;
        value = COMMA;
      } else if (isFuncName(token)) {
        current = Token.FUNC_NAME;
        value = token;
      } else {
        current = Token.PARAM_VALUE;
        value = token;
      }
    }

    private boolean isFuncName(CharSequence funcName) {
      char letter;
      for (int i = 0; i < funcName.length(); i++) {
        letter = funcName.charAt(i);
        if (!((A_LOWER <= letter && letter <= Z_LOWER) ||
              (A_UPPER <= letter && letter <= Z_UPPER) ||
              letter == MINUS)) {
          return false;
        }
      }
      return true;
    }

    private boolean isCharacterOrDigit(char letter) {
      return (ZERO <= letter && letter <= NINE) || (A_LOWER <= letter && letter <= Z_LOWER) ||
              (A_UPPER <= letter && letter <= Z_UPPER);
    }
  }

}
