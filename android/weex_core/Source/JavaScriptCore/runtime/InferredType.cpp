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

#include "config.h"
#include "InferredType.h"

#include "JSCInlines.h"

namespace JSC {

namespace {

class InferredTypeFireDetail : public FireDetail {
public:
    InferredTypeFireDetail(
        InferredType* inferredType, UniquedStringImpl* uid,
        const InferredType::Descriptor& oldDescriptor, const InferredType::Descriptor& newDescriptor,
        JSValue offendingValue)
        : m_inferredType(inferredType)
        , m_uid(uid)
        , m_oldDescriptor(oldDescriptor)
        , m_newDescriptor(newDescriptor)
        , m_offendingValue(offendingValue)
    {
    }

protected:
    void dump(PrintStream& out) const override
    {
        out.print(
            "Inferred type changed on ", RawPointer(m_inferredType), " for property ", m_uid, ": "
            "old type was ", m_oldDescriptor, " while desired type is ", m_newDescriptor);
        if (m_offendingValue)
            out.print(" due to ", m_offendingValue);
    }

private:
    InferredType* m_inferredType;
    RefPtr<UniquedStringImpl> m_uid;
    InferredType::Descriptor m_oldDescriptor;
    InferredType::Descriptor m_newDescriptor;
    JSValue m_offendingValue;
};

} // anonymous namespace

const ClassInfo InferredType::s_info = { "InferredType", 0, 0, CREATE_METHOD_TABLE(InferredType) };

InferredType* InferredType::create(VM& vm)
{
    InferredType* result = new (NotNull, allocateCell<InferredType>(vm.heap)) InferredType(vm);
    result->finishCreation(vm);
    return result;
}

void InferredType::destroy(JSCell* cell)
{
    InferredType* inferredType = static_cast<InferredType*>(cell);
    inferredType->InferredType::~InferredType();
}

Structure* InferredType::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

void InferredType::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    InferredType* inferredType = jsCast<InferredType*>(cell);
    
    ConcurrentJSLocker locker(inferredType->m_lock);

    if (inferredType->m_structure) {
        // The mutator may clear the structure before the GC runs finalizers, so we have to protect
        // the finalizer from being destroyed.
        inferredType->m_structure->ref();
        visitor.addUnconditionalFinalizer(&inferredType->m_structure->m_finalizer);
    }
}

InferredType::Kind InferredType::kindForFlags(PutByIdFlags flags)
{
    switch (flags & PutByIdPrimaryTypeMask) {
    case PutByIdPrimaryTypeSecondary:
        switch (flags & PutByIdSecondaryTypeMask) {
        case PutByIdSecondaryTypeBottom:
            return Bottom;
        case PutByIdSecondaryTypeBoolean:
            return Boolean;
        case PutByIdSecondaryTypeOther:
            return Other;
        case PutByIdSecondaryTypeInt32:
            return Int32;
        case PutByIdSecondaryTypeNumber:
            return Number;
        case PutByIdSecondaryTypeString:
            return String;
        case PutByIdSecondaryTypeSymbol:
            return Symbol;
        case PutByIdSecondaryTypeObject:
            return Object;
        case PutByIdSecondaryTypeObjectOrOther:
            return ObjectOrOther;
        case PutByIdSecondaryTypeTop:
            return Top;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return Bottom;
        }
    case PutByIdPrimaryTypeObjectWithStructure:
        return ObjectWithStructure;
    case PutByIdPrimaryTypeObjectWithStructureOrOther:
        return ObjectWithStructureOrOther;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return Bottom;
    }
}

InferredType::Descriptor InferredType::Descriptor::forValue(JSValue value)
{
    if (value.isBoolean())
        return Boolean;
    if (value.isUndefinedOrNull())
        return Other;
    if (value.isInt32())
        return Int32;
    if (value.isNumber())
        return Number;
    if (value.isCell()) {
        JSCell* cell = value.asCell();
        if (cell->isString())
            return String;
        if (cell->isSymbol())
            return Symbol;
        if (cell->isObject()) {
            if (cell->structure()->transitionWatchpointSetIsStillValid())
                return Descriptor(ObjectWithStructure, cell->structure());
            return Object;
        }
    }
    return Top;
}

