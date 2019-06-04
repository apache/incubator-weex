/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "JSFunctionInlines.h"
#include "ObjectPropertyConditionSet.h"

namespace JSC {

struct AccessGenerationState;

// An AccessCase describes one of the cases of a PolymorphicAccess. A PolymorphicAccess represents a
// planned (to generate in future) or generated stub for some inline cache. That stub contains fast
// path code for some finite number of fast cases, each described by an AccessCase object.
//
// An AccessCase object has a lifecycle that proceeds through several states. Note that the states
// of AccessCase have a lot to do with the global effect epoch (we'll say epoch for short). This is
// a simple way of reasoning about the state of the system outside this AccessCase. Any observable
// effect - like storing to a property, changing an object's structure, etc. - increments the epoch.
// The states are:
//
// Primordial:   This is an AccessCase that was just allocated. It does not correspond to any actual
//               code and it is not owned by any PolymorphicAccess. In this state, the AccessCase
//               assumes that it is in the same epoch as when it was created. This is important
//               because it may make claims about itself ("I represent a valid case so long as you
//               register a watchpoint on this set") that could be contradicted by some outside
//               effects (like firing and deleting the watchpoint set in question). This is also the
//               state that an AccessCase is in when it is cloned (AccessCase::clone()).
//
// Committed:    This happens as soon as some PolymorphicAccess takes ownership of this AccessCase.
//               In this state, the AccessCase no longer assumes anything about the epoch. To
//               accomplish this, PolymorphicAccess calls AccessCase::commit(). This must be done
//               during the same epoch when the AccessCase was created, either by the client or by
//               clone(). When created by the client, committing during the same epoch works because
//               we can be sure that whatever watchpoint sets they spoke of are still valid. When
//               created by clone(), we can be sure that the set is still valid because the original
//               of the clone still has watchpoints on it.
//
// Generated:    This is the state when the PolymorphicAccess generates code for this case by
//               calling AccessCase::generate() or AccessCase::generateWithGuard(). At this point
//               the case object will have some extra stuff in it, like possibly the CallLinkInfo
//               object associated with the inline cache.
//               FIXME: Moving into the Generated state should not mutate the AccessCase object or
//               put more stuff into it. If we fix this, then we can get rid of AccessCase::clone().
//               https://bugs.webkit.org/show_bug.cgi?id=156456
//
// An AccessCase may be destroyed while in any of these states.
//
// We will sometimes buffer committed AccessCases in the PolymorphicAccess object before generating
// code. This allows us to only regenerate once we've accumulated (hopefully) more than one new
// AccessCase.
class AccessCase {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum AccessType : uint8_t {
        Load,
        Transition,
        Replace,
        Miss,
        GetGetter,
        Getter,
        Setter,
        CustomValueGetter,
        CustomAccessorGetter,
        CustomValueSetter,
        CustomAccessorSetter,
        IntrinsicGetter,
        InHit,
        InMiss,
        ArrayLength,
        StringLength,
        DirectArgumentsLength,
        ScopedArgumentsLength,
        ModuleNamespaceLoad,
    };

    enum State : uint8_t {
        Primordial,
        Committed,
        Generated
    };

    template<typename T>
    T& as() { return *static_cast<T*>(this); }

    template<typename T>
    const T& as() const { return *static_cast<const T*>(this); }


    template<typename AccessCaseType, typename... Arguments>
    static std::unique_ptr<AccessCaseType> create(Arguments... arguments)
    {
        return std::unique_ptr<AccessCaseType>(new AccessCaseType(arguments...));
    }

    static std::unique_ptr<AccessCase> create(VM&, JSCell* owner, AccessType, PropertyOffset = invalidOffset,
        Structure* = nullptr, const ObjectPropertyConditionSet& = ObjectPropertyConditionSet());

