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
package com.taobao.weex.utils;

import com.alibaba.fastjson.JSON;
import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.wson.Wson;
import com.taobao.weex.wson.WsonUtils;

/**
 * Created by furture on 2018/5/17.
 */

public class WXWsonJSONSwitch {

    private static final String TAG = "WXSwitch";

    /**
     * switch input data to target according to config, if not use wson return normal data
     * */
    public static  final byte[] convertJSONToWsonIfUseWson(byte[] json){
        if(!USE_WSON){
            return json;
        }
        if(json == null){
            return null;
        }
        return WsonUtils.toWson(JSON.parse(json));
    }

    /**
     * parse json or wson data by config switch
     * */
    public static final Object parseWsonOrJSON(byte[] data){
        if(data == null){
            return  null;
        }
        try{
            if(USE_WSON){
                return  Wson.parse(data);
            }else{
                return  JSON.parse(new String(data, "UTF-8"));
            }
        }catch (Exception e){
            WXLogUtils.e(TAG, e);
            if(USE_WSON){  //fallback
                return  JSON.parse(new String(data));
            }else{
                return Wson.parse(data);
            }
        }
    }

    /**
     * to wson or wson WXJSObject
     * */
    public static final WXJSObject toWsonOrJsonWXJSObject(Object tasks){
        if(tasks == null){
            return new WXJSObject(null);
        }
        if(tasks.getClass() == WXJSObject.class){
            return (WXJSObject) tasks;
        }
        if(USE_WSON) {
            return new WXJSObject(WXJSObject.WSON, Wson.toWson(tasks));
        }else{
            return new WXJSObject(WXJSObject.JSON, WXJsonUtils.fromObjectToJSONString(tasks));
        }
    }



    /**
     * config whether use json or wson,  you should update this value by updateGlobalConfig(String config)
     * in WXBridgeManager class  method
     * */
    public static  boolean USE_WSON = true;


    /**wson off */
    public static final String WSON_OFF = "wson_off";


}
