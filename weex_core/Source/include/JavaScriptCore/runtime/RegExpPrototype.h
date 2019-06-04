/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2007-2008, 2016 Apple Inc. All Rights Reserved.
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
#include "RegExp.h"

namespace JSC {

class RegExpPrototype : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static RegExpPrototype* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        RegExpPrototype* prototype = new (NotNull, allocateCell<RegExpPrototype>(vm.heap)) RegExpPrototype(vm, structure);
        prototype->finishCreation(vm, globalObject);
        return prototype;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);

    RegExp* emptyRegExp() const { return m_emptyRegExp.get(); }

protected:
    RegExpPrototype(VM&, Structure*);

private:
    void finishCreation(VM&, JSGlobalObject*);

    WriteBarrier<RegExp> m_emptyRegExp;
};

EncodedJSValue JSC_HOST_CALL regExpProtoFuncMatchFast(ExecState*);
EncodedJSValue JSC_HOST_CALL regExpProtoFuncSearchFast(ExecState*);
EncodedJSValue JSC_HOST_CALL regExpProtoFuncSplitFast(ExecState*);
EncodedJSValue JSC_HOST_CALL regExpProtoFuncTestFast(ExecState*);

} // namespace JSC