InferredType::Descriptor InferredType::Descriptor::forFlags(VM& vm, PutByIdFlags flags)
{
    Kind kind = kindForFlags(flags);
    Structure* structure;
    if (hasStructure(kind))
        structure = vm.heap.structureIDTable().get(decodeStructureID(flags));
    else
        structure = nullptr;
    return Descriptor(kind, structure);
}

PutByIdFlags InferredType::Descriptor::putByIdFlags() const
{
    switch (m_kind) {
    case Bottom:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeBottom);
    case Boolean:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeBoolean);
    case Other:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeOther);
    case Int32:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeInt32);
    case Number:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeNumber);
    case String:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeString);
    case Symbol:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeSymbol);
    case Object:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeObject);
    case ObjectOrOther:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeObjectOrOther);
    case Top:
        return static_cast<PutByIdFlags>(PutByIdPrimaryTypeSecondary | PutByIdSecondaryTypeTop);
    case ObjectWithStructure:
        return static_cast<PutByIdFlags>(
            PutByIdPrimaryTypeObjectWithStructure | encodeStructureID(m_structure->id()));
    case ObjectWithStructureOrOther:
        return static_cast<PutByIdFlags>(
            PutByIdPrimaryTypeObjectWithStructureOrOther | encodeStructureID(m_structure->id()));
    }
    RELEASE_ASSERT_NOT_REACHED();
    return PutByIdNone;
}

