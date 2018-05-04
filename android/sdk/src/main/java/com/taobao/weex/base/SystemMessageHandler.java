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
package com.taobao.weex.base;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.Serializable;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;


public class SystemMessageHandler extends Handler implements Serializable {

    private static final String TAG = "SystemMessageHandler";

    private static final int SCHEDULED_WORK = 1;

    private long mMessagePumpDelegateNative = 0;

    private boolean mIsRunning = false;

    private Method mMessageMethodSetAsynchronous;

    private native void nativeRunWork(long delegateNative);

    private SystemMessageHandler(long messagePumpDelegateNative) {
        this.mMessagePumpDelegateNative = messagePumpDelegateNative;
        try {
            Class<?> messageClass = Class.forName("android.os.Message");
            mMessageMethodSetAsynchronous = messageClass.getMethod(
                    "setAsynchronous", new Class[]{boolean.class});
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "Failed to find android.os.Message class:" + e);
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "Failed to load Message.setAsynchronous method:" + e);
        } catch (RuntimeException e) {
            Log.e(TAG, "Exception while loading Message.setAsynchronous method: " + e);
        }
        mIsRunning = true;
    }

    @CalledByNative
    public static SystemMessageHandler create(long messagePumpDelegateNative) {
        return new SystemMessageHandler(messagePumpDelegateNative);
    }

    @CalledByNative
    private void scheduleWork() {
        sendMessage(obtainAsyncMessage(SCHEDULED_WORK));
    }

    @CalledByNative
    private void stop() {
        mIsRunning = false;
    }

    private Message obtainAsyncMessage(int what) {
        Message msg = Message.obtain();
        msg.what = what;
        if (mMessageMethodSetAsynchronous != null) {
            // If invocation fails, assume this is indicative of future
            // failures, and avoid log spam by nulling the reflected method.
            try {
                mMessageMethodSetAsynchronous.invoke(msg, true);
            } catch (IllegalAccessException e) {
                Log.e(TAG, "Illegal access to asynchronous message creation, disabling.");
                mMessageMethodSetAsynchronous = null;
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "Illegal argument for asynchronous message creation, disabling.");
                mMessageMethodSetAsynchronous = null;
            } catch (InvocationTargetException e) {
                Log.e(TAG, "Invocation exception during asynchronous message creation, disabling.");
                mMessageMethodSetAsynchronous = null;
            } catch (RuntimeException e) {
                Log.e(TAG, "Runtime exception during asynchronous message creation, disabling.");
                mMessageMethodSetAsynchronous = null;
            }
        }
        return msg;
    }

    @Override
    public void handleMessage(Message msg) {
        if(mIsRunning) {
            nativeRunWork(mMessagePumpDelegateNative);
        }
    }
}
