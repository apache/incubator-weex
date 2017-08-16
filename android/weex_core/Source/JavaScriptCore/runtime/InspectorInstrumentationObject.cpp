/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "InspectorInstrumentationObject.h"

#include "JSCInlines.h"
#include "Lookup.h"
#include <wtf/DataLog.h>

namespace JSC {

EncodedJSValue JSC_HOST_CALL inspectorInstrumentationObjectLog(ExecState*);

}

#include "InspectorInstrumentationObject.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(InspectorInstrumentationObject);

const ClassInfo InspectorInstrumentationObject::s_info = { "InspectorInstrumentation", &Base::s_info, &inspectorInstrumentationObjectTable, CREATE_METHOD_TABLE(InspectorInstrumentationObject) };

/* Source for InspectorInstrumentationObject.lut.h
@begin inspectorInstrumentationObjectTable
    log               inspectorInstrumentationObjectLog              DontEnum|Function 1
    promiseFulfilled  JSBuiltin                                      DontEnum|Function 3
    promiseRejected   JSBuiltin                                      DontEnum|Function 3
@end
*/

InspectorInstrumentationObject::InspectorInstrumentationObject(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void InspectorInstrumentationObject::finishCreation(VM& vm, JSGlobalObject*)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    putDirectWithoutTransition(vm, vm.propertyNames->isEnabled, jsBoolean(false));
}

bool InspectorInstrumentationObject::isEnabled(VM& vm) const
{
    return getDirect(vm, vm.propertyNames->isEnabled).asBoolean();
}

void InspectorInstrumentationObject::enable(VM& vm)
{
    putDirect(vm, vm.propertyNames->isEnabled, jsBoolean(true));
}

void InspectorInstrumentationObject::disable(VM& vm)
{
    putDirect(vm, vm.propertyNames->isEnabled, jsBoolean(false));
}

// ------------------------------ Functions --------------------------------

EncodedJSValue JSC_HOST_CALL inspectorInstrumentationObjectLog(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue target = exec->argument(0);
    String value = target.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    dataLog(value, "\n");
    return JSValue::encode(jsUndefined());
}

} // namespace JSC
