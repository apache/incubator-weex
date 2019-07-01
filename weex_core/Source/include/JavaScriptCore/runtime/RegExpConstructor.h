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

#include "InternalFunction.h"
#include "RegExp.h"
#include "RegExpCachedResult.h"
#include "RegExpObject.h"

namespace JSC {

class RegExpPrototype;
class GetterSetter;

class RegExpConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static RegExpConstructor* create(VM& vm, Structure* structure, RegExpPrototype* regExpPrototype, GetterSetter* species)
    {
        RegExpConstructor* constructor = new (NotNull, allocateCell<RegExpConstructor>(vm.heap)) RegExpConstructor(vm, structure, regExpPrototype);
        constructor->finishCreation(vm, regExpPrototype, species);
        return constructor;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;

    MatchResult performMatch(VM&, RegExp*, JSString*, const String&, int startOffset, int** ovector);
    MatchResult performMatch(VM&, RegExp*, JSString*, const String&, int startOffset);
    void recordMatch(VM&, RegExp*, JSString*, const MatchResult&);

    void setMultiline(bool multiline) { m_multiline = multiline; }
    bool multiline() const { return m_multiline; }

    JSValue getBackref(ExecState*, unsigned);
    JSValue getLastParen(ExecState*);
    JSValue getLeftContext(ExecState*);
    JSValue getRightContext(ExecState*);

    void setInput(ExecState* exec, JSString* string) { m_cachedResult.setInput(exec, this, string); }
    JSString* input() { return m_cachedResult.input(); }

    static void visitChildren(JSCell*, SlotVisitor&);

    static ptrdiff_t offsetOfCachedResult() { return OBJECT_OFFSETOF(RegExpConstructor, m_cachedResult); }

protected:
    void finishCreation(VM&, RegExpPrototype*, GetterSetter* species);

private:
    RegExpConstructor(VM&, Structure*, RegExpPrototype*);
    static void destroy(JSCell*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);

    RegExpCachedResult m_cachedResult;
    bool m_multiline;
    Vector<int> m_ovector;
};

RegExpConstructor* asRegExpConstructor(JSValue);

JSObject* constructRegExp(ExecState*, JSGlobalObject*, const ArgList&, JSObject* callee = nullptr, JSValue newTarget = jsUndefined());

inline RegExpConstructor* asRegExpConstructor(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), RegExpConstructor::info()));
    return static_cast<RegExpConstructor*>(asObject(value));
}

/* 
   To facilitate result caching, exec(), test(), match(), search(), and replace() dipatch regular
   expression matching through the performMatch function. We use cached results to calculate, 
   e.g., RegExp.lastMatch and RegExp.leftParen.
*/
ALWAYS_INLINE MatchResult RegExpConstructor::performMatch(VM& vm, RegExp* regExp, JSString* string, const String& input, int startOffset, int** ovector)
{
    int position = regExp->match(vm, input, startOffset, m_ovector);

    if (ovector)
        *ovector = m_ovector.data();

    if (position == -1)
        return MatchResult::failed();

    ASSERT(!m_ovector.isEmpty());
    ASSERT(m_ovector[0] == position);
    ASSERT(m_ovector[1] >= position);
    size_t end = m_ovector[1];

    m_cachedResult.record(vm, this, regExp, string, MatchResult(position, end));

    return MatchResult(position, end);
}
ALWAYS_INLINE MatchResult RegExpConstructor::performMatch(VM& vm, RegExp* regExp, JSString* string, const String& input, int startOffset)
{
    MatchResult result = regExp->match(vm, input, startOffset);
    if (result)
        m_cachedResult.record(vm, this, regExp, string, result);
    return result;
}

ALWAYS_INLINE void RegExpConstructor::recordMatch(VM& vm, RegExp* regExp, JSString* string, const MatchResult& result)
{
    ASSERT(result);
    m_cachedResult.record(vm, this, regExp, string, result);
}

ALWAYS_INLINE bool isRegExp(VM& vm, ExecState* exec, JSValue value)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (!value.isObject())
        return false;

    JSObject* object = asObject(value);
    JSValue matchValue = object->get(exec, vm.propertyNames->matchSymbol);
    RETURN_IF_EXCEPTION(scope, false);
    if (!matchValue.isUndefined())
        return matchValue.toBoolean(exec);

    return object->inherits(vm, RegExpObject::info());
}

EncodedJSValue JSC_HOST_CALL esSpecRegExpCreate(ExecState*);

} // namespace JSC
