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
package com.taobao.weex.jsEngine;

import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.utils.WXLogUtils;

import java.io.Serializable;
import java.util.concurrent.ConcurrentHashMap;

import static com.taobao.weex.jsEngine.JSEngine.mCreatedJSContext;

public class JSContext implements Serializable {
    private ConcurrentHashMap<String, JSFunction> funcMap = new ConcurrentHashMap<>();
    //JSEnginePtrContainer
    private long mNativeContextPtr = 0;

    private JSException mExceptionTransfer = null;

    protected JSContext() {
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                mNativeContextPtr = nativeCreateContext();
                mCreatedJSContext.put(mNativeContextPtr, JSContext.this);
            }
        });
    }

    public void registerException(JSException e) {
        mExceptionTransfer = e;
    }

    public void destroy() {
        mCreatedJSContext.remove(mNativeContextPtr);
        mExceptionTransfer = null;
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                if (mNativeContextPtr != 0) {
                    long tmp = mNativeContextPtr;
                    mNativeContextPtr = 0;
                    nativeDestroyContext(tmp);
                }
            }
        });
    }

    public void bindFunction(final String name, final JSFunction function) {
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                funcMap.put(name, function);
                nativeBindFunc(mNativeContextPtr, name);
            }
        });
    }

    public void unBindFunction(final String name) {
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                funcMap.remove(name);
                nativeUnBindFunc(mNativeContextPtr, name);
            }
        });
    }

    public void Eval(final String script) {
        WXBridgeManager.getInstance().post(new Runnable() {
            @Override
            public void run() {
                nativeExecJS(mNativeContextPtr, script);
            }
        });
    }


    @CalledByNative
    public String Invoke(String name, String args) {
        JSFunction jsFunction = funcMap.get(name);
        if (jsFunction != null) {
            WXLogUtils.d("jsEngine invoke " + name + " arg:" + args);
            return jsFunction.invoke(args);
        }
        return "";
    }

    @CalledByNative
    public void Exception(String exception) {
        if (exception != null && mExceptionTransfer != null) {
            this.mExceptionTransfer.exception(exception);
        }
    }

    private native long nativeCreateContext();

    private native void nativeDestroyContext(long ctxPtr);

    private native void nativeBindFunc(long ctxPtr, String name);

    private native void nativeUnBindFunc(long ctxPtr, String name);

    private native void nativeExecJS(long ctxPtr, String script);
}
