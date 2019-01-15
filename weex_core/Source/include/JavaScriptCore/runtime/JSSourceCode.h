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
#include "SourceCode.h"

namespace JSC {

class JSSourceCode : public JSCell {
public:
    using Base = JSCell;

    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;
    static const bool needsDestruction = true;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSSourceCodeType, StructureFlags), info());
    }

    static JSSourceCode* create(VM& vm, Structure* structure, SourceCode&& sourceCode)
    {
        auto* result = new (NotNull, allocateCell<JSSourceCode>(vm.heap)) JSSourceCode(vm, structure, WTFMove(sourceCode));
        result->finishCreation(vm);
        return result;
    }

    static JSSourceCode* create(VM& vm, SourceCode&& sourceCode)
    {
        return create(vm, vm.sourceCodeStructure.get(), WTFMove(sourceCode));
    }

    const SourceCode& sourceCode() const
    {
        return m_sourceCode;
    }

    static void destroy(JSCell*);

private:
    JSSourceCode(VM& vm, Structure* structure, SourceCode&& sourceCode)
        : Base(vm, structure)
        , m_sourceCode(WTFMove(sourceCode))
    {
    }

    SourceCode m_sourceCode;
};

} // namespace JSC
