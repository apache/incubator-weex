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

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.annotation.JSMethod;

import java.util.Map;

public class WXStorageModule extends WXSDKEngine.DestroyableModule implements IWXStorage {

    IWXStorageAdapter mStorageAdapter;

    private IWXStorageAdapter ability() {
        if (mStorageAdapter != null) {
            return mStorageAdapter;
        }
        mStorageAdapter = WXSDKEngine.getIWXStorageAdapter();
        return mStorageAdapter;
    }


    @Override
    @JSMethod(uiThread = false)
    public void setItem(String key, String value, @Nullable final JSCallback callback) {
        if (TextUtils.isEmpty(key) || value == null) {
            StorageResultHandler.handleInvalidParam(callback);
            return;
        }

        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.setItem(key, value, new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });


    }

    @Override
    @JSMethod(uiThread = false)
    public void getItem(String key, @Nullable final JSCallback callback) {
        if (TextUtils.isEmpty(key)) {
            StorageResultHandler.handleInvalidParam(callback);
            return;
        }

        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.getItem(key, new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });
    }

    @Override
    @JSMethod(uiThread = false)
    public void removeItem(String key, @Nullable final JSCallback callback) {
        if (TextUtils.isEmpty(key)) {
            StorageResultHandler.handleInvalidParam(callback);
            return;
        }

        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.removeItem(key, new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });
    }

    @Override
    @JSMethod(uiThread = false)
    public void length(@Nullable final JSCallback callback) {
        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.length(new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });
    }

    @Override
    @JSMethod(uiThread = false)
    public void getAllKeys(@Nullable final JSCallback callback) {
        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.getAllKeys(new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });
    }

    @Override
    @JSMethod(uiThread = false)
    public void setItemPersistent(String key, String value, @Nullable final JSCallback callback) {
        if (TextUtils.isEmpty(key) || value == null) {
            StorageResultHandler.handleInvalidParam(callback);
            return;
        }

        IWXStorageAdapter adapter = ability();
        if (adapter == null) {
            StorageResultHandler.handleNoHandlerError(callback);
            return;
        }
        adapter.setItemPersistent(key, value, new IWXStorageAdapter.OnResultReceivedListener() {
            @Override
            public void onReceived(Map<String, Object> data) {
                if(callback != null){
                    callback.invoke(data);
                }
            }
        });
    }

    @Override
    public void destroy() {
        IWXStorageAdapter adapter = ability();
        if (adapter != null) {
            adapter.close();
        }
    }
}
