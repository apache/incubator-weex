/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
#include "JSDataViewPrototype.h"

#include "Error.h"
#include "JSArrayBuffer.h"
#include "JSDataView.h"
#include "Lookup.h"
#include "JSCInlines.h"
#include "ToNativeFromValue.h"
#include "TypedArrayAdaptors.h"
#include <wtf/FlipBytes.h>

namespace JSC {

/* Source for JSDataViewPrototype.lut.h
@begin dataViewTable
  getInt8               dataViewProtoFuncGetInt8             DontEnum|Function       1
  getUint8              dataViewProtoFuncGetUint8            DontEnum|Function       1
  getInt16              dataViewProtoFuncGetInt16            DontEnum|Function       1
  getUint16             dataViewProtoFuncGetUint16           DontEnum|Function       1
  getInt32              dataViewProtoFuncGetInt32            DontEnum|Function       1
  getUint32             dataViewProtoFuncGetUint32           DontEnum|Function       1
  getFloat32            dataViewProtoFuncGetFloat32          DontEnum|Function       1
  getFloat64            dataViewProtoFuncGetFloat64          DontEnum|Function       1
  setInt8               dataViewProtoFuncSetInt8             DontEnum|Function       2
  setUint8              dataViewProtoFuncSetUint8            DontEnum|Function       2
  setInt16              dataViewProtoFuncSetInt16            DontEnum|Function       2
  setUint16             dataViewProtoFuncSetUint16           DontEnum|Function       2
  setInt32              dataViewProtoFuncSetInt32            DontEnum|Function       2
  setUint32             dataViewProtoFuncSetUint32           DontEnum|Function       2
  setFloat32            dataViewProtoFuncSetFloat32          DontEnum|Function       2
  setFloat64            dataViewProtoFuncSetFloat64          DontEnum|Function       2
  buffer                dataViewProtoGetterBuffer            DontEnum|Accessor       0
  byteLength            dataViewProtoGetterByteLength        DontEnum|Accessor       0
  byteOffset            dataViewProtoGetterByteOffset        DontEnum|Accessor       0
@end
*/

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt8(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt16(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint8(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint16(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetFloat32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetFloat64(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt8(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt16(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint8(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint16(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetFloat32(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetFloat64(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoGetterBuffer(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoGetterByteLength(ExecState*);
EncodedJSValue JSC_HOST_CALL dataViewProtoGetterByteOffset(ExecState*);

}

#include "JSDataViewPrototype.lut.h"

namespace JSC {

const ClassInfo JSDataViewPrototype::s_info = {
    "DataViewPrototype", &Base::s_info, &dataViewTable,
    CREATE_METHOD_TABLE(JSDataViewPrototype)
};

JSDataViewPrototype::JSDataViewPrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

JSDataViewPrototype* JSDataViewPrototype::create(VM& vm, Structure* structure)
{
    JSDataViewPrototype* prototype =
        new (NotNull, allocateCell<JSDataViewPrototype>(vm.heap))
        JSDataViewPrototype(vm, structure);
    prototype->finishCreation(vm);
    return prototype;
}

void JSDataViewPrototype::finishCreation(JSC::VM& vm)
{
    Base::finishCreation(vm);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "DataView"), DontEnum | ReadOnly);
}

Structure* JSDataViewPrototype::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

template<typename Adaptor>
EncodedJSValue getData(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSDataView* dataView = jsDynamicCast<JSDataView*>(vm, exec->thisValue());
    if (!dataView)
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver of DataView method must be a DataView"));
    
    unsigned byteOffset = exec->argument(0).toIndex(exec, "byteOffset");
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    bool littleEndian = false;
    unsigned elementSize = sizeof(typename Adaptor::Type);
    if (elementSize > 1 && exec->argumentCount() >= 2) {
        littleEndian = exec->uncheckedArgument(1).toBoolean(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    
    unsigned byteLength = dataView->length();
    if (elementSize > byteLength || byteOffset > byteLength - elementSize)
        return throwVMError(exec, scope, createRangeError(exec, ASCIILiteral("Out of bounds access")));

    const unsigned dataSize = sizeof(typename Adaptor::Type);
    union {
        typename Adaptor::Type value;
        uint8_t rawBytes[dataSize];
    } u = { };

    uint8_t* dataPtr = static_cast<uint8_t*>(dataView->vector()) + byteOffset;

    if (needToFlipBytesIfLittleEndian(littleEndian)) {
        for (unsigned i = dataSize; i--;)
            u.rawBytes[i] = *dataPtr++;
    } else {
        for (unsigned i = 0; i < dataSize; i++)
            u.rawBytes[i] = *dataPtr++;
    }

    return JSValue::encode(Adaptor::toJSValue(u.value));
}

template<typename Adaptor>
EncodedJSValue setData(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSDataView* dataView = jsDynamicCast<JSDataView*>(vm, exec->thisValue());
    if (!dataView)
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver of DataView method must be a DataView"));
    
    unsigned byteOffset = exec->argument(0).toIndex(exec, "byteOffset");
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    const unsigned dataSize = sizeof(typename Adaptor::Type);
    union {
        typename Adaptor::Type value;
        uint8_t rawBytes[dataSize];
    } u;

    u.value = toNativeFromValue<Adaptor>(exec, exec->argument(1));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    bool littleEndian = false;
    unsigned elementSize = sizeof(typename Adaptor::Type);
    if (elementSize > 1 && exec->argumentCount() >= 3) {
        littleEndian = exec->uncheckedArgument(2).toBoolean(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    
    unsigned byteLength = dataView->length();
    if (elementSize > byteLength || byteOffset > byteLength - elementSize)
        return throwVMError(exec, scope, createRangeError(exec, ASCIILiteral("Out of bounds access")));

    uint8_t* dataPtr = static_cast<uint8_t*>(dataView->vector()) + byteOffset;

    if (needToFlipBytesIfLittleEndian(littleEndian)) {
        for (unsigned i = dataSize; i--;)
            *dataPtr++ = u.rawBytes[i];
    } else {
        for (unsigned i = 0; i < dataSize; i++)
            *dataPtr++ = u.rawBytes[i];
    }

    return JSValue::encode(jsUndefined());
}

#if COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif

EncodedJSValue JSC_HOST_CALL dataViewProtoGetterBuffer(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSDataView* view = jsDynamicCast<JSDataView*>(vm, exec->thisValue());
    if (!view)
        return throwVMTypeError(exec, scope, "DataView.prototype.buffer expects |this| to be a DataView object");

    return JSValue::encode(view->possiblySharedJSBuffer(exec));
}

EncodedJSValue JSC_HOST_CALL dataViewProtoGetterByteLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSDataView* view = jsDynamicCast<JSDataView*>(vm, exec->thisValue());
    if (!view)
        return throwVMTypeError(exec, scope, "DataView.prototype.buffer expects |this| to be a DataView object");

    return JSValue::encode(jsNumber(view->length()));
}

EncodedJSValue JSC_HOST_CALL dataViewProtoGetterByteOffset(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSDataView* view = jsDynamicCast<JSDataView*>(vm, exec->thisValue());
    if (!view)
        return throwVMTypeError(exec, scope, "DataView.prototype.buffer expects |this| to be a DataView object");

    return JSValue::encode(jsNumber(view->byteOffset()));
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt8(ExecState* exec)
{
    return getData<Int8Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt16(ExecState* exec)
{
    return getData<Int16Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetInt32(ExecState* exec)
{
    return getData<Int32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint8(ExecState* exec)
{
    return getData<Uint8Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint16(ExecState* exec)
{
    return getData<Uint16Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetUint32(ExecState* exec)
{
    return getData<Uint32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetFloat32(ExecState* exec)
{
    return getData<Float32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncGetFloat64(ExecState* exec)
{
    return getData<Float64Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt8(ExecState* exec)
{
    return setData<Int8Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt16(ExecState* exec)
{
    return setData<Int16Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetInt32(ExecState* exec)
{
    return setData<Int32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint8(ExecState* exec)
{
    return setData<Uint8Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint16(ExecState* exec)
{
    return setData<Uint16Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetUint32(ExecState* exec)
{
    return setData<Uint32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetFloat32(ExecState* exec)
{
    return setData<Float32Adaptor>(exec);
}

EncodedJSValue JSC_HOST_CALL dataViewProtoFuncSetFloat64(ExecState* exec)
{
    return setData<Float64Adaptor>(exec);
}
#if COMPILER(CLANG)
#pragma clang diagnostic pop
#endif

} // namespace JSC
