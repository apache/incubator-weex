/*
 * Copyright (C) 2013, 2016 Apple, Inc. All rights reserved.
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

#include "IterationKind.h"
#include "JSObject.h"
#include "JSSet.h"

namespace JSC {

class JSSetIterator : public JSNonFinalObject {
    typedef HashMapBucket<HashMapBucketDataKey> HashMapBucketType;
public:
    typedef JSNonFinalObject Base;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSSetIterator* create(VM& vm, Structure* structure, JSSet* iteratedObject, IterationKind kind)
    {
        JSSetIterator* instance = new (NotNull, allocateCell<JSSetIterator>(vm.heap)) JSSetIterator(vm, structure, iteratedObject, kind);
        instance->finishCreation(vm, iteratedObject);
        return instance;
    }

    ALWAYS_INLINE HashMapBucketType* advanceIter(ExecState* exec)
    {
        HashMapBucketType* prev = m_iter.get();
        if (!prev)
            return nullptr;
        HashMapBucketType* bucket = m_iter->next();
        while (bucket && bucket->deleted())
            bucket = bucket->next();
        if (!bucket) {
            setIterator(exec->vm(), nullptr);
            return nullptr;
        }
        setIterator(exec->vm(), bucket); // We keep m_iter on the last value since the first thing we do in this function is call next().
        return bucket;
    }

    bool next(ExecState* exec, JSValue& value)
    {
        HashMapBucketType* bucket = advanceIter(exec);
        if (!bucket)
            return false;

        if (m_kind == IterateValue || m_kind == IterateKey)
            value = bucket->key();
        else
            value = createPair(exec, bucket->key(), bucket->key());
        return true;
    }

    IterationKind kind() const { return m_kind; }
    JSValue iteratedValue() const { return m_set.get(); }
    JSSetIterator* clone(ExecState*);

private:
    JSSetIterator(VM& vm, Structure* structure, JSSet*, IterationKind kind)
        : Base(vm, structure)
        , m_kind(kind)
    {
    }

    void setIterator(VM& vm, HashMapBucketType* bucket)
    {
        m_iter.setMayBeNull(vm, this, bucket); 
    }

    JS_EXPORT_PRIVATE void finishCreation(VM&, JSSet*);
    JS_EXPORT_PRIVATE JSValue createPair(CallFrame*, JSValue, JSValue);
    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<JSSet> m_set;
    WriteBarrier<HashMapBucketType> m_iter;
    IterationKind m_kind;
};
STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSSetIterator);

} // namespace JSC
