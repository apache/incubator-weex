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

#pragma once

#include "JSGlobalObject.h"
#include "JSObject.h"

namespace JSC {

class JSFixedArray : public JSCell {
    typedef JSCell Base;

public:
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFixedArrayType, StructureFlags), info());
    }

    ALWAYS_INLINE static JSFixedArray* createFromArray(ExecState* exec, VM& vm, JSArray* array)
    {
        IndexingType indexingType = array->indexingType() & IndexingShapeMask;
        unsigned length = array->length();
        JSFixedArray* result = JSFixedArray::create(vm, vm.fixedArrayStructure.get(), length);

        if (!length)
            return result;

        if (indexingType == ContiguousShape || indexingType == Int32Shape) {
            for (unsigned i = 0; i < length; i++) {
                JSValue value = array->butterfly()->contiguous()[i].get();
                value = !!value ? value : jsUndefined();
                result->buffer()[i].set(vm, result, value);
            }
            return result;
        }

        if (indexingType == DoubleShape) {
            for (unsigned i = 0; i < length; i++) {
                double d = array->butterfly()->contiguousDouble()[i];
                JSValue value = std::isnan(d) ? jsUndefined() : JSValue(JSValue::EncodeAsDouble, d);
                result->buffer()[i].set(vm, result, value);
            }
            return result;
        }


        auto throwScope = DECLARE_THROW_SCOPE(vm);
        for (unsigned i = 0; i < length; i++) {
            JSValue value = array->getDirectIndex(exec, i);
            if (!value) {
                // When we see a hole, we assume that it's safe to assume the get would have returned undefined.
                // We may still call into this function when !globalObject->isArrayIteratorProtocolFastAndNonObservable(),
                // however, if we do that, we ensure we're calling in with an array with all self properties between
                // [0, length).
                //
                // We may also call into this during OSR exit to materialize a phantom fixed array.
                // We may be creating a fixed array during OSR exit even after the iterator protocol changed.
                // But, when the phantom would have logically been created, the protocol hadn't been
                // changed. Therefore, it is sound to assume empty indices are jsUndefined().
                value = jsUndefined();
            }
            RETURN_IF_EXCEPTION(throwScope, nullptr);
            result->buffer()[i].set(vm, result, value);
        }
        return result;
    }

    ALWAYS_INLINE JSValue get(unsigned index)
    {
        ASSERT(index < m_size);
        return buffer()[index].get();
    }

    ALWAYS_INLINE WriteBarrier<Unknown>* buffer() { return bitwise_cast<WriteBarrier<Unknown>*>(bitwise_cast<char*>(this) + offsetOfData()); }

    static void visitChildren(JSCell*, SlotVisitor&);

    unsigned size() const { return m_size; }

    static size_t offsetOfSize() { return OBJECT_OFFSETOF(JSFixedArray, m_size); }

    static size_t offsetOfData()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<Unknown>)>(sizeof(JSFixedArray));
    }

private:
    unsigned m_size;

    ALWAYS_INLINE static JSFixedArray* create(VM& vm, Structure* structure, unsigned size)
    {
        JSFixedArray* result = new (NotNull, allocateCell<JSFixedArray>(vm.heap, allocationSize(size))) JSFixedArray(vm, structure, size);
        result->finishCreation(vm);
        return result;
    }


    JSFixedArray(VM& vm, Structure* structure, unsigned size)
        : Base(vm, structure)
        , m_size(size)
    {
        for (unsigned i = 0; i < m_size; i++)
            buffer()[i].setStartingValue(JSValue());
    }


    static size_t allocationSize(unsigned numItems)
    {
        return offsetOfData() + numItems * sizeof(WriteBarrier<Unknown>);
    }
};

} // namespace JSC
