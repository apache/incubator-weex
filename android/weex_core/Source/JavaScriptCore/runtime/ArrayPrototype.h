/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007, 2011, 2015 Apple Inc. All rights reserved.
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

#include "JSArray.h"
#include "Lookup.h"

namespace JSC {

class ArrayPrototypeAdaptiveInferredPropertyWatchpoint;

class ArrayPrototype : public JSArray {
private:
    ArrayPrototype(VM&, Structure*);

public:
    typedef JSArray Base;

    enum class SpeciesWatchpointStatus {
        Uninitialized,
        Initialized,
        Fired
    };

    static ArrayPrototype* create(VM&, JSGlobalObject*, Structure*);
        
    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(DerivedArrayType, StructureFlags), info(), ArrayClass);
    }

    void tryInitializeSpeciesWatchpoint(ExecState*);

    static const bool needsDestruction = false;
    // We don't need destruction since we use a finalizer.
    static void destroy(JSC::JSCell*);

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    // This bit is set if any user modifies the constructor property Array.prototype. This is used to optimize species creation for JSArrays.
    friend ArrayPrototypeAdaptiveInferredPropertyWatchpoint;
    std::unique_ptr<ArrayPrototypeAdaptiveInferredPropertyWatchpoint> m_constructorWatchpoint;
    std::unique_ptr<ArrayPrototypeAdaptiveInferredPropertyWatchpoint> m_constructorSpeciesWatchpoint;
};

EncodedJSValue JSC_HOST_CALL arrayProtoFuncToString(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncValues(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncConcatMemcpy(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncAppendMemcpy(ExecState*);

} // namespace JSC
