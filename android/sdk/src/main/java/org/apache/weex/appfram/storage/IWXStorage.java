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
package org.apache.weex.appfram.storage;

import android.support.annotation.Nullable;

import org.apache.weex.bridge.JSCallback;

interface IWXStorage {
    public void setItem(String key, String value,@Nullable JSCallback callback);
    public void getItem(String key,@Nullable JSCallback callback);
    public void removeItem(String key,@Nullable JSCallback callback);
    public void length(@Nullable JSCallback callback);
    public void getAllKeys(@Nullable JSCallback callback);
    public void setItemPersistent(String key, String value, @Nullable JSCallback callback);
}
