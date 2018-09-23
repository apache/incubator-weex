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
package com.taobao.weex.render.threads;


import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import java.lang.ref.WeakReference;

/**
 * Created by furture on 2018/7/27.
 */

public class WeakRefHandler extends Handler {
    private WeakReference<Callback> callbackRef;


    public WeakRefHandler(Callback callback) {
        callbackRef = new WeakReference<Callback>(callback);
    }

    public WeakRefHandler(Looper looper, Callback callback) {
        super(looper);
        callbackRef = new WeakReference<Callback>(callback);
    }

    @Override
    public void handleMessage(Message msg) {
        Callback callback = callbackRef.get();
        if(callback != null){
            callback.handleMessage(msg);
        }else{
            removeCallbacks(null);
        }
    }
}
