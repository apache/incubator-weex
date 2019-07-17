/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef TinyPtrSet_h
#define TinyPtrSet_h

#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>

namespace JSC { namespace DFG {
class StructureAbstractValue;
} } // namespace JSC::DFG

namespace WTF {

// FIXME: This currently only works for types that are pointer-like: they should have the size
// of a pointer and like a pointer they should not have assignment operators, copy constructors,
// non-trivial default constructors, and non-trivial destructors. It may be possible to lift all
// of these restrictions. If we succeeded then this should be renamed to just TinySet.
// https://bugs.webkit.org/show_bug.cgi?id=145741

template<typename T>
class TinyPtrSet {
    static_assert(sizeof(T) == sizeof(void*), "It's in the title of the class.");
public:
    TinyPtrSet()
        : m_pointer(0)
    {
        setEmpty();
    }
    
    TinyPtrSet(T element)
        : m_pointer(0)
    {
        set(element);
    }
    
    ALWAYS_INLINE TinyPtrSet(const TinyPtrSet& other)
        : m_pointer(0)
    {
        copyFrom(other);
    }
    
    ALWAYS_INLINE TinyPtrSet& operator=(const TinyPtrSet& other)
    {
        if (this == &other)
            return *this;
        deleteListIfNecessary();
        copyFrom(other);
        return *this;
    }
    
    ~TinyPtrSet()
    {
        deleteListIfNecessary();
    }
    
    void clear()
    {
        deleteListIfNecessary();
        setEmpty();
    }
    
    // Returns the only entry if the array has exactly one entry.
    T onlyEntry() const
    {
        if (isThin())
            return singleEntry();
        OutOfLineList* list = this->list();
        if (list->m_length != 1)
            return T();
        return list->list()[0];
    }
    
    bool isEmpty() const
    {
        bool result = isThin() && !singleEntry();
        if (result)
            ASSERT(m_pointer != reservedValue);
        return result;
    }
    
    // Returns true if the value was added, or false if the value was already there.
    bool add(T value)
    {
        ASSERT(value);
        if (isThin()) {
            if (singleEntry() == value)
                return false;
            if (!singleEntry()) {
                set(value);
                return true;
            }
            
            OutOfLineList* list = OutOfLineList::create(defaultStartingSize);
            list->m_length = 2;
            list->list()[0] = singleEntry();
            list->list()[1] = value;
            set(list);
            return true;
        }
        
        return addOutOfLine(value);
    }
    
    bool remove(T value)
    {
        if (isThin()) {
            if (singleEntry() == value) {
                setEmpty();
                return true;
            }
            return false;
        }
        
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (list->list()[i] != value)
                continue;
            list->list()[i] = list->list()[--list->m_length];
            if (!list->m_length) {
                OutOfLineList::destroy(list);
                setEmpty();
            }
            return true;
        }
        return false;
    }
    
    bool contains(T value) const
    {
        if (isThin())
            return singleEntry() == value;
        return containsOutOfLine(value);
    }
    
    bool merge(const TinyPtrSet& other)
    {
        if (other.isThin()) {
            if (other.singleEntry())
                return add(other.singleEntry());
            return false;
        }
        
        OutOfLineList* list = other.list();
        if (list->m_length >= 2) {
            if (isThin()) {
                OutOfLineList* myNewList = OutOfLineList::create(
                    list->m_length + !!singleEntry());
                if (singleEntry()) {
                    myNewList->m_length = 1;
                    myNewList->list()[0] = singleEntry();
                }
                set(myNewList);
            }
            bool changed = false;
            for (unsigned i = 0; i < list->m_length; ++i)
                changed |= addOutOfLine(list->list()[i]);
            return changed;
        }
        
        ASSERT(list->m_length);
        return add(list->list()[0]);
    }
    
    template<typename Functor>
    void forEach(const Functor& functor) const
    {
        if (isThin()) {
            if (!singleEntry())
                return;
            functor(singleEntry());
            return;
        }
        
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i)
            functor(list->list()[i]);
    }
        
    template<typename Functor>
    void genericFilter(const Functor& functor)
    {
        if (isThin()) {
            if (!singleEntry())
                return;
            if (functor(singleEntry()))
                return;
            clear();
            return;
        }
        
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (functor(list->list()[i]))
                continue;
            list->list()[i--] = list->list()[--list->m_length];
        }
        if (!list->m_length)
            clear();
    }
        
    void filter(const TinyPtrSet& other)
    {
        if (other.isThin()) {
            if (!other.singleEntry() || !contains(other.singleEntry()))
                clear();
            else {
                clear();
                set(other.singleEntry());
            }
            return;
        }
        
        genericFilter([&] (T value) { return other.containsOutOfLine(value); });
    }
    
    void exclude(const TinyPtrSet& other)
    {
        if (other.isThin()) {
            if (other.singleEntry())
                remove(other.singleEntry());
            return;
        }
        
        genericFilter([&] (T value) { return !other.containsOutOfLine(value); });
    }
    
    bool isSubsetOf(const TinyPtrSet& other) const
    {
        if (isThin()) {
            if (!singleEntry())
                return true;
            return other.contains(singleEntry());
        }
        
        if (other.isThin()) {
            if (!other.singleEntry())
                return false;
            OutOfLineList* list = this->list();
            if (list->m_length >= 2)
                return false;
            if (list->list()[0] == other.singleEntry())
                return true;
            return false;
        }
        
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (!other.containsOutOfLine(list->list()[i]))
                return false;
        }
        return true;
    }
    
    bool isSupersetOf(const TinyPtrSet& other) const
    {
        return other.isSubsetOf(*this);
    }
    
    bool overlaps(const TinyPtrSet& other) const
    {
        if (isThin()) {
            if (!singleEntry())
                return false;
            return other.contains(singleEntry());
        }
        
        if (other.isThin()) {
            if (!other.singleEntry())
                return false;
            return containsOutOfLine(other.singleEntry());
        }
        
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (other.containsOutOfLine(list->list()[i]))
                return true;
        }
        return false;
    }
    
    size_t size() const
    {
        if (isThin())
            return !!singleEntry();
        return list()->m_length;
    }
    
    T at(size_t i) const
    {
        if (isThin()) {
            ASSERT(!i);
            ASSERT(singleEntry());
            return singleEntry();
        }
        ASSERT(i < list()->m_length);
        return list()->list()[i];
    }
    
    T operator[](size_t i) const { return at(i); }
    
    T last() const
    {
        if (isThin()) {
            ASSERT(singleEntry());
            return singleEntry();
        }
        return list()->list()[list()->m_length - 1];
    }
    
    class iterator {
    public:
        iterator()
            : m_set(nullptr)
            , m_index(0)
        {
        }
        
        iterator(const TinyPtrSet* set, size_t index)
            : m_set(set)
            , m_index(index)
        {
        }
        
        T operator*() const { return m_set->at(m_index); }
        iterator& operator++()
        {
            m_index++;
            return *this;
        }
        bool operator==(const iterator& other) const { return m_index == other.m_index; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        
    private:
        const TinyPtrSet* m_set;
        size_t m_index;
    };
    
    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size()); }
    
    bool operator==(const TinyPtrSet& other) const
    {
        if (size() != other.size())
            return false;
        return isSubsetOf(other);
    }
    
