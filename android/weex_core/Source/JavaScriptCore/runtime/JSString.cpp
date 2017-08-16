/*
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2004, 2007-2008, 2015-2016 Apple Inc. All rights reserved.
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
#include "JSString.h"

#include "JSGlobalObject.h"
#include "JSGlobalObjectFunctions.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include "StringObject.h"
#include "StringPrototype.h"
#include "StrongInlines.h"

namespace JSC {
    
const ClassInfo JSString::s_info = { "string", 0, 0, CREATE_METHOD_TABLE(JSString) };

Structure* JSString::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
{
    return Structure::create(vm, globalObject, proto, TypeInfo(StringType, StructureFlags), info());
}

void JSRopeString::RopeBuilder::expand()
{
    ASSERT(m_index == JSRopeString::s_maxInternalRopeLength);
    JSString* jsString = m_jsString;
    RELEASE_ASSERT(jsString);
    m_jsString = jsStringBuilder(&m_vm);
    m_index = 0;
    append(jsString);
}

void JSString::destroy(JSCell* cell)
{
    static_cast<JSString*>(cell)->JSString::~JSString();
}

void JSString::dumpToStream(const JSCell* cell, PrintStream& out)
{
    VM& vm = *cell->vm();
    const JSString* thisObject = jsCast<const JSString*>(cell);
    out.printf("<%p, %s, [%u], ", thisObject, thisObject->className(vm), thisObject->length());
    if (thisObject->isRope())
        out.printf("[rope]");
    else {
        WTF::StringImpl* ourImpl = thisObject->m_value.impl();
        if (ourImpl->is8Bit())
            out.printf("[8 %p]", ourImpl->characters8());
        else
            out.printf("[16 %p]", ourImpl->characters16());
    }
    out.printf(">");
}

bool JSString::equalSlowCase(ExecState* exec, JSString* other) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    String str1 = value(exec);
    String str2 = other->value(exec);
    RETURN_IF_EXCEPTION(scope, false);
    return WTF::equal(*str1.impl(), *str2.impl());
}

size_t JSString::estimatedSize(JSCell* cell)
{
    JSString* thisObject = asString(cell);
    if (thisObject->isRope())
        return Base::estimatedSize(cell);
    return Base::estimatedSize(cell) + thisObject->m_value.impl()->costDuringGC();
}

void JSString::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSString* thisObject = asString(cell);
    Base::visitChildren(thisObject, visitor);
    
    if (thisObject->isRope())
        static_cast<JSRopeString*>(thisObject)->visitFibers(visitor);
    if (StringImpl* impl = thisObject->m_value.impl())
        visitor.reportExtraMemoryVisited(impl->costDuringGC());
}

void JSRopeString::visitFibers(SlotVisitor& visitor)
{
    if (isSubstring()) {
        visitor.append(substringBase());
        return;
    }
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i)
        visitor.append(fiber(i));
}

static const unsigned maxLengthForOnStackResolve = 2048;

void JSRopeString::resolveRopeInternal8(LChar* buffer) const
{
    if (isSubstring()) {
        StringImpl::copyChars(
            buffer, substringBase()->m_value.characters8() + substringOffset(), length());
        return;
    }
    
    resolveRopeInternal8NoSubstring(buffer);
}

void JSRopeString::resolveRopeInternal8NoSubstring(LChar* buffer) const
{
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i) {
        if (fiber(i)->isRope()) {
            resolveRopeSlowCase8(buffer);
            return;
        }
    }

    LChar* position = buffer;
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i) {
        const StringImpl& fiberString = *fiber(i)->m_value.impl();
        unsigned length = fiberString.length();
        StringImpl::copyChars(position, fiberString.characters8(), length);
        position += length;
    }
    ASSERT((buffer + length()) == position);
}

void JSRopeString::resolveRopeInternal16(UChar* buffer) const
{
    if (isSubstring()) {
        StringImpl::copyChars(
            buffer, substringBase()->m_value.characters16() + substringOffset(), length());
        return;
    }
    
    resolveRopeInternal16NoSubstring(buffer);
}

void JSRopeString::resolveRopeInternal16NoSubstring(UChar* buffer) const
{
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i) {
        if (fiber(i)->isRope()) {
            resolveRopeSlowCase(buffer);
            return;
        }
    }

    UChar* position = buffer;
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i) {
        const StringImpl& fiberString = *fiber(i)->m_value.impl();
        unsigned length = fiberString.length();
        if (fiberString.is8Bit())
            StringImpl::copyChars(position, fiberString.characters8(), length);
        else
            StringImpl::copyChars(position, fiberString.characters16(), length);
        position += length;
    }
    ASSERT((buffer + length()) == position);
}

void JSRopeString::resolveRopeToAtomicString(ExecState* exec) const
{
    if (length() > maxLengthForOnStackResolve) {
        resolveRope(exec);
        m_value = AtomicString(m_value);
        setIs8Bit(m_value.impl()->is8Bit());
        return;
    }

    if (is8Bit()) {
        LChar buffer[maxLengthForOnStackResolve];
        resolveRopeInternal8(buffer);
        m_value = AtomicString(buffer, length());
        setIs8Bit(m_value.impl()->is8Bit());
    } else {
        UChar buffer[maxLengthForOnStackResolve];
        resolveRopeInternal16(buffer);
        m_value = AtomicString(buffer, length());
        setIs8Bit(m_value.impl()->is8Bit());
    }

    clearFibers();

    // If we resolved a string that didn't previously exist, notify the heap that we've grown.
    if (m_value.impl()->hasOneRef())
        Heap::heap(this)->reportExtraMemoryAllocated(m_value.impl()->cost());
}

void JSRopeString::clearFibers() const
{
    for (size_t i = 0; i < s_maxInternalRopeLength; ++i)
        u[i].number = 0;
}

RefPtr<AtomicStringImpl> JSRopeString::resolveRopeToExistingAtomicString(ExecState* exec) const
{
    if (length() > maxLengthForOnStackResolve) {
        resolveRope(exec);
        if (RefPtr<AtomicStringImpl> existingAtomicString = AtomicStringImpl::lookUp(m_value.impl())) {
            m_value = *existingAtomicString;
            setIs8Bit(m_value.impl()->is8Bit());
            clearFibers();
            return existingAtomicString;
        }
        return nullptr;
    }
    
    if (is8Bit()) {
        LChar buffer[maxLengthForOnStackResolve];
        resolveRopeInternal8(buffer);
        if (RefPtr<AtomicStringImpl> existingAtomicString = AtomicStringImpl::lookUp(buffer, length())) {
            m_value = *existingAtomicString;
            setIs8Bit(m_value.impl()->is8Bit());
            clearFibers();
            return existingAtomicString;
        }
    } else {
        UChar buffer[maxLengthForOnStackResolve];
        resolveRopeInternal16(buffer);
        if (RefPtr<AtomicStringImpl> existingAtomicString = AtomicStringImpl::lookUp(buffer, length())) {
            m_value = *existingAtomicString;
            setIs8Bit(m_value.impl()->is8Bit());
            clearFibers();
            return existingAtomicString;
        }
    }

    return nullptr;
}

void JSRopeString::resolveRope(ExecState* exec) const
{
    ASSERT(isRope());
    
    if (isSubstring()) {
        ASSERT(!substringBase()->isRope());
        m_value = substringBase()->m_value.substringSharingImpl(substringOffset(), length());
        substringBase().clear();
        return;
    }
    
    if (is8Bit()) {
        LChar* buffer;
        if (auto newImpl = StringImpl::tryCreateUninitialized(length(), buffer)) {
            Heap::heap(this)->reportExtraMemoryAllocated(newImpl->cost());
            m_value = WTFMove(newImpl);
        } else {
            outOfMemory(exec);
            return;
        }
        resolveRopeInternal8NoSubstring(buffer);
        clearFibers();
        ASSERT(!isRope());
        return;
    }

    UChar* buffer;
    if (auto newImpl = StringImpl::tryCreateUninitialized(length(), buffer)) {
        Heap::heap(this)->reportExtraMemoryAllocated(newImpl->cost());
        m_value = WTFMove(newImpl);
    } else {
        outOfMemory(exec);
        return;
    }

    resolveRopeInternal16NoSubstring(buffer);
    clearFibers();
    ASSERT(!isRope());
}

// Overview: These functions convert a JSString from holding a string in rope form
// down to a simple String representation. It does so by building up the string
// backwards, since we want to avoid recursion, we expect that the tree structure
// representing the rope is likely imbalanced with more nodes down the left side
// (since appending to the string is likely more common) - and as such resolving
// in this fashion should minimize work queue size.  (If we built the queue forwards
// we would likely have to place all of the constituent StringImpls into the
// Vector before performing any concatenation, but by working backwards we likely
// only fill the queue with the number of substrings at any given level in a
// rope-of-ropes.)    
void JSRopeString::resolveRopeSlowCase8(LChar* buffer) const
{
    LChar* position = buffer + length(); // We will be working backwards over the rope.
    Vector<JSString*, 32, UnsafeVectorOverflow> workQueue; // Putting strings into a Vector is only OK because there are no GC points in this method.
    
    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i)
        workQueue.append(fiber(i).get());

    while (!workQueue.isEmpty()) {
        JSString* currentFiber = workQueue.last();
        workQueue.removeLast();

        const LChar* characters;
        
        if (currentFiber->isRope()) {
            JSRopeString* currentFiberAsRope = static_cast<JSRopeString*>(currentFiber);
            if (!currentFiberAsRope->isSubstring()) {
                for (size_t i = 0; i < s_maxInternalRopeLength && currentFiberAsRope->fiber(i); ++i)
                    workQueue.append(currentFiberAsRope->fiber(i).get());
                continue;
            }
            ASSERT(!currentFiberAsRope->substringBase()->isRope());
            characters =
                currentFiberAsRope->substringBase()->m_value.characters8() +
                currentFiberAsRope->substringOffset();
        } else
            characters = currentFiber->m_value.characters8();
        
        unsigned length = currentFiber->length();
        position -= length;
        StringImpl::copyChars(position, characters, length);
    }

    ASSERT(buffer == position);
}

void JSRopeString::resolveRopeSlowCase(UChar* buffer) const
{
    UChar* position = buffer + length(); // We will be working backwards over the rope.
    Vector<JSString*, 32, UnsafeVectorOverflow> workQueue; // These strings are kept alive by the parent rope, so using a Vector is OK.

    for (size_t i = 0; i < s_maxInternalRopeLength && fiber(i); ++i)
        workQueue.append(fiber(i).get());

    while (!workQueue.isEmpty()) {
        JSString* currentFiber = workQueue.last();
        workQueue.removeLast();

        if (currentFiber->isRope()) {
            JSRopeString* currentFiberAsRope = static_cast<JSRopeString*>(currentFiber);
            if (currentFiberAsRope->isSubstring()) {
                ASSERT(!currentFiberAsRope->substringBase()->isRope());
                StringImpl* string = static_cast<StringImpl*>(
                    currentFiberAsRope->substringBase()->m_value.impl());
                unsigned offset = currentFiberAsRope->substringOffset();
                unsigned length = currentFiberAsRope->length();
                position -= length;
                if (string->is8Bit())
                    StringImpl::copyChars(position, string->characters8() + offset, length);
                else
                    StringImpl::copyChars(position, string->characters16() + offset, length);
                continue;
            }
            for (size_t i = 0; i < s_maxInternalRopeLength && currentFiberAsRope->fiber(i); ++i)
                workQueue.append(currentFiberAsRope->fiber(i).get());
            continue;
        }

        StringImpl* string = static_cast<StringImpl*>(currentFiber->m_value.impl());
        unsigned length = string->length();
        position -= length;
        if (string->is8Bit())
            StringImpl::copyChars(position, string->characters8(), length);
        else
            StringImpl::copyChars(position, string->characters16(), length);
    }

    ASSERT(buffer == position);
}

void JSRopeString::outOfMemory(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    clearFibers();
    ASSERT(isRope());
    ASSERT(m_value.isNull());
    if (exec)
        throwOutOfMemoryError(exec, scope);
}

JSValue JSString::toPrimitive(ExecState*, PreferredPrimitiveType) const
{
    return const_cast<JSString*>(this);
}

bool JSString::getPrimitiveNumber(ExecState* exec, double& number, JSValue& result) const
{
    result = this;
    number = jsToNumber(unsafeView(*exec));
    return false;
}

double JSString::toNumber(ExecState* exec) const
{
    return jsToNumber(unsafeView(*exec));
}

inline StringObject* StringObject::create(VM& vm, JSGlobalObject* globalObject, JSString* string)
{
    StringObject* object = new (NotNull, allocateCell<StringObject>(vm.heap)) StringObject(vm, globalObject->stringObjectStructure());
    object->finishCreation(vm, string);
    return object;
}

JSObject* JSString::toObject(ExecState* exec, JSGlobalObject* globalObject) const
{
    return StringObject::create(exec->vm(), globalObject, const_cast<JSString*>(this));
}

JSValue JSString::toThis(JSCell* cell, ExecState* exec, ECMAMode ecmaMode)
{
    if (ecmaMode == StrictMode)
        return cell;
    return StringObject::create(exec->vm(), exec->lexicalGlobalObject(), asString(cell));
}

bool JSString::getStringPropertyDescriptor(ExecState* exec, PropertyName propertyName, PropertyDescriptor& descriptor)
{
    if (propertyName == exec->propertyNames().length) {
        descriptor.setDescriptor(jsNumber(length()), DontEnum | DontDelete | ReadOnly);
        return true;
    }
    
    std::optional<uint32_t> index = parseIndex(propertyName);
    if (index && index.value() < length()) {
        descriptor.setDescriptor(getIndex(exec, index.value()), DontDelete | ReadOnly);
        return true;
    }
    
    return false;
}

JSString* jsStringWithCacheSlowCase(VM& vm, StringImpl& stringImpl)
{
    if (JSString* string = vm.stringCache.get(&stringImpl))
        return string;

    JSString* string = jsString(&vm, String(stringImpl));
    vm.lastCachedString.set(vm, string);
    return string;
}

} // namespace JSC
