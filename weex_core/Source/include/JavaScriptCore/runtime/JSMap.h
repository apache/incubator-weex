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

#include "JSObject.h"
#include "MapBase.h"

namespace JSC {

class JSMapIterator;

class JSMap : public MapBase<HashMapBucket<HashMapBucketDataKeyValue>> {
    typedef MapBase<HashMapBucket<HashMapBucketDataKeyValue>> Base;
public:
    friend class JSMapIterator;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSMapType, StructureFlags), info());
    }

    static JSMap* create(ExecState* exec, VM& vm, Structure* structure)
    {
        JSMap* instance = new (NotNull, allocateCell<JSMap>(vm.heap)) JSMap(vm, structure);
        instance->finishCreation(exec, vm);
        return instance;
    }

    ALWAYS_INLINE JSValue get(ExecState* exec, JSValue key)
    {
        return m_map->get(exec, key);
    }

    ALWAYS_INLINE void set(ExecState* exec, JSValue key, JSValue value)
    {
        m_map->add(exec, key, value);
    }

private:
    JSMap(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    static String toStringName(const JSObject*, ExecState*);
};

} // namespace JSC
