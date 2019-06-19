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
package com.taobao.weex.ui.module;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;

public class WXDeviceInfoModule extends WXModule {
    
    @JSMethod(uiThread = false)
    public void enableFullScreenHeight(final JSCallback callback,JSONObject extend){
        if(mWXSDKInstance != null) {
            mWXSDKInstance.setEnableFullScreenHeight(true);
            if(callback != null) {
                long fullScreenHeight = WXViewUtils.getScreenHeight(mWXSDKInstance.getInstanceId());
                HashMap<String, String> ret = new HashMap();
                ret.put("fullScreenHeight", String.valueOf(fullScreenHeight));
                callback.invoke(ret);
            }

        }

    }

}