void InferredType::Descriptor::merge(const Descriptor& other)
{
    // Filter out common things to simplify the switch statement below.
    if (*this == other)
        return;
    if (other.m_kind == Bottom)
        return;

    switch (m_kind) {
    case Bottom:
        *this = other;
        return;
    case Boolean:
    case String:
    case Symbol:
        *this = Top;
        return;
    case Other:
        switch (other.m_kind) {
        case ObjectWithStructure:
        case ObjectWithStructureOrOther:
            *this = Descriptor(ObjectWithStructureOrOther, other.structure());
            return;
        case Object:
        case ObjectOrOther:
            *this = ObjectOrOther;
            return;
        default:
            *this = Top;
            return;
        }
    case Int32:
        switch (other.m_kind) {
        case Number:
            *this = Number;
            return;
        default:
            *this = Top;
            return;
        }
    case Number:
        switch (other.m_kind) {
        case Int32:
            return;
        default:
            *this = Top;
            return;
        }
    case ObjectWithStructure:
        switch (other.m_kind) {
        case ObjectWithStructure: // If we see this here, then we know that the structures didn't match.
        case Object:
            *this = Object;
            return;
        case ObjectWithStructureOrOther:
            if (m_structure == other.m_structure) {
                *this = other;
                return;
            }
            *this = ObjectOrOther;
            return;
        case ObjectOrOther:
            *this = ObjectOrOther;
            return;
        case Other:
            m_kind = ObjectWithStructureOrOther;
            return;
        default:
            *this = Top;
            return;
        }
    case ObjectWithStructureOrOther:
        switch (other.m_kind) {
        case ObjectWithStructure:
            if (m_structure == other.m_structure)
                return;
            *this = ObjectOrOther;
            return;
        case Object:
        case ObjectWithStructureOrOther: // If we see this here, then we know that the structures didn't match.
        case ObjectOrOther:
            *this = ObjectOrOther;
            return;
        case Other:
            return;
        default:
            *this = Top;
            return;
        }
    case Object:
        switch (other.m_kind) {
        case ObjectWithStructure:
            return;
        case ObjectWithStructureOrOther:
        case ObjectOrOther:
        case Other:
            *this = ObjectOrOther;
            return;
        default:
            *this = Top;
            return;
        }
    case ObjectOrOther:
        switch (other.m_kind) {
        case ObjectWithStructure:
        case ObjectWithStructureOrOther:
        case Object:
        case Other:
            return;
        default:
            *this = Top;
            return;
        }
    case Top:
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void InferredType::Descriptor::removeStructure()
{
    switch (m_kind) {
    case ObjectWithStructure:
        *this = Object;
        return;
    case ObjectWithStructureOrOther:
        *this = ObjectOrOther;
        return;
    default:
        return;
    }
}

bool InferredType::Descriptor::subsumes(const Descriptor& other) const
{
    Descriptor merged = *this;
    merged.merge(other);
    return *this == merged;
}

void InferredType::Descriptor::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print(m_kind);
    if (m_structure)
        out.print(":", inContext(*m_structure, context));
}

void InferredType::Descriptor::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

InferredType::InferredType(VM& vm)
    : Base(vm, vm.inferredTypeStructure.get())
    , m_watchpointSet(ClearWatchpoint)
{
}

InferredType::~InferredType()
{
}

bool InferredType::canWatch(const ConcurrentJSLocker& locker, const Descriptor& expected)
{
    if (expected.kind() == Top)
        return false;
    
    return descriptor(locker) == expected;
}

bool InferredType::canWatch(const Descriptor& expected)
{
    ConcurrentJSLocker locker(m_lock);
    return canWatch(locker, expected);
}

void InferredType::addWatchpoint(const ConcurrentJSLocker& locker, Watchpoint* watchpoint)
{
    RELEASE_ASSERT(descriptor(locker).kind() != Top);

    m_watchpointSet.add(watchpoint);
}

void InferredType::addWatchpoint(Watchpoint* watchpoint)
{
    ConcurrentJSLocker locker(m_lock);
    addWatchpoint(locker, watchpoint);
}

void InferredType::dump(PrintStream& out) const
{
    out.print(RawPointer(this), ":", descriptor());
}

bool InferredType::willStoreValueSlow(VM& vm, PropertyName propertyName, JSValue value)
{
    Descriptor oldType;
    Descriptor myType;
    bool result;
    {
        ConcurrentJSLocker locker(m_lock);
        oldType = descriptor(locker);
        myType = Descriptor::forValue(value);

        myType.merge(oldType);
        
        ASSERT(oldType != myType); // The type must have changed if we're on the slow path.

        bool setResult = set(locker, vm, myType);
        result = kind(locker) != Top;
        if (!setResult)
            return result;
    }
    
    InferredTypeFireDetail detail(this, propertyName.uid(), oldType, myType, value);
    m_watchpointSet.fireAll(vm, detail);
    return result;
}

void InferredType::makeTopSlow(VM& vm, PropertyName propertyName)
{
    Descriptor oldType;
    {
        ConcurrentJSLocker locker(m_lock);
        oldType = descriptor(locker);
        if (!set(locker, vm, Top))
            return;
    }

    InferredTypeFireDetail detail(this, propertyName.uid(), oldType, Top, JSValue());
    m_watchpointSet.fireAll(vm, detail);
}

bool InferredType::set(const ConcurrentJSLocker& locker, VM& vm, Descriptor newDescriptor)
{
    // We will trigger write barriers while holding our lock. Currently, write barriers don't GC, but that
    // could change. If it does, we don't want to deadlock. Note that we could have used
    // GCSafeConcurrentJSLocker in the caller, but the caller is on a fast path so maybe that wouldn't be
    // a good idea.
    DeferGCForAWhile deferGC(vm.heap);
    
    // Be defensive: if we're not really changing the type, then we don't have to do anything.
    if (descriptor(locker) == newDescriptor)
        return false;

    bool shouldFireWatchpointSet = false;
    
    // The new descriptor must be more general than the previous one.
    ASSERT(newDescriptor.subsumes(descriptor(locker)));

    // If the new descriptors have different structures, then it can only be because one is null.
    if (descriptor(locker).structure() != newDescriptor.structure())
        ASSERT(!descriptor(locker).structure() || !newDescriptor.structure());

    // We are changing the type, so make sure that if anyone was watching, they find out about it now. If
    // anyone is watching, we make sure to go to Top so that we don't do this sort of thing again.
    if (m_watchpointSet.state() != ClearWatchpoint) {
        // We cannot have been invalidated, since if we were, then we'd already be at Top.
        ASSERT(m_watchpointSet.state() != IsInvalidated);

        // We're about to do expensive things because some compiler thread decided to watch this type and
        // then the type changed. Assume that this property is crazy, and don't ever do any more things for
        // it.
        newDescriptor = Top;

        shouldFireWatchpointSet = true;
    }

    // Remove the old InferredStructure object if we no longer need it.
    if (!newDescriptor.structure())
        m_structure = nullptr;

    // Add a new InferredStructure object if we need one now.
    if (newDescriptor.structure()) {
        if (m_structure) {
            // We should agree on the structures if we get here.
            ASSERT(newDescriptor.structure() == m_structure->structure());
        } else {
            m_structure = adoptRef(new InferredStructure(vm, this, newDescriptor.structure()));
            newDescriptor.structure()->addTransitionWatchpoint(&m_structure->m_watchpoint);
        }
    }

    // Finally, set the descriptor kind.
    m_kind = newDescriptor.kind();

    // Assert that we did things.
    ASSERT(descriptor(locker) == newDescriptor);

    return shouldFireWatchpointSet;
}

void InferredType::removeStructure()
{
    // FIXME: Find an elegant and cheap way to thread information about why we got here into the fire
    // detail in set().
    
    VM& vm = *Heap::heap(this)->vm();

    Descriptor oldDescriptor;
    Descriptor newDescriptor;
    {
        ConcurrentJSLocker locker(m_lock);
        oldDescriptor = descriptor(locker);
        newDescriptor = oldDescriptor;
        newDescriptor.removeStructure();
        
        if (!set(locker, vm, newDescriptor))
            return;
    }

    InferredTypeFireDetail detail(this, nullptr, oldDescriptor, newDescriptor, JSValue());
    m_watchpointSet.fireAll(vm, detail);
}

void InferredType::InferredStructureWatchpoint::fireInternal(const FireDetail&)
{
    InferredStructure* inferredStructure =
        bitwise_cast<InferredStructure*>(
            bitwise_cast<char*>(this) - OBJECT_OFFSETOF(InferredStructure, m_watchpoint));

    inferredStructure->m_parent->removeStructure();
}

void InferredType::InferredStructureFinalizer::finalizeUnconditionally()
{
    InferredStructure* inferredStructure =
        bitwise_cast<InferredStructure*>(
            bitwise_cast<char*>(this) - OBJECT_OFFSETOF(InferredStructure, m_finalizer));
    
    ASSERT(Heap::isMarked(inferredStructure->m_parent));
    
    // Monotonicity ensures that we shouldn't see a new structure that is different from us, but we
    // could have been nulled. We only rely on it being the null case only in debug.
    if (inferredStructure == inferredStructure->m_parent->m_structure.get()) {
        if (!Heap::isMarked(inferredStructure->m_structure.get()))
            inferredStructure->m_parent->removeStructure();
    } else
        ASSERT(!inferredStructure->m_parent->m_structure);
    
    inferredStructure->deref();
}

InferredType::InferredStructure::InferredStructure(VM& vm, InferredType* parent, Structure* structure)
    : m_parent(parent)
    , m_structure(vm, parent, structure)
{
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, InferredType::Kind kind)
{
    switch (kind) {
    case InferredType::Bottom:
        out.print("Bottom");
        return;
    case InferredType::Boolean:
        out.print("Boolean");
        return;
    case InferredType::Other:
        out.print("Other");
        return;
    case InferredType::Int32:
        out.print("Int32");
        return;
    case InferredType::Number:
        out.print("Number");
        return;
    case InferredType::String:
        out.print("String");
        return;
    case InferredType::Symbol:
        out.print("Symbol");
        return;
    case InferredType::ObjectWithStructure:
        out.print("ObjectWithStructure");
        return;
    case InferredType::ObjectWithStructureOrOther:
        out.print("ObjectWithStructureOrOther");
        return;
    case InferredType::Object:
        out.print("Object");
        return;
    case InferredType::ObjectOrOther:
        out.print("ObjectOrOther");
        return;
    case InferredType::Top:
        out.print("Top");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

