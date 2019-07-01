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
// Created by 董亚运 on 2019-07-01.
//

#ifndef WEEX_PROJECT_TLOG_H
#define WEEX_PROJECT_TLOG_H

#include "weex_env.h"

namespace Weex {
    class TLog {
    public: static void tlog(const char* fmt, ...) {
            va_list arg_list;
            int size;
            va_start(arg_list, fmt);
            size = vsnprintf(nullptr, 0, fmt, arg_list);
            int byte_count = size + 1;
            char * buffer = (char*)malloc(byte_count);
            memset(buffer, 0, byte_count);
            va_end(arg_list);
            if (size > 0) {
                va_start(arg_list, fmt);
                vsnprintf(const_cast<char*>(buffer), byte_count, fmt, arg_list);
                va_end(arg_list);
            }
            WeexEnv::getEnv()->sendTLog("JSEngine", buffer);

            free(buffer);
        }
    };


}

#endif //WEEX_PROJECT_TLOG_H
