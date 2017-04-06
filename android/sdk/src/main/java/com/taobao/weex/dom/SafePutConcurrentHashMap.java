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
package com.taobao.weex.dom;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by sospartan on 8/23/16.
 */
public class SafePutConcurrentHashMap<K, V> extends ConcurrentHashMap<K, V> {


  @Override
  public void putAll(Map<? extends K, ? extends V> m) {
    Iterator<? extends Entry<? extends K, ? extends V>> iterator = m.entrySet().iterator();
    while (iterator.hasNext()) {
      Entry<? extends K, ? extends V> item = iterator.next();
      if (item.getKey() == null || item.getValue() == null) {
        iterator.remove();
      }
    }

    super.putAll(m);
  }

  @Override
  public V put(K key, V value) {
    if (key == null || value == null) {
      return null;
    }
    return super.put(key, value);
  }
}
