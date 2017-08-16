/*
 * Copyright (C) 2006-2017 Apple Inc.  All rights reserved.
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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
#include "JSCallbackObject.h"

#include "Heap.h"
#include "JSCInlines.h"
#include <wtf/text/StringHash.h>

namespace JSC {

// Define the two types of JSCallbackObjects we support.
template <> const ClassInfo JSCallbackObject<JSDestructibleObject>::s_info = { "CallbackObject", &Base::s_info, 0, CREATE_METHOD_TABLE(JSCallbackObject) };
template <> const ClassInfo JSCallbackObject<JSGlobalObject>::s_info = { "CallbackGlobalObject", &Base::s_info, 0, CREATE_METHOD_TABLE(JSCallbackObject) };

template<> const bool JSCallbackObject<JSDestructibleObject>::needsDestruction = true;
template<> const bool JSCallbackObject<JSGlobalObject>::needsDestruction = false;

template<>
JSCallbackObject<JSGlobalObject>* JSCallbackObject<JSGlobalObject>::create(VM& vm, JSClassRef classRef, Structure* structure)
{
    JSCallbackObject<JSGlobalObject>* callbackObject = new (NotNull, allocateCell<JSCallbackObject<JSGlobalObject>>(vm.heap)) JSCallbackObject(vm, classRef, structure);
    callbackObject->finishCreation(vm);
    return callbackObject;
}

template <>
Structure* JSCallbackObject<JSDestructibleObject>::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
{ 
    return Structure::create(vm, globalObject, proto, TypeInfo(ObjectType, StructureFlags), info()); 
}
    
template <>
Structure* JSCallbackObject<JSGlobalObject>::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
{ 
    return Structure::create(vm, globalObject, proto, TypeInfo(GlobalObjectType, StructureFlags), info()); 
}

} // namespace JSC
