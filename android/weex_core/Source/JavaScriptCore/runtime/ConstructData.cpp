/*
 * Copyright (C) 2008, 2016 Apple Inc. All Rights Reserved.
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
#include "ConstructData.h"

#include "Interpreter.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "ScriptProfilingScope.h"

namespace JSC {

JSObject* construct(ExecState* exec, JSValue constructorObject, const ArgList& args, const char* errorMessage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    ConstructData constructData;
    ConstructType constructType = getConstructData(constructorObject, constructData);
    if (constructType == ConstructType::None)
        return throwTypeError(exec, scope, errorMessage);

    scope.release();
    return construct(exec, constructorObject, constructType, constructData, args, constructorObject);
}


JSObject* construct(ExecState* exec, JSValue constructorObject, ConstructType constructType, const ConstructData& constructData, const ArgList& args, JSValue newTarget)
{
    ASSERT(constructType == ConstructType::JS || constructType == ConstructType::Host);
    return exec->interpreter()->executeConstruct(exec, asObject(constructorObject), constructType, constructData, args, newTarget);
}

JSObject* profiledConstruct(ExecState* exec, ProfilingReason reason, JSValue constructorObject, ConstructType constructType, const ConstructData& constructData, const ArgList& args, JSValue newTarget)
{
    ScriptProfilingScope profilingScope(exec->vmEntryGlobalObject(), reason);
    return construct(exec, constructorObject, constructType, constructData, args, newTarget);
}

} // namespace JSC
