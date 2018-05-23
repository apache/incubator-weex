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
// Created by 陈佩翰 on 2018/5/21.
//

#ifndef WEEXV8_JSBRIDGE_H
#define WEEXV8_JSBRIDGE_H


#include "wx_type_define.h"
#include <map>


namespace WeexCore {

    class JSBridge {

    protected:
        JSBridge() {}

    public:
        virtual bool createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> &params)=0;

        virtual bool destroyJSRunTime(uint32_t runTimeId)=0;

        virtual bool createJSContext(uint32_t runTimeId, uint32_t contextId) =0;

        virtual bool destroyJSContext(uint32_t runTimeId, uint32_t contextId) =0;

        virtual WXValue
        execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName, WXValue args[], uint8_t argsLength)=0;

        virtual WXValue executeJavascript(uint32_t runTimeId, uint32_t contextId, char *script)=0;

        virtual void reigsterJSVale(uint32_t runTimeId, uint32_t contextId, char *name, WXValue value)=0;

        virtual WXValue getJSVale(uint32_t runTimeId, uint32_t contextId, char *name)=0;

        virtual void reigsterJSFunc(uint32_t runTimeId, uint32_t contextId, WXFuncSignature func)=0;

        WXValue callExecNative(uint32_t runTimeId, uint32_t contextId, WXValue args[], uint8_t argsLength);

        void callReportJSException(int32_t runTimeId, int32_t contextId, char *exception,
                                   std::map<std::string, std::string> &extInfos);

        void callSetJSVersion(const char *jsVersion);
    };
}

#endif //WEEXV8_JSBRIDGE_H
