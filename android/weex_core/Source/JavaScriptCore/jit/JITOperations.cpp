/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
#include "JITOperations.h"

#if ENABLE(JIT)

#include "ArithProfile.h"
#include "ArrayConstructor.h"
#include "CommonSlowPaths.h"
#include "DFGCompilationMode.h"
#include "DFGDriver.h"
#include "DFGOSREntry.h"
#include "DFGThunks.h"
#include "DFGWorklist.h"
#include "Debugger.h"
#include "DirectArguments.h"
#include "Error.h"
#include "ErrorHandlingScope.h"
#include "EvalCodeBlock.h"
#include "ExceptionFuzz.h"
#include "FunctionCodeBlock.h"
#include "GetterSetter.h"
#include "HostCallReturnValue.h"
#include "ICStats.h"
#include "Interpreter.h"
#include "JIT.h"
#include "JITExceptions.h"
#include "JITToDFGDeferredCompilationCallback.h"
#include "JSAsyncFunction.h"
#include "JSCInlines.h"
#include "JSGeneratorFunction.h"
#include "JSGlobalObjectFunctions.h"
#include "JSLexicalEnvironment.h"
#include "JSPropertyNameEnumerator.h"
#include "ModuleProgramCodeBlock.h"
#include "ObjectConstructor.h"
#include "PolymorphicAccess.h"
#include "ProgramCodeBlock.h"
#include "PropertyName.h"
#include "RegExpObject.h"
#include "Repatch.h"
#include "ScopedArguments.h"
#include "ShadowChicken.h"
#include "StructureStubInfo.h"
#include "SuperSampler.h"
#include "TestRunnerUtils.h"
#include "TypeProfilerLog.h"
#include "VMInlines.h"
#include <wtf/InlineASM.h>

namespace JSC {

extern "C" {

#if COMPILER(MSVC)
void * _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

#define OUR_RETURN_ADDRESS _ReturnAddress()
#else
#define OUR_RETURN_ADDRESS __builtin_return_address(0)
#endif

#if ENABLE(OPCODE_SAMPLING)
#define CTI_SAMPLER vm->interpreter->sampler()
#else
#define CTI_SAMPLER 0
#endif


void JIT_OPERATION operationThrowStackOverflowError(ExecState* exec, CodeBlock* codeBlock)
{
    // We pass in our own code block, because the callframe hasn't been populated.
    VM* vm = codeBlock->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    VMEntryFrame* vmEntryFrame = vm->topVMEntryFrame;
    CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);
    if (!callerFrame) {
        callerFrame = exec;
        vmEntryFrame = vm->topVMEntryFrame;
    }

    NativeCallFrameTracerWithRestore tracer(vm, vmEntryFrame, callerFrame);
    throwStackOverflowError(callerFrame, scope);
}

#if ENABLE(WEBASSEMBLY)
void JIT_OPERATION operationThrowDivideError(ExecState* exec)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    VMEntryFrame* vmEntryFrame = vm->topVMEntryFrame;
    CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);

    NativeCallFrameTracerWithRestore tracer(vm, vmEntryFrame, callerFrame);
    ErrorHandlingScope errorScope(*vm);
    throwException(callerFrame, scope, createError(callerFrame, ASCIILiteral("Division by zero or division overflow.")));
}

void JIT_OPERATION operationThrowOutOfBoundsAccessError(ExecState* exec)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    VMEntryFrame* vmEntryFrame = vm->topVMEntryFrame;
    CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);

    NativeCallFrameTracerWithRestore tracer(vm, vmEntryFrame, callerFrame);
    ErrorHandlingScope errorScope(*vm);
    throwException(callerFrame, scope, createError(callerFrame, ASCIILiteral("Out-of-bounds access.")));
}
#endif

int32_t JIT_OPERATION operationCallArityCheck(ExecState* exec)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    int32_t missingArgCount = CommonSlowPaths::arityCheckFor(exec, *vm, CodeForCall);
    if (missingArgCount < 0) {
        VMEntryFrame* vmEntryFrame = vm->topVMEntryFrame;
        CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);
        NativeCallFrameTracerWithRestore tracer(vm, vmEntryFrame, callerFrame);
        throwStackOverflowError(callerFrame, scope);
    }

    return missingArgCount;
}

int32_t JIT_OPERATION operationConstructArityCheck(ExecState* exec)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    int32_t missingArgCount = CommonSlowPaths::arityCheckFor(exec, *vm, CodeForConstruct);
    if (missingArgCount < 0) {
        VMEntryFrame* vmEntryFrame = vm->topVMEntryFrame;
        CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);
        NativeCallFrameTracerWithRestore tracer(vm, vmEntryFrame, callerFrame);
        throwStackOverflowError(callerFrame, scope);
    }

    return missingArgCount;
}

EncodedJSValue JIT_OPERATION operationTryGetById(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, UniquedStringImpl* uid)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    Identifier ident = Identifier::fromUid(vm, uid);
    stubInfo->tookSlowPath = true;

    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::VMInquiry);
    baseValue.getPropertySlot(exec, ident, slot);

    return JSValue::encode(slot.getPureResult());
}


EncodedJSValue JIT_OPERATION operationTryGetByIdGeneric(ExecState* exec, EncodedJSValue base, UniquedStringImpl* uid)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    Identifier ident = Identifier::fromUid(vm, uid);

    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::VMInquiry);
    baseValue.getPropertySlot(exec, ident, slot);

    return JSValue::encode(slot.getPureResult());
}

EncodedJSValue JIT_OPERATION operationTryGetByIdOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, UniquedStringImpl* uid)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);
    Identifier ident = Identifier::fromUid(vm, uid);

    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::VMInquiry);

    baseValue.getPropertySlot(exec, ident, slot);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (stubInfo->considerCaching(exec->codeBlock(), baseValue.structureOrNull()) && !slot.isTaintedByOpaqueObject() && (slot.isCacheableValue() || slot.isCacheableGetter() || slot.isUnset()))
        repatchGetByID(exec, baseValue, ident, slot, *stubInfo, GetByIDKind::Try);

    return JSValue::encode(slot.getPureResult());
}

EncodedJSValue JIT_OPERATION operationGetById(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    stubInfo->tookSlowPath = true;
    
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::Get);
    Identifier ident = Identifier::fromUid(vm, uid);
    
    LOG_IC((ICEvent::OperationGetById, baseValue.classInfoOrNull(*vm), ident));
    return JSValue::encode(baseValue.get(exec, ident, slot));
}

EncodedJSValue JIT_OPERATION operationGetByIdGeneric(ExecState* exec, EncodedJSValue base, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue baseValue = JSValue::decode(base);
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::Get);
    Identifier ident = Identifier::fromUid(vm, uid);
    LOG_IC((ICEvent::OperationGetByIdGeneric, baseValue.classInfoOrNull(*vm), ident));
    return JSValue::encode(baseValue.get(exec, ident, slot));
}

EncodedJSValue JIT_OPERATION operationGetByIdOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    Identifier ident = Identifier::fromUid(vm, uid);

    JSValue baseValue = JSValue::decode(base);
    LOG_IC((ICEvent::OperationGetByIdOptimize, baseValue.classInfoOrNull(*vm), ident));

    return JSValue::encode(baseValue.getPropertySlot(exec, ident, [&] (bool found, PropertySlot& slot) -> JSValue {
        if (stubInfo->considerCaching(exec->codeBlock(), baseValue.structureOrNull()))
            repatchGetByID(exec, baseValue, ident, slot, *stubInfo, GetByIDKind::Normal);
        return found ? slot.getValue(exec, ident) : jsUndefined();
    }));
}

EncodedJSValue JIT_OPERATION operationGetByIdWithThisGeneric(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, EncodedJSValue thisEncoded, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);

    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    Identifier ident = Identifier::fromUid(vm, uid);

    stubInfo->tookSlowPath = true;

    JSValue baseValue = JSValue::decode(base);
    JSValue thisValue = JSValue::decode(thisEncoded);
    PropertySlot slot(thisValue, PropertySlot::InternalMethodType::Get);

    return JSValue::encode(baseValue.get(exec, ident, slot));
}

EncodedJSValue JIT_OPERATION operationGetByIdWithThisOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue base, EncodedJSValue thisEncoded, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    Identifier ident = Identifier::fromUid(vm, uid);

    JSValue baseValue = JSValue::decode(base);
    JSValue thisValue = JSValue::decode(thisEncoded);
    LOG_IC((ICEvent::OperationGetByIdWithThisOptimize, baseValue.classInfoOrNull(*vm), ident));

    PropertySlot slot(thisValue, PropertySlot::InternalMethodType::Get);
    return JSValue::encode(baseValue.getPropertySlot(exec, ident, slot, [&] (bool found, PropertySlot& slot) -> JSValue {
        if (stubInfo->considerCaching(exec->codeBlock(), baseValue.structureOrNull()))
            repatchGetByID(exec, baseValue, ident, slot, *stubInfo, GetByIDKind::WithThis);
        return found ? slot.getValue(exec, ident) : jsUndefined();
    }));
}

EncodedJSValue JIT_OPERATION operationInOptimize(ExecState* exec, StructureStubInfo* stubInfo, JSCell* base, UniquedStringImpl* key)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    if (!base->isObject()) {
        throwException(exec, scope, createInvalidInParameterError(exec, base));
        return JSValue::encode(jsUndefined());
    }
    
    AccessType accessType = static_cast<AccessType>(stubInfo->accessType);

    Identifier ident = Identifier::fromUid(vm, key);
    LOG_IC((ICEvent::OperationInOptimize, base->classInfo(*vm), ident));
    PropertySlot slot(base, PropertySlot::InternalMethodType::HasProperty);
    bool result = asObject(base)->getPropertySlot(exec, ident, slot);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    RELEASE_ASSERT(accessType == stubInfo->accessType);
    
    if (stubInfo->considerCaching(exec->codeBlock(), asObject(base)->structure()))
        repatchIn(exec, base, ident, result, slot, *stubInfo);
    
    return JSValue::encode(jsBoolean(result));
}

EncodedJSValue JIT_OPERATION operationIn(ExecState* exec, StructureStubInfo* stubInfo, JSCell* base, UniquedStringImpl* key)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    stubInfo->tookSlowPath = true;

    if (!base->isObject()) {
        throwException(exec, scope, createInvalidInParameterError(exec, base));
        return JSValue::encode(jsUndefined());
    }

    Identifier ident = Identifier::fromUid(vm, key);
    LOG_IC((ICEvent::OperationIn, base->classInfo(*vm), ident));
    return JSValue::encode(jsBoolean(asObject(base)->hasProperty(exec, ident)));
}

EncodedJSValue JIT_OPERATION operationGenericIn(ExecState* exec, JSCell* base, EncodedJSValue key)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return JSValue::encode(jsBoolean(CommonSlowPaths::opIn(exec, base, JSValue::decode(key))));
}

