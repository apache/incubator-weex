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
package com.taobao.weex.bridge;

import android.util.SparseArray;

/**
 * Manage async callbacks which waiting for the result of executing javascript
 */
/* default */ class ResultCallbackManager {
    // Unique id for callback after executing js
    private static long sCallbackId = 0;
    // Map for save callbacks, key is id
    private static SparseArray<ResultCallback> mResultCallbacks = new SparseArray<>();

    // Generate unique id
    static long generateCallbackId(ResultCallback callback) {
        if (sCallbackId >= Integer.MAX_VALUE) {
            sCallbackId = 0;
        }
        int id = (int) sCallbackId++;
        mResultCallbacks.put(id, callback);
        return id;
    }

    // Remove id from callback map
    static ResultCallback removeCallbackById(long id) {
        ResultCallback callback = mResultCallbacks.get((int) id);
        mResultCallbacks.remove((int) id);
        return callback;
    }
}
