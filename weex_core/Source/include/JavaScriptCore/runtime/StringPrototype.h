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

#include "JITOperations.h"
#include "StringObject.h"

namespace JSC {

class ObjectPrototype;
class RegExp;
class RegExpObject;

class StringPrototype : public StringObject {
private:
    StringPrototype(VM&, Structure*);

public:
    typedef StringObject Base;
    static const unsigned StructureFlags = HasStaticPropertyTable | Base::StructureFlags;

    static StringPrototype* create(VM&, JSGlobalObject*, Structure*);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;

protected:
    void finishCreation(VM&, JSGlobalObject*, JSString*);
};

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceGeneric(
    ExecState*, EncodedJSValue thisValue, EncodedJSValue searchValue, EncodedJSValue replaceValue);

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceRegExpEmptyStr(
    ExecState*, JSString* thisValue, RegExpObject* searchValue);

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceRegExpString(
    ExecState*, JSString* thisValue, RegExpObject* searchValue, JSString* replaceValue);

String substituteBackreferences(const String& replacement, StringView source, const int* ovector, RegExp* reg);

EncodedJSValue JSC_HOST_CALL stringProtoFuncRepeatCharacter(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSplitFast(ExecState*);

EncodedJSValue JSC_HOST_CALL builtinStringSubstrInternal(ExecState*);
EncodedJSValue JSC_HOST_CALL builtinStringIncludesInternal(ExecState*);

} // namespace JSC
