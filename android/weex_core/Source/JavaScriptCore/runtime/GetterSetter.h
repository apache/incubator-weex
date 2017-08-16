/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014 Apple Inc. All rights reserved.
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

#pragma once

#include "JSCell.h"

#include "CallFrame.h"
#include "JSGlobalObject.h"
#include "NullGetterFunction.h"
#include "NullSetterFunction.h"
#include "Structure.h"

namespace JSC {

class JSObject;

// This is an internal value object which stores getter and setter functions
// for a property. Instances of this class have the property that once a getter
// or setter is set to a non-null value, then they cannot be changed. This means
// that if a property holding a GetterSetter reference is constant-inferred and
// that constant is observed to have a non-null setter (or getter) then we can
// constant fold that setter (or getter).
class GetterSetter final : public JSNonFinalObject {
    friend class JIT;
    typedef JSNonFinalObject Base;
private:
    GetterSetter(VM& vm, JSGlobalObject* globalObject)
        : Base(vm, globalObject->getterSetterStructure())
    {
        m_getter.set(vm, this, globalObject->nullGetterFunction());
        m_setter.set(vm, this, globalObject->nullSetterFunction());
    }

public:

    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | StructureIsImmortal;

    static GetterSetter* create(VM& vm, JSGlobalObject* globalObject)
    {
        GetterSetter* getterSetter = new (NotNull, allocateCell<GetterSetter>(vm.heap)) GetterSetter(vm, globalObject);
        getterSetter->finishCreation(vm);
        return getterSetter;
    }

    static void visitChildren(JSCell*, SlotVisitor&);

    JSObject* getter() const { return m_getter.get(); }

    JSObject* getterConcurrently() const
    {
        JSObject* result = getter();
        WTF::loadLoadFence();
        return result;
    }

    bool isGetterNull() const { return !!jsDynamicCast<NullGetterFunction*>(*m_getter.get()->vm(), m_getter.get()); }
    bool isSetterNull() const { return !!jsDynamicCast<NullSetterFunction*>(*m_setter.get()->vm(), m_setter.get()); }

    // Set the getter. It's only valid to call this if you've never set the getter on this
    // object.
    void setGetter(VM& vm, JSGlobalObject* globalObject, JSObject* getter)
    {
        if (!getter)
            getter = jsCast<JSObject*>(globalObject->nullGetterFunction());

        RELEASE_ASSERT(isGetterNull());
        WTF::storeStoreFence();
        m_getter.set(vm, this, getter);
    }

    JSObject* setter() const { return m_setter.get(); }

    JSObject* setterConcurrently() const
    {
        JSObject* result = setter();
        WTF::loadLoadFence();
        return result;
    }

    // Set the setter. It's only valid to call this if you've never set the setter on this
    // object.
    void setSetter(VM& vm, JSGlobalObject* globalObject, JSObject* setter)
    {
        if (!setter)
            setter = jsCast<JSObject*>(globalObject->nullSetterFunction());

        RELEASE_ASSERT(isSetterNull());
        WTF::storeStoreFence();
        m_setter.set(vm, this, setter);
    }

    GetterSetter* withGetter(VM&, JSGlobalObject*, JSObject* getter);
    GetterSetter* withSetter(VM&, JSGlobalObject*, JSObject* setter);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(GetterSetterType), info());
    }

    static ptrdiff_t offsetOfGetter()
    {
        return OBJECT_OFFSETOF(GetterSetter, m_getter);
    }

    static ptrdiff_t offsetOfSetter()
    {
        return OBJECT_OFFSETOF(GetterSetter, m_setter);
    }

    DECLARE_EXPORT_INFO;

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&) { RELEASE_ASSERT_NOT_REACHED(); return false; }
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&) { RELEASE_ASSERT_NOT_REACHED(); return false; }
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool) { RELEASE_ASSERT_NOT_REACHED(); return false; }
    static bool deleteProperty(JSCell*, ExecState*, PropertyName) { RELEASE_ASSERT_NOT_REACHED(); return false; }

private:
    WriteBarrier<JSObject> m_getter;
    WriteBarrier<JSObject> m_setter;  
};

GetterSetter* asGetterSetter(JSValue);

inline GetterSetter* asGetterSetter(JSValue value)
{
    ASSERT_WITH_SECURITY_IMPLICATION(value.asCell()->isGetterSetter());
    return static_cast<GetterSetter*>(value.asCell());
}

JSValue callGetter(ExecState*, JSValue base, JSValue getterSetter);
JS_EXPORT_PRIVATE bool callSetter(ExecState*, JSValue base, JSValue getterSetter, JSValue, ECMAMode);

} // namespace JSC
