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

import java.util.List;

/**
 * block, has more than one tokens, but only first will be execute.
 * Created by furture on 2017/8/28.
 */
public class Block extends Token {
    private List<Token> tokens;
    public Block(List<Token> tokens,  int type) {
        super("", type);
        this.tokens = tokens;
    }

    @Override
    public Object execute(Object context){
        if(tokens == null || tokens.size() == 0){
            return  null;
        }
        Token token = tokens.get(0);
        return token.execute(context);
    }

    public List<Token> getTokens() {
        return tokens;
    }

    @Override
    public String toString() {
        return "{" + tokens + '}';
    }
}