void JIT_OPERATION operationPutByIdStrict(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    stubInfo->tookSlowPath = true;
    
    JSValue baseValue = JSValue::decode(encodedBase);
    Identifier ident = Identifier::fromUid(vm, uid);
    LOG_IC((ICEvent::OperationPutByIdStrict, baseValue.classInfoOrNull(*vm), ident));

    PutPropertySlot slot(baseValue, true, exec->codeBlock()->putByIdContext());
    baseValue.putInline(exec, ident, JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByIdNonStrict(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    stubInfo->tookSlowPath = true;
    
    JSValue baseValue = JSValue::decode(encodedBase);
    Identifier ident = Identifier::fromUid(vm, uid);
    LOG_IC((ICEvent::OperationPutByIdNonStrict, baseValue.classInfoOrNull(*vm), ident));
    PutPropertySlot slot(baseValue, false, exec->codeBlock()->putByIdContext());
    baseValue.putInline(exec, ident, JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByIdDirectStrict(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    stubInfo->tookSlowPath = true;
    
    JSValue baseValue = JSValue::decode(encodedBase);
    Identifier ident = Identifier::fromUid(vm, uid);
    LOG_IC((ICEvent::OperationPutByIdDirectStrict, baseValue.classInfoOrNull(*vm), ident));
    PutPropertySlot slot(baseValue, true, exec->codeBlock()->putByIdContext());
    asObject(baseValue)->putDirect(exec->vm(), ident, JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByIdDirectNonStrict(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    stubInfo->tookSlowPath = true;
    
    JSValue baseValue = JSValue::decode(encodedBase);
    Identifier ident = Identifier::fromUid(vm, uid);
    LOG_IC((ICEvent::OperationPutByIdDirectNonStrict, baseValue.classInfoOrNull(*vm), ident));
    PutPropertySlot slot(baseValue, false, exec->codeBlock()->putByIdContext());
    asObject(baseValue)->putDirect(exec->vm(), ident, JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByIdStrictOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    Identifier ident = Identifier::fromUid(vm, uid);
    AccessType accessType = static_cast<AccessType>(stubInfo->accessType);

    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue = JSValue::decode(encodedBase);
    LOG_IC((ICEvent::OperationPutByIdStrictOptimize, baseValue.classInfoOrNull(*vm), ident));
    CodeBlock* codeBlock = exec->codeBlock();
    PutPropertySlot slot(baseValue, true, codeBlock->putByIdContext());

    Structure* structure = baseValue.isCell() ? baseValue.asCell()->structure(*vm) : nullptr;
    baseValue.putInline(exec, ident, value, slot);
    RETURN_IF_EXCEPTION(scope, void());

    if (accessType != static_cast<AccessType>(stubInfo->accessType))
        return;
    
    if (stubInfo->considerCaching(codeBlock, structure))
        repatchPutByID(exec, baseValue, structure, ident, slot, *stubInfo, NotDirect);
}

void JIT_OPERATION operationPutByIdNonStrictOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    Identifier ident = Identifier::fromUid(vm, uid);
    AccessType accessType = static_cast<AccessType>(stubInfo->accessType);

    JSValue value = JSValue::decode(encodedValue);
    JSValue baseValue = JSValue::decode(encodedBase);
    LOG_IC((ICEvent::OperationPutByIdNonStrictOptimize, baseValue.classInfoOrNull(*vm), ident));
    CodeBlock* codeBlock = exec->codeBlock();
    PutPropertySlot slot(baseValue, false, codeBlock->putByIdContext());

    Structure* structure = baseValue.isCell() ? baseValue.asCell()->structure(*vm) : nullptr;    
    baseValue.putInline(exec, ident, value, slot);
    RETURN_IF_EXCEPTION(scope, void());

    if (accessType != static_cast<AccessType>(stubInfo->accessType))
        return;
    
    if (stubInfo->considerCaching(codeBlock, structure))
        repatchPutByID(exec, baseValue, structure, ident, slot, *stubInfo, NotDirect);
}

void JIT_OPERATION operationPutByIdDirectStrictOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    Identifier ident = Identifier::fromUid(vm, uid);
    AccessType accessType = static_cast<AccessType>(stubInfo->accessType);

    JSValue value = JSValue::decode(encodedValue);
    JSObject* baseObject = asObject(JSValue::decode(encodedBase));
    LOG_IC((ICEvent::OperationPutByIdDirectStrictOptimize, baseObject->classInfo(*vm), ident));
    CodeBlock* codeBlock = exec->codeBlock();
    PutPropertySlot slot(baseObject, true, codeBlock->putByIdContext());
    
    Structure* structure = baseObject->structure(*vm);
    baseObject->putDirect(exec->vm(), ident, value, slot);
    
    if (accessType != static_cast<AccessType>(stubInfo->accessType))
        return;
    
    if (stubInfo->considerCaching(codeBlock, structure))
        repatchPutByID(exec, baseObject, structure, ident, slot, *stubInfo, Direct);
}

void JIT_OPERATION operationPutByIdDirectNonStrictOptimize(ExecState* exec, StructureStubInfo* stubInfo, EncodedJSValue encodedValue, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    Identifier ident = Identifier::fromUid(vm, uid);
    AccessType accessType = static_cast<AccessType>(stubInfo->accessType);

    JSValue value = JSValue::decode(encodedValue);
    JSObject* baseObject = asObject(JSValue::decode(encodedBase));
    LOG_IC((ICEvent::OperationPutByIdDirectNonStrictOptimize, baseObject->classInfo(*vm), ident));
    CodeBlock* codeBlock = exec->codeBlock();
    PutPropertySlot slot(baseObject, false, codeBlock->putByIdContext());
    
    Structure* structure = baseObject->structure(*vm);
    baseObject->putDirect(exec->vm(), ident, value, slot);
    
    if (accessType != static_cast<AccessType>(stubInfo->accessType))
        return;
    
    if (stubInfo->considerCaching(codeBlock, structure))
        repatchPutByID(exec, baseObject, structure, ident, slot, *stubInfo, Direct);
}

ALWAYS_INLINE static bool isStringOrSymbol(JSValue value)
{
    return value.isString() || value.isSymbol();
}

static void putByVal(CallFrame* callFrame, JSValue baseValue, JSValue subscript, JSValue value, ByValInfo* byValInfo)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (LIKELY(subscript.isUInt32())) {
        byValInfo->tookSlowPath = true;
        uint32_t i = subscript.asUInt32();
        if (baseValue.isObject()) {
            JSObject* object = asObject(baseValue);
            if (object->canSetIndexQuickly(i))
                object->setIndexQuickly(callFrame->vm(), i, value);
            else {
                // FIXME: This will make us think that in-bounds typed array accesses are actually
                // out-of-bounds.
                // https://bugs.webkit.org/show_bug.cgi?id=149886
                byValInfo->arrayProfile->setOutOfBounds();
                object->methodTable(vm)->putByIndex(object, callFrame, i, value, callFrame->codeBlock()->isStrictMode());
            }
        } else
            baseValue.putByIndex(callFrame, i, value, callFrame->codeBlock()->isStrictMode());
        return;
    }

    auto property = subscript.toPropertyKey(callFrame);
    // Don't put to an object if toString threw an exception.
    RETURN_IF_EXCEPTION(scope, void());

    if (byValInfo->stubInfo && (!isStringOrSymbol(subscript) || byValInfo->cachedId != property))
        byValInfo->tookSlowPath = true;

    scope.release();
    PutPropertySlot slot(baseValue, callFrame->codeBlock()->isStrictMode());
    baseValue.putInline(callFrame, property, value, slot);
}

static void directPutByVal(CallFrame* callFrame, JSObject* baseObject, JSValue subscript, JSValue value, ByValInfo* byValInfo)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    bool isStrictMode = callFrame->codeBlock()->isStrictMode();
    if (LIKELY(subscript.isUInt32())) {
        // Despite its name, JSValue::isUInt32 will return true only for positive boxed int32_t; all those values are valid array indices.
        byValInfo->tookSlowPath = true;
        uint32_t index = subscript.asUInt32();
        ASSERT(isIndex(index));
        if (baseObject->canSetIndexQuicklyForPutDirect(index)) {
            baseObject->setIndexQuickly(callFrame->vm(), index, value);
            return;
        }

        // FIXME: This will make us think that in-bounds typed array accesses are actually
        // out-of-bounds.
        // https://bugs.webkit.org/show_bug.cgi?id=149886
        byValInfo->arrayProfile->setOutOfBounds();
        baseObject->putDirectIndex(callFrame, index, value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
        return;
    }

    if (subscript.isDouble()) {
        double subscriptAsDouble = subscript.asDouble();
        uint32_t subscriptAsUInt32 = static_cast<uint32_t>(subscriptAsDouble);
        if (subscriptAsDouble == subscriptAsUInt32 && isIndex(subscriptAsUInt32)) {
            byValInfo->tookSlowPath = true;
            baseObject->putDirectIndex(callFrame, subscriptAsUInt32, value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
            return;
        }
    }

    // Don't put to an object if toString threw an exception.
    auto property = subscript.toPropertyKey(callFrame);
    RETURN_IF_EXCEPTION(scope, void());

    if (std::optional<uint32_t> index = parseIndex(property)) {
        byValInfo->tookSlowPath = true;
        baseObject->putDirectIndex(callFrame, index.value(), value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
        return;
    }

    if (byValInfo->stubInfo && (!isStringOrSymbol(subscript) || byValInfo->cachedId != property))
        byValInfo->tookSlowPath = true;

    PutPropertySlot slot(baseObject, isStrictMode);
    baseObject->putDirect(callFrame->vm(), property, value, slot);
}

enum class OptimizationResult {
    NotOptimized,
    SeenOnce,
    Optimized,
    GiveUp,
};

static OptimizationResult tryPutByValOptimize(ExecState* exec, JSValue baseValue, JSValue subscript, ByValInfo* byValInfo, ReturnAddressPtr returnAddress)
{
    // See if it's worth optimizing at all.
    OptimizationResult optimizationResult = OptimizationResult::NotOptimized;

    VM& vm = exec->vm();

    if (baseValue.isObject() && subscript.isInt32()) {
        JSObject* object = asObject(baseValue);

        ASSERT(exec->bytecodeOffset());
        ASSERT(!byValInfo->stubRoutine);

        Structure* structure = object->structure(vm);
        if (hasOptimizableIndexing(structure)) {
            // Attempt to optimize.
            JITArrayMode arrayMode = jitArrayModeForStructure(structure);
            if (jitArrayModePermitsPut(arrayMode) && arrayMode != byValInfo->arrayMode) {
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->arrayProfile->computeUpdatedPrediction(locker, codeBlock, structure);

                JIT::compilePutByVal(&vm, codeBlock, byValInfo, returnAddress, arrayMode);
                optimizationResult = OptimizationResult::Optimized;
            }
        }

        // If we failed to patch and we have some object that intercepts indexed get, then don't even wait until 10 times.
        if (optimizationResult != OptimizationResult::Optimized && object->structure(vm)->typeInfo().interceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero())
            optimizationResult = OptimizationResult::GiveUp;
    }

    if (baseValue.isObject() && isStringOrSymbol(subscript)) {
        const Identifier propertyName = subscript.toPropertyKey(exec);
        if (subscript.isSymbol() || !parseIndex(propertyName)) {
            ASSERT(exec->bytecodeOffset());
            ASSERT(!byValInfo->stubRoutine);
            if (byValInfo->seen) {
                if (byValInfo->cachedId == propertyName) {
                    JIT::compilePutByValWithCachedId(&vm, exec->codeBlock(), byValInfo, returnAddress, NotDirect, propertyName);
                    optimizationResult = OptimizationResult::Optimized;
                } else {
                    // Seem like a generic property access site.
                    optimizationResult = OptimizationResult::GiveUp;
                }
            } else {
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->seen = true;
                byValInfo->cachedId = propertyName;
                if (subscript.isSymbol())
                    byValInfo->cachedSymbol.set(vm, codeBlock, asSymbol(subscript));
                optimizationResult = OptimizationResult::SeenOnce;
            }
        }
    }

    if (optimizationResult != OptimizationResult::Optimized && optimizationResult != OptimizationResult::SeenOnce) {
        // If we take slow path more than 10 times without patching then make sure we
        // never make that mistake again. For cases where we see non-index-intercepting
        // objects, this gives 10 iterations worth of opportunity for us to observe
        // that the put_by_val may be polymorphic. We count up slowPathCount even if
        // the result is GiveUp.
        if (++byValInfo->slowPathCount >= 10)
            optimizationResult = OptimizationResult::GiveUp;
    }

    return optimizationResult;
}

void JIT_OPERATION operationPutByValOptimize(ExecState* exec, EncodedJSValue encodedBaseValue, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue baseValue = JSValue::decode(encodedBaseValue);
    JSValue subscript = JSValue::decode(encodedSubscript);
    JSValue value = JSValue::decode(encodedValue);
    if (tryPutByValOptimize(exec, baseValue, subscript, byValInfo, ReturnAddressPtr(OUR_RETURN_ADDRESS)) == OptimizationResult::GiveUp) {
        // Don't ever try to optimize.
        byValInfo->tookSlowPath = true;
        ctiPatchCallByReturnAddress(ReturnAddressPtr(OUR_RETURN_ADDRESS), FunctionPtr(operationPutByValGeneric));
    }
    putByVal(exec, baseValue, subscript, value, byValInfo);
}

static OptimizationResult tryDirectPutByValOptimize(ExecState* exec, JSObject* object, JSValue subscript, ByValInfo* byValInfo, ReturnAddressPtr returnAddress)
{
    // See if it's worth optimizing at all.
    OptimizationResult optimizationResult = OptimizationResult::NotOptimized;

    VM& vm = exec->vm();

    if (subscript.isInt32()) {
        ASSERT(exec->bytecodeOffset());
        ASSERT(!byValInfo->stubRoutine);

        Structure* structure = object->structure(vm);
        if (hasOptimizableIndexing(structure)) {
            // Attempt to optimize.
            JITArrayMode arrayMode = jitArrayModeForStructure(structure);
            if (jitArrayModePermitsPut(arrayMode) && arrayMode != byValInfo->arrayMode) {
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->arrayProfile->computeUpdatedPrediction(locker, codeBlock, structure);

                JIT::compileDirectPutByVal(&vm, codeBlock, byValInfo, returnAddress, arrayMode);
                optimizationResult = OptimizationResult::Optimized;
            }
        }

        // If we failed to patch and we have some object that intercepts indexed get, then don't even wait until 10 times.
        if (optimizationResult != OptimizationResult::Optimized && object->structure(vm)->typeInfo().interceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero())
            optimizationResult = OptimizationResult::GiveUp;
    } else if (isStringOrSymbol(subscript)) {
        const Identifier propertyName = subscript.toPropertyKey(exec);
        if (subscript.isSymbol() || !parseIndex(propertyName)) {
            ASSERT(exec->bytecodeOffset());
            ASSERT(!byValInfo->stubRoutine);
            if (byValInfo->seen) {
                if (byValInfo->cachedId == propertyName) {
                    JIT::compilePutByValWithCachedId(&vm, exec->codeBlock(), byValInfo, returnAddress, Direct, propertyName);
                    optimizationResult = OptimizationResult::Optimized;
                } else {
                    // Seem like a generic property access site.
                    optimizationResult = OptimizationResult::GiveUp;
                }
            } else {
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->seen = true;
                byValInfo->cachedId = propertyName;
                if (subscript.isSymbol())
                    byValInfo->cachedSymbol.set(vm, codeBlock, asSymbol(subscript));
                optimizationResult = OptimizationResult::SeenOnce;
            }
        }
    }

    if (optimizationResult != OptimizationResult::Optimized && optimizationResult != OptimizationResult::SeenOnce) {
        // If we take slow path more than 10 times without patching then make sure we
        // never make that mistake again. For cases where we see non-index-intercepting
        // objects, this gives 10 iterations worth of opportunity for us to observe
        // that the get_by_val may be polymorphic. We count up slowPathCount even if
        // the result is GiveUp.
        if (++byValInfo->slowPathCount >= 10)
            optimizationResult = OptimizationResult::GiveUp;
    }

    return optimizationResult;
}

void JIT_OPERATION operationDirectPutByValOptimize(ExecState* exec, EncodedJSValue encodedBaseValue, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue baseValue = JSValue::decode(encodedBaseValue);
    JSValue subscript = JSValue::decode(encodedSubscript);
    JSValue value = JSValue::decode(encodedValue);
    RELEASE_ASSERT(baseValue.isObject());
    JSObject* object = asObject(baseValue);
    if (tryDirectPutByValOptimize(exec, object, subscript, byValInfo, ReturnAddressPtr(OUR_RETURN_ADDRESS)) == OptimizationResult::GiveUp) {
        // Don't ever try to optimize.
        byValInfo->tookSlowPath = true;
        ctiPatchCallByReturnAddress(ReturnAddressPtr(OUR_RETURN_ADDRESS), FunctionPtr(operationDirectPutByValGeneric));
    }

    directPutByVal(exec, object, subscript, value, byValInfo);
}

void JIT_OPERATION operationPutByValGeneric(ExecState* exec, EncodedJSValue encodedBaseValue, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    JSValue baseValue = JSValue::decode(encodedBaseValue);
    JSValue subscript = JSValue::decode(encodedSubscript);
    JSValue value = JSValue::decode(encodedValue);

    putByVal(exec, baseValue, subscript, value, byValInfo);
}


void JIT_OPERATION operationDirectPutByValGeneric(ExecState* exec, EncodedJSValue encodedBaseValue, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    JSValue baseValue = JSValue::decode(encodedBaseValue);
    JSValue subscript = JSValue::decode(encodedSubscript);
    JSValue value = JSValue::decode(encodedValue);
    RELEASE_ASSERT(baseValue.isObject());
    directPutByVal(exec, asObject(baseValue), subscript, value, byValInfo);
}

EncodedJSValue JIT_OPERATION operationCallEval(ExecState* exec, ExecState* execCallee)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    execCallee->setCodeBlock(0);
    
    if (!isHostFunction(execCallee->calleeAsValue(), globalFuncEval))
        return JSValue::encode(JSValue());

    JSValue result = eval(execCallee);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    return JSValue::encode(result);
}

static SlowPathReturnType handleHostCall(ExecState* execCallee, JSValue callee, CallLinkInfo* callLinkInfo)
{
    ExecState* exec = execCallee->callerFrame();
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    execCallee->setCodeBlock(0);

    if (callLinkInfo->specializationKind() == CodeForCall) {
        CallData callData;
        CallType callType = getCallData(callee, callData);
    
        ASSERT(callType != CallType::JS);
    
        if (callType == CallType::Host) {
            NativeCallFrameTracer tracer(vm, execCallee);
            execCallee->setCallee(asObject(callee));
            vm->hostCallReturnValue = JSValue::decode(callData.native.function(execCallee));
            if (UNLIKELY(scope.exception())) {
                return encodeResult(
                    vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                    reinterpret_cast<void*>(KeepTheFrame));
            }

            return encodeResult(
                bitwise_cast<void*>(getHostCallReturnValue),
                reinterpret_cast<void*>(callLinkInfo->callMode() == CallMode::Tail ? ReuseTheFrame : KeepTheFrame));
        }
    
        ASSERT(callType == CallType::None);
        throwException(exec, scope, createNotAFunctionError(exec, callee));
        return encodeResult(
            vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
            reinterpret_cast<void*>(KeepTheFrame));
    }

    ASSERT(callLinkInfo->specializationKind() == CodeForConstruct);
    
    ConstructData constructData;
    ConstructType constructType = getConstructData(callee, constructData);
    
    ASSERT(constructType != ConstructType::JS);
    
    if (constructType == ConstructType::Host) {
        NativeCallFrameTracer tracer(vm, execCallee);
        execCallee->setCallee(asObject(callee));
        vm->hostCallReturnValue = JSValue::decode(constructData.native.function(execCallee));
        if (UNLIKELY(scope.exception())) {
            return encodeResult(
                vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                reinterpret_cast<void*>(KeepTheFrame));
        }

        return encodeResult(bitwise_cast<void*>(getHostCallReturnValue), reinterpret_cast<void*>(KeepTheFrame));
    }
    
    ASSERT(constructType == ConstructType::None);
    throwException(exec, scope, createNotAConstructorError(exec, callee));
    return encodeResult(
        vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
        reinterpret_cast<void*>(KeepTheFrame));
}

SlowPathReturnType JIT_OPERATION operationLinkCall(ExecState* execCallee, CallLinkInfo* callLinkInfo)
{
    ExecState* exec = execCallee->callerFrame();
    VM* vm = &exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(*vm);

    CodeSpecializationKind kind = callLinkInfo->specializationKind();
    NativeCallFrameTracer tracer(vm, exec);
    
    RELEASE_ASSERT(!callLinkInfo->isDirect());
    
    JSValue calleeAsValue = execCallee->calleeAsValue();
    JSCell* calleeAsFunctionCell = getJSFunction(calleeAsValue);
    if (!calleeAsFunctionCell) {
        // FIXME: We should cache these kinds of calls. They can be common and currently they are
        // expensive.
        // https://bugs.webkit.org/show_bug.cgi?id=144458
        throwScope.release();
        return handleHostCall(execCallee, calleeAsValue, callLinkInfo);
    }

    JSFunction* callee = jsCast<JSFunction*>(calleeAsFunctionCell);
    JSScope* scope = callee->scopeUnchecked();
    ExecutableBase* executable = callee->executable();

    MacroAssemblerCodePtr codePtr;
    CodeBlock* codeBlock = 0;
    if (executable->isHostFunction()) {
        codePtr = executable->entrypointFor(kind, MustCheckArity);
    } else {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);

        if (!isCall(kind) && functionExecutable->constructAbility() == ConstructAbility::CannotConstruct) {
            throwException(exec, throwScope, createNotAConstructorError(exec, callee));
            return encodeResult(
                vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                reinterpret_cast<void*>(KeepTheFrame));
        }

        CodeBlock** codeBlockSlot = execCallee->addressOfCodeBlock();
        JSObject* error = functionExecutable->prepareForExecution<FunctionExecutable>(*vm, callee, scope, kind, *codeBlockSlot);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(error));
        if (error) {
            throwException(exec, throwScope, error);
            return encodeResult(
                vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                reinterpret_cast<void*>(KeepTheFrame));
        }
        codeBlock = *codeBlockSlot;
        ArityCheckMode arity;
        if (execCallee->argumentCountIncludingThis() < static_cast<size_t>(codeBlock->numParameters()) || callLinkInfo->isVarargs())
            arity = MustCheckArity;
        else
            arity = ArityCheckNotRequired;
        codePtr = functionExecutable->entrypointFor(kind, arity);
    }
    if (!callLinkInfo->seenOnce())
        callLinkInfo->setSeen();
    else
        linkFor(execCallee, *callLinkInfo, codeBlock, callee, codePtr);
    
    return encodeResult(codePtr.executableAddress(), reinterpret_cast<void*>(callLinkInfo->callMode() == CallMode::Tail ? ReuseTheFrame : KeepTheFrame));
}

void JIT_OPERATION operationLinkDirectCall(ExecState* exec, CallLinkInfo* callLinkInfo, JSFunction* callee)
{
    VM* vm = &exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(*vm);

    CodeSpecializationKind kind = callLinkInfo->specializationKind();
    NativeCallFrameTracer tracer(vm, exec);
    
    RELEASE_ASSERT(callLinkInfo->isDirect());
    
    // This would happen if the executable died during GC but the CodeBlock did not die. That should
    // not happen because the CodeBlock should have a weak reference to any executable it uses for
    // this purpose.
    RELEASE_ASSERT(callLinkInfo->executable());
    
    // Having a CodeBlock indicates that this is linked. We shouldn't be taking this path if it's
    // linked.
    RELEASE_ASSERT(!callLinkInfo->codeBlock());
    
    // We just don't support this yet.
    RELEASE_ASSERT(!callLinkInfo->isVarargs());
    
    ExecutableBase* executable = callLinkInfo->executable();
    RELEASE_ASSERT(callee->executable() == callLinkInfo->executable());

    JSScope* scope = callee->scopeUnchecked();

    MacroAssemblerCodePtr codePtr;
    CodeBlock* codeBlock = nullptr;
    if (executable->isHostFunction())
        codePtr = executable->entrypointFor(kind, MustCheckArity);
    else {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);

        RELEASE_ASSERT(isCall(kind) || functionExecutable->constructAbility() != ConstructAbility::CannotConstruct);
        
        JSObject* error = functionExecutable->prepareForExecution<FunctionExecutable>(*vm, callee, scope, kind, codeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(error));
        if (error) {
            throwException(exec, throwScope, error);
            return;
        }
        ArityCheckMode arity;
        unsigned argumentStackSlots = callLinkInfo->maxNumArguments();
        if (argumentStackSlots < static_cast<size_t>(codeBlock->numParameters()))
            arity = MustCheckArity;
        else
            arity = ArityCheckNotRequired;
        codePtr = functionExecutable->entrypointFor(kind, arity);
    }
    
    linkDirectFor(exec, *callLinkInfo, codeBlock, codePtr);
}

inline SlowPathReturnType virtualForWithFunction(
    ExecState* execCallee, CallLinkInfo* callLinkInfo, JSCell*& calleeAsFunctionCell)
{
    ExecState* exec = execCallee->callerFrame();
    VM* vm = &exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(*vm);

    CodeSpecializationKind kind = callLinkInfo->specializationKind();
    NativeCallFrameTracer tracer(vm, exec);

    JSValue calleeAsValue = execCallee->calleeAsValue();
    calleeAsFunctionCell = getJSFunction(calleeAsValue);
    if (UNLIKELY(!calleeAsFunctionCell))
        return handleHostCall(execCallee, calleeAsValue, callLinkInfo);
    
    JSFunction* function = jsCast<JSFunction*>(calleeAsFunctionCell);
    JSScope* scope = function->scopeUnchecked();
    ExecutableBase* executable = function->executable();
    if (UNLIKELY(!executable->hasJITCodeFor(kind))) {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);

        if (!isCall(kind) && functionExecutable->constructAbility() == ConstructAbility::CannotConstruct) {
            throwException(exec, throwScope, createNotAConstructorError(exec, function));
            return encodeResult(
                vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                reinterpret_cast<void*>(KeepTheFrame));
        }

        CodeBlock** codeBlockSlot = execCallee->addressOfCodeBlock();
        JSObject* error = functionExecutable->prepareForExecution<FunctionExecutable>(*vm, function, scope, kind, *codeBlockSlot);
        if (error) {
            throwException(exec, throwScope, error);
            return encodeResult(
                vm->getCTIStub(throwExceptionFromCallSlowPathGenerator).code().executableAddress(),
                reinterpret_cast<void*>(KeepTheFrame));
        }
    }
    return encodeResult(executable->entrypointFor(
        kind, MustCheckArity).executableAddress(),
        reinterpret_cast<void*>(callLinkInfo->callMode() == CallMode::Tail ? ReuseTheFrame : KeepTheFrame));
}

SlowPathReturnType JIT_OPERATION operationLinkPolymorphicCall(ExecState* execCallee, CallLinkInfo* callLinkInfo)
{
    ASSERT(callLinkInfo->specializationKind() == CodeForCall);
    JSCell* calleeAsFunctionCell;
    SlowPathReturnType result = virtualForWithFunction(execCallee, callLinkInfo, calleeAsFunctionCell);

    linkPolymorphicCall(execCallee, *callLinkInfo, CallVariant(calleeAsFunctionCell));
    
    return result;
}

SlowPathReturnType JIT_OPERATION operationVirtualCall(ExecState* execCallee, CallLinkInfo* callLinkInfo)
{
    JSCell* calleeAsFunctionCellIgnored;
    return virtualForWithFunction(execCallee, callLinkInfo, calleeAsFunctionCellIgnored);
}

size_t JIT_OPERATION operationCompareLess(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    return jsLess<true>(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

size_t JIT_OPERATION operationCompareLessEq(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return jsLessEq<true>(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

size_t JIT_OPERATION operationCompareGreater(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return jsLess<false>(exec, JSValue::decode(encodedOp2), JSValue::decode(encodedOp1));
}

size_t JIT_OPERATION operationCompareGreaterEq(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return jsLessEq<false>(exec, JSValue::decode(encodedOp2), JSValue::decode(encodedOp1));
}

size_t JIT_OPERATION operationCompareEq(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return JSValue::equalSlowCaseInline(exec, JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
}

#if USE(JSVALUE64)
EncodedJSValue JIT_OPERATION operationCompareStringEq(ExecState* exec, JSCell* left, JSCell* right)
#else
size_t JIT_OPERATION operationCompareStringEq(ExecState* exec, JSCell* left, JSCell* right)
#endif
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    bool result = asString(left)->equal(exec, asString(right));
#if USE(JSVALUE64)
    return JSValue::encode(jsBoolean(result));
#else
    return result;
#endif
}

EncodedJSValue JIT_OPERATION operationNewArrayWithProfile(ExecState* exec, ArrayAllocationProfile* profile, const JSValue* values, int size)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    return JSValue::encode(constructArrayNegativeIndexed(exec, profile, values, size));
}

EncodedJSValue JIT_OPERATION operationNewArrayBufferWithProfile(ExecState* exec, ArrayAllocationProfile* profile, const JSValue* values, int size)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    return JSValue::encode(constructArray(exec, profile, values, size));
}

EncodedJSValue JIT_OPERATION operationNewArrayWithSizeAndProfile(ExecState* exec, ArrayAllocationProfile* profile, EncodedJSValue size)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    JSValue sizeValue = JSValue::decode(size);
    return JSValue::encode(constructArrayWithSizeQuirk(exec, profile, exec->lexicalGlobalObject(), sizeValue));
}

}

template<typename FunctionType>
static EncodedJSValue operationNewFunctionCommon(ExecState* exec, JSScope* scope, JSCell* functionExecutable, bool isInvalidated)
{
    VM& vm = exec->vm();
    ASSERT(functionExecutable->inherits(vm, FunctionExecutable::info()));
    NativeCallFrameTracer tracer(&vm, exec);
    if (isInvalidated)
        return JSValue::encode(FunctionType::createWithInvalidatedReallocationWatchpoint(vm, static_cast<FunctionExecutable*>(functionExecutable), scope));
    return JSValue::encode(FunctionType::create(vm, static_cast<FunctionExecutable*>(functionExecutable), scope));
}

extern "C" {

EncodedJSValue JIT_OPERATION operationNewFunction(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSFunction>(exec, scope, functionExecutable, false);
}

EncodedJSValue JIT_OPERATION operationNewFunctionWithInvalidatedReallocationWatchpoint(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSFunction>(exec, scope, functionExecutable, true);
}

EncodedJSValue JIT_OPERATION operationNewGeneratorFunction(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSGeneratorFunction>(exec, scope, functionExecutable, false);
}

EncodedJSValue JIT_OPERATION operationNewGeneratorFunctionWithInvalidatedReallocationWatchpoint(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSGeneratorFunction>(exec, scope, functionExecutable, true);
}

EncodedJSValue JIT_OPERATION operationNewAsyncFunction(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSAsyncFunction>(exec, scope, functionExecutable, false);
}

EncodedJSValue JIT_OPERATION operationNewAsyncFunctionWithInvalidatedReallocationWatchpoint(ExecState* exec, JSScope* scope, JSCell* functionExecutable)
{
    return operationNewFunctionCommon<JSAsyncFunction>(exec, scope, functionExecutable, true);
}

void JIT_OPERATION operationSetFunctionName(ExecState* exec, JSCell* funcCell, EncodedJSValue encodedName)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    JSFunction* func = jsCast<JSFunction*>(funcCell);
    JSValue name = JSValue::decode(encodedName);
    func->setFunctionName(exec, name);
}

JSCell* JIT_OPERATION operationNewObject(ExecState* exec, Structure* structure)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return constructEmptyObject(exec, structure);
}

