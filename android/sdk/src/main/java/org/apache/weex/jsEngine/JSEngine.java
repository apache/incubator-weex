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
package org.apache.weex.jsEngine;

import static org.apache.weex.jsEngine.CallBackCode.ERROR_JSENGINE_CRASHED;
import static org.apache.weex.jsEngine.CallBackCode.JSENGINE_INIT_FINISH;

import android.annotation.SuppressLint;
import android.support.annotation.Nullable;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import org.apache.weex.WXSDKEngine;
import org.apache.weex.utils.WXLogUtils;

public class JSEngine implements Serializable {
    private static JSEngine mJsEngine = null;

    public static final ConcurrentHashMap<Long, JSContext> mCreatedJSContext = new ConcurrentHashMap<>();

    private static final ConcurrentHashMap<JSBiz, EnvCallback> mEnvCallbacks = new ConcurrentHashMap<>();

    private JSEngine() {
    }

    public static JSEngine getInstance() {
        if (mJsEngine == null) {
            synchronized (JSEngine.class) {
                if (mJsEngine == null) {
                    mJsEngine = new JSEngine();
                }
            }
        }
        return mJsEngine;
    }

    public void registerEnvCallback(JSBiz biz, EnvCallback e) {
        if (biz == null || e == null) {
            return;
        }
        mEnvCallbacks.put(biz, e);
    }

    public void unRegisterEnvCallback(JSBiz biz) {
        if (biz == null) {
            return;
        }
        mEnvCallbacks.remove(biz);
    }


    @Nullable
    public JSContext createJSContext() {
        if (!WXSDKEngine.isInitialized()) {
            WXLogUtils.e("Create JSContext Failed because of weex has not been initialized");
            return null;
        }
        return new JSContext();
    }

    public void engineInitFinished() {
        for (Map.Entry<JSBiz, EnvCallback> next : mEnvCallbacks.entrySet()) {
            next.getValue().error(JSENGINE_INIT_FINISH);
        }
    }

    @SuppressLint("UseSparseArrays")
    public void engineCrashed() {
        for (Map.Entry<JSBiz, EnvCallback> next : mEnvCallbacks.entrySet()) {
            next.getValue().error(ERROR_JSENGINE_CRASHED);
        }

        HashMap<Long, JSContext> map = new HashMap<>(mCreatedJSContext);
        mCreatedJSContext.clear();
        for (Map.Entry<Long, JSContext> next : map.entrySet()) {
            JSContext value = next.getValue();
            value.destroy();
        }
        map.clear();
    }
}
