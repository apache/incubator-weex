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

#include "FunctionExecutable.h"
#include "JSCell.h"
#include "JSFunction.h"
#include "NativeExecutable.h"

namespace JSC {

// The CallVariant class is meant to encapsulate a callee in a way that is useful for call linking
// and inlining. Because JavaScript has closures, and because JSC implements the notion of internal
// non-function objects that nevertheless provide call traps, the call machinery wants to see a
// callee in one of the following four forms:
//
// JSFunction callee: This means that we expect the callsite to always call a particular function
//     instance, that is associated with a particular lexical environment. This pinpoints not
//     just the code that will be called (i.e. the executable) but also the scope within which
//     the code runs.
//
// Executable callee: This corresponds to a call to a closure. In this case, we know that the
//     callsite will call a JSFunction, but we do not know which particular JSFunction. We do know
//     what code will be called - i.e. we know the executable.
//
// InternalFunction callee: JSC supports a special kind of native functions that support bizarre
//     semantics. These are always singletons. If we know that the callee is an InternalFunction
//     then we know both the code that will be called and the scope; in fact the "scope" is really
//     just the InternalFunction itself.
//
// Something else: It's possible call all manner of rubbish in JavaScript. This implicitly supports
//     bizarre object callees, but it can't really tell you anything interesting about them other
//     than the fact that they don't fall into any of the above categories.
//
// This class serves as a kind of union over these four things. It does so by just holding a
// JSCell*. We determine which of the modes its in by doing type checks on the cell. Note that we
// cannot use WriteBarrier<> here because this gets used inside the compiler.

class CallVariant {
public:
    explicit CallVariant(JSCell* callee = nullptr)
        : m_callee(callee)
    {
    }
    
    CallVariant(WTF::HashTableDeletedValueType)
        : m_callee(deletedToken())
    {
    }
    
    bool operator!() const { return !m_callee; }
    
    // If this variant refers to a function, change it to refer to its executable.
    ALWAYS_INLINE CallVariant despecifiedClosure() const
    {
        if (m_callee->type() == JSFunctionType)
            return CallVariant(jsCast<JSFunction*>(m_callee)->executable());
        return *this;
    }
    
    JSCell* rawCalleeCell() const { return m_callee; }
    
    InternalFunction* internalFunction() const
    {
        return jsDynamicCast<InternalFunction*>(*m_callee->vm(), m_callee);
    }
    
    JSFunction* function() const
    {
        return jsDynamicCast<JSFunction*>(*m_callee->vm(), m_callee);
    }
    
    bool isClosureCall() const { return !!jsDynamicCast<ExecutableBase*>(*m_callee->vm(), m_callee); }
    
    ExecutableBase* executable() const
    {
        if (JSFunction* function = this->function())
            return function->executable();
        return jsDynamicCast<ExecutableBase*>(*m_callee->vm(), m_callee);
    }
    
    JSCell* nonExecutableCallee() const
    {
        RELEASE_ASSERT(!isClosureCall());
        return m_callee;
    }
    
    Intrinsic intrinsicFor(CodeSpecializationKind kind) const
    {
        if (ExecutableBase* executable = this->executable())
            return executable->intrinsicFor(kind);
        return NoIntrinsic;
    }
    
    FunctionExecutable* functionExecutable() const
    {
        if (ExecutableBase* executable = this->executable())
            return jsDynamicCast<FunctionExecutable*>(*m_callee->vm(), executable);
        return nullptr;
    }

    NativeExecutable* nativeExecutable() const
    {
        if (ExecutableBase* executable = this->executable())
            return jsDynamicCast<NativeExecutable*>(*m_callee->vm(), executable);
        return nullptr;
    }

    const DOMJIT::Signature* signatureFor(CodeSpecializationKind kind) const
    {
        if (NativeExecutable* nativeExecutable = this->nativeExecutable())
            return nativeExecutable->signatureFor(kind);
        return nullptr;
    }
    
    void dump(PrintStream& out) const;
    
    bool isHashTableDeletedValue() const
    {
        return m_callee == deletedToken();
    }
    
    bool operator==(const CallVariant& other) const
    {
        return m_callee == other.m_callee;
    }
    
    bool operator!=(const CallVariant& other) const
    {
        return !(*this == other);
    }
    
    bool operator<(const CallVariant& other) const
    {
        return m_callee < other.m_callee;
    }
    
    bool operator>(const CallVariant& other) const
    {
        return other < *this;
    }
    
    bool operator<=(const CallVariant& other) const
    {
        return !(*this < other);
    }
    
    bool operator>=(const CallVariant& other) const
    {
        return other <= *this;
    }
    
    unsigned hash() const
    {
        return WTF::PtrHash<JSCell*>::hash(m_callee);
    }
    
private:
    static JSCell* deletedToken() { return bitwise_cast<JSCell*>(static_cast<uintptr_t>(1)); }
    
    JSCell* m_callee;
};

struct CallVariantHash {
    static unsigned hash(const CallVariant& key) { return key.hash(); }
    static bool equal(const CallVariant& a, const CallVariant& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

typedef Vector<CallVariant, 1> CallVariantList;

// Returns a new variant list by attempting to either append the given variant or merge it with one
// of the variants we already have by despecifying closures.
CallVariantList variantListWithVariant(const CallVariantList&, CallVariant);

// Returns a new list where every element is despecified, and the list is deduplicated.
CallVariantList despecifiedVariantList(const CallVariantList&);

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::CallVariant> {
    typedef JSC::CallVariantHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::CallVariant> : SimpleClassHashTraits<JSC::CallVariant> { };

} // namespace WTF
