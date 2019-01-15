/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
