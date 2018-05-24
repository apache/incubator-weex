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

#include "jsc_bridge_impl_android.h"

namespace WeexCore {

    bool JSCBridgeAndroid::createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> &params) {
        auto search = vmMap.find(runTimeId);
        if (search == vmMap.end()) {
            return true;
        }

        //todo createRunTime
        //INITfRAMEWORK



        bool contextCreated = createJSContext(runTimeId, 0);
        if (contextCreated) {
            std::list<uint32_t> *contextList = new std::list<uint32_t>();
            contextList->push_back(0);
            vmMap.insert(std::pair<uint32_t, std::list<uint32_t> *>(runTimeId, contextList));
        }
        return true;
    }

    bool JSCBridgeAndroid::destroyJSRunTime(uint32_t runTimeId) {
        auto search = vmMap.find(runTimeId);
        if (search == vmMap.end()) {
            return true;
        }

        std::list<uint32_t> *contextList = search->second;
        if (!contextList) {
            return true;
        }
        for (auto it = contextList->begin(); it != contextList->end(); it++) {
            destroyJSContext(runTimeId, *it);
        }

        contextList->clear();
        vmMap.erase(search);
        delete contextList;
        return true;
    }

    bool JSCBridgeAndroid::createJSContext(uint32_t runTimeId, uint32_t contextId) {

    }

    bool JSCBridgeAndroid::destroyJSContext(uint32_t runTimeId, uint32_t contextId) {

    }


    WXValue JSCBridgeAndroid::execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName, WXValue args[],
                                           uint8_t argsLength) {

    }

    WXValue JSCBridgeAndroid::executeJavascript(uint32_t runTimeId, uint32_t contextId, char *script) {

    }

    void JSCBridgeAndroid::reigsterJSVale(uint32_t runTimeId, uint32_t contextId, char *name, WXValue value) {

    }

    WXValue JSCBridgeAndroid::getJSVale(uint32_t runTimeId, uint32_t contextId, char *name) {

    }

    void JSCBridgeAndroid::reigsterJSFunc(uint32_t runTimeId, uint32_t contextId, WXFuncSignature func) {

    }
}

