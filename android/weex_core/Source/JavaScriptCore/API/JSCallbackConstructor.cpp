/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
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
#include "JSCallbackConstructor.h"

#include "APICallbackFunction.h"
#include "APICast.h"
#include "Error.h"
#include "JSGlobalObject.h"
#include "JSLock.h"
#include "ObjectPrototype.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSCallbackConstructor::s_info = { "CallbackConstructor", &Base::s_info, 0, CREATE_METHOD_TABLE(JSCallbackConstructor) };

JSCallbackConstructor::JSCallbackConstructor(JSGlobalObject* globalObject, Structure* structure, JSClassRef jsClass, JSObjectCallAsConstructorCallback callback)
    : JSDestructibleObject(globalObject->vm(), structure)
    , m_class(jsClass)
    , m_callback(callback)
{
}

void JSCallbackConstructor::finishCreation(JSGlobalObject* globalObject, JSClassRef jsClass)
{
    Base::finishCreation(globalObject->vm());
    ASSERT(inherits(*vm(), info()));
    if (m_class)
        JSClassRetain(jsClass);
}

JSCallbackConstructor::~JSCallbackConstructor()
{
    if (m_class)
        JSClassRelease(m_class);
}

void JSCallbackConstructor::destroy(JSCell* cell)
{
    static_cast<JSCallbackConstructor*>(cell)->JSCallbackConstructor::~JSCallbackConstructor();
}

ConstructType JSCallbackConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = APICallbackFunction::construct<JSCallbackConstructor>;
    return ConstructType::Host;
}

} // namespace JSC
