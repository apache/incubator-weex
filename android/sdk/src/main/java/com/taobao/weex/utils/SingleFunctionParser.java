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

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class SingleFunctionParser<V> extends FunctionParser<String, List<V>> {

  public interface FlatMapper<V> {

    V map(String raw);
  }

  public interface NonUniformMapper<V>{
    List<V> map(List<String> raw);
  }

  /**
   * Construct a function parser for uniform parameters.
   * @param source the raw string representation of a group of function(s)
   * @param mapper the mapping rule between string and corresponding type of object.
   */
  public SingleFunctionParser(@NonNull String source, @NonNull final FlatMapper<V> mapper) {
    super(source, new Mapper<String, List<V>>() {
      @Override
      public Map<String, List<V>> map(String functionName, List<String> raw) {
        Map<String, List<V>> map = new HashMap<String, List<V>>();
        List<V> list = new LinkedList<V>();
        for (String item : raw) {
          list.add(mapper.map(item));
        }
        map.put(functionName, list);
        return map;
      }
    });
  }

  /**
   * Construct a function parser for non-uniform parameters.
   * @param source the raw string representation of a group of function(s)
   * @param mapper the mapping rule between string and corresponding type of object.
   */
  public SingleFunctionParser(@NonNull String source, @NonNull final NonUniformMapper<V> mapper) {
    super(source, new Mapper<String, List<V>>() {
      @Override
      public Map<String, List<V>> map(String functionName, List<String> raw) {
        Map<String, List<V>> map = new HashMap<String, List<V>>();
        map.put(functionName, mapper.map(raw));
        return map;
      }
    });
  }

  public List<V> parse(String functionName) {
    Map<String, List<V>> map = parse();
    if(map.containsKey(functionName)){
      return map.get(functionName);
    }
    return null;
  }
}
