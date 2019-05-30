/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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

#include "JSWrapperObject.h"
#include "Symbol.h"

namespace JSC {

class SymbolObject : public JSWrapperObject {
public:
    typedef JSWrapperObject Base;

    static SymbolObject* create(VM& vm, Structure* structure)
    {
        Symbol* symbol = Symbol::create(vm);
        SymbolObject* object = new (NotNull, allocateCell<SymbolObject>(vm.heap)) SymbolObject(vm, structure);
        object->finishCreation(vm, symbol);
        return object;
    }
    static SymbolObject* create(VM& vm, Structure* structure, Symbol* symbol)
    {
        SymbolObject* object = new (NotNull, allocateCell<SymbolObject>(vm.heap)) SymbolObject(vm, structure);
        object->finishCreation(vm, symbol);
        return object;
    }
    static SymbolObject* create(VM&, JSGlobalObject*, Symbol*);

    DECLARE_EXPORT_INFO;

    Symbol* internalValue() const { return asSymbol(JSWrapperObject::internalValue()); }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSValue defaultValue(const JSObject*, ExecState*, PreferredPrimitiveType);

    static String toStringName(const JSObject*, ExecState*);

protected:
    JS_EXPORT_PRIVATE void finishCreation(VM&, Symbol*);
    JS_EXPORT_PRIVATE SymbolObject(VM&, Structure*);
};

} // namespace JSC
