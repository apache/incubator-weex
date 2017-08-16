/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
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

#include "InternalFunction.h"

namespace WTF {
class TextPosition;
}

namespace JSC {

class FunctionPrototype;

class FunctionConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

    static FunctionConstructor* create(VM& vm, Structure* structure, FunctionPrototype* functionPrototype)
    {
        FunctionConstructor* constructor = new (NotNull, allocateCell<FunctionConstructor>(vm.heap)) FunctionConstructor(vm, structure);
        constructor->finishCreation(vm, functionPrototype);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    { 
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info()); 
    }

private:
    FunctionConstructor(VM&, Structure*);
    void finishCreation(VM&, FunctionPrototype*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

enum class FunctionConstructionMode {
    Function,
    Generator,
    Async,
};

JSObject* constructFunction(ExecState*, JSGlobalObject*, const ArgList&, const Identifier& functionName, const SourceOrigin&, const String& sourceURL, const WTF::TextPosition&, FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());
JSObject* constructFunction(ExecState*, JSGlobalObject*, const ArgList&, FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());

JS_EXPORT_PRIVATE JSObject* constructFunctionSkippingEvalEnabledCheck(
    ExecState*, JSGlobalObject*, const ArgList&, const Identifier&, const SourceOrigin&,
    const String&, const WTF::TextPosition&, int overrideLineNumber = -1,
    FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());

} // namespace JSC
