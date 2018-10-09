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
package com.taobao.weex.instance;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.ui.WXRenderManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Created by furture on 2018/8/23.
 */

public abstract  class InstanceOnFireEventInterceptor {

    private List<String> listenEvents;

    public InstanceOnFireEventInterceptor() {
        this.listenEvents = new ArrayList<>();
    }

    public void addInterceptEvent(String event){
        if(!listenEvents.contains(event)){
            this.listenEvents.add(event);
        }
    }

    public List<String> getListenEvents() {
        return listenEvents;
    }

    public void onInterceptFireEvent(String instanceId, String elementRef, String type, Map<String, Object> params, Map<String, Object> domChanges){
        if(params == null){
            return;
        }
        if(this.listenEvents.contains(type)){
            onFireEvent(instanceId, elementRef, type, params, domChanges);
        }
    }

    public  abstract void onFireEvent(String instanceId, String elementRef,String type, Map<String, Object> params, Map<String, Object> domChanges);
}