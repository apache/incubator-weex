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

#include "IterationKind.h"
#include "JSMap.h"
#include "JSObject.h"

namespace JSC {

class JSMapIterator : public JSNonFinalObject {
    typedef HashMapBucket<HashMapBucketDataKeyValue> HashMapBucketType;
public:
    typedef JSNonFinalObject Base;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSMapIterator* create(VM& vm, Structure* structure, JSMap* iteratedObject, IterationKind kind)
    {
        JSMapIterator* instance = new (NotNull, allocateCell<JSMapIterator>(vm.heap)) JSMapIterator(vm, structure, iteratedObject, kind);
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

        if (m_kind == IterateValue)
            value = bucket->value();
        else if (m_kind == IterateKey)
            value = bucket->key();
        else
            value = createPair(exec, bucket->key(), bucket->value());
        return true;
    }

    bool nextKeyValue(ExecState* exec, JSValue& key, JSValue& value)
    {
        HashMapBucketType* bucket = advanceIter(exec);
        if (!bucket)
            return false;

        key = bucket->key();
        value = bucket->value();
        return true;
    }

    IterationKind kind() const { return m_kind; }
    JSValue iteratedValue() const { return m_map.get(); }
    JSMapIterator* clone(ExecState*);

private:
    JSMapIterator(VM& vm, Structure* structure, JSMap*, IterationKind kind)
        : Base(vm, structure)
        , m_kind(kind)
    { }

    void setIterator(VM& vm, HashMapBucketType* bucket)
    {
        m_iter.setMayBeNull(vm, this, bucket);
    }

    JS_EXPORT_PRIVATE void finishCreation(VM&, JSMap*);
    JSValue createPair(CallFrame*, JSValue, JSValue);
    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<JSMap> m_map;
    WriteBarrier<HashMapBucketType> m_iter;
    IterationKind m_kind;
};
STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSMapIterator);

} // namespace JSC