EncodedJSValue JIT_OPERATION operationNewRegexp(ExecState* exec, void* regexpPtr)
{
    SuperSamplerScope superSamplerScope(false);
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    RegExp* regexp = static_cast<RegExp*>(regexpPtr);
    if (!regexp->isValid()) {
        throwException(exec, scope, createSyntaxError(exec, regexp->errorMessage()));
        return JSValue::encode(jsUndefined());
    }

    return JSValue::encode(RegExpObject::create(vm, exec->lexicalGlobalObject()->regExpStructure(), regexp));
}

// The only reason for returning an UnusedPtr (instead of void) is so that we can reuse the
// existing DFG slow path generator machinery when creating the slow path for CheckTraps
// in the DFG. If a DFG slow path generator that supports a void return type is added in the
// future, we can switch to using that then.
UnusedPtr JIT_OPERATION operationHandleTraps(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    ASSERT(vm.needTrapHandling());
    vm.handleTraps(exec);
    return nullptr;
}

void JIT_OPERATION operationDebug(ExecState* exec, int32_t debugHookType)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    vm.interpreter->debug(exec, static_cast<DebugHookType>(debugHookType));
}

#if ENABLE(DFG_JIT)
static void updateAllPredictionsAndOptimizeAfterWarmUp(CodeBlock* codeBlock)
{
    codeBlock->updateAllPredictions();
    codeBlock->optimizeAfterWarmUp();
}

