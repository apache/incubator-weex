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

#pragma once

#include "ConcurrentJSLock.h"
#include "JSCell.h"
#include "PropertyName.h"
#include "PutByIdFlags.h"
#include "Watchpoint.h"
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC {

// This is an object used for the inference of the types of object properties.

class InferredType final : public JSCell {
public:
    typedef JSCell Base;

    static InferredType* create(VM&);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static const unsigned StructureFlags = StructureIsImmortal | Base::StructureFlags;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_INFO;

    enum Kind : uint8_t {
        Bottom,
        Boolean,
        Other,
        Int32,
        Number,
        String,
        Symbol,
        ObjectWithStructure,
        ObjectWithStructureOrOther,
        Object,
        ObjectOrOther,
        Top
    };

    static Kind kindForFlags(PutByIdFlags);
    
    static bool hasStructure(Kind kind)
    {
        switch (kind) {
        case ObjectWithStructure:
        case ObjectWithStructureOrOther:
            return true;
        default:
            return false;
        }
    }

    class Descriptor {
    public:
        Descriptor()
            : m_kind(Bottom)
            , m_structure(nullptr)
        {
        }

        Descriptor(Kind kind, Structure* structure = nullptr)
            : m_kind(kind)
            , m_structure(structure)
        {
            if (hasStructure(kind))
                ASSERT(structure);
        }

        static Descriptor forValue(JSValue);

        static Descriptor forFlags(VM&, PutByIdFlags);

        explicit operator bool() const
        {
            return m_kind != Bottom || m_structure;
        }

        Kind kind() const { return m_kind; }
        Structure* structure() const { return m_structure; }

        PutByIdFlags putByIdFlags() const;

        ALWAYS_INLINE bool includesValue(JSValue value)
        {
            switch (m_kind) {
            case Bottom:
                return false;
            case Boolean:
                return value.isBoolean();
            case Other:
                return value.isUndefinedOrNull();
            case Int32:
                return value.isInt32();
            case Number:
                return value.isNumber();
            case String:
                return value.isString();
            case Symbol:
                return value.isSymbol();
            case ObjectWithStructure:
                return value.isCell() && value.asCell()->structure() == m_structure;
            case ObjectWithStructureOrOther:
                return value.isUndefinedOrNull()
                    || (value.isCell() && value.asCell()->structure() == m_structure);
            case Object:
                return value.isObject();
            case ObjectOrOther:
                return value.isUndefinedOrNull() || value.isObject();
            case Top:
                return true;
            }

            RELEASE_ASSERT_NOT_REACHED();
        }

        bool operator==(const Descriptor& other) const
        {
            return m_kind == other.m_kind
                && m_structure == other.m_structure;
        }

        bool operator!=(const Descriptor& other) const
        {
            return !(*this == other);
        }

        unsigned hash() const
        {
            return WTF::PtrHash<Structure*>::hash(m_structure) ^ static_cast<unsigned>(m_kind);
        }
    
        void merge(const Descriptor&);
        void removeStructure();

        // Returns true if this descriptor is more general than the other one.
        bool subsumes(const Descriptor&) const;

        void dumpInContext(PrintStream&, DumpContext*) const;
        void dump(PrintStream&) const;

    private:
        Kind m_kind;
        Structure* m_structure;
    };

    ConcurrentJSLock& lock() const { return m_lock; }

    Descriptor descriptorMainThread() const
    {
        return Descriptor(m_kind, m_structure ? m_structure->structure() : nullptr);
    }
    
    Descriptor descriptor(const ConcurrentJSLocker&) const
    {
        return descriptorMainThread();
    }
    Descriptor descriptor() const
    {
        ConcurrentJSLocker locker(m_lock);
        return descriptor(locker);
    }
    
    Kind kind(const ConcurrentJSLocker& locker) const { return descriptor(locker).kind(); }

    bool isTop() const { return m_kind == Top; }
    bool isRelevant() const { return m_kind != Top; }

    // Returns true if the InferredType is still relevant after the store. It's not relevant if it's Top.
    ALWAYS_INLINE bool willStoreValue(VM& vm, PropertyName propertyName, JSValue value)
    {
        Descriptor currentDescriptor = descriptorMainThread();
        if (currentDescriptor.includesValue(value))
            return currentDescriptor.kind() != Top;
        return willStoreValueSlow(vm, propertyName, value);
    }

    // Immediately makes this type irrelevant.
    void makeTop(VM& vm, PropertyName propertyName)
    {
        if (isTop())
            return;
        makeTopSlow(vm, propertyName);
    }

    // Returns true if it currently makes sense to watch this InferredType for this descriptor. Note that
    // this will always return false for Top.
    bool canWatch(const ConcurrentJSLocker&, const Descriptor&);
    bool canWatch(const Descriptor&);
    
    void addWatchpoint(const ConcurrentJSLocker&, Watchpoint*);
    void addWatchpoint(Watchpoint*);

    void dump(PrintStream&) const;

private:
    InferredType(VM&);
    ~InferredType();

    bool willStoreValueSlow(VM&, PropertyName, JSValue);
    void makeTopSlow(VM&, PropertyName);

    // Helper for willStoreValueSlow() and makeTopSlow(). This returns true if we should fire the
    // watchpoint set.
    bool set(const ConcurrentJSLocker&, VM&, Descriptor);
    
    void removeStructure();

    mutable ConcurrentJSLock m_lock;
    
    Kind m_kind { Bottom };

    class InferredStructureWatchpoint : public Watchpoint {
    public:
        InferredStructureWatchpoint() { }
    protected:
        void fireInternal(const FireDetail&) override;
    };

    class InferredStructureFinalizer : public UnconditionalFinalizer {
    public:
        InferredStructureFinalizer() { }
    protected:
        void finalizeUnconditionally() override;
    };

    class InferredStructure : public ThreadSafeRefCounted<InferredStructure> {
    public:
        InferredStructure(VM&, InferredType* parent, Structure*);

        Structure* structure() const { return m_structure.get(); };

    private:
        friend class InferredType;
        friend class InferredStructureWatchpoint;
        friend class InferredStructureFinalizer;
        
        InferredType* m_parent;
        WriteBarrier<Structure> m_structure;

        InferredStructureWatchpoint m_watchpoint;
        InferredStructureFinalizer m_finalizer;
    };

    RefPtr<InferredStructure> m_structure;

    // NOTE: If this is being watched, we transform to Top because that implies that it wouldn't be
    // profitable to watch it again. Also, this set is initialized clear, and is never exposed to the DFG
    // thread. The DFG will use the InferredType as the thing that it watches.
    InlineWatchpointSet m_watchpointSet;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::InferredType::Kind);

} // namespace WTF
