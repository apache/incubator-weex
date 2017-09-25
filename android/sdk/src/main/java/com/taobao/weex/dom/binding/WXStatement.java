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
package com.taobao.weex.dom.binding;

import android.support.annotation.NonNull;
import android.support.v4.util.ArrayMap;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

/**
 * Created by furture on 2017/8/17.
 * statement expression for template list, like v-if v-for
 */
public class WXStatement implements Map<String, Object>,Cloneable  {

    /**
     * v-for statement, like
     * <code>
     *     <div v-for="(item, i) in dataset.panels">
     *     <text>{{i}}: {{item.name}}</text>
     *     </div>
     * </code>
     * json command:
     * <code>
     *      attr: {
     *          '[[repeat]]': {
     *          '@expression': 'dataList',
     *          '@index': 'index',
     *          '@alias': 'item'
     *         }
     *      }
     * </code>
     * */
    public static final String WX_FOR = "[[repeat]]";
    public static final String WX_FOR_INDEX = "@index";
    public static final String WX_FOR_ITEM = "@alias";
    public static final String WX_FOR_LIST = "@expression";


    /**
     * v-if statement, like:
     * <code>
     *   <p v-if="male">Male</p>
     * </code>
     * json command
     * attr: {
     *      '[[match]]': 'condition'
     *    }
     * */
    public static final String WX_IF = "[[match]]";


    private final ArrayMap<String, Object> map;

    public WXStatement() {
        this.map =  new ArrayMap<>();
    }

    public WXStatement(ArrayMap<String, Object> map) {
        this.map = map;
    }

    @Override
    public boolean equals(Object o) {
        return map.equals(o);
    }

    @Override
    public int hashCode() {
        return map.hashCode();
    }


    @Override
    public int size() {
        return map.size();
    }

    @Override
    public boolean isEmpty() {
        return map.isEmpty();
    }

    @Override
    public boolean containsKey(Object key) {
        return map.containsKey(key);
    }

    @Override
    public boolean containsValue(Object value) {
        return map.containsValue(value);
    }

    @Override
    public Object get(Object key) {
        return map.get(key);
    }

    @Override
    public Object put(String key, Object value) {
        return map.put(key, value);
    }

    @Override
    public Object remove(Object key) {
        return map.remove(key);
    }

    @Override
    public void putAll(@NonNull Map<? extends String, ?> m) {
        map.putAll(m);
    }

    @Override
    public void clear() {
        map.clear();
    }

    @NonNull
    @Override
    public Set<String> keySet() {
        return map.keySet();
    }

    @NonNull
    @Override
    public Collection<Object> values() {
        return map.values();
    }

    @NonNull
    @Override
    public Set<Entry<String, Object>> entrySet() {
        return map.entrySet();
    }


    @Override
    protected WXStatement clone(){
        return new WXStatement(map);
    }
}
