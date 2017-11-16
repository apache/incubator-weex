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
package com.taobao.weex.ui.component.helper;

import android.support.annotation.Nullable;
import android.widget.TextView;

import com.taobao.weex.appfram.pickers.DatePickerImpl;
import com.taobao.weex.ui.component.AbstractEditComponent;

/**
 * Created by moxun on 16/10/12.
 */

public class WXTimeInputHelper {
    public static void pickDate(String max, String min, final AbstractEditComponent component) {
        final TextView target = component.getHostView();

        DatePickerImpl.pickDate(
                target.getContext(),
                target.getText().toString(),
                max,
                min,
                new DatePickerImpl.OnPickListener() {
                    @Override
                    public void onPick(boolean set, @Nullable String result) {
                        if (set) {
                            target.setText(result);
                            component.performOnChange(result);
                        }
                    }
                },
                null);
    }

    public static void pickTime(final AbstractEditComponent component) {
        final TextView target = component.getHostView();

        DatePickerImpl.pickTime(
                target.getContext(),
                target.getText().toString(),
                new DatePickerImpl.OnPickListener() {
                    @Override
                    public void onPick(boolean set, @Nullable String result) {
                        if (set) {
                            target.setText(result);
                            component.performOnChange(result);
                        }
                    }
                },
                null
        );
    }
}
