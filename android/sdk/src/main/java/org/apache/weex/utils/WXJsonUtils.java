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
package org.apache.weex.utils;


import android.support.annotation.NonNull;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.serializer.SerializerFeature;
import org.apache.weex.WXEnvironment;
import org.apache.weex.common.WXRuntimeException;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Tool for parse JSON
 */
public class WXJsonUtils {


  public @NonNull static <T> List<T> getList(String json, Class<T> clazz) {
    List<T> result = null;
    try {
      result = JSONObject.parseArray(json, clazz);
    } catch (Exception e) {
      e.printStackTrace();
      result = new ArrayList<>();
    }
    return result;
  }

  public @NonNull static String fromObjectToJSONString(Object obj, boolean WriteNonStringKeyAsString){
    try {
      if(WriteNonStringKeyAsString) {
        return JSON.toJSONString(obj, SerializerFeature.WriteNonStringKeyAsString);
      }else {
        return JSON.toJSONString(obj);
      }
    }catch(Exception e){
      if(WXEnvironment.isApkDebugable()){
        throw new WXRuntimeException("fromObjectToJSONString parse error!");
      }
      WXLogUtils.e("fromObjectToJSONString error:", e);
      return "{}";
    }
  }
  public @NonNull static String fromObjectToJSONString(Object obj) {

    return fromObjectToJSONString(obj,false);
  }

  /**
   * Put the map info in the JSONObject to the container.
   * This method check for null value in the JSONObject
   * and won't put the null value in the container.
   * As {@link ConcurrentHashMap#putAll(Map)} will throws an exception if the key or value to
   * be put is null, it is necessary to invoke this method as replacement of
   * {@link Map#putAll(Map)}
   * @param container container to contain the JSONObject.
   * @param rawValue jsonObject, contains map info.
   */
  public static void putAll(Map<String, Object> container, JSONObject rawValue) {
    String key;
    Object value;
    for (Map.Entry<String, Object> entry : rawValue.entrySet()) {
      key = entry.getKey();
      value = entry.getValue();
      if (key != null && value != null) {
        container.put(key, value);
      }
    }
  }
}
