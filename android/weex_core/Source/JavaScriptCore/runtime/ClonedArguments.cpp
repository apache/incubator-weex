/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "ClonedArguments.h"

#include "GetterSetter.h"
#include "InlineCallFrame.h"
#include "JSCInlines.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ClonedArguments);

const ClassInfo ClonedArguments::s_info = { "Arguments", &Base::s_info, 0, CREATE_METHOD_TABLE(ClonedArguments) };

ClonedArguments::ClonedArguments(VM& vm, Structure* structure, Butterfly* butterfly)
    : Base(vm, structure, butterfly)
{
}

ClonedArguments* ClonedArguments::createEmpty(
    VM& vm, Structure* structure, JSFunction* callee, unsigned length)
{
    unsigned vectorLength = length;
    if (vectorLength > MAX_STORAGE_VECTOR_LENGTH)
        return 0;

    Butterfly* butterfly;
    if (UNLIKELY(structure->needsSlowPutIndexing())) {
        butterfly = createArrayStorageButterfly(vm, nullptr, structure, length, vectorLength);
        butterfly->arrayStorage()->m_numValuesInVector = vectorLength;

    } else {
        void* temp = vm.auxiliarySpace.tryAllocate(Butterfly::totalSize(0, structure->outOfLineCapacity(), true, vectorLength * sizeof(EncodedJSValue)));
        if (!temp)
            return 0;
        butterfly = Butterfly::fromBase(temp, 0, structure->outOfLineCapacity());
        butterfly->setVectorLength(vectorLength);
        butterfly->setPublicLength(length);
        
        for (unsigned i = length; i < vectorLength; ++i)
            butterfly->contiguous()[i].clear();
    }
    
    ClonedArguments* result =
        new (NotNull, allocateCell<ClonedArguments>(vm.heap))
        ClonedArguments(vm, structure, butterfly);
    result->finishCreation(vm);

    result->m_callee.set(vm, result, callee);
    result->putDirect(vm, clonedArgumentsLengthPropertyOffset, jsNumber(length));
    return result;
}

ClonedArguments* ClonedArguments::createEmpty(ExecState* exec, JSFunction* callee, unsigned length)
{
    VM& vm = exec->vm();
    // NB. Some clients might expect that the global object of of this object is the global object
    // of the callee. We don't do this for now, but maybe we should.
    ClonedArguments* result = createEmpty(vm, exec->lexicalGlobalObject()->clonedArgumentsStructure(), callee, length);
    ASSERT(!result->structure(vm)->needsSlowPutIndexing() || shouldUseSlowPut(result->structure(vm)->indexingType()));
    return result;
}

ClonedArguments* ClonedArguments::createWithInlineFrame(ExecState* myFrame, ExecState* targetFrame, InlineCallFrame* inlineCallFrame, ArgumentsMode mode)
{
    VM& vm = myFrame->vm();
    
    JSFunction* callee;
    
    if (inlineCallFrame)
        callee = jsCast<JSFunction*>(inlineCallFrame->calleeRecovery.recover(targetFrame));
    else
        callee = jsCast<JSFunction*>(targetFrame->jsCallee());

    ClonedArguments* result = nullptr;
    
    unsigned length = 0; // Initialize because VC needs it.
    switch (mode) {
    case ArgumentsMode::Cloned: {
        if (inlineCallFrame) {
            if (inlineCallFrame->argumentCountRegister.isValid())
                length = targetFrame->r(inlineCallFrame->argumentCountRegister).unboxedInt32();
            else
                length = inlineCallFrame->arguments.size();
            length--;
            result = createEmpty(myFrame, callee, length);

            for (unsigned i = length; i--;)
                result->initializeIndex(vm, i, inlineCallFrame->arguments[i + 1].recover(targetFrame));
        } else {
            length = targetFrame->argumentCount();
            result = createEmpty(myFrame, callee, length);

            for (unsigned i = length; i--;)
                result->initializeIndex(vm, i, targetFrame->uncheckedArgument(i));
        }
        break;
    }
        
    case ArgumentsMode::FakeValues: {
        result = createEmpty(myFrame, callee, 0);
        break;
    } }

    ASSERT(myFrame->lexicalGlobalObject()->clonedArgumentsStructure() == result->structure());
    ASSERT(!result->structure(vm)->needsSlowPutIndexing() || shouldUseSlowPut(result->structure(vm)->indexingType()));
    return result;
}

ClonedArguments* ClonedArguments::createWithMachineFrame(ExecState* myFrame, ExecState* targetFrame, ArgumentsMode mode)
{
    ClonedArguments* result = createWithInlineFrame(myFrame, targetFrame, nullptr, mode);
    ASSERT(!result->structure()->needsSlowPutIndexing() || shouldUseSlowPut(result->structure()->indexingType()));
    return result;
}

