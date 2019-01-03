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
#include "RuntimeType.h"
#include "SourceProvider.h"

namespace JSC {

class ErrorInstance : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    enum SourceTextWhereErrorOccurred { FoundExactSource, FoundApproximateSource };
    typedef String (*SourceAppender) (const String& originalMessage, const String& sourceText, RuntimeType, SourceTextWhereErrorOccurred);

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ErrorInstanceType, StructureFlags), info());
    }

    static ErrorInstance* create(ExecState* exec, VM& vm, Structure* structure, const String& message, SourceAppender appender = nullptr, RuntimeType type = TypeNothing, bool useCurrentFrame = true)
    {
        ErrorInstance* instance = new (NotNull, allocateCell<ErrorInstance>(vm.heap)) ErrorInstance(vm, structure);
        instance->m_sourceAppender = appender;
        instance->m_runtimeTypeForCause = type;
        instance->finishCreation(exec, vm, message, useCurrentFrame);
        return instance;
    }

    static ErrorInstance* create(ExecState*, Structure*, JSValue message, SourceAppender = nullptr, RuntimeType = TypeNothing, bool useCurrentFrame = true);

    static void addErrorInfo(ExecState*, VM&, JSObject*, bool = true);

    bool hasSourceAppender() const { return !!m_sourceAppender; }
    SourceAppender sourceAppender() const { return m_sourceAppender; }
    void setSourceAppender(SourceAppender appender) { m_sourceAppender = appender; }
    void clearSourceAppender() { m_sourceAppender = nullptr; }
    void setRuntimeTypeForCause(RuntimeType type) { m_runtimeTypeForCause = type; }
    RuntimeType runtimeTypeForCause() const { return m_runtimeTypeForCause; }
    void clearRuntimeTypeForCause() { m_runtimeTypeForCause = TypeNothing; }

    JS_EXPORT_PRIVATE String sanitizedToString(ExecState*);

protected:
    explicit ErrorInstance(VM&, Structure*);

    void finishCreation(ExecState*, VM&, const String&, bool useCurrentFrame = true);

    SourceAppender m_sourceAppender { nullptr };
    RuntimeType m_runtimeTypeForCause { TypeNothing };
};

} // namespace JSC
