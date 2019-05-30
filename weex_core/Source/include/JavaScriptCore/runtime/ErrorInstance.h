/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008, 2016 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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
