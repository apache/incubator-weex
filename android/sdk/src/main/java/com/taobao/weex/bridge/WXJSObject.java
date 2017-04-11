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
package com.taobao.weex.bridge;


import com.taobao.weex.utils.WXJsonUtils;

public class WXJSObject {

    public static final int NUMBER = 1;
    public static final int String = 2;
    public static final int JSON = 3;

    public Object data;
    public int type;

    public WXJSObject(int type, Object data) {
        this.type = type;
        this.data = data;
    }

    public WXJSObject(Object object) {

         if(null == object){
             type= String;
             data = "";
             return;
         }

        data = object;
        if (object instanceof Integer) {
            type = NUMBER;
            data = new Double(((Integer) object).intValue());
        } else if (object instanceof Double) {
            type = NUMBER;
        } else if (object instanceof Float) {
            type = NUMBER;
            data = new Double(((Float) object).intValue());
        } else if (object instanceof String) {
            type = String;
        } else if (object instanceof Object) {
            type = JSON;
            data = WXJsonUtils.fromObjectToJSONString(object,true);
        }
    }


}
