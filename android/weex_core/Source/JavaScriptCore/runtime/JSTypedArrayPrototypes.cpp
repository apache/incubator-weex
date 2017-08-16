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
#include "JSTypedArrayPrototypes.h"

#include "JSGenericTypedArrayViewPrototypeInlines.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSTypedArrayViewPrototype::s_info = {"Prototype", &JSTypedArrayViewPrototype::Base::s_info, 0,
    CREATE_METHOD_TABLE(JSTypedArrayViewPrototype)};

#define MAKE_S_INFO(type) \
    template<> const ClassInfo JS##type##Prototype::s_info = {#type "Prototype", &JS##type##Prototype::Base::s_info, 0, CREATE_METHOD_TABLE(JS##type##Prototype)}

MAKE_S_INFO(Int8Array);
MAKE_S_INFO(Int16Array);
MAKE_S_INFO(Int32Array);
MAKE_S_INFO(Uint8Array);
MAKE_S_INFO(Uint8ClampedArray);
MAKE_S_INFO(Uint16Array);
MAKE_S_INFO(Uint32Array);
MAKE_S_INFO(Float32Array);
MAKE_S_INFO(Float64Array);

} // namespace JSC

