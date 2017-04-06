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
package com.taobao.weex.appfram.pickers;

import android.content.DialogInterface;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;

import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXModule;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by moxun on 16/10/27.
 */

public class WXPickersModule extends WXModule {

    private static final String SUCCESS = "success";
    private static final String CANCEL = "cancel";
    private static final String ERROR = "error";

    private static final String RESULT = "result";
    private static final String DATA = "data";

    private static final String KEY_VALUE = "value";
    private static final String KEY_INDEX = "index";
    private static final String KEY_TITLE = "title";
    private static final String KEY_MAX = "max";
    private static final String KEY_MIN = "min";
    private static final String KEY_ITEMS = "items";

    private int selected;

    @JSMethod
    public void pick(Map<String, Object> options, JSCallback callback) {
        List<String> items = safeConvert(getOption(options, KEY_ITEMS, new ArrayList<String>()));
        int index = getOption(options, KEY_INDEX, 0);
        String title = getOption(options, KEY_TITLE, null);
        performSinglePick(items, index, title, callback);
    }

    @JSMethod
    public void pickDate(Map<String, Object> options, JSCallback callback) {
        performPickDate(options, callback);
    }

    @JSMethod
    public void pickTime(Map<String, Object> options, JSCallback callback) {
        performPickTime(options, callback);
    }

    private List<String> safeConvert(List src) {
        List<String> result = new ArrayList<>(src.size());
        for (Object obj : src) {
            result.add(String.valueOf(obj));
        }
        return result;
    }

    private <T> T getOption(Map<String, Object> options, String key, T defValue) {
        Object value = options.get(key);
        if (value == null) {
            return defValue;
        } else {
            try {
                return (T) value;
            } catch (Exception e) {
                e.printStackTrace();
                return defValue;
            }
        }
    }

    private void performPickTime(Map<String, Object> options, final JSCallback callback) {
        String value = getOption(options, KEY_VALUE, "");
        DatePickerImpl.pickTime(
                mWXSDKInstance.getContext(),
                value,
                new DatePickerImpl.OnPickListener() {
                    @Override
                    public void onPick(boolean set, @Nullable String result) {
                        if (set) {
                            Map<String, Object> ret = new HashMap<>(2);
                            ret.put(RESULT, SUCCESS);
                            ret.put(DATA, result);
                            callback.invoke(ret);
                        } else {
                            Map<String, Object> ret = new HashMap<>(2);
                            ret.put(RESULT, CANCEL);
                            ret.put(DATA, null);
                            callback.invoke(ret);
                        }
                    }
                }
        );
    }

    private void performPickDate(Map<String, Object> options, final JSCallback callback) {
        String value = getOption(options, KEY_VALUE, "");
        String max = getOption(options, KEY_MAX, "");
        String min = getOption(options, KEY_MIN, "");
        DatePickerImpl.pickDate(
                mWXSDKInstance.getContext(),
                value,
                max,
                min,
                new DatePickerImpl.OnPickListener() {
                    @Override
                    public void onPick(boolean set, @Nullable String result) {
                        if (set) {
                            Map<String, Object> ret = new HashMap<>(2);
                            ret.put(RESULT, SUCCESS);
                            ret.put(DATA, result);
                            callback.invoke(ret);
                        } else {
                            Map<String, Object> ret = new HashMap<>(2);
                            ret.put(RESULT, CANCEL);
                            ret.put(DATA, null);
                            callback.invoke(ret);
                        }
                    }
                });

    }

    private void performSinglePick(List<String> items, int checked, String title, final JSCallback callback) {
        selected = checked;
        new AlertDialog.Builder(mWXSDKInstance.getContext())
                .setTitle(title)
                .setSingleChoiceItems(items.toArray(new String[items.size()]), checked, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        selected = which;
                    }
                })
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //which == -1
                        Map<String, Object> ret = new HashMap<>(2);
                        ret.put(RESULT, SUCCESS);
                        ret.put(DATA, selected);
                        callback.invoke(ret);
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //which == -2
                        Map<String, Object> ret = new HashMap<>(2);
                        ret.put(RESULT, CANCEL);
                        ret.put(DATA, -1);
                        callback.invoke(ret);
                    }
                })
                .show();
    }
}