    // This create method should be used for transitions.
    static std::unique_ptr<AccessCase> create(VM&, JSCell* owner, PropertyOffset, Structure* oldStructure,
        Structure* newStructure, const ObjectPropertyConditionSet& = ObjectPropertyConditionSet());

    static std::unique_ptr<AccessCase> fromStructureStubInfo(VM&, JSCell* owner, StructureStubInfo&);

    AccessType type() const { return m_type; }
    State state() const { return m_state; }
    PropertyOffset offset() const { return m_offset; }

    Structure* structure() const
    {
        if (m_type == Transition)
            return m_structure->previousID();
        return m_structure.get();
    }
    bool guardedByStructureCheck() const;

    Structure* newStructure() const
    {
        ASSERT(m_type == Transition);
        return m_structure.get();
    }

    ObjectPropertyConditionSet conditionSet() const { return m_conditionSet; }

    virtual JSObject* alternateBase() const { return conditionSet().slotBaseCondition().object(); }
    virtual WatchpointSet* additionalSet() const { return nullptr; }
    virtual bool viaProxy() const { return false; }

    // If you supply the optional vector, this will append the set of cells that this will need to keep alive
    // past the call.
    bool doesCalls(Vector<JSCell*>* cellsToMark = nullptr) const;

    bool isGetter() const
    {
        switch (type()) {
        case Getter:
        case CustomValueGetter:
        case CustomAccessorGetter:
            return true;
        default:
            return false;
        }
    }

    bool isAccessor() const { return isGetter() || type() == Setter; }

    // Is it still possible for this case to ever be taken? Must call this as a prerequisite for
    // calling generate() and friends. If this returns true, then you can call generate(). If
    // this returns false, then generate() will crash. You must call generate() in the same epoch
    // as when you called couldStillSucceed().
    bool couldStillSucceed() const;

    // If this method returns true, then it's a good idea to remove 'other' from the access once 'this'
    // is added. This method assumes that in case of contradictions, 'this' represents a newer, and so
    // more useful, truth. This method can be conservative; it will return false when it doubt.
    bool canReplace(const AccessCase& other) const;

    void dump(PrintStream& out) const;
    virtual void dumpImpl(PrintStream&, CommaPrinter&) const { }

    virtual ~AccessCase();

protected:
    AccessCase(VM&, JSCell* owner, AccessType, PropertyOffset, Structure*, const ObjectPropertyConditionSet&);
    AccessCase(const AccessCase&) = default;
    AccessCase& operator=(const AccessCase&) = delete;
    void resetState() { m_state = Primordial; }

private:
    friend class CodeBlock;
    friend class PolymorphicAccess;

    bool visitWeak(VM&) const;
    bool propagateTransitions(SlotVisitor&) const;

    // FIXME: This only exists because of how AccessCase puts post-generation things into itself.
    // https://bugs.webkit.org/show_bug.cgi?id=156456
    virtual std::unique_ptr<AccessCase> clone() const;

    // Perform any action that must be performed before the end of the epoch in which the case
    // was created. Returns a set of watchpoint sets that will need to be watched.
    Vector<WatchpointSet*, 2> commit(VM&, const Identifier&);

    // Fall through on success. Two kinds of failures are supported: fall-through, which means that we
    // should try a different case; and failure, which means that this was the right case but it needs
    // help from the slow path.
    void generateWithGuard(AccessGenerationState&, MacroAssembler::JumpList& fallThrough);

    // Fall through on success, add a jump to the failure list on failure.
    void generate(AccessGenerationState&);

    void generateImpl(AccessGenerationState&);

    AccessType m_type;
    State m_state { Primordial };
    PropertyOffset m_offset;

    // Usually this is the structure that we expect the base object to have. But, this is the *new*
    // structure for a transition and we rely on the fact that it has a strong reference to the old
    // structure. For proxies, this is the structure of the object behind the proxy.
    WriteBarrier<Structure> m_structure;

    ObjectPropertyConditionSet m_conditionSet;
};

} // namespace JSC

#endif