SlowPathReturnType JIT_OPERATION operationOptimize(ExecState* exec, int32_t bytecodeIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    // Defer GC for a while so that it doesn't run between when we enter into this
    // slow path and when we figure out the state of our code block. This prevents
    // a number of awkward reentrancy scenarios, including:
    //
    // - The optimized version of our code block being jettisoned by GC right after
    //   we concluded that we wanted to use it, but have not planted it into the JS
    //   stack yet.
    //
    // - An optimized version of our code block being installed just as we decided
    //   that it wasn't ready yet.
    //
    // Note that jettisoning won't happen if we already initiated OSR, because in
    // that case we would have already planted the optimized code block into the JS
    // stack.
    DeferGCForAWhile deferGC(vm.heap);
    
    CodeBlock* codeBlock = exec->codeBlock();
    if (codeBlock->jitType() != JITCode::BaselineJIT) {
        dataLog("Unexpected code block in Baseline->DFG tier-up: ", *codeBlock, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    if (bytecodeIndex) {
        // If we're attempting to OSR from a loop, assume that this should be
        // separately optimized.
        codeBlock->m_shouldAlwaysBeInlined = false;
    }

    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered optimize with bytecodeIndex = ", bytecodeIndex,
            ", executeCounter = ", codeBlock->jitExecuteCounter(),
            ", optimizationDelayCounter = ", codeBlock->reoptimizationRetryCounter(),
            ", exitCounter = ");
        if (codeBlock->hasOptimizedReplacement())
            dataLog(codeBlock->replacement()->osrExitCounter());
        else
            dataLog("N/A");
        dataLog("\n");
    }

    if (!codeBlock->checkIfOptimizationThresholdReached()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("counter = ", codeBlock->jitExecuteCounter()));
        codeBlock->updateAllPredictions();
        if (Options::verboseOSR())
            dataLog("Choosing not to optimize ", *codeBlock, " yet, because the threshold hasn't been reached.\n");
        return encodeResult(0, 0);
    }
    
    Debugger* debugger = codeBlock->globalObject()->debugger();
    if (debugger && (debugger->isStepping() || codeBlock->baselineAlternative()->hasDebuggerRequests())) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("debugger is stepping or has requests"));
        updateAllPredictionsAndOptimizeAfterWarmUp(codeBlock);
        return encodeResult(0, 0);
    }

    if (codeBlock->m_shouldAlwaysBeInlined) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("should always be inlined"));
        updateAllPredictionsAndOptimizeAfterWarmUp(codeBlock);
        if (Options::verboseOSR())
            dataLog("Choosing not to optimize ", *codeBlock, " yet, because m_shouldAlwaysBeInlined == true.\n");
        return encodeResult(0, 0);
    }

    // We cannot be in the process of asynchronous compilation and also have an optimized
    // replacement.
    DFG::Worklist* worklist = DFG::existingGlobalDFGWorklistOrNull();
    ASSERT(
        !worklist
        || !(worklist->compilationState(DFG::CompilationKey(codeBlock, DFG::DFGMode)) != DFG::Worklist::NotKnown
        && codeBlock->hasOptimizedReplacement()));

    DFG::Worklist::State worklistState;
    if (worklist) {
        // The call to DFG::Worklist::completeAllReadyPlansForVM() will complete all ready
        // (i.e. compiled) code blocks. But if it completes ours, we also need to know
        // what the result was so that we don't plow ahead and attempt OSR or immediate
        // reoptimization. This will have already also set the appropriate JIT execution
        // count threshold depending on what happened, so if the compilation was anything
        // but successful we just want to return early. See the case for worklistState ==
        // DFG::Worklist::Compiled, below.
        
        // Note that we could have alternatively just called Worklist::compilationState()
        // here, and if it returned Compiled, we could have then called
        // completeAndScheduleOSR() below. But that would have meant that it could take
        // longer for code blocks to be completed: they would only complete when *their*
        // execution count trigger fired; but that could take a while since the firing is
        // racy. It could also mean that code blocks that never run again after being
        // compiled would sit on the worklist until next GC. That's fine, but it's
        // probably a waste of memory. Our goal here is to complete code blocks as soon as
        // possible in order to minimize the chances of us executing baseline code after
        // optimized code is already available.
        worklistState = worklist->completeAllReadyPlansForVM(
            vm, DFG::CompilationKey(codeBlock, DFG::DFGMode));
    } else
        worklistState = DFG::Worklist::NotKnown;

    if (worklistState == DFG::Worklist::Compiling) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("compiling"));
        // We cannot be in the process of asynchronous compilation and also have an optimized
        // replacement.
        RELEASE_ASSERT(!codeBlock->hasOptimizedReplacement());
        codeBlock->setOptimizationThresholdBasedOnCompilationResult(CompilationDeferred);
        return encodeResult(0, 0);
    }

    if (worklistState == DFG::Worklist::Compiled) {
        // If we don't have an optimized replacement but we did just get compiled, then
        // the compilation failed or was invalidated, in which case the execution count
        // thresholds have already been set appropriately by
        // CodeBlock::setOptimizationThresholdBasedOnCompilationResult() and we have
        // nothing left to do.
        if (!codeBlock->hasOptimizedReplacement()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("compiled and failed"));
            codeBlock->updateAllPredictions();
            if (Options::verboseOSR())
                dataLog("Code block ", *codeBlock, " was compiled but it doesn't have an optimized replacement.\n");
            return encodeResult(0, 0);
        }
    } else if (codeBlock->hasOptimizedReplacement()) {
        if (Options::verboseOSR())
            dataLog("Considering OSR ", *codeBlock, " -> ", *codeBlock->replacement(), ".\n");
        // If we have an optimized replacement, then it must be the case that we entered
        // cti_optimize from a loop. That's because if there's an optimized replacement,
        // then all calls to this function will be relinked to the replacement and so
        // the prologue OSR will never fire.
        
        // This is an interesting threshold check. Consider that a function OSR exits
        // in the middle of a loop, while having a relatively low exit count. The exit
        // will reset the execution counter to some target threshold, meaning that this
        // code won't be reached until that loop heats up for >=1000 executions. But then
        // we do a second check here, to see if we should either reoptimize, or just
        // attempt OSR entry. Hence it might even be correct for
        // shouldReoptimizeFromLoopNow() to always return true. But we make it do some
        // additional checking anyway, to reduce the amount of recompilation thrashing.
        if (codeBlock->replacement()->shouldReoptimizeFromLoopNow()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("should reoptimize from loop now"));
            if (Options::verboseOSR()) {
                dataLog(
                    "Triggering reoptimization of ", *codeBlock,
                    "(", *codeBlock->replacement(), ") (in loop).\n");
            }
            codeBlock->replacement()->jettison(Profiler::JettisonDueToBaselineLoopReoptimizationTrigger, CountReoptimization);
            return encodeResult(0, 0);
        }
    } else {
        if (!codeBlock->shouldOptimizeNow()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("insufficient profiling"));
            if (Options::verboseOSR()) {
                dataLog(
                    "Delaying optimization for ", *codeBlock,
                    " because of insufficient profiling.\n");
            }
            return encodeResult(0, 0);
        }

        if (Options::verboseOSR())
            dataLog("Triggering optimized compilation of ", *codeBlock, "\n");

        unsigned numVarsWithValues;
        if (bytecodeIndex)
            numVarsWithValues = codeBlock->m_numCalleeLocals;
        else
            numVarsWithValues = 0;
        Operands<JSValue> mustHandleValues(codeBlock->numParameters(), numVarsWithValues);
        int localsUsedForCalleeSaves = static_cast<int>(CodeBlock::llintBaselineCalleeSaveSpaceAsVirtualRegisters());
        for (size_t i = 0; i < mustHandleValues.size(); ++i) {
            int operand = mustHandleValues.operandForIndex(i);
            if (operandIsLocal(operand) && VirtualRegister(operand).toLocal() < localsUsedForCalleeSaves)
                continue;
            mustHandleValues[i] = exec->uncheckedR(operand).jsValue();
        }

        CodeBlock* replacementCodeBlock = codeBlock->newReplacement();
        CompilationResult result = DFG::compile(
            vm, replacementCodeBlock, nullptr, DFG::DFGMode, bytecodeIndex,
            mustHandleValues, JITToDFGDeferredCompilationCallback::create());
        
        if (result != CompilationSuccessful) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("compilation failed"));
            return encodeResult(0, 0);
        }
    }
    
    CodeBlock* optimizedCodeBlock = codeBlock->replacement();
    ASSERT(JITCode::isOptimizingJIT(optimizedCodeBlock->jitType()));
    
    if (void* dataBuffer = DFG::prepareOSREntry(exec, optimizedCodeBlock, bytecodeIndex)) {
        CODEBLOCK_LOG_EVENT(optimizedCodeBlock, "osrEntry", ("at bc#", bytecodeIndex));
        if (Options::verboseOSR()) {
            dataLog(
                "Performing OSR ", *codeBlock, " -> ", *optimizedCodeBlock, ".\n");
        }

        codeBlock->optimizeSoon();
        codeBlock->unlinkedCodeBlock()->setDidOptimize(TrueTriState);
        return encodeResult(vm.getCTIStub(DFG::osrEntryThunkGenerator).code().executableAddress(), dataBuffer);
    }

    if (Options::verboseOSR()) {
        dataLog(
            "Optimizing ", *codeBlock, " -> ", *codeBlock->replacement(),
            " succeeded, OSR failed, after a delay of ",
            codeBlock->optimizationDelayCounter(), ".\n");
    }

    // Count the OSR failure as a speculation failure. If this happens a lot, then
    // reoptimize.
    optimizedCodeBlock->countOSRExit();

    // We are a lot more conservative about triggering reoptimization after OSR failure than
    // before it. If we enter the optimize_from_loop trigger with a bucket full of fail
    // already, then we really would like to reoptimize immediately. But this case covers
    // something else: there weren't many (or any) speculation failures before, but we just
    // failed to enter the speculative code because some variable had the wrong value or
    // because the OSR code decided for any spurious reason that it did not want to OSR
    // right now. So, we only trigger reoptimization only upon the more conservative (non-loop)
    // reoptimization trigger.
    if (optimizedCodeBlock->shouldReoptimizeNow()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("should reoptimize now"));
        if (Options::verboseOSR()) {
            dataLog(
                "Triggering reoptimization of ", *codeBlock, " -> ",
                *codeBlock->replacement(), " (after OSR fail).\n");
        }
        optimizedCodeBlock->jettison(Profiler::JettisonDueToBaselineLoopReoptimizationTriggerOnOSREntryFail, CountReoptimization);
        return encodeResult(0, 0);
    }

    // OSR failed this time, but it might succeed next time! Let the code run a bit
    // longer and then try again.
    codeBlock->optimizeAfterWarmUp();
    
    CODEBLOCK_LOG_EVENT(codeBlock, "delayOptimizeToDFG", ("OSR failed"));
    return encodeResult(0, 0);
}
#endif

