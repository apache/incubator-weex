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
package org.apache.weex.appfram.storage;

import android.support.annotation.Nullable;

import org.apache.weex.bridge.JSCallback;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class StorageResultHandler {

    private StorageResultHandler() {
    }

    private static final String RESULT = "result";
    private static final String DATA = "data";


    private static final String UNDEFINED = "undefined";
    private static final String RESULT_FAILED_NO_HANDLER = "no_handler";
    private static final String RESULT_FAILED_INVALID_PARAM = "invalid_param";


    private static final String RESULT_OK = "success";
    private static final String RESULT_FAILED = "failed";


    public static Map<String, Object> getItemResult(String result) {
        Map<String, Object> map = new HashMap<>(4);
        map.put(RESULT, result != null ? RESULT_OK : RESULT_FAILED);
        map.put(DATA, result != null ? result : UNDEFINED);
        return map;
    }

    public static Map<String, Object> setItemResult(boolean result) {
        Map<String, Object> map = new HashMap<>(4);
        map.put(RESULT, result ? RESULT_OK : RESULT_FAILED);
        map.put(DATA, UNDEFINED);
        return map;
    }


    public static Map<String, Object> removeItemResult(boolean result) {
        Map<String, Object> map = new HashMap<>(4);
        map.put(RESULT, result ? RESULT_OK : RESULT_FAILED);
        map.put(DATA, UNDEFINED);
        return map;
    }

    public static Map<String, Object> getLengthResult(long result) {
        Map<String, Object> map = new HashMap<>(4);
        map.put(RESULT, RESULT_OK);
        map.put(DATA, result);
        return map;
    }

    public static Map<String, Object> getAllkeysResult(List<String> result) {
        if (result == null) {
            result = new ArrayList<>(1);
        }
        Map<String, Object> map = new HashMap<>(4);
        map.put(RESULT, RESULT_OK);
        map.put(DATA, result);
        return map;
    }


    private static void handleResult(@Nullable JSCallback callback, String result, Object data) {
        if (callback == null) {
            return;
        }
        Map<String, Object> retVal = new HashMap<>(4);
        retVal.put(RESULT, result);
        retVal.put(DATA, data);
        callback.invoke(retVal);
    }

    public static void handleNoHandlerError(@Nullable JSCallback callback) {
        handleResult(callback, RESULT_FAILED, RESULT_FAILED_NO_HANDLER);
    }

    public static void handleInvalidParam(@Nullable JSCallback callback) {
        handleResult(callback, RESULT_FAILED, RESULT_FAILED_INVALID_PARAM);
    }


}
