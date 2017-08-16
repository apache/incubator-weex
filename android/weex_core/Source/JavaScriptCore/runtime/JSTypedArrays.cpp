/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSTypedArrays.h"

#include "GenericTypedArrayViewInlines.h"
#include "JSGenericTypedArrayViewInlines.h"
#include "JSCInlines.h"

namespace JSC {

#define MAKE_S_INFO(type) \
    template<> const ClassInfo JS##type##Array::s_info = { \
        #type "Array", &JS##type##Array::Base::s_info, 0, \
        CREATE_METHOD_TABLE(JS##type##Array) \
    }; \
    const ClassInfo* get##type##ArrayClassInfo() { return &JS##type##Array::s_info; }

MAKE_S_INFO(Int8);
MAKE_S_INFO(Int16);
MAKE_S_INFO(Int32);
MAKE_S_INFO(Uint8);
MAKE_S_INFO(Uint8Clamped);
MAKE_S_INFO(Uint16);
MAKE_S_INFO(Uint32);
MAKE_S_INFO(Float32);
MAKE_S_INFO(Float64);

JSUint8Array* createUint8TypedArray(ExecState* exec, Structure* structure, RefPtr<ArrayBuffer>&& buffer, unsigned byteOffset, unsigned length)
{
    return JSUint8Array::create(exec, structure, WTFMove(buffer), byteOffset, length);
}


} // namespace JSC

