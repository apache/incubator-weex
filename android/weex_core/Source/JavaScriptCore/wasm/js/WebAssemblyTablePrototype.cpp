/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "WebAssemblyTablePrototype.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSWebAssemblyHelpers.h"
#include "JSWebAssemblyTable.h"

#include "WebAssemblyTablePrototype.lut.h"

namespace JSC {

const ClassInfo WebAssemblyTablePrototype::s_info = { "WebAssembly.Table.prototype", &Base::s_info, &prototypeTableWebAssemblyTable, CREATE_METHOD_TABLE(WebAssemblyTablePrototype) };

/* Source for WebAssemblyTablePrototype.lut.h
 @begin prototypeTableWebAssemblyTable
 @end
 */

static ALWAYS_INLINE JSWebAssemblyTable* getTable(ExecState* exec, VM& vm, JSValue v)
{
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    JSWebAssemblyTable* result = jsDynamicCast<JSWebAssemblyTable*>(vm, v);
    if (!result) {
        throwException(exec, throwScope, 
            createTypeError(exec, ASCIILiteral("expected |this| value to be an instance of WebAssembly.Table")));
        return nullptr;
    }
    return result;
}

EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncLength(ExecState*);
EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncGrow(ExecState*);
EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncGet(ExecState*);
EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncSet(ExecState*);

EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyTable* table = getTable(exec, vm, exec->thisValue());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
    return JSValue::encode(jsNumber(table->size()));
}

EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncGrow(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyTable* table = getTable(exec, vm, exec->thisValue());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());

    uint32_t index = toNonWrappingUint32(exec, exec->argument(0));
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
    if (!table->grow(index)) {
        throwException(exec, throwScope,
            createTypeError(exec, ASCIILiteral("WebAssembly.Table.prototype.grow could not grow the table")));
        return { };
    }

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncGet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyTable* table = getTable(exec, vm, exec->thisValue());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());

    uint32_t index = toNonWrappingUint32(exec, exec->argument(0));
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
    if (index >= table->size()) {
        throwException(exec, throwScope,
            createRangeError(exec, ASCIILiteral("WebAssembly.Table.prototype.get expects an integer less than the size of the table")));
        return { };
    }

    if (JSObject* result = table->getFunction(index))
        return JSValue::encode(result);
    return JSValue::encode(jsNull());
}

EncodedJSValue JSC_HOST_CALL webAssemblyTableProtoFuncSet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyTable* table = getTable(exec, vm, exec->thisValue());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());

    JSValue value = exec->argument(1);
    WebAssemblyFunction* wasmFunction;
    WebAssemblyWrapperFunction* wasmWrapperFunction;
    if (!value.isNull() && !isWebAssemblyHostFunction(vm, value, wasmFunction, wasmWrapperFunction)) {
        throwException(exec, throwScope,
            createTypeError(exec, ASCIILiteral("WebAssembly.Table.prototype.set expects the second argument to be null or an instance of WebAssembly.Function")));
        return { };
    }

    uint32_t index = toNonWrappingUint32(exec, exec->argument(0));
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());

    if (index >= table->size()) {
        throwException(exec, throwScope,
            createRangeError(exec, ASCIILiteral("WebAssembly.Table.prototype.set expects an integer less than the size of the table")));
        return { };
    }

    if (value.isNull())
        table->clearFunction(index);
    else {
        ASSERT(value.isObject() && isWebAssemblyHostFunction(vm, jsCast<JSObject*>(value), wasmFunction, wasmWrapperFunction));
        ASSERT(!!wasmFunction || !!wasmWrapperFunction);
        if (wasmFunction)
            table->setFunction(vm, index, wasmFunction);
        else
            table->setFunction(vm, index, wasmWrapperFunction);
    }
    
    return JSValue::encode(jsUndefined());
}

WebAssemblyTablePrototype* WebAssemblyTablePrototype::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<WebAssemblyTablePrototype>(vm.heap)) WebAssemblyTablePrototype(vm, structure);
    object->finishCreation(vm, globalObject);
    return object;
}

Structure* WebAssemblyTablePrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyTablePrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);

    JSC_NATIVE_GETTER("length", webAssemblyTableProtoFuncLength, DontEnum | Accessor);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("grow", webAssemblyTableProtoFuncGrow, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("get", webAssemblyTableProtoFuncGet, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("set", webAssemblyTableProtoFuncSet, DontEnum, 2);
}

WebAssemblyTablePrototype::WebAssemblyTablePrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