private:
    friend class JSC::DFG::StructureAbstractValue;

    static const uintptr_t fatFlag = 1;
    static const uintptr_t reservedFlag = 2;
    static const uintptr_t flags = fatFlag | reservedFlag;
    static const uintptr_t reservedValue = 4;

    static const unsigned defaultStartingSize = 4;
    
    bool addOutOfLine(T value)
    {
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (list->list()[i] == value)
                return false;
        }
        
        if (list->m_length < list->m_capacity) {
            list->list()[list->m_length++] = value;
            return true;
        }
        
        OutOfLineList* newList = OutOfLineList::create(list->m_capacity * 2);
        newList->m_length = list->m_length + 1;
        for (unsigned i = list->m_length; i--;)
            newList->list()[i] = list->list()[i];
        newList->list()[list->m_length] = value;
        OutOfLineList::destroy(list);
        set(newList);
        return true;
    }
    
    bool containsOutOfLine(T value) const
    {
        OutOfLineList* list = this->list();
        for (unsigned i = 0; i < list->m_length; ++i) {
            if (list->list()[i] == value)
                return true;
        }
        return false;
    }
    
    ALWAYS_INLINE void copyFrom(const TinyPtrSet& other)
    {
        if (other.isThin() || other.m_pointer == reservedValue) {
            bool value = getReservedFlag();
            m_pointer = other.m_pointer;
            setReservedFlag(value);
            return;
        }
        copyFromOutOfLine(other);
    }
    
    NEVER_INLINE void copyFromOutOfLine(const TinyPtrSet& other)
    {
        ASSERT(!other.isThin() && other.m_pointer != reservedValue);
        OutOfLineList* otherList = other.list();
        OutOfLineList* myList = OutOfLineList::create(otherList->m_length);
        myList->m_length = otherList->m_length;
        for (unsigned i = otherList->m_length; i--;)
            myList->list()[i] = otherList->list()[i];
        set(myList);
    }
    
    class OutOfLineList {
    public:
        static OutOfLineList* create(unsigned capacity)
        {
            return new (NotNull, fastMalloc(sizeof(OutOfLineList) + capacity * sizeof(T))) OutOfLineList(0, capacity);
        }
        
        static void destroy(OutOfLineList* list)
        {
            fastFree(list);
        }
        
        T* list() { return bitwise_cast<T*>(this + 1); }
        
        OutOfLineList(unsigned length, unsigned capacity)
            : m_length(length)
            , m_capacity(capacity)
        {
        }

        unsigned m_length;
        unsigned m_capacity;
    };
    
    ALWAYS_INLINE void deleteListIfNecessary()
    {
        if (!isThin()) {
            ASSERT(m_pointer != reservedValue);
            OutOfLineList::destroy(list());
        }
    }
    
    bool isThin() const { return !(m_pointer & fatFlag); }
    
    void* pointer() const
    {
        return bitwise_cast<void*>(m_pointer & ~flags);
    }
    
    T singleEntry() const
    {
        ASSERT(isThin());
        return bitwise_cast<T>(pointer());
    }
    
    OutOfLineList* list() const
    {
        ASSERT(!isThin());
        return static_cast<OutOfLineList*>(pointer());
    }
    
    void set(T value)
    {
        set(bitwise_cast<uintptr_t>(value), true);
    }
    void set(OutOfLineList* list)
    {
        set(bitwise_cast<uintptr_t>(list), false);
    }
    void setEmpty()
    {
        set(0, true);
    }
    void set(uintptr_t pointer, bool singleEntry)
    {
        m_pointer = pointer | (singleEntry ? 0 : fatFlag) | (m_pointer & reservedFlag);
    }
    bool getReservedFlag() const { return m_pointer & reservedFlag; }
    void setReservedFlag(bool value)
    {
        if (value)
            m_pointer |= reservedFlag;
        else
            m_pointer &= ~reservedFlag;
    }
    
    uintptr_t m_pointer;
};

} // namespace WTF

using WTF::TinyPtrSet;

#endif // TinyPtrSet_h

