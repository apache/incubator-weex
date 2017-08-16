/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007-2008, 2013, 2016 Apple Inc. All rights reserved.
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
