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
#ifndef ThreadFunctionInvocation_h
#define ThreadFunctionInvocation_h

namespace WTF {

typedef void (*ThreadFunction)(void* argument);

struct ThreadFunctionInvocation {
    WTF_MAKE_FAST_ALLOCATED;
public:
    ThreadFunctionInvocation(ThreadFunction function, void* data)
        : function(function)
        , data(data)
    {
    }

    ThreadFunction function;
    void* data;
};

} // namespace WTF

#endif // ThreadFunctionInvocation_h
