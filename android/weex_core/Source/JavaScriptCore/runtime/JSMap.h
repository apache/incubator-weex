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
