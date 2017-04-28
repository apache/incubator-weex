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
package com.taobao.weex.appfram.storage;

import java.util.Map;

/**
 * interface for {@link WXStorageModule} class.
 * this interface works as an adapter for different storage strategy.
 * the default is use {@link android.database.sqlite.SQLiteDatabase} to store k-v pairs.
 * You can call {@link com.taobao.weex.InitConfig.Builder#setStorageAdapter(IWXStorageAdapter)} to inject your own
 * storage implementation.
 * */
public interface IWXStorageAdapter {
    void setItem(String key, String value,OnResultReceivedListener listener);

    void getItem(String key,OnResultReceivedListener listener);

    void removeItem(String key,OnResultReceivedListener listener);

    void length(OnResultReceivedListener listener);

    void getAllKeys(OnResultReceivedListener listener);

    void setItemPersistent(String key, String value, OnResultReceivedListener listener);

    void close();

    /**
     * the callback of storage operation.
     * */
    interface OnResultReceivedListener {
        void onReceived(Map<String,Object> data);
    }

}
