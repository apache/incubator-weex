/*
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
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
#include "ArgList.h"

#include "JSCJSValue.h"
#include "JSObject.h"
#include "JSCInlines.h"

using std::min;

namespace JSC {

void MarkedArgumentBuffer::addMarkSet(JSValue v)
{
    if (m_markSet)
        return;

    Heap* heap = Heap::heap(v);
    if (!heap)
        return;

    m_markSet = &heap->markListSet();
    m_markSet->add(this);
}

void ArgList::getSlice(int startIndex, ArgList& result) const
{
    if (startIndex <= 0 || startIndex >= m_argCount) {
        result = ArgList();
        return;
    }

    result.m_args = m_args + startIndex;
    result.m_argCount =  m_argCount - startIndex;
}

void MarkedArgumentBuffer::markLists(SlotVisitor& visitor, ListSet& markSet)
{
    ListSet::iterator end = markSet.end();
    for (ListSet::iterator it = markSet.begin(); it != end; ++it) {
        MarkedArgumentBuffer* list = *it;
        for (int i = 0; i < list->m_size; ++i)
            visitor.appendUnbarriered(JSValue::decode(list->slotFor(i)));
    }
}

void MarkedArgumentBuffer::slowEnsureCapacity(size_t requestedCapacity)
{
    int newCapacity = Checked<int>(requestedCapacity).unsafeGet();
    expandCapacity(newCapacity);
}

void MarkedArgumentBuffer::expandCapacity()
{
    int newCapacity = (Checked<int>(m_capacity) * 2).unsafeGet();
    expandCapacity(newCapacity);
}

void MarkedArgumentBuffer::expandCapacity(int newCapacity)
{
    ASSERT(m_capacity < newCapacity);
    size_t size = (Checked<size_t>(newCapacity) * sizeof(EncodedJSValue)).unsafeGet();
    EncodedJSValue* newBuffer = static_cast<EncodedJSValue*>(fastMalloc(size));
    for (int i = 0; i < m_size; ++i) {
        newBuffer[i] = m_buffer[i];
        addMarkSet(JSValue::decode(m_buffer[i]));
    }

    if (EncodedJSValue* base = mallocBase())
        fastFree(base);

    m_buffer = newBuffer;
    m_capacity = newCapacity;
}

void MarkedArgumentBuffer::slowAppend(JSValue v)
{
    ASSERT(m_size <= m_capacity);
    if (m_size == m_capacity)
        expandCapacity();

    slotFor(m_size) = JSValue::encode(v);
    ++m_size;
    addMarkSet(v);
}

} // namespace JSC
