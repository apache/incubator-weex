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

#ifndef WEEXCORE_BASE_JS_RUNTIME_H
#define WEEXCORE_BASE_JS_RUNTIME_H

#include <cstdint>
#include <map>
#include "base_js_context.h"

namespace WeexCore {
    class BaseJSContext;

    class BaseJSRunTime {
    protected:
        std::map<uint32_t, BaseJSContext *> contextMap;
    public:
        uint32_t runTimeId;

        BaseJSRunTime() {}

        ~BaseJSRunTime() {
            contextMap.clear();
        }

    public:

        bool initRunTime();

        void destroy();

        BaseJSContext *createContext();

        void destroyContext(BaseJSContext *context);
    };
}

#endif //WEEXCORE_BASE_JS_RUNTIME_H
