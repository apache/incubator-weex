/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "PropertyCondition.h"

#include "GetterSetter.h"
#include "JSCInlines.h"
#include "TrackedReferences.h"

namespace JSC {

static bool verbose = false;

void PropertyCondition::dumpInContext(PrintStream& out, DumpContext* context) const
{
    if (!*this) {
        out.print("<invalid>");
        return;
    }
    
    out.print(m_kind, " of ", m_uid);
    switch (m_kind) {
    case Presence:
        out.print(" at ", offset(), " with attributes ", attributes());
        return;
    case Absence:
    case AbsenceOfSetter:
        out.print(" with prototype ", inContext(JSValue(prototype()), context));
        return;
    case Equivalence:
        out.print(" with ", inContext(requiredValue(), context));
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void PropertyCondition::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

bool PropertyCondition::isStillValidAssumingImpurePropertyWatchpoint(
    Structure* structure, JSObject* base) const
{
    if (verbose) {
        dataLog(
            "Determining validity of ", *this, " with structure ", pointerDump(structure), " and base ",
            JSValue(base), " assuming impure property watchpoints are set.\n");
    }
    
    if (!*this) {
        if (verbose)
            dataLog("Invalid because unset.\n");
        return false;
    }
    
    if (!structure->propertyAccessesAreCacheable()) {
        if (verbose)
            dataLog("Invalid because accesses are not cacheable.\n");
        return false;
    }
    
    switch (m_kind) {
    case Presence: {
        unsigned currentAttributes;
        PropertyOffset currentOffset = structure->getConcurrently(uid(), currentAttributes);
        if (currentOffset != offset() || currentAttributes != attributes()) {
            if (verbose) {
                dataLog(
                    "Invalid because we need offset, attributes to be ", offset(), ", ", attributes(),
                    " but they are ", currentOffset, ", ", currentAttributes, "\n");
            }
            return false;
        }
        return true;
    }
        
    case Absence: {
        if (structure->isDictionary()) {
            if (verbose)
                dataLog("Invalid because it's a dictionary.\n");
            return false;
        }

        PropertyOffset currentOffset = structure->getConcurrently(uid());
        if (currentOffset != invalidOffset) {
            if (verbose)
                dataLog("Invalid because the property exists at offset: ", currentOffset, "\n");
            return false;
        }
        
        if (structure->storedPrototypeObject() != prototype()) {
            if (verbose) {
                dataLog(
                    "Invalid because the prototype is ", structure->storedPrototype(), " even though "
                    "it should have been ", JSValue(prototype()), "\n");
            }
            return false;
        }
        
        return true;
    }
    
    case AbsenceOfSetter: {
        if (structure->isDictionary()) {
            if (verbose)
                dataLog("Invalid because it's a dictionary.\n");
            return false;
        }
        
        unsigned currentAttributes;
        PropertyOffset currentOffset = structure->getConcurrently(uid(), currentAttributes);
        if (currentOffset != invalidOffset) {
            if (currentAttributes & (Accessor | CustomAccessor)) {
                if (verbose) {
                    dataLog(
                        "Invalid because we expected not to have a setter, but we have one at offset ",
                        currentOffset, " with attributes ", currentAttributes, "\n");
                }
                return false;
            }
        }
        
        if (structure->storedPrototypeObject() != prototype()) {
            if (verbose) {
                dataLog(
                    "Invalid because the prototype is ", structure->storedPrototype(), " even though "
                    "it should have been ", JSValue(prototype()), "\n");
            }
            return false;
        }
        
        return true;
    }
        
    case Equivalence: {
        if (!base || base->structure() != structure) {
            // Conservatively return false, since we cannot verify this one without having the
            // object.
            if (verbose) {
                dataLog(
                    "Invalid because we don't have a base or the base has the wrong structure: ",
                    RawPointer(base), "\n");
            }
            return false;
        }
        
        // FIXME: This is somewhat racy, and maybe more risky than we want.
        // https://bugs.webkit.org/show_bug.cgi?id=134641
        
        PropertyOffset currentOffset = structure->getConcurrently(uid());
        if (currentOffset == invalidOffset) {
            if (verbose) {
                dataLog(
                    "Invalid because the base no long appears to have ", uid(), " on its structure: ",
                        RawPointer(base), "\n");
            }
            return false;
        }

        JSValue currentValue = base->getDirect(currentOffset);
        if (currentValue != requiredValue()) {
            if (verbose) {
                dataLog(
                    "Invalid because the value is ", currentValue, " but we require ", requiredValue(),
                    "\n");
            }
            return false;
        }
        
        return true;
    } }
    
    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

bool PropertyCondition::validityRequiresImpurePropertyWatchpoint(Structure* structure) const
{
    if (!*this)
        return false;
    
    switch (m_kind) {
    case Presence:
    case Absence:
    case Equivalence:
        return structure->needImpurePropertyWatchpoint();
    default:
        return false;
    }
}

bool PropertyCondition::isStillValid(Structure* structure, JSObject* base) const
{
    if (!isStillValidAssumingImpurePropertyWatchpoint(structure, base))
        return false;

    // Currently we assume that an impure property can cause a property to appear, and can also
    // "shadow" an existing JS property on the same object. Hence it affects both presence and
    // absence. It doesn't affect AbsenceOfSetter because impure properties aren't ever setters.
    switch (m_kind) {
    case Absence:
        if (structure->typeInfo().getOwnPropertySlotIsImpure() || structure->typeInfo().getOwnPropertySlotIsImpureForPropertyAbsence())
            return false;
        break;
    case Presence:
    case Equivalence:
        if (structure->typeInfo().getOwnPropertySlotIsImpure())
            return false;
        break;
    default:
        break;
    }
    
    return true;
}

bool PropertyCondition::isWatchableWhenValid(
    Structure* structure, WatchabilityEffort effort) const
{
    if (structure->transitionWatchpointSetHasBeenInvalidated())
        return false;
    
    switch (m_kind) {
    case Equivalence: {
        PropertyOffset offset = structure->getConcurrently(uid());
        
        // This method should only be called when some variant of isValid returned true, which
        // implies that we already confirmed that the structure knows of the property. We should
        // also have verified that the Structure is a cacheable dictionary, which means we
        // shouldn't have a TOCTOU race either.
        RELEASE_ASSERT(offset != invalidOffset);
        
        WatchpointSet* set = nullptr;
        switch (effort) {
        case MakeNoChanges:
            set = structure->propertyReplacementWatchpointSet(offset);
            break;
        case EnsureWatchability:
            set = structure->ensurePropertyReplacementWatchpointSet(
                *Heap::heap(structure)->vm(), offset);
            break;
        }
        
        if (!set || !set->isStillValid())
            return false;
        
        break;
    }
        
    default:
        break;
    }
    
    return true;
}

bool PropertyCondition::isWatchableAssumingImpurePropertyWatchpoint(
    Structure* structure, JSObject* base, WatchabilityEffort effort) const
{
    return isStillValidAssumingImpurePropertyWatchpoint(structure, base)
        && isWatchableWhenValid(structure, effort);
}

bool PropertyCondition::isWatchable(
    Structure* structure, JSObject* base, WatchabilityEffort effort) const
{
    return isStillValid(structure, base)
        && isWatchableWhenValid(structure, effort);
}

bool PropertyCondition::isStillLive() const
{
    if (hasPrototype() && prototype() && !Heap::isMarked(prototype()))
        return false;
    
    if (hasRequiredValue()
        && requiredValue()
        && requiredValue().isCell()
        && !Heap::isMarked(requiredValue().asCell()))
        return false;
    
    return true;
}

void PropertyCondition::validateReferences(const TrackedReferences& tracked) const
{
    if (hasPrototype())
        tracked.check(prototype());
    
    if (hasRequiredValue())
        tracked.check(requiredValue());
}

bool PropertyCondition::isValidValueForAttributes(VM& vm, JSValue value, unsigned attributes)
{
    bool attributesClaimAccessor = !!(attributes & Accessor);
    bool valueClaimsAccessor = !!jsDynamicCast<GetterSetter*>(vm, value);
    return attributesClaimAccessor == valueClaimsAccessor;
}

bool PropertyCondition::isValidValueForPresence(VM& vm, JSValue value) const
{
    return isValidValueForAttributes(vm, value, attributes());
}

PropertyCondition PropertyCondition::attemptToMakeEquivalenceWithoutBarrier(VM& vm, JSObject* base) const
{
    Structure* structure = base->structure();
    if (!structure->isValidOffset(offset()))
        return PropertyCondition();
    JSValue value = base->getDirect(offset());
    if (!isValidValueForPresence(vm, value))
        return PropertyCondition();
    return equivalenceWithoutBarrier(uid(), value);
}

} // namespace JSC

namespace WTF {

void printInternal(PrintStream& out, JSC::PropertyCondition::Kind condition)
{
    switch (condition) {
    case JSC::PropertyCondition::Presence:
        out.print("Presence");
        return;
    case JSC::PropertyCondition::Absence:
        out.print("Absence");
        return;
    case JSC::PropertyCondition::AbsenceOfSetter:
        out.print("Absence");
        return;
    case JSC::PropertyCondition::Equivalence:
        out.print("Equivalence");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF
