/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "ErrorInstance.h"
#include "InternalFunction.h"

namespace JSC {

class ErrorPrototype;
class GetterSetter;

class ErrorConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

    static ErrorConstructor* create(VM& vm, Structure* structure, ErrorPrototype* errorPrototype, GetterSetter*)
    {
        ErrorConstructor* constructor = new (NotNull, allocateCell<ErrorConstructor>(vm.heap)) ErrorConstructor(vm, structure);
        constructor->finishCreation(vm, errorPrototype);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    { 
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info()); 
    }

protected:
    void finishCreation(VM&, ErrorPrototype*);
        
private:
    ErrorConstructor(VM&, Structure*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

} // namespace JSC
