/*
 *  Copyright (C) 2006, 2008, 2012 Apple Inc. All rights reserved.
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

#include "CallFrame.h"
#include "Identifier.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

namespace JSC {

// FIXME: Rename to PropertyNameArray.
class PropertyNameArrayData : public RefCounted<PropertyNameArrayData> {
public:
    typedef Vector<Identifier, 20> PropertyNameVector;

    static Ref<PropertyNameArrayData> create() { return adoptRef(*new PropertyNameArrayData); }

    PropertyNameVector& propertyNameVector() { return m_propertyNameVector; }

private:
    PropertyNameArrayData()
    {
    }

    PropertyNameVector m_propertyNameVector;
};

// FIXME: Rename to PropertyNameArrayBuilder.
class PropertyNameArray {
public:
    PropertyNameArray(VM* vm, PropertyNameMode mode)
        : m_data(PropertyNameArrayData::create())
        , m_vm(vm)
        , m_mode(mode)
    {
    }

    PropertyNameArray(ExecState* exec, PropertyNameMode mode)
        : PropertyNameArray(&exec->vm(), mode)
    {
    }

    VM* vm() { return m_vm; }

    void add(uint32_t index)
    {
        add(Identifier::from(m_vm, index));
    }

    void add(const Identifier&);
    void add(UniquedStringImpl*);
    void addUnchecked(UniquedStringImpl*);

    Identifier& operator[](unsigned i) { return m_data->propertyNameVector()[i]; }
    const Identifier& operator[](unsigned i) const { return m_data->propertyNameVector()[i]; }

    PropertyNameArrayData* data() { return m_data.get(); }
    RefPtr<PropertyNameArrayData> releaseData() { return WTFMove(m_data); }

    // FIXME: Remove these functions.
    bool canAddKnownUniqueForStructure() const { return m_data->propertyNameVector().isEmpty(); }
    typedef PropertyNameArrayData::PropertyNameVector::const_iterator const_iterator;
    size_t size() const { return m_data->propertyNameVector().size(); }
    const_iterator begin() const { return m_data->propertyNameVector().begin(); }
    const_iterator end() const { return m_data->propertyNameVector().end(); }

    PropertyNameMode mode() const { return m_mode; }
    bool includeSymbolProperties() const;
    bool includeStringProperties() const;

private:
    bool isUidMatchedToTypeMode(UniquedStringImpl* identifier);

    RefPtr<PropertyNameArrayData> m_data;
    HashSet<UniquedStringImpl*> m_set;
    VM* m_vm;
    PropertyNameMode m_mode;
};

ALWAYS_INLINE void PropertyNameArray::add(const Identifier& identifier)
{
    add(identifier.impl());
}

ALWAYS_INLINE void PropertyNameArray::addUnchecked(UniquedStringImpl* identifier)
{
    if (!isUidMatchedToTypeMode(identifier))
        return;
    m_data->propertyNameVector().append(Identifier::fromUid(m_vm, identifier));
}

ALWAYS_INLINE void PropertyNameArray::add(UniquedStringImpl* identifier)
{
    static const unsigned setThreshold = 20;

    ASSERT(identifier);

    if (!isUidMatchedToTypeMode(identifier))
        return;

    if (size() < setThreshold) {
        if (m_data->propertyNameVector().contains(identifier))
            return;
    } else {
        if (m_set.isEmpty()) {
            for (Identifier& name : m_data->propertyNameVector())
                m_set.add(name.impl());
        }
        if (!m_set.add(identifier).isNewEntry)
            return;
    }

    addUnchecked(identifier);
}

ALWAYS_INLINE bool PropertyNameArray::isUidMatchedToTypeMode(UniquedStringImpl* identifier)
{
    if (identifier->isSymbol())
        return includeSymbolProperties();
    return includeStringProperties();
}

ALWAYS_INLINE bool PropertyNameArray::includeSymbolProperties() const
{
    return static_cast<std::underlying_type<PropertyNameMode>::type>(m_mode) & static_cast<std::underlying_type<PropertyNameMode>::type>(PropertyNameMode::Symbols);
}

ALWAYS_INLINE bool PropertyNameArray::includeStringProperties() const
{
    return static_cast<std::underlying_type<PropertyNameMode>::type>(m_mode) & static_cast<std::underlying_type<PropertyNameMode>::type>(PropertyNameMode::Strings);
}

} // namespace JSC