void JIT_OPERATION operationPutByIndex(ExecState* exec, EncodedJSValue encodedArrayValue, int32_t index, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue arrayValue = JSValue::decode(encodedArrayValue);
    ASSERT(isJSArray(arrayValue));
    asArray(arrayValue)->putDirectIndex(exec, index, JSValue::decode(encodedValue));
}

enum class AccessorType {
    Getter,
    Setter
};

static void putAccessorByVal(ExecState* exec, JSObject* base, JSValue subscript, int32_t attribute, JSObject* accessor, AccessorType accessorType)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    auto propertyKey = subscript.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());

    if (accessorType == AccessorType::Getter)
        base->putGetter(exec, propertyKey, accessor, attribute);
    else
        base->putSetter(exec, propertyKey, accessor, attribute);
}

void JIT_OPERATION operationPutGetterById(ExecState* exec, JSCell* object, UniquedStringImpl* uid, int32_t options, JSCell* getter)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(object && object->isObject());
    JSObject* baseObj = object->getObject();

    ASSERT(getter->isObject());
    baseObj->putGetter(exec, uid, getter, options);
}

void JIT_OPERATION operationPutSetterById(ExecState* exec, JSCell* object, UniquedStringImpl* uid, int32_t options, JSCell* setter)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(object && object->isObject());
    JSObject* baseObj = object->getObject();

    ASSERT(setter->isObject());
    baseObj->putSetter(exec, uid, setter, options);
}

void JIT_OPERATION operationPutGetterByVal(ExecState* exec, JSCell* base, EncodedJSValue encodedSubscript, int32_t attribute, JSCell* getter)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    putAccessorByVal(exec, asObject(base), JSValue::decode(encodedSubscript), attribute, asObject(getter), AccessorType::Getter);
}

void JIT_OPERATION operationPutSetterByVal(ExecState* exec, JSCell* base, EncodedJSValue encodedSubscript, int32_t attribute, JSCell* setter)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    putAccessorByVal(exec, asObject(base), JSValue::decode(encodedSubscript), attribute, asObject(setter), AccessorType::Setter);
}

#if USE(JSVALUE64)
void JIT_OPERATION operationPutGetterSetter(ExecState* exec, JSCell* object, UniquedStringImpl* uid, int32_t attribute, EncodedJSValue encodedGetterValue, EncodedJSValue encodedSetterValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(object && object->isObject());
    JSObject* baseObj = asObject(object);

    GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());

    JSValue getter = JSValue::decode(encodedGetterValue);
    JSValue setter = JSValue::decode(encodedSetterValue);
    ASSERT(getter.isObject() || getter.isUndefined());
    ASSERT(setter.isObject() || setter.isUndefined());
    ASSERT(getter.isObject() || setter.isObject());

    if (!getter.isUndefined())
        accessor->setGetter(vm, exec->lexicalGlobalObject(), asObject(getter));
    if (!setter.isUndefined())
        accessor->setSetter(vm, exec->lexicalGlobalObject(), asObject(setter));
    baseObj->putDirectAccessor(exec, uid, accessor, attribute);
}

#else
void JIT_OPERATION operationPutGetterSetter(ExecState* exec, JSCell* object, UniquedStringImpl* uid, int32_t attribute, JSCell* getter, JSCell* setter)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(object && object->isObject());
    JSObject* baseObj = asObject(object);

    GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());

    ASSERT(!getter || getter->isObject());
    ASSERT(!setter || setter->isObject());
    ASSERT(getter || setter);

    if (getter)
        accessor->setGetter(vm, exec->lexicalGlobalObject(), getter->getObject());
    if (setter)
        accessor->setSetter(vm, exec->lexicalGlobalObject(), setter->getObject());
    baseObj->putDirectAccessor(exec, uid, accessor, attribute);
}
#endif

void JIT_OPERATION operationPopScope(ExecState* exec, int32_t scopeReg)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSScope* scope = exec->uncheckedR(scopeReg).Register::scope();
    exec->uncheckedR(scopeReg) = scope->next();
}

int32_t JIT_OPERATION operationInstanceOfCustom(ExecState* exec, EncodedJSValue encodedValue, JSObject* constructor, EncodedJSValue encodedHasInstance)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue value = JSValue::decode(encodedValue);
    JSValue hasInstanceValue = JSValue::decode(encodedHasInstance);

    ASSERT(hasInstanceValue != exec->lexicalGlobalObject()->functionProtoHasInstanceSymbolFunction() || !constructor->structure()->typeInfo().implementsDefaultHasInstance());

    if (constructor->hasInstance(exec, value, hasInstanceValue))
        return 1;
    return 0;
}

}

static bool canAccessArgumentIndexQuickly(JSObject& object, uint32_t index)
{
    switch (object.structure()->typeInfo().type()) {
    case DirectArgumentsType: {
        DirectArguments* directArguments = jsCast<DirectArguments*>(&object);
        if (directArguments->isMappedArgumentInDFG(index))
            return true;
        break;
    }
    case ScopedArgumentsType: {
        ScopedArguments* scopedArguments = jsCast<ScopedArguments*>(&object);
        if (scopedArguments->isMappedArgumentInDFG(index))
            return true;
        break;
    }
    default:
        break;
    }
    return false;
}

