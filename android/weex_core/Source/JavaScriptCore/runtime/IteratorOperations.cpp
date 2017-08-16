/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "IteratorOperations.h"

#include "Error.h"
#include "JSCInlines.h"
#include "ObjectConstructor.h"

using namespace WTF;

namespace JSC {

JSValue iteratorNext(ExecState* exec, JSValue iterator, JSValue value)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue nextFunction = iterator.get(exec, vm.propertyNames->next);
    RETURN_IF_EXCEPTION(scope, JSValue());

    CallData nextFunctionCallData;
    CallType nextFunctionCallType = getCallData(nextFunction, nextFunctionCallData);
    if (nextFunctionCallType == CallType::None)
        return throwTypeError(exec, scope);

    MarkedArgumentBuffer nextFunctionArguments;
    if (!value.isEmpty())
        nextFunctionArguments.append(value);
    JSValue result = call(exec, nextFunction, nextFunctionCallType, nextFunctionCallData, iterator, nextFunctionArguments);
    RETURN_IF_EXCEPTION(scope, JSValue());

    if (!result.isObject())
        return throwTypeError(exec, scope, ASCIILiteral("Iterator result interface is not an object."));

    return result;
}

JSValue iteratorNext(ExecState* exec, JSValue iterator)
{
    return iteratorNext(exec, iterator, JSValue());
}

JSValue iteratorValue(ExecState* exec, JSValue iterResult)
{
    return iterResult.get(exec, exec->vm().propertyNames->value);
}

bool iteratorComplete(ExecState* exec, JSValue iterResult)
{
    JSValue done = iterResult.get(exec, exec->vm().propertyNames->done);
    return done.toBoolean(exec);
}

JSValue iteratorStep(ExecState* exec, JSValue iterator)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue result = iteratorNext(exec, iterator);
    RETURN_IF_EXCEPTION(scope, JSValue());
    bool done = iteratorComplete(exec, result);
    RETURN_IF_EXCEPTION(scope, JSValue());
    if (done)
        return jsBoolean(false);
    return result;
}

void iteratorClose(ExecState* exec, JSValue iterator)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    Exception* exception = nullptr;
    if (UNLIKELY(catchScope.exception())) {
        exception = catchScope.exception();
        catchScope.clearException();
    }
    JSValue returnFunction = iterator.get(exec, vm.propertyNames->returnKeyword);
    RETURN_IF_EXCEPTION(throwScope, void());

    if (returnFunction.isUndefined()) {
        if (exception)
            throwException(exec, throwScope, exception);
        return;
    }

    CallData returnFunctionCallData;
    CallType returnFunctionCallType = getCallData(returnFunction, returnFunctionCallData);
    if (returnFunctionCallType == CallType::None) {
        if (exception)
            throwException(exec, throwScope, exception);
        else
            throwTypeError(exec, throwScope);
        return;
    }

    MarkedArgumentBuffer returnFunctionArguments;
    JSValue innerResult = call(exec, returnFunction, returnFunctionCallType, returnFunctionCallData, iterator, returnFunctionArguments);

    if (exception) {
        throwException(exec, throwScope, exception);
        return;
    }

    RETURN_IF_EXCEPTION(throwScope, void());

    if (!innerResult.isObject()) {
        throwTypeError(exec, throwScope, ASCIILiteral("Iterator result interface is not an object."));
        return;
    }
}

static const PropertyOffset donePropertyOffset = 0;
static const PropertyOffset valuePropertyOffset = 1;

Structure* createIteratorResultObjectStructure(VM& vm, JSGlobalObject& globalObject)
{
    Structure* iteratorResultStructure = vm.prototypeMap.emptyObjectStructureForPrototype(&globalObject, globalObject.objectPrototype(), JSFinalObject::defaultInlineCapacity());
    PropertyOffset offset;
    iteratorResultStructure = Structure::addPropertyTransition(vm, iteratorResultStructure, vm.propertyNames->done, 0, offset);
    RELEASE_ASSERT(offset == donePropertyOffset);
    iteratorResultStructure = Structure::addPropertyTransition(vm, iteratorResultStructure, vm.propertyNames->value, 0, offset);
    RELEASE_ASSERT(offset == valuePropertyOffset);
    return iteratorResultStructure;
}

JSObject* createIteratorResultObject(ExecState* exec, JSValue value, bool done)
{
    VM& vm = exec->vm();
    JSObject* resultObject = constructEmptyObject(exec, exec->lexicalGlobalObject()->iteratorResultObjectStructure());
    resultObject->putDirect(vm, donePropertyOffset, jsBoolean(done));
    resultObject->putDirect(vm, valuePropertyOffset, value);
    return resultObject;
}

bool hasIteratorMethod(ExecState& state, JSValue value)
{
    auto& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!value.isObject())
        return false;

    JSObject* object = asObject(value);
    CallData callData;
    CallType callType;
    JSValue applyMethod = object->getMethod(&state, callData, callType, vm.propertyNames->iteratorSymbol, ASCIILiteral("Symbol.iterator property should be callable"));
    RETURN_IF_EXCEPTION(scope, false);

    return !applyMethod.isUndefined();
}

JSValue iteratorForIterable(ExecState* state, JSValue iterable)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    JSValue iteratorFunction = iterable.get(state, vm.propertyNames->iteratorSymbol);
    RETURN_IF_EXCEPTION(scope, JSValue());
    
    CallData iteratorFunctionCallData;
    CallType iteratorFunctionCallType = getCallData(iteratorFunction, iteratorFunctionCallData);
    if (iteratorFunctionCallType == CallType::None) {
        throwTypeError(state, scope);
        return JSValue();
    }

    ArgList iteratorFunctionArguments;
    JSValue iterator = call(state, iteratorFunction, iteratorFunctionCallType, iteratorFunctionCallData, iterable, iteratorFunctionArguments);
    RETURN_IF_EXCEPTION(scope, JSValue());

    if (!iterator.isObject()) {
        throwTypeError(state, scope);
        return JSValue();
    }

    return iterator;
}

} // namespace JSC
