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

#include "CatchScope.h"
#include "ErrorInstance.h"
#include "JSObject.h"
#include "ThrowScope.h"

namespace JSC {

typedef JSObject* (*ErrorFactory)(ExecState*, const String&, ErrorInstance::SourceAppender);

String defaultSourceAppender(const String&, const String&, RuntimeType, ErrorInstance::SourceTextWhereErrorOccurred);

JSObject* createTerminatedExecutionException(VM*);
JS_EXPORT_PRIVATE bool isTerminatedExecutionException(VM&, Exception*);
JS_EXPORT_PRIVATE JSObject* createError(ExecState*, JSValue, const String&, ErrorInstance::SourceAppender);
JS_EXPORT_PRIVATE JSObject* createStackOverflowError(ExecState*);
JSObject* createUndefinedVariableError(ExecState*, const Identifier&);
JSObject* createTDZError(ExecState*);
JSObject* createNotAnObjectError(ExecState*, JSValue);
JSObject* createInvalidFunctionApplyParameterError(ExecState*, JSValue);
JSObject* createInvalidInParameterError(ExecState*, JSValue);
JSObject* createInvalidInstanceofParameterErrorNotFunction(ExecState*, JSValue);
JSObject* createInvalidInstanceofParameterErrorhasInstanceValueNotFunction(ExecState*, JSValue);
JSObject* createNotAConstructorError(ExecState*, JSValue);
JSObject* createNotAFunctionError(ExecState*, JSValue);
JSObject* createErrorForInvalidGlobalAssignment(ExecState*, const String&);
JSString* errorDescriptionForValue(ExecState*, JSValue);

JS_EXPORT_PRIVATE JSObject* throwOutOfMemoryError(ExecState*, ThrowScope&);
JS_EXPORT_PRIVATE JSObject* throwStackOverflowError(ExecState*, ThrowScope&);
JS_EXPORT_PRIVATE JSObject* throwTerminatedExecutionException(ExecState*, ThrowScope&);


class TerminatedExecutionError final : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static TerminatedExecutionError* create(VM& vm)
    {
        TerminatedExecutionError* error = new (NotNull, allocateCell<TerminatedExecutionError>(vm.heap)) TerminatedExecutionError(vm);
        error->finishCreation(vm);
        return error;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

private:
    explicit TerminatedExecutionError(VM& vm)
        : JSNonFinalObject(vm, vm.terminatedExecutionErrorStructure.get())
    {
    }

    static JSValue defaultValue(const JSObject*, ExecState*, PreferredPrimitiveType);

};

} // namespace JSC