static JSValue getByVal(ExecState* exec, JSValue baseValue, JSValue subscript, ByValInfo* byValInfo, ReturnAddressPtr returnAddress)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (LIKELY(baseValue.isCell() && subscript.isString())) {
        Structure& structure = *baseValue.asCell()->structure(vm);
        if (JSCell::canUseFastGetOwnProperty(structure)) {
            if (RefPtr<AtomicStringImpl> existingAtomicString = asString(subscript)->toExistingAtomicString(exec)) {
                if (JSValue result = baseValue.asCell()->fastGetOwnProperty(vm, structure, existingAtomicString.get())) {
                    ASSERT(exec->bytecodeOffset());
                    if (byValInfo->stubInfo && byValInfo->cachedId.impl() != existingAtomicString)
                        byValInfo->tookSlowPath = true;
                    return result;
                }
            }
        }
    }

    if (subscript.isUInt32()) {
        ASSERT(exec->bytecodeOffset());
        byValInfo->tookSlowPath = true;

        uint32_t i = subscript.asUInt32();
        if (isJSString(baseValue)) {
            if (asString(baseValue)->canGetIndex(i)) {
                ctiPatchCallByReturnAddress(returnAddress, FunctionPtr(operationGetByValString));
                return asString(baseValue)->getIndex(exec, i);
            }
            byValInfo->arrayProfile->setOutOfBounds();
        } else if (baseValue.isObject()) {
            JSObject* object = asObject(baseValue);
            if (object->canGetIndexQuickly(i))
                return object->getIndexQuickly(i);

            if (!canAccessArgumentIndexQuickly(*object, i)) {
                // FIXME: This will make us think that in-bounds typed array accesses are actually
                // out-of-bounds.
                // https://bugs.webkit.org/show_bug.cgi?id=149886
                byValInfo->arrayProfile->setOutOfBounds();
            }
        }

        return baseValue.get(exec, i);
    }

    baseValue.requireObjectCoercible(exec);
    RETURN_IF_EXCEPTION(scope, JSValue());
    auto property = subscript.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, JSValue());

    ASSERT(exec->bytecodeOffset());
    if (byValInfo->stubInfo && (!isStringOrSymbol(subscript) || byValInfo->cachedId != property))
        byValInfo->tookSlowPath = true;

    return baseValue.get(exec, property);
}

static OptimizationResult tryGetByValOptimize(ExecState* exec, JSValue baseValue, JSValue subscript, ByValInfo* byValInfo, ReturnAddressPtr returnAddress)
{
    // See if it's worth optimizing this at all.
    OptimizationResult optimizationResult = OptimizationResult::NotOptimized;

    VM& vm = exec->vm();

    if (baseValue.isObject() && subscript.isInt32()) {
        JSObject* object = asObject(baseValue);

        ASSERT(exec->bytecodeOffset());
        ASSERT(!byValInfo->stubRoutine);

        if (hasOptimizableIndexing(object->structure(vm))) {
            // Attempt to optimize.
            Structure* structure = object->structure(vm);
            JITArrayMode arrayMode = jitArrayModeForStructure(structure);
            if (arrayMode != byValInfo->arrayMode) {
                // If we reached this case, we got an interesting array mode we did not expect when we compiled.
                // Let's update the profile to do better next time.
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->arrayProfile->computeUpdatedPrediction(locker, codeBlock, structure);

                JIT::compileGetByVal(&vm, exec->codeBlock(), byValInfo, returnAddress, arrayMode);
                optimizationResult = OptimizationResult::Optimized;
            }
        }

        // If we failed to patch and we have some object that intercepts indexed get, then don't even wait until 10 times.
        if (optimizationResult != OptimizationResult::Optimized && object->structure(vm)->typeInfo().interceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero())
            optimizationResult = OptimizationResult::GiveUp;
    }

    if (baseValue.isObject() && isStringOrSymbol(subscript)) {
        const Identifier propertyName = subscript.toPropertyKey(exec);
        if (subscript.isSymbol() || !parseIndex(propertyName)) {
            ASSERT(exec->bytecodeOffset());
            ASSERT(!byValInfo->stubRoutine);
            if (byValInfo->seen) {
                if (byValInfo->cachedId == propertyName) {
                    JIT::compileGetByValWithCachedId(&vm, exec->codeBlock(), byValInfo, returnAddress, propertyName);
                    optimizationResult = OptimizationResult::Optimized;
                } else {
                    // Seem like a generic property access site.
                    optimizationResult = OptimizationResult::GiveUp;
                }
            } else {
                CodeBlock* codeBlock = exec->codeBlock();
                ConcurrentJSLocker locker(codeBlock->m_lock);
                byValInfo->seen = true;
                byValInfo->cachedId = propertyName;
                if (subscript.isSymbol())
                    byValInfo->cachedSymbol.set(vm, codeBlock, asSymbol(subscript));
                optimizationResult = OptimizationResult::SeenOnce;
            }
        }
    }

    if (optimizationResult != OptimizationResult::Optimized && optimizationResult != OptimizationResult::SeenOnce) {
        // If we take slow path more than 10 times without patching then make sure we
        // never make that mistake again. For cases where we see non-index-intercepting
        // objects, this gives 10 iterations worth of opportunity for us to observe
        // that the get_by_val may be polymorphic. We count up slowPathCount even if
        // the result is GiveUp.
        if (++byValInfo->slowPathCount >= 10)
            optimizationResult = OptimizationResult::GiveUp;
    }

    return optimizationResult;
}

extern "C" {

EncodedJSValue JIT_OPERATION operationGetByValGeneric(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue subscript = JSValue::decode(encodedSubscript);

    JSValue result = getByVal(exec, baseValue, subscript, byValInfo, ReturnAddressPtr(OUR_RETURN_ADDRESS));
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationGetByValOptimize(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue subscript = JSValue::decode(encodedSubscript);
    ReturnAddressPtr returnAddress = ReturnAddressPtr(OUR_RETURN_ADDRESS);
    if (tryGetByValOptimize(exec, baseValue, subscript, byValInfo, returnAddress) == OptimizationResult::GiveUp) {
        // Don't ever try to optimize.
        byValInfo->tookSlowPath = true;
        ctiPatchCallByReturnAddress(returnAddress, FunctionPtr(operationGetByValGeneric));
    }

    return JSValue::encode(getByVal(exec, baseValue, subscript, byValInfo, returnAddress));
}

EncodedJSValue JIT_OPERATION operationHasIndexedPropertyDefault(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue subscript = JSValue::decode(encodedSubscript);
    
    ASSERT(baseValue.isObject());
    ASSERT(subscript.isUInt32());

    JSObject* object = asObject(baseValue);
    bool didOptimize = false;

    ASSERT(exec->bytecodeOffset());
    ASSERT(!byValInfo->stubRoutine);
    
    if (hasOptimizableIndexing(object->structure(vm))) {
        // Attempt to optimize.
        JITArrayMode arrayMode = jitArrayModeForStructure(object->structure(vm));
        if (arrayMode != byValInfo->arrayMode) {
            JIT::compileHasIndexedProperty(&vm, exec->codeBlock(), byValInfo, ReturnAddressPtr(OUR_RETURN_ADDRESS), arrayMode);
            didOptimize = true;
        }
    }
    
    if (!didOptimize) {
        // If we take slow path more than 10 times without patching then make sure we
        // never make that mistake again. Or, if we failed to patch and we have some object
        // that intercepts indexed get, then don't even wait until 10 times. For cases
        // where we see non-index-intercepting objects, this gives 10 iterations worth of
        // opportunity for us to observe that the get_by_val may be polymorphic.
        if (++byValInfo->slowPathCount >= 10
            || object->structure(vm)->typeInfo().interceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero()) {
            // Don't ever try to optimize.
            ctiPatchCallByReturnAddress(ReturnAddressPtr(OUR_RETURN_ADDRESS), FunctionPtr(operationHasIndexedPropertyGeneric));
        }
    }

    uint32_t index = subscript.asUInt32();
    if (object->canGetIndexQuickly(index))
        return JSValue::encode(JSValue(JSValue::JSTrue));

    if (!canAccessArgumentIndexQuickly(*object, index)) {
        // FIXME: This will make us think that in-bounds typed array accesses are actually
        // out-of-bounds.
        // https://bugs.webkit.org/show_bug.cgi?id=149886
        byValInfo->arrayProfile->setOutOfBounds();
    }
    return JSValue::encode(jsBoolean(object->hasPropertyGeneric(exec, index, PropertySlot::InternalMethodType::GetOwnProperty)));
}
    
EncodedJSValue JIT_OPERATION operationHasIndexedPropertyGeneric(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue subscript = JSValue::decode(encodedSubscript);
    
    ASSERT(baseValue.isObject());
    ASSERT(subscript.isUInt32());

    JSObject* object = asObject(baseValue);
    uint32_t index = subscript.asUInt32();
    if (object->canGetIndexQuickly(index))
        return JSValue::encode(JSValue(JSValue::JSTrue));

    if (!canAccessArgumentIndexQuickly(*object, index)) {
        // FIXME: This will make us think that in-bounds typed array accesses are actually
        // out-of-bounds.
        // https://bugs.webkit.org/show_bug.cgi?id=149886
        byValInfo->arrayProfile->setOutOfBounds();
    }
    return JSValue::encode(jsBoolean(object->hasPropertyGeneric(exec, subscript.asUInt32(), PropertySlot::InternalMethodType::GetOwnProperty)));
}
    
EncodedJSValue JIT_OPERATION operationGetByValString(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedSubscript, ByValInfo* byValInfo)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue subscript = JSValue::decode(encodedSubscript);
    
    JSValue result;
    if (LIKELY(subscript.isUInt32())) {
        uint32_t i = subscript.asUInt32();
        if (isJSString(baseValue) && asString(baseValue)->canGetIndex(i))
            result = asString(baseValue)->getIndex(exec, i);
        else {
            result = baseValue.get(exec, i);
            if (!isJSString(baseValue)) {
                ASSERT(exec->bytecodeOffset());
                ctiPatchCallByReturnAddress(ReturnAddressPtr(OUR_RETURN_ADDRESS), FunctionPtr(byValInfo->stubRoutine ? operationGetByValGeneric : operationGetByValOptimize));
            }
        }
    } else {
        baseValue.requireObjectCoercible(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        auto property = subscript.toPropertyKey(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        result = baseValue.get(exec, property);
    }

    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationDeleteByIdJSResult(ExecState* exec, EncodedJSValue base, UniquedStringImpl* uid)
{
    return JSValue::encode(jsBoolean(operationDeleteById(exec, base, uid)));
}

size_t JIT_OPERATION operationDeleteById(ExecState* exec, EncodedJSValue encodedBase, UniquedStringImpl* uid)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* baseObj = JSValue::decode(encodedBase).toObject(exec);
    if (!baseObj)
        return false;
    bool couldDelete = baseObj->methodTable(vm)->deleteProperty(baseObj, exec, Identifier::fromUid(&vm, uid));
    if (!couldDelete && exec->codeBlock()->isStrictMode())
        throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
    return couldDelete;
}

EncodedJSValue JIT_OPERATION operationDeleteByValJSResult(ExecState* exec, EncodedJSValue base,  EncodedJSValue key)
{
    return JSValue::encode(jsBoolean(operationDeleteByVal(exec, base, key)));
}

size_t JIT_OPERATION operationDeleteByVal(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedKey)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* baseObj = JSValue::decode(encodedBase).toObject(exec);
    JSValue key = JSValue::decode(encodedKey);
    if (!baseObj)
        return false;

    bool couldDelete;
    uint32_t index;
    if (key.getUInt32(index))
        couldDelete = baseObj->methodTable(vm)->deletePropertyByIndex(baseObj, exec, index);
    else {
        RETURN_IF_EXCEPTION(scope, false);
        Identifier property = key.toPropertyKey(exec);
        RETURN_IF_EXCEPTION(scope, false);
        couldDelete = baseObj->methodTable(vm)->deleteProperty(baseObj, exec, property);
    }
    if (!couldDelete && exec->codeBlock()->isStrictMode())
        throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
    return couldDelete;
}

EncodedJSValue JIT_OPERATION operationInstanceOf(ExecState* exec, EncodedJSValue encodedValue, EncodedJSValue encodedProto)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue value = JSValue::decode(encodedValue);
    JSValue proto = JSValue::decode(encodedProto);
    
    bool result = JSObject::defaultHasInstance(exec, value, proto);
    return JSValue::encode(jsBoolean(result));
}

int32_t JIT_OPERATION operationSizeFrameForForwardArguments(ExecState* exec, EncodedJSValue, int32_t numUsedStackSlots, int32_t)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return sizeFrameForForwardArguments(exec, vm, numUsedStackSlots);
}

