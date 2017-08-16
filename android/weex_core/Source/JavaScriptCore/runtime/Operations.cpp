/*
 * Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 * Copyright (C) 2008, 2016 Apple Inc. All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "Operations.h"

#include "Error.h"
#include "JSCInlines.h"
#include "JSObject.h"
#include "JSString.h"
#include <wtf/MathExtras.h>

namespace JSC {

bool JSValue::equalSlowCase(ExecState* exec, JSValue v1, JSValue v2)
{
    return equalSlowCaseInline(exec, v1, v2);
}

bool JSValue::strictEqualSlowCase(ExecState* exec, JSValue v1, JSValue v2)
{
    return strictEqualSlowCaseInline(exec, v1, v2);
}

NEVER_INLINE JSValue jsAddSlowCase(CallFrame* callFrame, JSValue v1, JSValue v2)
{
    // exception for the Date exception in defaultValue()
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue p1 = v1.toPrimitive(callFrame);
    RETURN_IF_EXCEPTION(scope, { });
    JSValue p2 = v2.toPrimitive(callFrame);
    RETURN_IF_EXCEPTION(scope, { });

    if (p1.isString()) {
        JSString* p2String = p2.toString(callFrame);
        RETURN_IF_EXCEPTION(scope, { });
        scope.release();
        return jsString(callFrame, asString(p1), p2String);
    }

    if (p2.isString()) {
        JSString* p1String = p1.toString(callFrame);
        RETURN_IF_EXCEPTION(scope, { });
        scope.release();
        return jsString(callFrame, p1String, asString(p2));
    }

    double p1Number = p1.toNumber(callFrame);
    RETURN_IF_EXCEPTION(scope, { });
    scope.release();
    return jsNumber(p1Number + p2.toNumber(callFrame));
}

JSValue jsTypeStringForValue(VM& vm, JSGlobalObject* globalObject, JSValue v)
{
    if (v.isUndefined())
        return vm.smallStrings.undefinedString();
    if (v.isBoolean())
        return vm.smallStrings.booleanString();
    if (v.isNumber())
        return vm.smallStrings.numberString();
    if (v.isString())
        return vm.smallStrings.stringString();
    if (v.isSymbol())
        return vm.smallStrings.symbolString();
    if (v.isObject()) {
        JSObject* object = asObject(v);
        // Return "undefined" for objects that should be treated
        // as null when doing comparisons.
        if (object->structure(vm)->masqueradesAsUndefined(globalObject))
            return vm.smallStrings.undefinedString();
        if (object->type() == JSFunctionType)
            return vm.smallStrings.functionString();
        if (object->inlineTypeFlags() & TypeOfShouldCallGetCallData) {
            CallData callData;
            JSObject* object = asObject(v);
            if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
                return vm.smallStrings.functionString();
        }
    }
    return vm.smallStrings.objectString();
}

JSValue jsTypeStringForValue(CallFrame* callFrame, JSValue v)
{
    return jsTypeStringForValue(callFrame->vm(), callFrame->lexicalGlobalObject(), v);
}

bool jsIsObjectTypeOrNull(CallFrame* callFrame, JSValue v)
{
    VM& vm = callFrame->vm();
    if (!v.isCell())
        return v.isNull();

    JSType type = v.asCell()->type();
    if (type == StringType || type == SymbolType)
        return false;
    if (type >= ObjectType) {
        if (asObject(v)->structure(vm)->masqueradesAsUndefined(callFrame->lexicalGlobalObject()))
            return false;
        CallData callData;
        JSObject* object = asObject(v);
        if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
            return false;
    }
    return true;
}

bool jsIsFunctionType(JSValue v)
{
    if (v.isObject()) {
        CallData callData;
        JSObject* object = asObject(v);
        if (object->methodTable()->getCallData(object, callData) != CallType::None)
            return true;
    }
    return false;
}

size_t normalizePrototypeChain(CallFrame* callFrame, Structure* structure)
{
    VM& vm = callFrame->vm();
    size_t count = 0;
    while (1) {
        if (structure->isProxy())
            return InvalidPrototypeChain;
        JSValue v = structure->prototypeForLookup(callFrame);
        if (v.isNull())
            return count;

        JSCell* base = v.asCell();
        structure = base->structure(vm);
        if (structure->isDictionary()) {
            if (structure->hasBeenFlattenedBefore())
                return InvalidPrototypeChain;
            structure->flattenDictionaryStructure(vm, asObject(base));
        }

        ++count;
    }
}

} // namespace JSC