ClonedArguments* ClonedArguments::createByCopyingFrom(
    ExecState* exec, Structure* structure, Register* argumentStart, unsigned length,
    JSFunction* callee)
{
    VM& vm = exec->vm();
    ClonedArguments* result = createEmpty(vm, structure, callee, length);
    
    for (unsigned i = length; i--;)
        result->initializeIndex(vm, i, argumentStart[i].jsValue());
    ASSERT(!result->structure(vm)->needsSlowPutIndexing() || shouldUseSlowPut(result->structure(vm)->indexingType()));
    return result;
}

Structure* ClonedArguments::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype, IndexingType indexingType)
{
    Structure* structure = Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info(), indexingType);
    PropertyOffset offset;
    structure = structure->addPropertyTransition(vm, structure, vm.propertyNames->length, DontEnum, offset);
    ASSERT(offset == clonedArgumentsLengthPropertyOffset);
    return structure;
}

Structure* ClonedArguments::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    // We use contiguous storage because optimizations in the FTL assume that cloned arguments creation always produces the same initial structure.
    return createStructure(vm, globalObject, prototype, NonArrayWithContiguous);
}

Structure* ClonedArguments::createSlowPutStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return createStructure(vm, globalObject, prototype, NonArrayWithSlowPutArrayStorage);
}

bool ClonedArguments::getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName ident, PropertySlot& slot)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(object);
    VM& vm = exec->vm();

    if (!thisObject->specialsMaterialized()) {
        FunctionExecutable* executable = jsCast<FunctionExecutable*>(thisObject->m_callee->executable());
        bool isStrictMode = executable->isStrictMode();

        if (ident == vm.propertyNames->callee) {
            if (isStrictMode) {
                slot.setGetterSlot(thisObject, DontDelete | DontEnum | Accessor, thisObject->globalObject()->throwTypeErrorArgumentsCalleeAndCallerGetterSetter());
                return true;
            }
            slot.setValue(thisObject, 0, thisObject->m_callee.get());
            return true;
        }

        if (ident == vm.propertyNames->iteratorSymbol) {
            slot.setValue(thisObject, DontEnum, thisObject->globalObject()->arrayProtoValuesFunction());
            return true;
        }
    }

    return Base::getOwnPropertySlot(thisObject, exec, ident, slot);
}

void ClonedArguments::getOwnPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& array, EnumerationMode mode)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(object);
    thisObject->materializeSpecialsIfNecessary(exec);
    Base::getOwnPropertyNames(thisObject, exec, array, mode);
}

bool ClonedArguments::put(JSCell* cell, ExecState* exec, PropertyName ident, JSValue value, PutPropertySlot& slot)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(cell);
    VM& vm = exec->vm();
    
    if (ident == vm.propertyNames->callee
        || ident == vm.propertyNames->iteratorSymbol) {
        thisObject->materializeSpecialsIfNecessary(exec);
        PutPropertySlot dummy = slot; // Shadow the given PutPropertySlot to prevent caching.
        return Base::put(thisObject, exec, ident, value, dummy);
    }
    
    return Base::put(thisObject, exec, ident, value, slot);
}

bool ClonedArguments::deleteProperty(JSCell* cell, ExecState* exec, PropertyName ident)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(cell);
    VM& vm = exec->vm();
    
    if (ident == vm.propertyNames->callee
        || ident == vm.propertyNames->iteratorSymbol)
        thisObject->materializeSpecialsIfNecessary(exec);
    
    return Base::deleteProperty(thisObject, exec, ident);
}

bool ClonedArguments::defineOwnProperty(JSObject* object, ExecState* exec, PropertyName ident, const PropertyDescriptor& descriptor, bool shouldThrow)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(object);
    VM& vm = exec->vm();
    
    if (ident == vm.propertyNames->callee
        || ident == vm.propertyNames->iteratorSymbol)
        thisObject->materializeSpecialsIfNecessary(exec);
    
    return Base::defineOwnProperty(object, exec, ident, descriptor, shouldThrow);
}

void ClonedArguments::materializeSpecials(ExecState* exec)
{
    RELEASE_ASSERT(!specialsMaterialized());
    VM& vm = exec->vm();
    
    FunctionExecutable* executable = jsCast<FunctionExecutable*>(m_callee->executable());
    bool isStrictMode = executable->isStrictMode();
    
    if (isStrictMode)
        putDirectAccessor(exec, vm.propertyNames->callee, globalObject()->throwTypeErrorArgumentsCalleeAndCallerGetterSetter(), DontDelete | DontEnum | Accessor);
    else
        putDirect(vm, vm.propertyNames->callee, JSValue(m_callee.get()));

    putDirect(vm, vm.propertyNames->iteratorSymbol, globalObject()->arrayProtoValuesFunction(), DontEnum);
    
    m_callee.clear();
}

void ClonedArguments::materializeSpecialsIfNecessary(ExecState* exec)
{
    if (!specialsMaterialized())
        materializeSpecials(exec);
}

void ClonedArguments::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    ClonedArguments* thisObject = jsCast<ClonedArguments*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_callee);
}

} // namespace JSC