int32_t JIT_OPERATION operationSizeFrameForVarargs(ExecState* exec, EncodedJSValue encodedArguments, int32_t numUsedStackSlots, int32_t firstVarArgOffset)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue arguments = JSValue::decode(encodedArguments);
    return sizeFrameForVarargs(exec, vm, arguments, numUsedStackSlots, firstVarArgOffset);
}

CallFrame* JIT_OPERATION operationSetupForwardArgumentsFrame(ExecState* exec, CallFrame* newCallFrame, EncodedJSValue, int32_t, int32_t length)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    setupForwardArgumentsFrame(exec, newCallFrame, length);
    return newCallFrame;
}

CallFrame* JIT_OPERATION operationSetupVarargsFrame(ExecState* exec, CallFrame* newCallFrame, EncodedJSValue encodedArguments, int32_t firstVarArgOffset, int32_t length)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue arguments = JSValue::decode(encodedArguments);
    setupVarargsFrame(exec, newCallFrame, arguments, firstVarArgOffset, length);
    return newCallFrame;
}

EncodedJSValue JIT_OPERATION operationToObject(ExecState* exec, EncodedJSValue value)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSObject* obj = JSValue::decode(value).toObject(exec);
    if (!obj)
        return JSValue::encode(JSValue());
    return JSValue::encode(obj);
}

char* JIT_OPERATION operationSwitchCharWithUnknownKeyType(ExecState* exec, EncodedJSValue encodedKey, size_t tableIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue key = JSValue::decode(encodedKey);
    CodeBlock* codeBlock = exec->codeBlock();

    SimpleJumpTable& jumpTable = codeBlock->switchJumpTable(tableIndex);
    void* result = jumpTable.ctiDefault.executableAddress();

    if (key.isString()) {
        StringImpl* value = asString(key)->value(exec).impl();
        if (value->length() == 1)
            result = jumpTable.ctiForValue((*value)[0]).executableAddress();
    }

    return reinterpret_cast<char*>(result);
}

char* JIT_OPERATION operationSwitchImmWithUnknownKeyType(ExecState* exec, EncodedJSValue encodedKey, size_t tableIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue key = JSValue::decode(encodedKey);
    CodeBlock* codeBlock = exec->codeBlock();

    SimpleJumpTable& jumpTable = codeBlock->switchJumpTable(tableIndex);
    void* result;
    if (key.isInt32())
        result = jumpTable.ctiForValue(key.asInt32()).executableAddress();
    else if (key.isDouble() && key.asDouble() == static_cast<int32_t>(key.asDouble()))
        result = jumpTable.ctiForValue(static_cast<int32_t>(key.asDouble())).executableAddress();
    else
        result = jumpTable.ctiDefault.executableAddress();
    return reinterpret_cast<char*>(result);
}

char* JIT_OPERATION operationSwitchStringWithUnknownKeyType(ExecState* exec, EncodedJSValue encodedKey, size_t tableIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue key = JSValue::decode(encodedKey);
    CodeBlock* codeBlock = exec->codeBlock();

    void* result;
    StringJumpTable& jumpTable = codeBlock->stringSwitchJumpTable(tableIndex);

    if (key.isString()) {
        StringImpl* value = asString(key)->value(exec).impl();
        result = jumpTable.ctiForValue(value).executableAddress();
    } else
        result = jumpTable.ctiDefault.executableAddress();

    return reinterpret_cast<char*>(result);
}

EncodedJSValue JIT_OPERATION operationGetFromScope(ExecState* exec, Instruction* bytecodePC)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    CodeBlock* codeBlock = exec->codeBlock();
    Instruction* pc = bytecodePC;

    const Identifier& ident = codeBlock->identifier(pc[3].u.operand);
    JSObject* scope = jsCast<JSObject*>(exec->uncheckedR(pc[2].u.operand).jsValue());
    GetPutInfo getPutInfo(pc[4].u.operand);

    // ModuleVar is always converted to ClosureVar for get_from_scope.
    ASSERT(getPutInfo.resolveType() != ModuleVar);

    return JSValue::encode(scope->getPropertySlot(exec, ident, [&] (bool found, PropertySlot& slot) -> JSValue {
        if (!found) {
            if (getPutInfo.resolveMode() == ThrowIfNotFound)
                throwException(exec, throwScope, createUndefinedVariableError(exec, ident));
            return jsUndefined();
        }

        JSValue result = JSValue();
        if (scope->isGlobalLexicalEnvironment()) {
            // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
            result = slot.getValue(exec, ident);
            if (result == jsTDZValue()) {
                throwException(exec, throwScope, createTDZError(exec));
                return jsUndefined();
            }
        }

        CommonSlowPaths::tryCacheGetFromScopeGlobal(exec, vm, pc, scope, slot, ident);

        if (!result)
            return slot.getValue(exec, ident);
        return result;
    }));
}

void JIT_OPERATION operationPutToScope(ExecState* exec, Instruction* bytecodePC)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    Instruction* pc = bytecodePC;

    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[2].u.operand);
    JSObject* scope = jsCast<JSObject*>(exec->uncheckedR(pc[1].u.operand).jsValue());
    JSValue value = exec->r(pc[3].u.operand).jsValue();
    GetPutInfo getPutInfo = GetPutInfo(pc[4].u.operand);

    // ModuleVar does not keep the scope register value alive in DFG.
    ASSERT(getPutInfo.resolveType() != ModuleVar);

    if (getPutInfo.resolveType() == LocalClosureVar) {
        JSLexicalEnvironment* environment = jsCast<JSLexicalEnvironment*>(scope);
        environment->variableAt(ScopeOffset(pc[6].u.operand)).set(vm, environment, value);
        if (WatchpointSet* set = pc[5].u.watchpointSet)
            set->touch(vm, "Executed op_put_scope<LocalClosureVar>");
        return;
    }

    bool hasProperty = scope->hasProperty(exec, ident);
    if (hasProperty
        && scope->isGlobalLexicalEnvironment()
        && !isInitialization(getPutInfo.initializationMode())) {
        // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
        PropertySlot slot(scope, PropertySlot::InternalMethodType::Get);
        JSGlobalLexicalEnvironment::getOwnPropertySlot(scope, exec, ident, slot);
        if (slot.getValue(exec, ident) == jsTDZValue()) {
            throwException(exec, throwScope, createTDZError(exec));
            return;
        }
    }

    if (getPutInfo.resolveMode() == ThrowIfNotFound && !hasProperty) {
        throwException(exec, throwScope, createUndefinedVariableError(exec, ident));
        return;
    }

    PutPropertySlot slot(scope, codeBlock->isStrictMode(), PutPropertySlot::UnknownContext, isInitialization(getPutInfo.initializationMode()));
    scope->methodTable()->put(scope, exec, ident, value, slot);
    
    RETURN_IF_EXCEPTION(throwScope, void());

    CommonSlowPaths::tryCachePutToScopeGlobal(exec, codeBlock, pc, scope, getPutInfo, slot, ident);
}

void JIT_OPERATION operationThrow(ExecState* exec, EncodedJSValue encodedExceptionValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue exceptionValue = JSValue::decode(encodedExceptionValue);
    throwException(exec, scope, exceptionValue);

    // Results stored out-of-band in vm.targetMachinePCForThrow & vm.callFrameForCatch
    genericUnwind(vm, exec);
}

char* JIT_OPERATION operationReallocateButterflyToHavePropertyStorageWithInitialCapacity(ExecState* exec, JSObject* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(!object->structure()->outOfLineCapacity());
    Butterfly* result = object->allocateMoreOutOfLineStorage(vm, 0, initialOutOfLineCapacity);
    object->nukeStructureAndSetButterfly(vm, object->structureID(), result);
    return reinterpret_cast<char*>(result);
}

char* JIT_OPERATION operationReallocateButterflyToGrowPropertyStorage(ExecState* exec, JSObject* object, size_t newSize)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    Butterfly* result = object->allocateMoreOutOfLineStorage(vm, object->structure()->outOfLineCapacity(), newSize);
    object->nukeStructureAndSetButterfly(vm, object->structureID(), result);
    return reinterpret_cast<char*>(result);
}

void JIT_OPERATION operationOSRWriteBarrier(ExecState* exec, JSCell* cell)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    vm->heap.writeBarrier(cell);
}

void JIT_OPERATION operationWriteBarrierSlowPath(ExecState* exec, JSCell* cell)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    vm->heap.writeBarrierSlowPath(cell);
}

void JIT_OPERATION lookupExceptionHandler(VM* vm, ExecState* exec)
{
    NativeCallFrameTracer tracer(vm, exec);
    genericUnwind(vm, exec);
    ASSERT(vm->targetMachinePCForThrow);
}

void JIT_OPERATION lookupExceptionHandlerFromCallerFrame(VM* vm, ExecState* exec)
{
    vm->topCallFrame = exec->callerFrame();
    genericUnwind(vm, exec, UnwindFromCallerFrame);
    ASSERT(vm->targetMachinePCForThrow);
}

void JIT_OPERATION operationVMHandleException(ExecState* exec)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    genericUnwind(vm, exec);
}

// This function "should" just take the ExecState*, but doing so would make it more difficult
// to call from exception check sites. So, unlike all of our other functions, we allow
// ourselves to play some gnarly ABI tricks just to simplify the calling convention. This is
// particularly safe here since this is never called on the critical path - it's only for
// testing.
void JIT_OPERATION operationExceptionFuzz(ExecState* exec)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);
    UNUSED_PARAM(scope);
#if COMPILER(GCC_OR_CLANG)
    void* returnPC = __builtin_return_address(0);
    doExceptionFuzzing(exec, scope, "JITOperations", returnPC);
#endif // COMPILER(GCC_OR_CLANG)
}

EncodedJSValue JIT_OPERATION operationHasGenericProperty(ExecState* exec, EncodedJSValue encodedBaseValue, JSCell* propertyName)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue baseValue = JSValue::decode(encodedBaseValue);
    if (baseValue.isUndefinedOrNull())
        return JSValue::encode(jsBoolean(false));

    JSObject* base = baseValue.toObject(exec);
    if (!base)
        return JSValue::encode(JSValue());
    return JSValue::encode(jsBoolean(base->hasPropertyGeneric(exec, asString(propertyName)->toIdentifier(exec), PropertySlot::InternalMethodType::GetOwnProperty)));
}

EncodedJSValue JIT_OPERATION operationHasIndexedProperty(ExecState* exec, JSCell* baseCell, int32_t subscript, int32_t internalMethodType)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSObject* object = baseCell->toObject(exec, exec->lexicalGlobalObject());
    return JSValue::encode(jsBoolean(object->hasPropertyGeneric(exec, subscript, static_cast<PropertySlot::InternalMethodType>(internalMethodType))));
}
    
JSCell* JIT_OPERATION operationGetPropertyEnumerator(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSObject* base = cell->toObject(exec, exec->lexicalGlobalObject());

    return propertyNameEnumerator(exec, base);
}

EncodedJSValue JIT_OPERATION operationNextEnumeratorPname(ExecState* exec, JSCell* enumeratorCell, int32_t index)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSPropertyNameEnumerator* enumerator = jsCast<JSPropertyNameEnumerator*>(enumeratorCell);
    JSString* propertyName = enumerator->propertyNameAtIndex(index);
    return JSValue::encode(propertyName ? propertyName : jsNull());
}

JSCell* JIT_OPERATION operationToIndexString(ExecState* exec, int32_t index)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return jsString(exec, Identifier::from(exec, index).string());
}

