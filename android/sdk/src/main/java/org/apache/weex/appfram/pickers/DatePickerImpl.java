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

import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.TimePicker;

import org.apache.weex.common.WXThread;
import org.apache.weex.utils.WXLogUtils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.Map;

/**
 * Created by moxun on 16/11/23.
 */

public class DatePickerImpl {

    private static final int DEFAULT_START_YEAR = 1900;
    private static final int DEFAULT_END_YEAR = 2100;

    private static SimpleDateFormat timeFormatter;
    private static SimpleDateFormat dateFormatter;

    public static void pickDate(@NonNull Context context, String value, String max, String min, @NonNull final OnPickListener listener, @Nullable Map<String, Object> extras) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(parseDate(value));
        final DatePickerDialog dialog = new DatePickerDialog(
                context,
                new DatePickerDialog.OnDateSetListener() {
                    @Override
                    public void onDateSet(DatePicker view, int year, int monthOfYear, int dayOfMonth) {
                        int realMonth = monthOfYear + 1;
                        String realMonthString = realMonth < 10 ? "0" + realMonth : String.valueOf(realMonth);
                        String realDayString = dayOfMonth < 10 ? "0" + dayOfMonth : String.valueOf(dayOfMonth);
                        String result = year + "-" + realMonthString + "-" + realDayString;
                        listener.onPick(true, result);
                    }
                },
                calendar.get(Calendar.YEAR),
                calendar.get(Calendar.MONTH),
                calendar.get(Calendar.DAY_OF_MONTH)
        );

        final DatePicker datePicker = dialog.getDatePicker();

        final Calendar defaultMinDate = Calendar.getInstance(Locale.getDefault());
        final Calendar defaultMaxDate = Calendar.getInstance(Locale.getDefault());

        defaultMinDate.set(DEFAULT_START_YEAR, Calendar.JANUARY, 1);
        defaultMaxDate.set(DEFAULT_END_YEAR, Calendar.DECEMBER, 31);

        if (!TextUtils.isEmpty(min)) {
            long minDate = parseDate(min).getTime();
            if (datePicker.getMaxDate() >= minDate) {
                datePicker.setMinDate(parseDate(min).getTime());
            } else {
                datePicker.setMinDate(defaultMinDate.getTimeInMillis());
                datePicker.setMaxDate(defaultMaxDate.getTimeInMillis());
            }
        }
        if (!TextUtils.isEmpty(max)) {
            long maxDate = parseDate(max).getTime();
            if (datePicker.getMinDate() <= maxDate) {
                datePicker.setMaxDate(parseDate(max).getTime());
            } else {
                datePicker.setMinDate(defaultMinDate.getTimeInMillis());
                datePicker.setMaxDate(defaultMaxDate.getTimeInMillis());
            }
        }

        dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                listener.onPick(false, null);
            }
        });

        setButtonText(dialog, DialogInterface.BUTTON_NEGATIVE, String.valueOf(extras != null ? extras.get("cancelTitle") : null));
        setButtonText(dialog, DialogInterface.BUTTON_POSITIVE, String.valueOf(extras != null ? extras.get("confirmTitle") : null));

        dialog.show();
    }

    public static void pickTime(@NonNull Context context, String value, @NonNull final OnPickListener listener, @Nullable Map<String, Object> extras) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(parseTime(value));
        TimePickerDialog dialog = new TimePickerDialog(
                context,
                new TimePickerDialog.OnTimeSetListener() {
                    @Override
                    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
                        String h = hourOfDay < 10 ? "0" + hourOfDay : String.valueOf(hourOfDay);
                        String m = minute < 10 ? "0" + minute : String.valueOf(minute);
                        String result = h + ":" + m;
                        listener.onPick(true, result);
                    }
                },
                calendar.get(Calendar.HOUR_OF_DAY),
                calendar.get(Calendar.MINUTE),
                false
        );

        dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                listener.onPick(false, null);
            }
        });

        setButtonText(dialog, DialogInterface.BUTTON_NEGATIVE, String.valueOf(extras != null ? extras.get("cancelTitle") : null));
        setButtonText(dialog, DialogInterface.BUTTON_POSITIVE, String.valueOf(extras != null ? extras.get("confirmTitle") : null));

        dialog.show();
    }

    public interface OnPickListener {
        void onPick(boolean set, @Nullable String result);
    }

    private static Date parseDate(String s) {
        if (dateFormatter == null) {
            dateFormatter = new SimpleDateFormat("yyyy-MM-dd", Locale.getDefault());
        }

        try {
            return dateFormatter.parse(s);
        } catch (ParseException e) {
            //don't worry
            WXLogUtils.w("[DatePickerImpl] " + e.toString());
        }
        return new Date();
    }

    private static Date parseTime(String s) {
        if (timeFormatter == null) {
            timeFormatter = new SimpleDateFormat("HH:mm", Locale.getDefault());
        }

        try {
            return timeFormatter.parse(s);
        } catch (ParseException e) {
            //don't worry
            WXLogUtils.w("[DatePickerImpl] " + e.toString());
        }
        return new Date();
    }

    private static void setButtonText(final AlertDialog dialog, final int which, final CharSequence text) {
        if (TextUtils.isEmpty(text) || "null".equals(text)) {
            return;
        }
        try {
            dialog.getWindow().getDecorView().post(WXThread.secure(new Runnable() {
                @Override
                public void run() {
                    Button button = dialog.getButton(which);
                    if (button != null) {
                        button.setAllCaps(false);
                        button.setText(text);
                    }
                }
            }));
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
