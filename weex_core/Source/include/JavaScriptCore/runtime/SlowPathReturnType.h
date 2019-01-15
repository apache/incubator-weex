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
#pragma once

#include <wtf/StdLibExtras.h>

namespace JSC {

#if USE(JSVALUE64)
// According to C++ rules, a type used for the return signature of function with C linkage (i.e.
// 'extern "C"') needs to be POD; hence putting any constructors into it could cause either compiler
// warnings, or worse, a change in the ABI used to return these types.
struct SlowPathReturnType {
    void* a;
    void* b;
};

inline SlowPathReturnType encodeResult(void* a, void* b)
{
    SlowPathReturnType result;
    result.a = a;
    result.b = b;
    return result;
}

inline void decodeResult(SlowPathReturnType result, void*& a, void*& b)
{
    a = result.a;
    b = result.b;
}

#else // USE(JSVALUE32_64)
typedef int64_t SlowPathReturnType;

typedef union {
    struct {
        void* a;
        void* b;
    } pair;
    int64_t i;
} SlowPathReturnTypeEncoding;

inline SlowPathReturnType encodeResult(void* a, void* b)
{
    SlowPathReturnTypeEncoding u;
    u.pair.a = a;
    u.pair.b = b;
    return u.i;
}

inline void decodeResult(SlowPathReturnType result, void*& a, void*& b)
{
    SlowPathReturnTypeEncoding u;
    u.i = result;
    a = u.pair.a;
    b = u.pair.b;
}
#endif // USE(JSVALUE32_64)

} // namespace JSC
