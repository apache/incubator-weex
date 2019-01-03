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

#include "ExecutableBase.h"

namespace JSC {
namespace DOMJIT {
class Signature;
}

class NativeExecutable final : public ExecutableBase {
    friend class JIT;
    friend class LLIntOffsetsExtractor;
public:
    typedef ExecutableBase Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static NativeExecutable* create(VM&, Ref<JITCode>&& callThunk, NativeFunction function, Ref<JITCode>&& constructThunk, NativeFunction constructor, Intrinsic, const DOMJIT::Signature*, const String& name);

    static void destroy(JSCell*);

    CodeBlockHash hashFor(CodeSpecializationKind) const;

    NativeFunction function() { return m_function; }
    NativeFunction constructor() { return m_constructor; }
        
    NativeFunction nativeFunctionFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return function();
        ASSERT(kind == CodeForConstruct);
        return constructor();
    }
        
    static ptrdiff_t offsetOfNativeFunctionFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return OBJECT_OFFSETOF(NativeExecutable, m_function);
        ASSERT(kind == CodeForConstruct);
        return OBJECT_OFFSETOF(NativeExecutable, m_constructor);
    }

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue proto);
        
    DECLARE_INFO;

    const String& name() const { return m_name; }
    const DOMJIT::Signature* signature() const { return m_signature; }

    const DOMJIT::Signature* signatureFor(CodeSpecializationKind kind) const
    {
        if (isCall(kind))
            return signature();
        return nullptr;
    }

protected:
    void finishCreation(VM&, Ref<JITCode>&& callThunk, Ref<JITCode>&& constructThunk, const String& name);

private:
    friend class ExecutableBase;

    NativeExecutable(VM&, NativeFunction function, NativeFunction constructor, Intrinsic, const DOMJIT::Signature*);

    NativeFunction m_function;
    NativeFunction m_constructor;
    const DOMJIT::Signature* m_signature;

    String m_name;
};

} // namespace JSC
