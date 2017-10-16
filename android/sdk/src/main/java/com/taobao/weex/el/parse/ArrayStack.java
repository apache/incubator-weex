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

import java.util.ArrayList;
import java.util.List;

/**
 * stack for parse and execute code
 * Created by jianbai.gbj on 2017/9/1.
 */
public class ArrayStack<T> {

    private ArrayList<T> stack;

    public ArrayStack() {
        this.stack = new ArrayList<T>(4);
    }

    public int size() {
        return stack.size();
    }

    public T pop() {
        return stack.remove(stack.size()-1);
    }

    public void push(T token) {
        stack.add(token);
    }


    public T peek() {
        return stack.get(stack.size()-1);
    }

    public T get(int i) {
        return stack.get(i);
    }

    public T remove(int i) {
        return stack.remove(i);
    }

    public void add(int i, T t) {
        stack.add(i, t);
    }

    public boolean isEmpty() {
        return   stack.isEmpty();
    }

    public List<T> getList(){
        return stack;
    }
}
