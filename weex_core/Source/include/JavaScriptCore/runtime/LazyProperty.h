/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "VM.h"

namespace JSC {

class JSCell;
class SlotVisitor;
class VM;

// Note that if all you're doing is calling LazyProperty::get(), it's completely safe to bitcast
// this LazyProperty<JSCell, JSCell>.
template<typename OwnerType, typename ElementType>
class LazyProperty {
public:
    struct Initializer {
        Initializer(OwnerType* owner, LazyProperty& property)
            : vm(*Heap::heap(owner)->vm())
            , owner(owner)
            , property(property)
        {
        }
        
        void set(ElementType* value) const;

        VM& vm;
        OwnerType* owner;
        LazyProperty& property;
    };

private:
    typedef ElementType* (*FuncType)(const Initializer&);

public:
    LazyProperty()
    {
    }

    // Tell the property to run the given callback next time someone tries to get the value
    // using get(). The passed callback must be stateless. For example:
    //
    //     property.initLater(
    //         [] (const LazyProperty<Foo, Bar>::Initializer& init) {
    //             init.set(...things...);
    //         });
    //
    // This method is always inlineable and should always compile to a store of a constant
    // pointer no matter how complicated the callback is.
    template<typename Func>
    void initLater(const Func&);

    // This lazily initializes the property. Note that this gracefully supports recursive calls.
    // If this gets called while the property is being initialized, it will simply return null.
    ElementType* get(const OwnerType* owner) const
    {
        ASSERT(!isCompilationThread());
        if (UNLIKELY(m_pointer & lazyTag)) {
            FuncType func = *bitwise_cast<FuncType*>(m_pointer & ~(lazyTag | initializingTag));
            return func(Initializer(const_cast<OwnerType*>(owner), *const_cast<LazyProperty*>(this)));
        }
        return bitwise_cast<ElementType*>(m_pointer);
    }
    
    ElementType* getConcurrently() const
    {
        uintptr_t pointer = m_pointer;
        if (pointer & lazyTag)
            return nullptr;
        return bitwise_cast<ElementType*>(pointer);
    }
    
    void setMayBeNull(VM&, const OwnerType* owner, ElementType*);
    void set(VM&, const OwnerType* owner, ElementType*);
    
    void visit(SlotVisitor&);
    
    void dump(PrintStream&) const;
    
private:
    template<typename Func>
    static ElementType* callFunc(const Initializer&);
    
    static const uintptr_t lazyTag = 1;
    static const uintptr_t initializingTag = 2;
    
    uintptr_t m_pointer { 0 };
};

// It's valid to bitcast any LazyProperty to LazyCellProperty if you're just going to call get()
// or getConcurrently().
typedef LazyProperty<JSCell, JSCell> LazyCellProperty;

} // namespace JSC
