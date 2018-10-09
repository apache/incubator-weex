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
package com.taobao.weex.ui;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

class WXRenderHandler extends Handler {

    public WXRenderHandler() {
        super(Looper.getMainLooper());
    }

    public final boolean post(String instanceId, Runnable r) {
        Message msg = Message.obtain(this, r);
        // Use what to match runnable. Make sure don't override callback method.
        msg.what = instanceId.hashCode();
        return sendMessageDelayed(msg, 0);
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);
    }
}