ALWAYS_INLINE static EncodedJSValue unprofiledAdd(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    return JSValue::encode(jsAdd(exec, op1, op2));
}

ALWAYS_INLINE static EncodedJSValue profiledAdd(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile& arithProfile)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    arithProfile.observeLHSAndRHS(op1, op2);
    JSValue result = jsAdd(exec, op1, op2);
    arithProfile.observeResult(result);

    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationValueAdd(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    return unprofiledAdd(exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueAddProfiled(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile* arithProfile)
{
    ASSERT(arithProfile);
    return profiledAdd(exec, encodedOp1, encodedOp2, *arithProfile);
}

EncodedJSValue JIT_OPERATION operationValueAddProfiledOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC* addIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    ArithProfile* arithProfile = addIC->arithProfile();
    ASSERT(arithProfile);
    arithProfile->observeLHSAndRHS(op1, op2);
    auto nonOptimizeVariant = operationValueAddProfiledNoOptimize;
    addIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif
    
    JSValue result = jsAdd(exec, op1, op2);
    arithProfile->observeResult(result);

    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationValueAddProfiledNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC* addIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ArithProfile* arithProfile = addIC->arithProfile();
    ASSERT(arithProfile);
    return profiledAdd(exec, encodedOp1, encodedOp2, *arithProfile);
}

EncodedJSValue JIT_OPERATION operationValueAddOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC* addIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    auto nonOptimizeVariant = operationValueAddNoOptimize;
    if (ArithProfile* arithProfile = addIC->arithProfile())
        arithProfile->observeLHSAndRHS(op1, op2);
    addIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    return JSValue::encode(jsAdd(exec, op1, op2));
}

EncodedJSValue JIT_OPERATION operationValueAddNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITAddIC*)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    JSValue result = jsAdd(exec, op1, op2);

    return JSValue::encode(result);
}

ALWAYS_INLINE static EncodedJSValue unprofiledMul(VM& vm, ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    double b = op2.toNumber(exec);
    return JSValue::encode(jsNumber(a * b));
}

ALWAYS_INLINE static EncodedJSValue profiledMul(VM& vm, ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile& arithProfile, bool shouldObserveLHSAndRHSTypes = true)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    if (shouldObserveLHSAndRHSTypes)
        arithProfile.observeLHSAndRHS(op1, op2);

    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    double b = op2.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    JSValue result = jsNumber(a * b);
    arithProfile.observeResult(result);
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationValueMul(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return unprofiledMul(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueMulNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC*)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return unprofiledMul(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueMulOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC* mulIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    auto nonOptimizeVariant = operationValueMulNoOptimize;
    if (ArithProfile* arithProfile = mulIC->arithProfile())
        arithProfile->observeLHSAndRHS(JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
    mulIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    return unprofiledMul(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueMulProfiled(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile* arithProfile)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ASSERT(arithProfile);
    return profiledMul(*vm, exec, encodedOp1, encodedOp2, *arithProfile);
}

EncodedJSValue JIT_OPERATION operationValueMulProfiledOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC* mulIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ArithProfile* arithProfile = mulIC->arithProfile();
    ASSERT(arithProfile);
    arithProfile->observeLHSAndRHS(JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
    auto nonOptimizeVariant = operationValueMulProfiledNoOptimize;
    mulIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    return profiledMul(*vm, exec, encodedOp1, encodedOp2, *arithProfile, false);
}

EncodedJSValue JIT_OPERATION operationValueMulProfiledNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITMulIC* mulIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ArithProfile* arithProfile = mulIC->arithProfile();
    ASSERT(arithProfile);
    return profiledMul(*vm, exec, encodedOp1, encodedOp2, *arithProfile);
}

ALWAYS_INLINE static EncodedJSValue unprofiledNegate(ExecState* exec, EncodedJSValue encodedOperand)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    NativeCallFrameTracer tracer(&vm, exec);
    
    JSValue operand = JSValue::decode(encodedOperand);
    double number = operand.toNumber(exec);
    if (UNLIKELY(scope.exception()))
        return JSValue::encode(JSValue());
    return JSValue::encode(jsNumber(-number));
}

ALWAYS_INLINE static EncodedJSValue profiledNegate(ExecState* exec, EncodedJSValue encodedOperand, ArithProfile& arithProfile)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue operand = JSValue::decode(encodedOperand);
    arithProfile.observeLHS(operand);
    double number = operand.toNumber(exec);
    if (UNLIKELY(scope.exception()))
        return JSValue::encode(JSValue());

    JSValue result = jsNumber(-number);
    arithProfile.observeResult(result);
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationArithNegate(ExecState* exec, EncodedJSValue operand)
{
    return unprofiledNegate(exec, operand);
}

EncodedJSValue JIT_OPERATION operationArithNegateProfiled(ExecState* exec, EncodedJSValue operand, ArithProfile* arithProfile)
{
    ASSERT(arithProfile);
    return profiledNegate(exec, operand, *arithProfile);
}

EncodedJSValue JIT_OPERATION operationArithNegateProfiledOptimize(ExecState* exec, EncodedJSValue encodedOperand, JITNegIC* negIC)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    NativeCallFrameTracer tracer(&vm, exec);
    
    JSValue operand = JSValue::decode(encodedOperand);

    ArithProfile* arithProfile = negIC->arithProfile();
    ASSERT(arithProfile);
    arithProfile->observeLHS(operand);
    negIC->generateOutOfLine(vm, exec->codeBlock(), operationArithNegateProfiled);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif
    
    double number = operand.toNumber(exec);
    if (UNLIKELY(scope.exception()))
        return JSValue::encode(JSValue());
    JSValue result = jsNumber(-number);
    arithProfile->observeResult(result);
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationArithNegateOptimize(ExecState* exec, EncodedJSValue encodedOperand, JITNegIC* negIC)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue operand = JSValue::decode(encodedOperand);

    if (ArithProfile* arithProfile = negIC->arithProfile())
        arithProfile->observeLHS(operand);
    negIC->generateOutOfLine(vm, exec->codeBlock(), operationArithNegate);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    double number = operand.toNumber(exec);
    if (UNLIKELY(scope.exception()))
        return JSValue::encode(JSValue());
    return JSValue::encode(jsNumber(-number));
}

ALWAYS_INLINE static EncodedJSValue unprofiledSub(VM& vm, ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    double b = op2.toNumber(exec);
    return JSValue::encode(jsNumber(a - b));
}

ALWAYS_INLINE static EncodedJSValue profiledSub(VM& vm, ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile& arithProfile, bool shouldObserveLHSAndRHSTypes = true)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    if (shouldObserveLHSAndRHSTypes)
        arithProfile.observeLHSAndRHS(op1, op2);

    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    double b = op2.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    JSValue result = jsNumber(a - b);
    arithProfile.observeResult(result);
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationValueSub(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    return unprofiledSub(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueSubProfiled(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, ArithProfile* arithProfile)
{
    ASSERT(arithProfile);

    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return profiledSub(*vm, exec, encodedOp1, encodedOp2, *arithProfile);
}

EncodedJSValue JIT_OPERATION operationValueSubOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC* subIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    auto nonOptimizeVariant = operationValueSubNoOptimize;
    if (ArithProfile* arithProfile = subIC->arithProfile())
        arithProfile->observeLHSAndRHS(JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
    subIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    return unprofiledSub(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueSubNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC*)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return unprofiledSub(*vm, exec, encodedOp1, encodedOp2);
}

EncodedJSValue JIT_OPERATION operationValueSubProfiledOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC* subIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ArithProfile* arithProfile = subIC->arithProfile();
    ASSERT(arithProfile);
    arithProfile->observeLHSAndRHS(JSValue::decode(encodedOp1), JSValue::decode(encodedOp2));
    auto nonOptimizeVariant = operationValueSubProfiledNoOptimize;
    subIC->generateOutOfLine(*vm, exec->codeBlock(), nonOptimizeVariant);

#if ENABLE(MATH_IC_STATS)
    exec->codeBlock()->dumpMathICStats();
#endif

    return profiledSub(*vm, exec, encodedOp1, encodedOp2, *arithProfile, false);
}

EncodedJSValue JIT_OPERATION operationValueSubProfiledNoOptimize(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2, JITSubIC* subIC)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    ArithProfile* arithProfile = subIC->arithProfile();
    ASSERT(arithProfile);
    return profiledSub(*vm, exec, encodedOp1, encodedOp2, *arithProfile);
}

void JIT_OPERATION operationProcessTypeProfilerLog(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    vm.typeProfilerLog()->processLogEntries(ASCIILiteral("Log Full, called from inside baseline JIT"));
}

void JIT_OPERATION operationProcessShadowChickenLog(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    vm.shadowChicken().update(vm, exec);
}

int32_t JIT_OPERATION operationCheckIfExceptionIsUncatchableAndNotifyProfiler(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);
    RELEASE_ASSERT(!!scope.exception());

    if (isTerminatedExecutionException(vm, scope.exception())) {
        genericUnwind(&vm, exec);
        return 1;
    }
    return 0;
}

} // extern "C"

// Note: getHostCallReturnValueWithExecState() needs to be placed before the
// definition of getHostCallReturnValue() below because the Windows build
// requires it.
extern "C" EncodedJSValue HOST_CALL_RETURN_VALUE_OPTION getHostCallReturnValueWithExecState(ExecState* exec)
{
    if (!exec)
        return JSValue::encode(JSValue());
    return JSValue::encode(exec->vm().hostCallReturnValue);
}

#if COMPILER(GCC_OR_CLANG) && CPU(X86_64)
asm (
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "lea -8(%rsp), %rdi\n"
    "jmp " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
);

#elif COMPILER(GCC_OR_CLANG) && CPU(X86)
asm (
".text" "\n" \
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "push %ebp\n"
    "mov %esp, %eax\n"
    "leal -4(%esp), %esp\n"
    "push %eax\n"
    "call " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
    "leal 8(%esp), %esp\n"
    "pop %ebp\n"
    "ret\n"
);

#elif COMPILER(GCC_OR_CLANG) && CPU(ARM_THUMB2)
asm (
".text" "\n"
".align 2" "\n"
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
".thumb" "\n"
".thumb_func " THUMB_FUNC_PARAM(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "sub r0, sp, #8" "\n"
    "b " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
);

#elif COMPILER(GCC_OR_CLANG) && CPU(ARM_TRADITIONAL)
asm (
".text" "\n"
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
INLINE_ARM_FUNCTION(getHostCallReturnValue)
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    "sub r0, sp, #8" "\n"
    "b " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
);

#elif CPU(ARM64)
asm (
".text" "\n"
".align 2" "\n"
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
     "sub x0, sp, #16" "\n"
     "b " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
);

#elif COMPILER(GCC_OR_CLANG) && CPU(MIPS)

#if WTF_MIPS_PIC
#define LOAD_FUNCTION_TO_T9(function) \
        ".set noreorder" "\n" \
        ".cpload $25" "\n" \
        ".set reorder" "\n" \
        "la $t9, " LOCAL_REFERENCE(function) "\n"
#else
#define LOAD_FUNCTION_TO_T9(function) "" "\n"
#endif

asm (
".text" "\n"
".globl " SYMBOL_STRING(getHostCallReturnValue) "\n"
HIDE_SYMBOL(getHostCallReturnValue) "\n"
SYMBOL_STRING(getHostCallReturnValue) ":" "\n"
    LOAD_FUNCTION_TO_T9(getHostCallReturnValueWithExecState)
    "addi $a0, $sp, -8" "\n"
    "b " LOCAL_REFERENCE(getHostCallReturnValueWithExecState) "\n"
);

#elif COMPILER(MSVC) && CPU(X86)
extern "C" {
    __declspec(naked) EncodedJSValue HOST_CALL_RETURN_VALUE_OPTION getHostCallReturnValue()
    {
        __asm lea eax, [esp - 4]
        __asm mov [esp + 4], eax;
        __asm jmp getHostCallReturnValueWithExecState
    }
}
#endif

} // namespace JSC

#endif // ENABLE(JIT)
