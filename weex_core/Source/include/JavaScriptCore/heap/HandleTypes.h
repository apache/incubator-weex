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

#include "JSCJSValue.h"

namespace JSC {

typedef enum { } Unknown;
typedef JSValue* HandleSlot;

template<typename T> struct HandleTypes {
    typedef T* ExternalType;
    static ExternalType getFromSlot(HandleSlot slot) { return (slot && *slot) ? reinterpret_cast<ExternalType>(static_cast<void*>(slot->asCell())) : 0; }
    static JSValue toJSValue(T* cell) { return reinterpret_cast<JSCell*>(cell); }
    template<typename U> static void validateUpcast() { T* temp; temp = (U*)0; }
};

template<> struct HandleTypes<Unknown> {
    typedef JSValue ExternalType;
    static ExternalType getFromSlot(HandleSlot slot) { return slot ? *slot : JSValue(); }
    static JSValue toJSValue(const JSValue& v) { return v; }
    template<typename U> static void validateUpcast() { }
};

} // namespace JSC
