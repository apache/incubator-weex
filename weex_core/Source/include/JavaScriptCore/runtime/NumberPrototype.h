/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008, 2011 Apple Inc. All rights reserved.
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

#include "NumberObject.h"

namespace JSC {

class NumberPrototype : public NumberObject {
public:
    typedef NumberObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static NumberPrototype* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        NumberPrototype* prototype = new (NotNull, allocateCell<NumberPrototype>(vm.heap)) NumberPrototype(vm, structure);
        prototype->finishCreation(vm, globalObject);
        return prototype;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(NumberObjectType, StructureFlags), info());
    }

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    NumberPrototype(VM&, Structure*);
};

EncodedJSValue JSC_HOST_CALL numberProtoFuncValueOf(ExecState*);

} // namespace JSC
