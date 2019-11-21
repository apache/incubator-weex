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

import android.text.TextUtils;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Type;
import java.math.BigDecimal;

public class WXReflectionUtils {

  public static Object parseArgument(Type paramClazz, Object value) {
    if(value != null){
      if(value.getClass() == paramClazz){
        return  value;
      }
      if(paramClazz instanceof  Class){
        if( ((Class<?>) paramClazz).isAssignableFrom(value.getClass()))   {
          return value;
        }
      }
    }
    if (paramClazz == String.class) {
      return value instanceof String ? value : JSON.toJSONString(value);
    } else if (paramClazz == int.class) {
      return value.getClass().isAssignableFrom(int.class) ? value : WXUtils.getInt(value);
    } else if (paramClazz == long.class) {
      return value.getClass().isAssignableFrom(long.class) ? value : WXUtils.getLong(value);
    } else if (paramClazz == double.class) {
      return value.getClass().isAssignableFrom(double.class) ? value : WXUtils.getDouble(value);
    } else if (paramClazz == float.class) {
      return value.getClass().isAssignableFrom(float.class) ? value : WXUtils.getFloat(value);
    } else if (paramClazz == JSONArray.class && value != null && value.getClass() == JSONArray.class) {
      return  value;
    } else if (paramClazz == JSONObject.class && value != null && value.getClass() == JSONObject.class) {
      return  value;
    } else {
      return JSON.parseObject(value instanceof String ? (String) value : JSON.toJSONString(value), paramClazz);
    }
  }


  public static void setValue(Object obj, String fieldName, Object value) {
    if (obj == null || TextUtils.isEmpty(fieldName)) {
      return;
    }

    try {
      // Field field = obj.getClass().getDeclaredField(fieldName);
      Field field = getDeclaredField(obj, fieldName);

      Object realValue = value;
      if (value instanceof BigDecimal || value instanceof Number || value instanceof String) {
        if (field.getType() == Float.class || field.getType() == float.class) {
          realValue = Float.parseFloat(value.toString());
        } else if (field.getType() == Double.class || field.getType() == double.class) {
          realValue = Double.parseDouble(value.toString());
        } else if (field.getType() == Integer.class || field.getType() == int.class) {
          realValue = (int) Double.parseDouble(value.toString());
        } else if (field.getType() == Boolean.class || field.getType() == boolean.class) {
          realValue = Boolean.valueOf(value.toString());
        }
      }

      if (field.getType() == boolean.class || field.getType() == Boolean.class) {
        if (value != null) {
          realValue = Boolean.valueOf(value.toString());
        }
      }

      setProperty(obj, field, realValue);
    } catch (Exception e) {
      return;
    }
  }

  /**
   * get field form object and it's parent
   */
  public static Field getDeclaredField(Object object, String fieldName) {
    Field field = null;

    Class<?> clazz = object.getClass();

    for (; clazz != Object.class; clazz = clazz.getSuperclass()) {
      try {
        field = clazz.getDeclaredField(fieldName);
        return field;
      } catch (Exception e) {

      }
    }

    return null;
  }

  /**
   * Set property(field) of the specified object.
   * @param bean The object which has the given property
   * @param field The field to be set
   * @param value The value to be set to the field
   * @throws IllegalAccessException
   * @throws InvocationTargetException
   * @throws NoSuchMethodException
   */
  public static void setProperty(Object bean, Field field, Object value) throws IllegalAccessException,
          InvocationTargetException,
          NoSuchMethodException {

    if (bean == null || field == null) {
      return;
    }

    try {
      field.setAccessible(true);
      field.set(bean, value);
    } catch (Exception e) {
    }

  }
}
