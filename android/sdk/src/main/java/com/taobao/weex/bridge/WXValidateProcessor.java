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
package com.taobao.weex.bridge;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.component.WXComponent;

/**
 * Created by fengjunjie
 */
public interface WXValidateProcessor {

    WXModuleValidateResult onModuleValidate(WXSDKInstance wxsdkInstance, String moduleStr,
                                            String methodStr, JSONArray params,
                                            JSONObject options);

    WXComponentValidateResult onComponentValidate(WXSDKInstance wxsdkInstance,
                                                  String componentName,
                                                  WXComponent parentComponent);

    boolean needValidate(String bundleUrl);

    class WXComponentValidateResult {

        public boolean isSuccess;

        public String replacedComponent;

        public JSONObject validateInfo;

    }

    class WXModuleValidateResult {

        public boolean isSuccess;

        public JSONObject validateInfo;

    }
}
