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
package org.apache.weex.appfram.pickers;

import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Checkable;
import android.widget.ListView;
import android.widget.TextView;

import org.apache.weex.annotation.JSMethod;
import org.apache.weex.bridge.JSCallback;
import org.apache.weex.common.WXModule;
import org.apache.weex.common.WXThread;
import org.apache.weex.utils.WXResourceUtils;
import org.apache.weex.utils.WXViewUtils;

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

    private static final String KEY_TITLE_COLOR = "titleColor";
    private static final String KEY_CANCEL_TITLE_COLOR = "cancelTitleColor";
    private static final String KEY_CONFIRM_TITLE = "confirmTitle";
    private static final String KEY_CANCEL_TITLE = "cancelTitle";
    private static final String KEY_CONFIRM_TITLE_COLOR = "confirmTitleColor";
    private static final String KEY_TITLE_BACKGROUND_COLOR = "titleBackgroundColor";
    private static final String KEY_TEXT_COLOR = "textColor";
    private static final String KEY_SELECTION_COLOR = "selectionColor";

    private int selected;

    @JSMethod
    public void pick(Map<String, Object> options, JSCallback callback) {
        List<String> items = safeConvert(getOption(options, KEY_ITEMS, new ArrayList<String>()));
        try {
            performSinglePick(items, options, callback);
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
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

    private int getColor(Map<String, Object> options, String key, int defValue) {
        Object value = getOption(options, key, null);
        if (value == null) {
            return defValue;
        }
        return WXResourceUtils.getColor(value.toString(), defValue);
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
                },
                options);
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
                },
                options);

    }

    private void performSinglePick(final List<String> items, final Map<String, Object> options, final JSCallback callback) {
        selected = getOption(options, KEY_INDEX, 0);
        final int textColor = getColor(options, KEY_TEXT_COLOR, Color.TRANSPARENT);
        final int selectionColor = getColor(options, KEY_SELECTION_COLOR, Color.TRANSPARENT);
        final ArrayAdapter adapter = new ArrayAdapter<String>(
            mWXSDKInstance.getContext(),
            android.R.layout.simple_list_item_single_choice,
            items) {
            @NonNull
            @Override
            public View getView(int position, View convertView, @Nullable ViewGroup parent) {
                View itemView =  super.getView(position, convertView, parent);

                if (itemView != null && itemView instanceof Checkable) {
                    boolean needSelected = position == selected;
                    ((Checkable) itemView).setChecked(needSelected);

                    if (needSelected) {
                        itemView.setBackgroundColor(selectionColor);
                    } else {
                        itemView.setBackgroundColor(Color.TRANSPARENT);
                    }
                }

                if (itemView instanceof TextView && textColor != Color.TRANSPARENT) {
                    ((TextView) itemView).setTextColor(textColor);
                }

                return itemView;
            }
        };
        final AlertDialog dialog =  new AlertDialog.Builder(mWXSDKInstance.getContext())
                .setAdapter(adapter, null)
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
                .setCustomTitle(makeTitleView(mWXSDKInstance.getContext(), options))
                .create();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            //pre create the content view on dialog.
            //if not , the content view will not be created until dialog.show() called
            dialog.create();
        }

        final ListView listView = dialog.getListView();
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                selected = position;
                adapter.notifyDataSetChanged();
            }
        });

        dialog.getWindow().getDecorView().post(WXThread.secure(new Runnable() {
            @Override
            public void run() {
                Button confirm = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
                Button cancel = dialog.getButton(DialogInterface.BUTTON_NEGATIVE);

                if (confirm != null) {
                    String confirmTitle = getOption(options, KEY_CONFIRM_TITLE, null);
                    int confirmColor = getColor(options, KEY_CONFIRM_TITLE_COLOR, Color.TRANSPARENT);

                    if (confirmTitle != null) {
                        confirm.setText(confirmTitle);
                        confirm.setAllCaps(false);
                    }

                    if (confirmColor != Color.TRANSPARENT) {
                        confirm.setTextColor(confirmColor);
                        confirm.setAllCaps(false);
                    }
                }

                if (cancel != null) {
                    String cancelTitle = getOption(options, KEY_CANCEL_TITLE, null);
                    int cancelColor = getColor(options, KEY_CANCEL_TITLE_COLOR, Color.TRANSPARENT);

                    if (cancelTitle != null) {
                        cancel.setText(cancelTitle);
                    }

                    if (cancelColor != Color.TRANSPARENT) {
                        cancel.setTextColor(cancelColor);
                    }
                }
            }
        }));

        dialog.show();
    }

    private TextView makeTitleView(Context context, Map<String, Object> options) {
        String text = getOption(options, KEY_TITLE, null);
        if (text == null) {
            return null;
        }
        TextView textView = new TextView(context);
        textView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 20);
        int padding = WXViewUtils.dip2px(12);
        textView.setPadding(padding, padding, padding, padding);
        textView.getPaint().setFakeBoldText(true);
        textView.setBackgroundColor(getColor(options, KEY_TITLE_BACKGROUND_COLOR, Color.TRANSPARENT));
        textView.setTextColor(getColor(options, KEY_TITLE_COLOR, Color.BLACK));
        textView.setText(text);
        return textView;
    }
}
