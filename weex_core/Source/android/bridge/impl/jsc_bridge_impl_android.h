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
//
// Created by 陈佩翰 on 2018/5/22.
//

#ifndef WEEXV8_JSC_BRIDGE_IMPL_ANDROID_H
#define WEEXV8_JSC_BRIDGE_IMPL_ANDROID_H

#include <core/bridge/js_bridge.h>
#include <list>

namespace WeexCore {
    class JSCBridgeAndroid : public JSBridge {
    protected:
        //<runTimeId,contextId>
        std::map<uint32_t, std::list<uint32_t> *> vmMap;

    protected:
        JSCBridgeAndroid() {

        }

        ~JSCBridgeAndroid() {
            for (auto it = vmMap.begin(); it != vmMap.end(); it++) {
                destroyJSRunTime(it->first);
            }
            // delete vmMap;
        }


    public:
        bool createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> &params);

        bool destroyJSRunTime(uint32_t runTimeId);

        bool createJSContext(uint32_t runTimeId, uint32_t contextId);

        bool destroyJSContext(uint32_t runTimeId, uint32_t contextId);

        WXValue
        execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName, WXValue args[], uint8_t argsLength);

        WXValue executeJavascript(uint32_t runTimeId, uint32_t contextId, char *script);

        void reigsterJSVale(uint32_t runTimeId, uint32_t contextId, char *name, WXValue value);

        WXValue getJSVale(uint32_t runTimeId, uint32_t contextId, char *name);

        void reigsterJSFunc(uint32_t runTimeId, uint32_t contextId, WXFuncSignature func);
    };
}


#endif //WEEXV8_JSC_BRIDGE_IMPL_ANDROID_H
