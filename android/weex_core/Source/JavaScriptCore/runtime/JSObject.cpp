/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel (eric@webkit.org)
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
#include "JSObject.h"

#include "ButterflyInlines.h"
#include "CustomGetterSetter.h"
#include "DatePrototype.h"
#include "ErrorConstructor.h"
#include "Exception.h"
#include "GetterSetter.h"
#include "HeapSnapshotBuilder.h"
#include "IndexingHeaderInlines.h"
#include "JSCInlines.h"
#include "JSCustomGetterSetterFunction.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "Lookup.h"
#include "NativeErrorConstructor.h"
#include "Nodes.h"
#include "ObjectPrototype.h"
#include "PropertyDescriptor.h"
#include "PropertyNameArray.h"
#include "PrototypeMapInlines.h"
#include "ProxyObject.h"
#include "SlotVisitorInlines.h"
#include "TypeError.h"
#include <math.h>
#include <wtf/Assertions.h>

namespace JSC {

// We keep track of the size of the last array after it was grown. We use this
// as a simple heuristic for as the value to grow the next array from size 0.
// This value is capped by the constant FIRST_VECTOR_GROW defined in
// ArrayConventions.h.
static unsigned lastArraySize = 0;

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSObject);
STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSFinalObject);

const char* const NonExtensibleObjectPropertyDefineError = "Attempting to define property on object that is not extensible.";
const char* const ReadonlyPropertyWriteError = "Attempted to assign to readonly property.";
const char* const ReadonlyPropertyChangeError = "Attempting to change value of a readonly property.";
const char* const UnableToDeletePropertyError = "Unable to delete property.";
const char* const UnconfigurablePropertyChangeAccessMechanismError = "Attempting to change access mechanism for an unconfigurable property.";
const char* const UnconfigurablePropertyChangeConfigurabilityError = "Attempting to change configurable attribute of unconfigurable property.";
const char* const UnconfigurablePropertyChangeEnumerabilityError = "Attempting to change enumerable attribute of unconfigurable property.";
const char* const UnconfigurablePropertyChangeWritabilityError = "Attempting to change writable attribute of unconfigurable property.";

const ClassInfo JSObject::s_info = { "Object", 0, 0, CREATE_METHOD_TABLE(JSObject) };

const ClassInfo JSFinalObject::s_info = { "Object", &Base::s_info, 0, CREATE_METHOD_TABLE(JSFinalObject) };

static inline void getClassPropertyNames(ExecState* exec, const ClassInfo* classInfo, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    VM& vm = exec->vm();

    // Add properties from the static hashtables of properties
    for (; classInfo; classInfo = classInfo->parentClass) {
        const HashTable* table = classInfo->staticPropHashTable;
        if (!table)
            continue;

        for (auto iter = table->begin(); iter != table->end(); ++iter) {
            if (!(iter->attributes() & DontEnum) || mode.includeDontEnumProperties())
                propertyNames.add(Identifier::fromString(&vm, iter.key()));
        }
    }
}

ALWAYS_INLINE void JSObject::markAuxiliaryAndVisitOutOfLineProperties(SlotVisitor& visitor, Butterfly* butterfly, Structure* structure, PropertyOffset lastOffset)
{
    // We call this when we found everything without races.
    ASSERT(structure);
    
    if (!butterfly)
        return;
    
    bool hasIndexingHeader = structure->hasIndexingHeader(this);
    size_t preCapacity;
    if (hasIndexingHeader)
        preCapacity = butterfly->indexingHeader()->preCapacity(structure);
    else
        preCapacity = 0;
    
    HeapCell* base = bitwise_cast<HeapCell*>(
        butterfly->base(preCapacity, Structure::outOfLineCapacity(lastOffset)));
    
    ASSERT(Heap::heap(base) == visitor.heap());
    
    visitor.markAuxiliary(base);
    
    unsigned outOfLineSize = Structure::outOfLineSize(lastOffset);
    visitor.appendValuesHidden(butterfly->propertyStorage() - outOfLineSize, outOfLineSize);
}

ALWAYS_INLINE Structure* JSObject::visitButterfly(SlotVisitor& visitor)
{
    static const char* raceReason = "JSObject::visitButterfly";
    Structure* result = visitButterflyImpl(visitor);
    if (!result)
        visitor.didRace(this, raceReason);
    return result;
}

ALWAYS_INLINE Structure* JSObject::visitButterflyImpl(SlotVisitor& visitor)
{
    VM& vm = visitor.vm();
    
    Butterfly* butterfly;
    Structure* structure;
    PropertyOffset lastOffset;
    
    if (visitor.mutatorIsStopped()) {
        butterfly = this->butterfly();
        structure = this->structure(vm);
        lastOffset = structure->lastOffset();
        
        markAuxiliaryAndVisitOutOfLineProperties(visitor, butterfly, structure, lastOffset);
        
        switch (structure->indexingType()) {
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            visitor.appendValuesHidden(butterfly->contiguous().data(), butterfly->publicLength());
            break;
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            visitor.appendValuesHidden(butterfly->arrayStorage()->m_vector, butterfly->arrayStorage()->vectorLength());
            if (butterfly->arrayStorage()->m_sparseMap)
                visitor.append(butterfly->arrayStorage()->m_sparseMap);
            break;
        default:
            break;
        }
        return structure;
    }
    
    // We want to ensure that we only scan the butterfly if we have an exactly matched structure and an
    // exactly matched size. The mutator is required to perform the following shenanigans when
    // reallocating the butterfly with a concurrent collector, with all fencing necessary to ensure
    // that this executes as if under sequential consistency:
    //
    //     object->structure = nuke(object->structure)
    //     object->butterfly = newButterfly
    //     structure->m_offset = newLastOffset
    //     object->structure = newStructure
    //
    // It's OK to skip this when reallocating the butterfly in a way that does not affect the m_offset.
    // We have other protocols in place for that.
    //
    // Note that the m_offset can change without the structure changing, but in that case the mutator
    // will still store null to the structure.
    //
    // The collector will ensure that it always sees a matched butterfly/structure by reading the
    // structure before and after reading the butterfly. For simplicity, let's first consider the case
    // where the only way to change the outOfLineCapacity is to change the structure. This works
    // because the mutator performs the following steps sequentially:
    //
    //     NukeStructure ChangeButterfly PutNewStructure
    //
    // Meanwhile the collector performs the following steps sequentially:
    //
    //     ReadStructureEarly ReadButterfly ReadStructureLate
    //
    // The collector is allowed to do any of these three things:
    //
    // BEFORE: Scan the object with the structure and butterfly *before* the mutator's transition.
    // AFTER: Scan the object with the structure and butterfly *after* the mutator's transition.
    // IGNORE: Ignore the butterfly and call didRace to schedule us to be revisted again in the future.
    //
    // In other words, the collector will never see any torn structure/butterfly mix. It will
    // always see the structure/butterfly before the transition or after but not in between.
    //
    // We can prove that this is correct by exhaustively considering all interleavings:
    //
    // NukeStructure ChangeButterfly PutNewStructure ReadStructureEarly ReadButterfly ReadStructureLate: AFTER, trivially.
    // NukeStructure ChangeButterfly ReadStructureEarly PutNewStructure ReadButterfly ReadStructureLate: IGNORE, because nuked structure read early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadButterfly PutNewStructure ReadStructureLate: IGNORE, because nuked structure read early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ChangeButterfly PutNewStructure ReadButterfly ReadStructureLate: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadButterfly PutNewStructure ReadStructureLate: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ReadButterfly ChangeButterfly PutNewStructure ReadStructureLate: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ReadButterfly ChangeButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read early
    // NukeStructure ReadStructureEarly ReadButterfly ReadStructureLate ChangeButterfly PutNewStructure: IGNORE, because nuked structure read early
    // ReadStructureEarly NukeStructure ChangeButterfly PutNewStructure ReadButterfly ReadStructureLate: IGNORE, because early and late structures don't match
    // ReadStructureEarly NukeStructure ChangeButterfly ReadButterfly PutNewStructure ReadStructureLate: IGNORE, because early and late structures don't match
    // ReadStructureEarly NukeStructure ChangeButterfly ReadButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read late
    // ReadStructureEarly NukeStructure ReadButterfly ChangeButterfly PutNewStructure ReadStructureLate: IGNORE, because early and late structures don't match
    // ReadStructureEarly NukeStructure ReadButterfly ChangeButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read late
    // ReadStructureEarly NukeStructure ReadButterfly ReadStructureLate ChangeButterfly PutNewStructure: IGNORE, because nuked structure read late
    // ReadStructureEarly ReadButterfly NukeStructure ChangeButterfly PutNewStructure ReadStructureLate: IGNORE, because early and late structures don't match
    // ReadStructureEarly ReadButterfly NukeStructure ChangeButterfly ReadStructureLate PutNewStructure: IGNORE, because nuked structure read late
    // ReadStructureEarly ReadButterfly NukeStructure ReadStructureLate ChangeButterfly PutNewStructure: IGNORE, because nuked structure read late
    // ReadStructureEarly ReadButterfly ReadStructureLate NukeStructure ChangeButterfly PutNewStructure: BEFORE, trivially.
    //
    // But we additionally have to worry about the size changing. We make this work by requiring that
    // the collector reads the size early and late as well. Lets consider the interleaving of the
    // mutator changing the size without changing the structure:
    //
    //     NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure
    //
    // Meanwhile the collector does:
    //
    //     ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate
    //
    // The collector can detect races by not only comparing the early structure to the late structure
    // (which will be the same before and after the algorithm runs) but also by comparing the early and
    // late lastOffsets.  Note: the IGNORE proofs do not cite all of the reasons why the collector will
    // ignore the case, since we only need to identify one to say that we're in the ignore case.
    //
    // NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate: AFTER, trivially
    // NukeStructure ChangeButterfly ChangeLastOffset ReadStructureEarly RestoreStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ChangeLastOffset ReadStructureEarly ReadLastOffsetEarly RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ChangeLastOffset ReadStructureEarly ReadLastOffsetEarly ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ChangeLastOffset ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ChangeLastOffset ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ChangeLastOffset RestoreStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ChangeLastOffset ReadLastOffsetEarly RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ChangeLastOffset ReadLastOffsetEarly ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ChangeButterfly ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ChangeLastOffset ReadLastOffsetEarly RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // NukeStructure ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeButterfly ChangeLastOffset RestoreStructure: IGNORE, read nuked structure early
    // ReadStructureEarly NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate: AFTER, the ReadStructureEarly sees the same structure as after and everything else runs after.
    // ReadStructureEarly NukeStructure ChangeButterfly ChangeLastOffset ReadLastOffsetEarly RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: AFTER, as above and the ReadLastOffsetEarly sees the lastOffset after.
    // ReadStructureEarly NukeStructure ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: AFTER, as above and the ReadButterfly sees the right butterfly after.
    // ReadStructureEarly NukeStructure ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ChangeButterfly ChangeLastOffset ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ChangeButterfly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ChangeButterfly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ChangeButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ChangeButterfly ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly NukeStructure ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeButterfly ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure ReadButterfly ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ChangeLastOffset ReadButterfly RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ChangeLastOffset ReadButterfly ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ChangeButterfly ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ChangeButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ReadStructureLate ChangeButterfly ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ReadStructureLate ChangeButterfly ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly NukeStructure ReadButterfly ReadStructureLate ReadLastOffsetLate ChangeButterfly ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure ReadStructureLate ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ChangeLastOffset ReadStructureLate RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ChangeLastOffset ReadStructureLate ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ReadStructureLate ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ReadStructureLate ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ChangeButterfly ReadStructureLate ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ReadStructureLate ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ReadStructureLate ChangeButterfly ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ReadStructureLate ChangeButterfly ReadLastOffsetLate ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly NukeStructure ReadStructureLate ReadLastOffsetLate ChangeButterfly ChangeLastOffset RestoreStructure: IGNORE, read nuked structure late
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure ReadLastOffsetLate: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate NukeStructure ChangeButterfly ChangeLastOffset ReadLastOffsetLate RestoreStructure: IGNORE, read different offsets
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate NukeStructure ChangeButterfly ReadLastOffsetLate ChangeLastOffset RestoreStructure: BEFORE, reads the offset before, everything else happens before
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate NukeStructure ReadLastOffsetLate ChangeButterfly ChangeLastOffset RestoreStructure: BEFORE, reads the offset before, everything else happens before
    // ReadStructureEarly ReadLastOffsetEarly ReadButterfly ReadStructureLate ReadLastOffsetLate NukeStructure ChangeButterfly ChangeLastOffset RestoreStructure: BEFORE, trivially
    //
    // Whew.
    //
    // What the collector is doing is just the "double collect" snapshot from "The Unbounded Single-
    // Writer Algorithm" from Yehuda Afek et al's "Atomic Snapshots of Shared Memory" in JACM 1993,
    // also available here:
    //
    // http://people.csail.mit.edu/shanir/publications/AADGMS.pdf
    //
    // Unlike Afek et al's algorithm, ours does not require extra hacks to force wait-freedom (see
    // "Observation 2" in the paper). This simplifies the whole algorithm. Instead we are happy with
    // obstruction-freedom, and like any good obstruction-free algorithm, we ensure progress using
    // scheduling. We also only collect the butterfly once instead of twice; this optimization seems
    // to hold up in my proofs above and I'm not sure it's part of Afek et al's algos.
    //
    // For more background on this kind of madness, I like this paper; it's where I learned about
    // both the snapshot algorithm and obstruction-freedom:
    //
    // Lunchangco, Moir, Shavit. "Nonblocking k-compare-single-swap." SPAA '03
    // https://pdfs.semanticscholar.org/343f/7182cde7669ca2a7de3dc01127927f384ef7.pdf
    
    StructureID structureID = this->structureID();
    if (isNuked(structureID))
        return nullptr;
    structure = vm.getStructure(structureID);
    lastOffset = structure->lastOffset();
    IndexingType indexingType = structure->indexingType();
    Dependency indexingTypeDependency = dependency(indexingType);
    Locker<JSCell> locker(NoLockingNecessary);
    switch (indexingType) {
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        // We need to hold this lock to protect against changes to the innards of the butterfly
        // that can happen when the butterfly is used for array storage. We conservatively
        // assume that a contiguous butterfly may transform into an array storage one, though
        // this is probably more conservative than necessary.
        locker = Locker<JSCell>(*this);
        break;
    default:
        break;
    }
    butterfly = consume(this, indexingTypeDependency)->butterfly();
    Dependency butterflyDependency = dependency(butterfly);
    if (!butterfly)
        return structure;
    if (consume(this, butterflyDependency)->structureID() != structureID)
        return nullptr;
    if (consume(structure, butterflyDependency)->lastOffset() != lastOffset)
        return nullptr;
    
    markAuxiliaryAndVisitOutOfLineProperties(visitor, butterfly, structure, lastOffset);
    
    ASSERT(indexingType == structure->indexingType());
    
    switch (indexingType) {
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        visitor.appendValuesHidden(butterfly->contiguous().data(), butterfly->publicLength());
        break;
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        visitor.appendValuesHidden(butterfly->arrayStorage()->m_vector, butterfly->arrayStorage()->vectorLength());
        if (butterfly->arrayStorage()->m_sparseMap)
            visitor.append(butterfly->arrayStorage()->m_sparseMap);
        break;
    default:
        break;
    }
    
    return structure;
}

size_t JSObject::estimatedSize(JSCell* cell)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    size_t butterflyOutOfLineSize = thisObject->m_butterfly ? thisObject->structure()->outOfLineSize() : 0;
    return Base::estimatedSize(cell) + butterflyOutOfLineSize;
}

void JSObject::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
#if !ASSERT_DISABLED
    bool wasCheckingForDefaultMarkViolation = visitor.m_isCheckingForDefaultMarkViolation;
    visitor.m_isCheckingForDefaultMarkViolation = false;
#endif
    
    JSCell::visitChildren(thisObject, visitor);
    
    thisObject->visitButterfly(visitor);
    
#if !ASSERT_DISABLED
    visitor.m_isCheckingForDefaultMarkViolation = wasCheckingForDefaultMarkViolation;
#endif
}

void JSObject::heapSnapshot(JSCell* cell, HeapSnapshotBuilder& builder)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    Base::heapSnapshot(cell, builder);

    Structure* structure = thisObject->structure();
    for (auto& entry : structure->getPropertiesConcurrently()) {
        JSValue toValue = thisObject->getDirect(entry.offset);
        if (toValue && toValue.isCell())
            builder.appendPropertyNameEdge(thisObject, toValue.asCell(), entry.key);
    }

    Butterfly* butterfly = thisObject->m_butterfly.get();
    if (butterfly) {
        WriteBarrier<Unknown>* data = nullptr;
        uint32_t count = 0;

        switch (thisObject->indexingType()) {
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            data = butterfly->contiguous().data();
            count = butterfly->publicLength();
            break;
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            data = butterfly->arrayStorage()->m_vector;
            count = butterfly->arrayStorage()->vectorLength();
            break;
        default:
            break;
        }

        for (uint32_t i = 0; i < count; ++i) {
            JSValue toValue = data[i].get();
            if (toValue && toValue.isCell())
                builder.appendIndexEdge(thisObject, toValue.asCell(), i);
        }
    }
}

void JSFinalObject::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSFinalObject* thisObject = jsCast<JSFinalObject*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
#if !ASSERT_DISABLED
    bool wasCheckingForDefaultMarkViolation = visitor.m_isCheckingForDefaultMarkViolation;
    visitor.m_isCheckingForDefaultMarkViolation = false;
#endif
    
    JSCell::visitChildren(thisObject, visitor);
    
    if (Structure* structure = thisObject->visitButterfly(visitor)) {
        if (unsigned storageSize = structure->inlineSize())
            visitor.appendValuesHidden(thisObject->inlineStorage(), storageSize);
    }
    
#if !ASSERT_DISABLED
    visitor.m_isCheckingForDefaultMarkViolation = wasCheckingForDefaultMarkViolation;
#endif
}

String JSObject::className(const JSObject* object)
{
    const ClassInfo* info = object->classInfo(*object->vm());
    ASSERT(info);
    return info->className;
}

String JSObject::toStringName(const JSObject* object, ExecState*)
{
    const ClassInfo* info = object->classInfo(*object->vm());
    ASSERT(info);
    return info->methodTable.className(object);
}

String JSObject::calculatedClassName(JSObject* object)
{
    String prototypeFunctionName;
    auto globalObject = object->globalObject();
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    ExecState* exec = globalObject->globalExec();
    PropertySlot slot(object->getPrototypeDirect(), PropertySlot::InternalMethodType::VMInquiry);
    PropertyName constructor(exec->propertyNames().constructor);
    if (object->getPropertySlot(exec, constructor, slot)) {
        if (slot.isValue()) {
            JSValue constructorValue = slot.getValue(exec, constructor);
            if (constructorValue.isCell()) {
                if (JSCell* constructorCell = constructorValue.asCell()) {
                    if (JSObject* ctorObject = constructorCell->getObject()) {
                        if (JSFunction* constructorFunction = jsDynamicCast<JSFunction*>(vm, ctorObject))
                            prototypeFunctionName = constructorFunction->calculatedDisplayName(vm);
                        else if (InternalFunction* constructorFunction = jsDynamicCast<InternalFunction*>(vm, ctorObject))
                            prototypeFunctionName = constructorFunction->calculatedDisplayName(vm);
                    }
                }
            }
        }
    }
    ASSERT(!scope.exception() || prototypeFunctionName.isNull());
    if (UNLIKELY(scope.exception()))
        scope.clearException();

    if (prototypeFunctionName.isNull() || prototypeFunctionName == "Object") {
        String tableClassName = object->methodTable()->className(object);
        if (!tableClassName.isNull() && tableClassName != "Object")
            return tableClassName;

        String classInfoName = object->classInfo(vm)->className;
        if (!classInfoName.isNull())
            return classInfoName;

        if (prototypeFunctionName.isNull())
            return ASCIILiteral("Object");
    }

    return prototypeFunctionName;
}

bool JSObject::getOwnPropertySlotByIndex(JSObject* thisObject, ExecState* exec, unsigned i, PropertySlot& slot)
{
    // NB. The fact that we're directly consulting our indexed storage implies that it is not
    // legal for anyone to override getOwnPropertySlot() without also overriding
    // getOwnPropertySlotByIndex().
    
    if (i > MAX_ARRAY_INDEX)
        return thisObject->methodTable(exec->vm())->getOwnPropertySlot(thisObject, exec, Identifier::from(exec, i), slot);
    
    switch (thisObject->indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
    case ALL_UNDECIDED_INDEXING_TYPES:
        break;
        
    case ALL_INT32_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES: {
        Butterfly* butterfly = thisObject->butterfly();
        if (i >= butterfly->vectorLength())
            return false;
        
        JSValue value = butterfly->contiguous()[i].get();
        if (value) {
            slot.setValue(thisObject, None, value);
            return true;
        }
        
        return false;
    }
        
    case ALL_DOUBLE_INDEXING_TYPES: {
        Butterfly* butterfly = thisObject->butterfly();
        if (i >= butterfly->vectorLength())
            return false;
        
        double value = butterfly->contiguousDouble()[i];
        if (value == value) {
            slot.setValue(thisObject, None, JSValue(JSValue::EncodeAsDouble, value));
            return true;
        }
        
        return false;
    }
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = thisObject->m_butterfly.get()->arrayStorage();
        if (i >= storage->length())
            return false;
        
        if (i < storage->vectorLength()) {
            JSValue value = storage->m_vector[i].get();
            if (value) {
                slot.setValue(thisObject, None, value);
                return true;
            }
        } else if (SparseArrayValueMap* map = storage->m_sparseMap.get()) {
            SparseArrayValueMap::iterator it = map->find(i);
            if (it != map->notFound()) {
                it->value.get(thisObject, slot);
                return true;
            }
        }
        break;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
    
    return false;
}

// https://tc39.github.io/ecma262/#sec-ordinaryset
bool ordinarySetSlow(ExecState* exec, JSObject* object, PropertyName propertyName, JSValue value, JSValue receiver, bool shouldThrow)
{
    // If we find the receiver is not the same to the object, we fall to this slow path.
    // Currently, there are 3 candidates.
    // 1. Reflect.set can alter the receiver with an arbitrary value.
    // 2. Window Proxy.
    // 3. ES6 Proxy.

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* current = object;
    PropertyDescriptor ownDescriptor;
    while (true) {
        if (current->type() == ProxyObjectType && propertyName != vm.propertyNames->underscoreProto) {
            ProxyObject* proxy = jsCast<ProxyObject*>(current);
            PutPropertySlot slot(receiver, shouldThrow);
            return proxy->ProxyObject::put(proxy, exec, propertyName, value, slot);
        }

        // 9.1.9.1-2 Let ownDesc be ? O.[[GetOwnProperty]](P).
        bool ownDescriptorFound = current->getOwnPropertyDescriptor(exec, propertyName, ownDescriptor);
        RETURN_IF_EXCEPTION(scope, false);

        if (!ownDescriptorFound) {
            // 9.1.9.1-3-a Let parent be ? O.[[GetPrototypeOf]]().
            JSValue prototype = current->getPrototype(vm, exec);
            RETURN_IF_EXCEPTION(scope, false);

            // 9.1.9.1-3-b If parent is not null, then
            if (!prototype.isNull()) {
                // 9.1.9.1-3-b-i Return ? parent.[[Set]](P, V, Receiver).
                current = asObject(prototype);
                continue;
            }
            // 9.1.9.1-3-c-i Let ownDesc be the PropertyDescriptor{[[Value]]: undefined, [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}.
            ownDescriptor = PropertyDescriptor(jsUndefined(), None);
        }
        break;
    }

    // 9.1.9.1-4 If IsDataDescriptor(ownDesc) is true, then
    if (ownDescriptor.isDataDescriptor()) {
        // 9.1.9.1-4-a If ownDesc.[[Writable]] is false, return false.
        if (!ownDescriptor.writable())
            return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

        // 9.1.9.1-4-b If Type(Receiver) is not Object, return false.
        if (!receiver.isObject())
            return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

        // In OrdinarySet, the receiver may not be the same to the object.
        // So, we perform [[GetOwnProperty]] onto the receiver while we already perform [[GetOwnProperty]] onto the object.

        // 9.1.9.1-4-c Let existingDescriptor be ? Receiver.[[GetOwnProperty]](P).
        JSObject* receiverObject = asObject(receiver);
        PropertyDescriptor existingDescriptor;
        bool existingDescriptorFound = receiverObject->getOwnPropertyDescriptor(exec, propertyName, existingDescriptor);
        RETURN_IF_EXCEPTION(scope, false);

        // 9.1.9.1-4-d If existingDescriptor is not undefined, then
        if (existingDescriptorFound) {
            // 9.1.9.1-4-d-i If IsAccessorDescriptor(existingDescriptor) is true, return false.
            if (existingDescriptor.isAccessorDescriptor())
                return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

            // 9.1.9.1-4-d-ii If existingDescriptor.[[Writable]] is false, return false.
            if (!existingDescriptor.writable())
                return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

            // 9.1.9.1-4-d-iii Let valueDesc be the PropertyDescriptor{[[Value]]: V}.
            PropertyDescriptor valueDescriptor;
            valueDescriptor.setValue(value);

            // 9.1.9.1-4-d-iv Return ? Receiver.[[DefineOwnProperty]](P, valueDesc).
            return receiverObject->methodTable(vm)->defineOwnProperty(receiverObject, exec, propertyName, valueDescriptor, shouldThrow);
        }

        // 9.1.9.1-4-e Else Receiver does not currently have a property P,
        // 9.1.9.1-4-e-i Return ? CreateDataProperty(Receiver, P, V).
        return receiverObject->methodTable(vm)->defineOwnProperty(receiverObject, exec, propertyName, PropertyDescriptor(value, None), shouldThrow);
    }

    // 9.1.9.1-5 Assert: IsAccessorDescriptor(ownDesc) is true.
    ASSERT(ownDescriptor.isAccessorDescriptor());

    // 9.1.9.1-6 Let setter be ownDesc.[[Set]].
    // 9.1.9.1-7 If setter is undefined, return false.
    JSValue setter = ownDescriptor.setter();
    if (!setter.isObject())
        return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

    // 9.1.9.1-8 Perform ? Call(setter, Receiver, << V >>).
    JSObject* setterObject = asObject(setter);
    MarkedArgumentBuffer args;
    args.append(value);

    CallData callData;
    CallType callType = setterObject->methodTable(vm)->getCallData(setterObject, callData);
    call(exec, setterObject, callType, callData, receiver, args);

    // 9.1.9.1-9 Return true.
    return true;
}

// ECMA 8.6.2.2
bool JSObject::put(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    return putInline(cell, exec, propertyName, value, slot);
}

bool JSObject::putInlineSlow(ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    ASSERT(!isThisValueAltered(slot, this));

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* obj = this;
    for (;;) {
        unsigned attributes;
        PropertyOffset offset = obj->structure(vm)->get(vm, propertyName, attributes);
        if (isValidOffset(offset)) {
            if (attributes & ReadOnly) {
                ASSERT(structure(vm)->prototypeChainMayInterceptStoreTo(vm, propertyName) || obj == this);
                return typeError(exec, scope, slot.isStrictMode(), ASCIILiteral(ReadonlyPropertyWriteError));
            }

            JSValue gs = obj->getDirect(offset);
            if (gs.isGetterSetter()) {
                bool result = callSetter(exec, slot.thisValue(), gs, value, slot.isStrictMode() ? StrictMode : NotStrictMode);
                if (!structure()->isDictionary())
                    slot.setCacheableSetter(obj, offset);
                return result;
            }
            if (gs.isCustomGetterSetter()) {
                bool result = callCustomSetter(exec, gs, attributes & CustomAccessor, obj, slot.thisValue(), value);
                if (attributes & CustomAccessor)
                    slot.setCustomAccessor(obj, jsCast<CustomGetterSetter*>(gs.asCell())->setter());
                else
                    slot.setCustomValue(obj, jsCast<CustomGetterSetter*>(gs.asCell())->setter());
                return result;
            }
            ASSERT(!(attributes & Accessor));

            // If there's an existing property on the object or one of its 
            // prototypes it should be replaced, so break here.
            break;
        }
        if (!obj->staticPropertiesReified()) {
            if (obj->classInfo(vm)->hasStaticSetterOrReadonlyProperties()) {
                if (auto* entry = obj->findPropertyHashEntry(vm, propertyName))
                    return putEntry(exec, entry, obj, this, propertyName, value, slot);
            }
        }
        if (obj->type() == ProxyObjectType && propertyName != vm.propertyNames->underscoreProto) {
            // FIXME: We shouldn't unconditionally perform [[Set]] here.
            // We need to do more because this is observable behavior.
            // https://bugs.webkit.org/show_bug.cgi?id=155012
            ProxyObject* proxy = jsCast<ProxyObject*>(obj);
            return proxy->ProxyObject::put(proxy, exec, propertyName, value, slot);
        }
        JSValue prototype = obj->getPrototypeDirect();
        if (prototype.isNull())
            break;
        obj = asObject(prototype);
    }

    ASSERT(!structure(vm)->prototypeChainMayInterceptStoreTo(vm, propertyName) || obj == this);
    if (!putDirectInternal<PutModePut>(vm, propertyName, value, 0, slot))
        return typeError(exec, scope, slot.isStrictMode(), ASCIILiteral(ReadonlyPropertyWriteError));
    return true;
}

bool JSObject::putByIndex(JSCell* cell, ExecState* exec, unsigned propertyName, JSValue value, bool shouldThrow)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    
    if (propertyName > MAX_ARRAY_INDEX) {
        PutPropertySlot slot(cell, shouldThrow);
        return thisObject->methodTable()->put(thisObject, exec, Identifier::from(exec, propertyName), value, slot);
    }
    
    switch (thisObject->indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
        break;
        
    case ALL_UNDECIDED_INDEXING_TYPES: {
        thisObject->convertUndecidedForValue(exec->vm(), value);
        // Reloop.
        return putByIndex(cell, exec, propertyName, value, shouldThrow);
    }
        
    case ALL_INT32_INDEXING_TYPES: {
        if (!value.isInt32()) {
            thisObject->convertInt32ForValue(exec->vm(), value);
            return putByIndex(cell, exec, propertyName, value, shouldThrow);
        }
        FALLTHROUGH;
    }
        
    case ALL_CONTIGUOUS_INDEXING_TYPES: {
        Butterfly* butterfly = thisObject->butterfly();
        if (propertyName >= butterfly->vectorLength())
            break;
        butterfly->contiguous()[propertyName].set(exec->vm(), thisObject, value);
        if (propertyName >= butterfly->publicLength())
            butterfly->setPublicLength(propertyName + 1);
        return true;
    }
        
    case ALL_DOUBLE_INDEXING_TYPES: {
        if (!value.isNumber()) {
            thisObject->convertDoubleToContiguous(exec->vm());
            // Reloop.
            return putByIndex(cell, exec, propertyName, value, shouldThrow);
        }
        double valueAsDouble = value.asNumber();
        if (valueAsDouble != valueAsDouble) {
            thisObject->convertDoubleToContiguous(exec->vm());
            // Reloop.
            return putByIndex(cell, exec, propertyName, value, shouldThrow);
        }
        Butterfly* butterfly = thisObject->butterfly();
        if (propertyName >= butterfly->vectorLength())
            break;
        butterfly->contiguousDouble()[propertyName] = valueAsDouble;
        if (propertyName >= butterfly->publicLength())
            butterfly->setPublicLength(propertyName + 1);
        return true;
    }
        
    case NonArrayWithArrayStorage:
    case ArrayWithArrayStorage: {
        ArrayStorage* storage = thisObject->m_butterfly.get()->arrayStorage();
        
        if (propertyName >= storage->vectorLength())
            break;
        
        WriteBarrier<Unknown>& valueSlot = storage->m_vector[propertyName];
        unsigned length = storage->length();
        
        // Update length & m_numValuesInVector as necessary.
        if (propertyName >= length) {
            length = propertyName + 1;
            storage->setLength(length);
            ++storage->m_numValuesInVector;
        } else if (!valueSlot)
            ++storage->m_numValuesInVector;
        
        valueSlot.set(exec->vm(), thisObject, value);
        return true;
    }
        
    case NonArrayWithSlowPutArrayStorage:
    case ArrayWithSlowPutArrayStorage: {
        ArrayStorage* storage = thisObject->m_butterfly.get()->arrayStorage();
        
        if (propertyName >= storage->vectorLength())
            break;
        
        WriteBarrier<Unknown>& valueSlot = storage->m_vector[propertyName];
        unsigned length = storage->length();
        
        // Update length & m_numValuesInVector as necessary.
        if (propertyName >= length) {
            bool putResult = false;
            if (thisObject->attemptToInterceptPutByIndexOnHole(exec, propertyName, value, shouldThrow, putResult))
                return putResult;
            length = propertyName + 1;
            storage->setLength(length);
            ++storage->m_numValuesInVector;
        } else if (!valueSlot) {
            bool putResult = false;
            if (thisObject->attemptToInterceptPutByIndexOnHole(exec, propertyName, value, shouldThrow, putResult))
                return putResult;
            ++storage->m_numValuesInVector;
        }
        
        valueSlot.set(exec->vm(), thisObject, value);
        return true;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    return thisObject->putByIndexBeyondVectorLength(exec, propertyName, value, shouldThrow);
}

ArrayStorage* JSObject::enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(VM& vm, ArrayStorage* storage)
{
    SparseArrayValueMap* map = storage->m_sparseMap.get();

    if (!map)
        map = allocateSparseIndexMap(vm);

    if (map->sparseMode())
        return storage;

    map->setSparseMode();

    unsigned usedVectorLength = std::min(storage->length(), storage->vectorLength());
    for (unsigned i = 0; i < usedVectorLength; ++i) {
        JSValue value = storage->m_vector[i].get();
        // This will always be a new entry in the map, so no need to check we can write,
        // and attributes are default so no need to set them.
        if (value)
            map->add(this, i).iterator->value.set(vm, map, value);
    }

    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = storage->butterfly()->resizeArray(vm, this, structure(vm), 0, ArrayStorage::sizeFor(0));
    RELEASE_ASSERT(newButterfly);
    newButterfly->arrayStorage()->m_indexBias = 0;
    newButterfly->arrayStorage()->setVectorLength(0);
    newButterfly->arrayStorage()->m_sparseMap.set(vm, this, map);
    setButterfly(vm, newButterfly);
    
    return newButterfly->arrayStorage();
}

void JSObject::enterDictionaryIndexingMode(VM& vm)
{
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
    case ALL_UNDECIDED_INDEXING_TYPES:
    case ALL_INT32_INDEXING_TYPES:
    case ALL_DOUBLE_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        // NOTE: this is horribly inefficient, as it will perform two conversions. We could optimize
        // this case if we ever cared. Note that ensureArrayStorage() can return null if the object
        // doesn't support traditional indexed properties. At the time of writing, this just affects
        // typed arrays.
        if (ArrayStorage* storage = ensureArrayStorageSlow(vm))
            enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, storage);
        break;
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, m_butterfly.get()->arrayStorage());
        break;
        
    default:
        break;
    }
}

void JSObject::notifyPresenceOfIndexedAccessors(VM& vm)
{
    if (mayInterceptIndexedAccesses())
        return;
    
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AddIndexedAccessors));
    
    if (!vm.prototypeMap.isPrototype(this))
        return;
    
    globalObject(vm)->haveABadTime(vm);
}

Butterfly* JSObject::createInitialIndexedStorage(VM& vm, unsigned length)
{
    ASSERT(length < MAX_ARRAY_INDEX);
    IndexingType oldType = indexingType();
    ASSERT_UNUSED(oldType, !hasIndexedProperties(oldType));
    ASSERT(!structure()->needsSlowPutIndexing());
    ASSERT(!indexingShouldBeSparse());
    Structure* structure = this->structure(vm);
    unsigned propertyCapacity = structure->outOfLineCapacity();
    unsigned vectorLength = Butterfly::optimalContiguousVectorLength(propertyCapacity, length);
    Butterfly* newButterfly = Butterfly::createOrGrowArrayRight(
        m_butterfly.get(), vm, this, structure, propertyCapacity, false, 0,
        sizeof(EncodedJSValue) * vectorLength);
    newButterfly->setPublicLength(length);
    newButterfly->setVectorLength(vectorLength);
    return newButterfly;
}

Butterfly* JSObject::createInitialUndecided(VM& vm, unsigned length)
{
    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = createInitialIndexedStorage(vm, length);
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, NonPropertyTransition::AllocateUndecided);
    nukeStructureAndSetButterfly(vm, oldStructureID, newButterfly);
    setStructure(vm, newStructure);
    return newButterfly;
}

ContiguousJSValues JSObject::createInitialInt32(VM& vm, unsigned length)
{
    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = createInitialIndexedStorage(vm, length);
    for (unsigned i = newButterfly->vectorLength(); i--;)
        newButterfly->contiguousInt32()[i].setWithoutWriteBarrier(JSValue());
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, NonPropertyTransition::AllocateInt32);
    nukeStructureAndSetButterfly(vm, oldStructureID, newButterfly);
    setStructure(vm, newStructure);
    return newButterfly->contiguousInt32();
}

ContiguousDoubles JSObject::createInitialDouble(VM& vm, unsigned length)
{
    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = createInitialIndexedStorage(vm, length);
    for (unsigned i = newButterfly->vectorLength(); i--;)
        newButterfly->contiguousDouble()[i] = PNaN;
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, NonPropertyTransition::AllocateDouble);
    nukeStructureAndSetButterfly(vm, oldStructureID, newButterfly);
    setStructure(vm, newStructure);
    return newButterfly->contiguousDouble();
}

ContiguousJSValues JSObject::createInitialContiguous(VM& vm, unsigned length)
{
    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = createInitialIndexedStorage(vm, length);
    for (unsigned i = newButterfly->vectorLength(); i--;)
        newButterfly->contiguous()[i].setWithoutWriteBarrier(JSValue());
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, NonPropertyTransition::AllocateContiguous);
    nukeStructureAndSetButterfly(vm, oldStructureID, newButterfly);
    setStructure(vm, newStructure);
    return newButterfly->contiguous();
}

Butterfly* JSObject::createArrayStorageButterfly(VM& vm, JSCell* intendedOwner, Structure* structure, unsigned length, unsigned vectorLength, Butterfly* oldButterfly)
{
    Butterfly* newButterfly = Butterfly::createOrGrowArrayRight(
        oldButterfly, vm, intendedOwner, structure, structure->outOfLineCapacity(), false, 0,
        ArrayStorage::sizeFor(vectorLength));
    RELEASE_ASSERT(newButterfly);

    ArrayStorage* result = newButterfly->arrayStorage();
    result->setLength(length);
    result->setVectorLength(vectorLength);
    result->m_sparseMap.clear();
    result->m_numValuesInVector = 0;
    result->m_indexBias = 0;
    for (size_t i = vectorLength; i--;)
        result->m_vector[i].setWithoutWriteBarrier(JSValue());

    return newButterfly;
}

ArrayStorage* JSObject::createArrayStorage(VM& vm, unsigned length, unsigned vectorLength)
{
    DeferGC deferGC(vm.heap);
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    IndexingType oldType = indexingType();
    ASSERT_UNUSED(oldType, !hasIndexedProperties(oldType));

    Butterfly* newButterfly = createArrayStorageButterfly(vm, this, oldStructure, length, vectorLength, m_butterfly.get());
    ArrayStorage* result = newButterfly->arrayStorage();
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, oldStructure->suggestedArrayStorageTransition());
    nukeStructureAndSetButterfly(vm, oldStructureID, newButterfly);
    setStructure(vm, newStructure);
    return result;
}

ArrayStorage* JSObject::createInitialArrayStorage(VM& vm)
{
    return createArrayStorage(
        vm, 0, ArrayStorage::optimalVectorLength(0, structure(vm)->outOfLineCapacity(), 0));
}

ContiguousJSValues JSObject::convertUndecidedToInt32(VM& vm)
{
    ASSERT(hasUndecided(indexingType()));

    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = butterfly->vectorLength(); i--;)
        butterfly->contiguousInt32()[i].setWithoutWriteBarrier(JSValue());

    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateInt32));
    return m_butterfly.get()->contiguousInt32();
}

ContiguousDoubles JSObject::convertUndecidedToDouble(VM& vm)
{
    ASSERT(hasUndecided(indexingType()));

    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = butterfly->vectorLength(); i--;)
        butterfly->contiguousDouble()[i] = PNaN;
    
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateDouble));
    return m_butterfly.get()->contiguousDouble();
}

ContiguousJSValues JSObject::convertUndecidedToContiguous(VM& vm)
{
    ASSERT(hasUndecided(indexingType()));

    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = butterfly->vectorLength(); i--;)
        butterfly->contiguous()[i].setWithoutWriteBarrier(JSValue());

    WTF::storeStoreFence();
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateContiguous));
    return m_butterfly.get()->contiguous();
}

ArrayStorage* JSObject::constructConvertedArrayStorageWithoutCopyingElements(VM& vm, unsigned neededLength)
{
    Structure* structure = this->structure(vm);
    unsigned publicLength = m_butterfly.get()->publicLength();
    unsigned propertyCapacity = structure->outOfLineCapacity();
    unsigned propertySize = structure->outOfLineSize();
    
    Butterfly* newButterfly = Butterfly::createUninitialized(
        vm, this, 0, propertyCapacity, true, ArrayStorage::sizeFor(neededLength));
    
    memcpy(
        newButterfly->propertyStorage() - propertySize,
        m_butterfly.get()->propertyStorage() - propertySize,
        propertySize * sizeof(EncodedJSValue));
    
    ArrayStorage* newStorage = newButterfly->arrayStorage();
    newStorage->setVectorLength(neededLength);
    newStorage->setLength(publicLength);
    newStorage->m_sparseMap.clear();
    newStorage->m_indexBias = 0;
    newStorage->m_numValuesInVector = 0;
    
    return newStorage;
}

ArrayStorage* JSObject::convertUndecidedToArrayStorage(VM& vm, NonPropertyTransition transition)
{
    DeferGC deferGC(vm.heap);
    ASSERT(hasUndecided(indexingType()));

    unsigned vectorLength = m_butterfly.get()->vectorLength();
    ArrayStorage* storage = constructConvertedArrayStorageWithoutCopyingElements(vm, vectorLength);
    
    for (unsigned i = vectorLength; i--;)
        storage->m_vector[i].setWithoutWriteBarrier(JSValue());
    
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, transition);
    nukeStructureAndSetButterfly(vm, oldStructureID, storage->butterfly());
    setStructure(vm, newStructure);
    return storage;
}

ArrayStorage* JSObject::convertUndecidedToArrayStorage(VM& vm)
{
    return convertUndecidedToArrayStorage(vm, structure(vm)->suggestedArrayStorageTransition());
}

ContiguousDoubles JSObject::convertInt32ToDouble(VM& vm)
{
    ASSERT(hasInt32(indexingType()));

    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = butterfly->vectorLength(); i--;) {
        WriteBarrier<Unknown>* current = &butterfly->contiguousInt32()[i];
        double* currentAsDouble = bitwise_cast<double*>(current);
        JSValue v = current->get();
        // NOTE: Since this may be used during initialization, v could be garbage. If it's garbage,
        // that means it will be overwritten later.
        if (!v.isInt32()) {
            *currentAsDouble = PNaN;
            continue;
        }
        *currentAsDouble = v.asInt32();
    }
    
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateDouble));
    return m_butterfly.get()->contiguousDouble();
}

ContiguousJSValues JSObject::convertInt32ToContiguous(VM& vm)
{
    ASSERT(hasInt32(indexingType()));
    
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateContiguous));
    return m_butterfly.get()->contiguous();
}

ArrayStorage* JSObject::convertInt32ToArrayStorage(VM& vm, NonPropertyTransition transition)
{
    DeferGC deferGC(vm.heap);
    ASSERT(hasInt32(indexingType()));

    unsigned vectorLength = m_butterfly.get()->vectorLength();
    ArrayStorage* newStorage = constructConvertedArrayStorageWithoutCopyingElements(vm, vectorLength);
    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = 0; i < vectorLength; i++) {
        JSValue v = butterfly->contiguous()[i].get();
        newStorage->m_vector[i].setWithoutWriteBarrier(v);
        if (v)
            newStorage->m_numValuesInVector++;
    }
    
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, transition);
    nukeStructureAndSetButterfly(vm, oldStructureID, newStorage->butterfly());
    setStructure(vm, newStructure);
    return newStorage;
}

ArrayStorage* JSObject::convertInt32ToArrayStorage(VM& vm)
{
    return convertInt32ToArrayStorage(vm, structure(vm)->suggestedArrayStorageTransition());
}

ContiguousJSValues JSObject::convertDoubleToContiguous(VM& vm)
{
    ASSERT(hasDouble(indexingType()));

    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = butterfly->vectorLength(); i--;) {
        double* current = &butterfly->contiguousDouble()[i];
        WriteBarrier<Unknown>* currentAsValue = bitwise_cast<WriteBarrier<Unknown>*>(current);
        double value = *current;
        if (value != value) {
            currentAsValue->clear();
            continue;
        }
        JSValue v = JSValue(JSValue::EncodeAsDouble, value);
        currentAsValue->setWithoutWriteBarrier(v);
    }
    
    WTF::storeStoreFence();
    setStructure(vm, Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::AllocateContiguous));
    return m_butterfly.get()->contiguous();
}

ArrayStorage* JSObject::convertDoubleToArrayStorage(VM& vm, NonPropertyTransition transition)
{
    DeferGC deferGC(vm.heap);
    ASSERT(hasDouble(indexingType()));

    unsigned vectorLength = m_butterfly.get()->vectorLength();
    ArrayStorage* newStorage = constructConvertedArrayStorageWithoutCopyingElements(vm, vectorLength);
    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = 0; i < vectorLength; i++) {
        double value = butterfly->contiguousDouble()[i];
        newStorage->m_vector[i].setWithoutWriteBarrier(JSValue(JSValue::EncodeAsDouble, value));
        if (value == value)
            newStorage->m_numValuesInVector++;
    }
    
    StructureID oldStructureID = this->structureID();
    Structure* oldStructure = vm.getStructure(oldStructureID);
    Structure* newStructure = Structure::nonPropertyTransition(vm, oldStructure, transition);
    nukeStructureAndSetButterfly(vm, oldStructureID, newStorage->butterfly());
    setStructure(vm, newStructure);
    return newStorage;
}

ArrayStorage* JSObject::convertDoubleToArrayStorage(VM& vm)
{
    return convertDoubleToArrayStorage(vm, structure(vm)->suggestedArrayStorageTransition());
}

ArrayStorage* JSObject::convertContiguousToArrayStorage(VM& vm, NonPropertyTransition transition)
{
    DeferGC deferGC(vm.heap);
    ASSERT(hasContiguous(indexingType()));

    unsigned vectorLength = m_butterfly.get()->vectorLength();
    ArrayStorage* newStorage = constructConvertedArrayStorageWithoutCopyingElements(vm, vectorLength);
    Butterfly* butterfly = m_butterfly.get();
    for (unsigned i = 0; i < vectorLength; i++) {
        JSValue v = butterfly->contiguous()[i].get();
        newStorage->m_vector[i].setWithoutWriteBarrier(v);
        if (v)
            newStorage->m_numValuesInVector++;
    }
    
    Structure* newStructure = Structure::nonPropertyTransition(vm, structure(vm), transition);

    // This has a crazy race with the garbage collector. When changing the butterfly and structure,
    // the mutator always sets the structure last. The collector will always read the structure
    // first. We probably have to follow that convention here as well. This means that the collector
    // will sometimes see the new butterfly (the one with ArrayStorage) with the only structure (the
    // one that says that the butterfly is Contiguous). When scanning Contiguous, the collector will
    // mark word at addresses greater than or equal to the butterfly pointer, up to the publicLength
    // in the butterfly. But an ArrayStorage has some non-pointer header data at low positive
    // offsets from the butterfly - so when this race happens, the collector will surely crash
    // because it will fail to decode two consecutive int32s as if it was a JSValue.
    //
    // Fortunately, we have the JSCell lock for this purpose!
    
    if (vm.heap.mutatorShouldBeFenced()) {
        auto locker = holdLock(*this);
        setStructureIDDirectly(nuke(structureID()));
        WTF::storeStoreFence();
        m_butterfly.set(vm, this, newStorage->butterfly());
        WTF::storeStoreFence();
        setStructure(vm, newStructure);
    } else {
        m_butterfly.set(vm, this, newStorage->butterfly());
        setStructure(vm, newStructure);
    }
    
    return newStorage;
}

ArrayStorage* JSObject::convertContiguousToArrayStorage(VM& vm)
{
    return convertContiguousToArrayStorage(vm, structure(vm)->suggestedArrayStorageTransition());
}

void JSObject::convertUndecidedForValue(VM& vm, JSValue value)
{
    IndexingType type = indexingTypeForValue(value);
    if (type == Int32Shape) {
        convertUndecidedToInt32(vm);
        return;
    }
    
    if (type == DoubleShape) {
        convertUndecidedToDouble(vm);
        return;
    }

    ASSERT(type == ContiguousShape);
    convertUndecidedToContiguous(vm);
}

void JSObject::createInitialForValueAndSet(VM& vm, unsigned index, JSValue value)
{
    if (value.isInt32()) {
        createInitialInt32(vm, index + 1)[index].set(vm, this, value);
        return;
    }
    
    if (value.isDouble()) {
        double doubleValue = value.asNumber();
        if (doubleValue == doubleValue) {
            createInitialDouble(vm, index + 1)[index] = doubleValue;
            return;
        }
    }
    
    createInitialContiguous(vm, index + 1)[index].set(vm, this, value);
}

void JSObject::convertInt32ForValue(VM& vm, JSValue value)
{
    ASSERT(!value.isInt32());
    
    if (value.isDouble() && !std::isnan(value.asDouble())) {
        convertInt32ToDouble(vm);
        return;
    }
    
    convertInt32ToContiguous(vm);
}

void JSObject::setIndexQuicklyToUndecided(VM& vm, unsigned index, JSValue value)
{
    ASSERT(index < m_butterfly.get()->publicLength());
    ASSERT(index < m_butterfly.get()->vectorLength());
    convertUndecidedForValue(vm, value);
    setIndexQuickly(vm, index, value);
}

void JSObject::convertInt32ToDoubleOrContiguousWhilePerformingSetIndex(VM& vm, unsigned index, JSValue value)
{
    ASSERT(!value.isInt32());
    convertInt32ForValue(vm, value);
    setIndexQuickly(vm, index, value);
}

void JSObject::convertDoubleToContiguousWhilePerformingSetIndex(VM& vm, unsigned index, JSValue value)
{
    ASSERT(!value.isNumber() || value.asNumber() != value.asNumber());
    convertDoubleToContiguous(vm);
    setIndexQuickly(vm, index, value);
}

ContiguousJSValues JSObject::ensureInt32Slow(VM& vm)
{
    ASSERT(inherits(vm, info()));
    
    if (structure(vm)->hijacksIndexingHeader())
        return ContiguousJSValues();
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
        if (UNLIKELY(indexingShouldBeSparse() || structure(vm)->needsSlowPutIndexing()))
            return ContiguousJSValues();
        return createInitialInt32(vm, 0);
        
    case ALL_UNDECIDED_INDEXING_TYPES:
        return convertUndecidedToInt32(vm);
        
    case ALL_DOUBLE_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        return ContiguousJSValues();
        
    default:
        CRASH();
        return ContiguousJSValues();
    }
}

ContiguousDoubles JSObject::ensureDoubleSlow(VM& vm)
{
    ASSERT(inherits(vm, info()));
    
    if (structure(vm)->hijacksIndexingHeader())
        return ContiguousDoubles();
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
        if (UNLIKELY(indexingShouldBeSparse() || structure(vm)->needsSlowPutIndexing()))
            return ContiguousDoubles();
        return createInitialDouble(vm, 0);
        
    case ALL_UNDECIDED_INDEXING_TYPES:
        return convertUndecidedToDouble(vm);
        
    case ALL_INT32_INDEXING_TYPES:
        return convertInt32ToDouble(vm);
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        return ContiguousDoubles();
        
    default:
        CRASH();
        return ContiguousDoubles();
    }
}

ContiguousJSValues JSObject::ensureContiguousSlow(VM& vm)
{
    ASSERT(inherits(vm, info()));
    
    if (structure(vm)->hijacksIndexingHeader())
        return ContiguousJSValues();
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
        if (UNLIKELY(indexingShouldBeSparse() || structure(vm)->needsSlowPutIndexing()))
            return ContiguousJSValues();
        return createInitialContiguous(vm, 0);
        
    case ALL_UNDECIDED_INDEXING_TYPES:
        return convertUndecidedToContiguous(vm);
        
    case ALL_INT32_INDEXING_TYPES:
        return convertInt32ToContiguous(vm);
        
    case ALL_DOUBLE_INDEXING_TYPES:
        return convertDoubleToContiguous(vm);
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        return ContiguousJSValues();
        
    default:
        CRASH();
        return ContiguousJSValues();
    }
}

ArrayStorage* JSObject::ensureArrayStorageSlow(VM& vm)
{
    ASSERT(inherits(vm, info()));

    if (structure(vm)->hijacksIndexingHeader())
        return nullptr;
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
        if (UNLIKELY(indexingShouldBeSparse()))
            return ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm);
        return createInitialArrayStorage(vm);
        
    case ALL_UNDECIDED_INDEXING_TYPES:
        ASSERT(!indexingShouldBeSparse());
        ASSERT(!structure(vm)->needsSlowPutIndexing());
        return convertUndecidedToArrayStorage(vm);
        
    case ALL_INT32_INDEXING_TYPES:
        ASSERT(!indexingShouldBeSparse());
        ASSERT(!structure(vm)->needsSlowPutIndexing());
        return convertInt32ToArrayStorage(vm);
        
    case ALL_DOUBLE_INDEXING_TYPES:
        ASSERT(!indexingShouldBeSparse());
        ASSERT(!structure(vm)->needsSlowPutIndexing());
        return convertDoubleToArrayStorage(vm);
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        ASSERT(!indexingShouldBeSparse());
        ASSERT(!structure(vm)->needsSlowPutIndexing());
        return convertContiguousToArrayStorage(vm);
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

ArrayStorage* JSObject::ensureArrayStorageExistsAndEnterDictionaryIndexingMode(VM& vm)
{
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES: {
        createArrayStorage(vm, 0, 0);
        SparseArrayValueMap* map = allocateSparseIndexMap(vm);
        map->setSparseMode();
        return arrayStorage();
    }
        
    case ALL_UNDECIDED_INDEXING_TYPES:
        return enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, convertUndecidedToArrayStorage(vm));
        
    case ALL_INT32_INDEXING_TYPES:
        return enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, convertInt32ToArrayStorage(vm));
        
    case ALL_DOUBLE_INDEXING_TYPES:
        return enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, convertDoubleToArrayStorage(vm));
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        return enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, convertContiguousToArrayStorage(vm));
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        return enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(vm, m_butterfly.get()->arrayStorage());
        
    default:
        CRASH();
        return 0;
    }
}

void JSObject::switchToSlowPutArrayStorage(VM& vm)
{
    switch (indexingType()) {
    case ALL_UNDECIDED_INDEXING_TYPES:
        convertUndecidedToArrayStorage(vm, NonPropertyTransition::AllocateSlowPutArrayStorage);
        break;
        
    case ALL_INT32_INDEXING_TYPES:
        convertInt32ToArrayStorage(vm, NonPropertyTransition::AllocateSlowPutArrayStorage);
        break;
        
    case ALL_DOUBLE_INDEXING_TYPES:
        convertDoubleToArrayStorage(vm, NonPropertyTransition::AllocateSlowPutArrayStorage);
        break;
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        convertContiguousToArrayStorage(vm, NonPropertyTransition::AllocateSlowPutArrayStorage);
        break;
        
    case NonArrayWithArrayStorage:
    case ArrayWithArrayStorage: {
        Structure* newStructure = Structure::nonPropertyTransition(vm, structure(vm), NonPropertyTransition::SwitchToSlowPutArrayStorage);
        setStructure(vm, newStructure);
        break;
    }
        
    default:
        CRASH();
        break;
    }
}

void JSObject::setPrototypeDirect(VM& vm, JSValue prototype)
{
    ASSERT(prototype);
    if (prototype.isObject())
        vm.prototypeMap.addPrototype(asObject(prototype));
    
    Structure* newStructure = Structure::changePrototypeTransition(vm, structure(vm), prototype);
    setStructure(vm, newStructure);
    
    if (!newStructure->anyObjectInChainMayInterceptIndexedAccesses())
        return;
    
    if (vm.prototypeMap.isPrototype(this)) {
        newStructure->globalObject()->haveABadTime(vm);
        return;
    }
    
    if (!hasIndexedProperties(indexingType()))
        return;
    
    if (shouldUseSlowPut(indexingType()))
        return;
    
    switchToSlowPutArrayStorage(vm);
}

bool JSObject::setPrototypeWithCycleCheck(VM& vm, ExecState* exec, JSValue prototype, bool shouldThrowIfCantSet)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    ASSERT(methodTable(vm)->toThis(this, exec, NotStrictMode) == this);

    if (this->getPrototypeDirect() == prototype)
        return true;

    if (this->structure(vm)->isImmutablePrototypeExoticObject())
        return typeError(exec, scope, shouldThrowIfCantSet, ASCIILiteral("Cannot set prototype of immutable prototype object"));

    bool isExtensible = this->isExtensible(exec);
    RETURN_IF_EXCEPTION(scope, false);

    if (!isExtensible)
        return typeError(exec, scope, shouldThrowIfCantSet, ASCIILiteral(ReadonlyPropertyWriteError));

    JSValue nextPrototype = prototype;
    while (nextPrototype && nextPrototype.isObject()) {
        if (nextPrototype == this)
            return typeError(exec, scope, shouldThrowIfCantSet, ASCIILiteral("cyclic __proto__ value"));
        // FIXME: The specification currently says we should check if the [[GetPrototypeOf]] internal method of nextPrototype
        // is not the ordinary object internal method. However, we currently restrict this to Proxy objects as it would allow
        // for cycles with certain HTML objects (WindowProxy, Location) otherwise.
        // https://bugs.webkit.org/show_bug.cgi?id=161534
        if (UNLIKELY(asObject(nextPrototype)->type() == ProxyObjectType))
            break; // We're done. Set the prototype.
        nextPrototype = asObject(nextPrototype)->getPrototypeDirect();
    }
    setPrototypeDirect(vm, prototype);
    return true;
}

bool JSObject::setPrototype(JSObject* object, ExecState* exec, JSValue prototype, bool shouldThrowIfCantSet)
{
    return object->setPrototypeWithCycleCheck(exec->vm(), exec, prototype, shouldThrowIfCantSet);
}

JSValue JSObject::getPrototype(JSObject* object, ExecState*)
{
    return object->getPrototypeDirect();
}

bool JSObject::setPrototype(VM& vm, ExecState* exec, JSValue prototype, bool shouldThrowIfCantSet)
{
    return methodTable(vm)->setPrototype(this, exec, prototype, shouldThrowIfCantSet);
}

bool JSObject::putGetter(ExecState* exec, PropertyName propertyName, JSValue getter, unsigned attributes)
{
    PropertyDescriptor descriptor;
    descriptor.setGetter(getter);

    ASSERT(attributes & Accessor);
    if (!(attributes & ReadOnly))
        descriptor.setConfigurable(true);
    if (!(attributes & DontEnum))
        descriptor.setEnumerable(true);

    return defineOwnProperty(this, exec, propertyName, descriptor, false);
}

bool JSObject::putSetter(ExecState* exec, PropertyName propertyName, JSValue setter, unsigned attributes)
{
    PropertyDescriptor descriptor;
    descriptor.setSetter(setter);

    ASSERT(attributes & Accessor);
    if (!(attributes & ReadOnly))
        descriptor.setConfigurable(true);
    if (!(attributes & DontEnum))
        descriptor.setEnumerable(true);

    return defineOwnProperty(this, exec, propertyName, descriptor, false);
}

bool JSObject::putDirectAccessor(ExecState* exec, PropertyName propertyName, JSValue value, unsigned attributes)
{
    ASSERT(value.isGetterSetter() && (attributes & Accessor));

    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return putDirectIndex(exec, index.value(), value, attributes, PutDirectIndexLikePutDirect);

    return putDirectNonIndexAccessor(exec->vm(), propertyName, value, attributes);
}

bool JSObject::putDirectCustomAccessor(VM& vm, PropertyName propertyName, JSValue value, unsigned attributes)
{
    ASSERT(!parseIndex(propertyName));

    PutPropertySlot slot(this);
    bool result = putDirectInternal<PutModeDefineOwnProperty>(vm, propertyName, value, attributes, slot);

    ASSERT(slot.type() == PutPropertySlot::NewProperty);

    Structure* structure = this->structure(vm);
    if (attributes & ReadOnly)
        structure->setContainsReadOnlyProperties();
    structure->setHasCustomGetterSetterPropertiesWithProtoCheck(propertyName == vm.propertyNames->underscoreProto);
    return result;
}

bool JSObject::putDirectNonIndexAccessor(VM& vm, PropertyName propertyName, JSValue value, unsigned attributes)
{
    PutPropertySlot slot(this);
    bool result = putDirectInternal<PutModeDefineOwnProperty>(vm, propertyName, value, attributes, slot);

    Structure* structure = this->structure(vm);
    if (attributes & ReadOnly)
        structure->setContainsReadOnlyProperties();

    structure->setHasGetterSetterPropertiesWithProtoCheck(propertyName == vm.propertyNames->underscoreProto);
    return result;
}

// HasProperty(O, P) from Section 7.3.10 of the spec.
// http://www.ecma-international.org/ecma-262/6.0/index.html#sec-hasproperty
bool JSObject::hasProperty(ExecState* exec, PropertyName propertyName) const
{
    return hasPropertyGeneric(exec, propertyName, PropertySlot::InternalMethodType::HasProperty);
}

bool JSObject::hasProperty(ExecState* exec, unsigned propertyName) const
{
    return hasPropertyGeneric(exec, propertyName, PropertySlot::InternalMethodType::HasProperty);
}

bool JSObject::hasPropertyGeneric(ExecState* exec, PropertyName propertyName, PropertySlot::InternalMethodType internalMethodType) const
{
    PropertySlot slot(this, internalMethodType);
    return const_cast<JSObject*>(this)->getPropertySlot(exec, propertyName, slot);
}

bool JSObject::hasPropertyGeneric(ExecState* exec, unsigned propertyName, PropertySlot::InternalMethodType internalMethodType) const
{
    PropertySlot slot(this, internalMethodType);
    return const_cast<JSObject*>(this)->getPropertySlot(exec, propertyName, slot);
}

// ECMA 8.6.2.5
bool JSObject::deleteProperty(JSCell* cell, ExecState* exec, PropertyName propertyName)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    VM& vm = exec->vm();
    
    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return thisObject->methodTable(vm)->deletePropertyByIndex(thisObject, exec, index.value());

    unsigned attributes;

    if (!thisObject->staticPropertiesReified()) {
        if (auto* entry = thisObject->findPropertyHashEntry(vm, propertyName)) {
            // If the static table contains a non-configurable (DontDelete) property then we can return early;
            // if there is a property in the storage array it too must be non-configurable (the language does
            // not allow repacement of a non-configurable property with a configurable one).
            if (entry->attributes() & DontDelete && vm.deletePropertyMode() != VM::DeletePropertyMode::IgnoreConfigurable) {
                ASSERT(!isValidOffset(thisObject->structure(vm)->get(vm, propertyName, attributes)) || attributes & DontDelete);
                return false;
            }
            thisObject->reifyAllStaticProperties(exec);
        }
    }

    Structure* structure = thisObject->structure(vm);

    bool propertyIsPresent = isValidOffset(structure->get(vm, propertyName, attributes));
    if (propertyIsPresent) {
        if (attributes & DontDelete && vm.deletePropertyMode() != VM::DeletePropertyMode::IgnoreConfigurable)
            return false;

        PropertyOffset offset;
        if (structure->isUncacheableDictionary())
            offset = structure->removePropertyWithoutTransition(vm, propertyName, [] (const ConcurrentJSLocker&, PropertyOffset) { });
        else
            thisObject->setStructure(vm, Structure::removePropertyTransition(vm, structure, propertyName, offset));

        if (offset != invalidOffset)
            thisObject->putDirectUndefined(offset);
    }

    return true;
}

bool JSObject::deletePropertyByIndex(JSCell* cell, ExecState* exec, unsigned i)
{
    JSObject* thisObject = jsCast<JSObject*>(cell);
    
    if (i > MAX_ARRAY_INDEX)
        return thisObject->methodTable(exec->vm())->deleteProperty(thisObject, exec, Identifier::from(exec, i));
    
    switch (thisObject->indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
    case ALL_UNDECIDED_INDEXING_TYPES:
        return true;
        
    case ALL_INT32_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES: {
        Butterfly* butterfly = thisObject->butterfly();
        if (i >= butterfly->vectorLength())
            return true;
        butterfly->contiguous()[i].clear();
        return true;
    }
        
    case ALL_DOUBLE_INDEXING_TYPES: {
        Butterfly* butterfly = thisObject->butterfly();
        if (i >= butterfly->vectorLength())
            return true;
        butterfly->contiguousDouble()[i] = PNaN;
        return true;
    }
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = thisObject->m_butterfly.get()->arrayStorage();
        
        if (i < storage->vectorLength()) {
            WriteBarrier<Unknown>& valueSlot = storage->m_vector[i];
            if (valueSlot) {
                valueSlot.clear();
                --storage->m_numValuesInVector;
            }
        } else if (SparseArrayValueMap* map = storage->m_sparseMap.get()) {
            SparseArrayValueMap::iterator it = map->find(i);
            if (it != map->notFound()) {
                if (it->value.attributes & DontDelete)
                    return false;
                map->remove(it);
            }
        }
        
        return true;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
}

enum class TypeHintMode { TakesHint, DoesNotTakeHint };

template<TypeHintMode mode = TypeHintMode::DoesNotTakeHint>
static ALWAYS_INLINE JSValue callToPrimitiveFunction(ExecState* exec, const JSObject* object, PropertyName propertyName, PreferredPrimitiveType hint)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue function = object->get(exec, propertyName);
    RETURN_IF_EXCEPTION(scope, scope.exception());
    if (function.isUndefined() && mode == TypeHintMode::TakesHint)
        return JSValue();
    CallData callData;
    CallType callType = getCallData(function, callData);
    if (callType == CallType::None)
        return scope.exception();

    MarkedArgumentBuffer callArgs;
    if (mode == TypeHintMode::TakesHint) {
        JSString* hintString = nullptr;
        switch (hint) {
        case NoPreference:
            hintString = vm.smallStrings.defaultString();
            break;
        case PreferNumber:
            hintString = vm.smallStrings.numberString();
            break;
        case PreferString:
            hintString = vm.smallStrings.stringString();
            break;
        }
        callArgs.append(hintString);
    }

    JSValue result = call(exec, function, callType, callData, const_cast<JSObject*>(object), callArgs);
    RETURN_IF_EXCEPTION(scope, scope.exception());
    ASSERT(!result.isGetterSetter());
    if (result.isObject())
        return mode == TypeHintMode::DoesNotTakeHint ? JSValue() : throwTypeError(exec, scope, ASCIILiteral("Symbol.toPrimitive returned an object"));
    return result;
}

// ECMA 7.1.1
JSValue JSObject::ordinaryToPrimitive(ExecState* exec, PreferredPrimitiveType hint) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // Make sure that whatever default value methods there are on object's prototype chain are
    // being watched.
    this->structure()->startWatchingInternalPropertiesIfNecessaryForEntireChain(vm);

    JSValue value;
    if (hint == PreferString) {
        value = callToPrimitiveFunction(exec, this, exec->propertyNames().toString, hint);
        ASSERT(!scope.exception() || scope.exception() == value.asCell());
        if (value)
            return value;
        value = callToPrimitiveFunction(exec, this, exec->propertyNames().valueOf, hint);
        ASSERT(!scope.exception() || scope.exception() == value.asCell());
        if (value)
            return value;
    } else {
        value = callToPrimitiveFunction(exec, this, exec->propertyNames().valueOf, hint);
        ASSERT(!scope.exception() || scope.exception() == value.asCell());
        if (value)
            return value;
        value = callToPrimitiveFunction(exec, this, exec->propertyNames().toString, hint);
        ASSERT(!scope.exception() || scope.exception() == value.asCell());
        if (value)
            return value;
    }

    ASSERT(!scope.exception());

    return throwTypeError(exec, scope, ASCIILiteral("No default value"));
}

JSValue JSObject::defaultValue(const JSObject* object, ExecState* exec, PreferredPrimitiveType hint)
{
    return object->ordinaryToPrimitive(exec, hint);
}

JSValue JSObject::toPrimitive(ExecState* exec, PreferredPrimitiveType preferredType) const
{
    JSValue value = callToPrimitiveFunction<TypeHintMode::TakesHint>(exec, this, exec->propertyNames().toPrimitiveSymbol, preferredType);
    if (value)
        return value;

    return this->methodTable(exec->vm())->defaultValue(this, exec, preferredType);
}

bool JSObject::getPrimitiveNumber(ExecState* exec, double& number, JSValue& result) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    result = toPrimitive(exec, PreferNumber);
    RETURN_IF_EXCEPTION(scope, false);
    scope.release();
    number = result.toNumber(exec);
    return !result.isString();
}

bool JSObject::getOwnStaticPropertySlot(VM& vm, PropertyName propertyName, PropertySlot& slot)
{
    for (auto* info = classInfo(vm); info; info = info->parentClass) {
        if (auto* table = info->staticPropHashTable) {
            if (getStaticPropertySlotFromTable(vm, *table, this, propertyName, slot))
                return true;
        }
    }
    return false;
}

const HashTableValue* JSObject::findPropertyHashEntry(VM& vm, PropertyName propertyName) const
{
    for (const ClassInfo* info = classInfo(vm); info; info = info->parentClass) {
        if (const HashTable* propHashTable = info->staticPropHashTable) {
            if (const HashTableValue* entry = propHashTable->entry(propertyName))
                return entry;
        }
    }
    return 0;
}

bool JSObject::hasInstance(ExecState* exec, JSValue value, JSValue hasInstanceValue)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    

    if (!hasInstanceValue.isUndefinedOrNull() && hasInstanceValue != exec->lexicalGlobalObject()->functionProtoHasInstanceSymbolFunction()) {
        CallData callData;
        CallType callType = JSC::getCallData(hasInstanceValue, callData);
        if (callType == CallType::None) {
            throwException(exec, scope, createInvalidInstanceofParameterErrorhasInstanceValueNotFunction(exec, this));
            return false;
        }

        MarkedArgumentBuffer args;
        args.append(value);
        JSValue result = call(exec, hasInstanceValue, callType, callData, this, args);
        return result.toBoolean(exec);
    }

    TypeInfo info = structure(vm)->typeInfo();
    if (info.implementsDefaultHasInstance())
        return defaultHasInstance(exec, value, get(exec, exec->propertyNames().prototype));
    if (info.implementsHasInstance())
        return methodTable(vm)->customHasInstance(this, exec, value);
    throwException(exec, scope, createInvalidInstanceofParameterErrorNotFunction(exec, this));
    return false;
}

bool JSObject::hasInstance(ExecState* exec, JSValue value)
{
    JSValue hasInstanceValue = get(exec, exec->propertyNames().hasInstanceSymbol);

    return hasInstance(exec, value, hasInstanceValue);
}

bool JSObject::defaultHasInstance(ExecState* exec, JSValue value, JSValue proto)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!value.isObject())
        return false;

    if (!proto.isObject()) {
        throwTypeError(exec, scope, ASCIILiteral("instanceof called on an object with an invalid prototype property."));
        return false;
    }

    JSObject* object = asObject(value);
    while (true) {
        JSValue objectValue = object->getPrototype(vm, exec);
        RETURN_IF_EXCEPTION(scope, false);
        if (!objectValue.isObject())
            return false;
        object = asObject(objectValue);
        if (proto == object)
            return true;
    }
    ASSERT_NOT_REACHED();
}

EncodedJSValue JSC_HOST_CALL objectPrivateFuncInstanceOf(ExecState* exec)
{
    JSValue value = exec->uncheckedArgument(0);
    JSValue proto = exec->uncheckedArgument(1);

    return JSValue::encode(jsBoolean(JSObject::defaultHasInstance(exec, value, proto)));
}

void JSObject::getPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, propertyNames, mode);
    RETURN_IF_EXCEPTION(scope, void());

    JSValue nextProto = object->getPrototype(vm, exec);
    RETURN_IF_EXCEPTION(scope, void());
    if (nextProto.isNull())
        return;

    JSObject* prototype = asObject(nextProto);
    while(1) {
        if (prototype->structure(vm)->typeInfo().overridesGetPropertyNames()) {
            prototype->methodTable(vm)->getPropertyNames(prototype, exec, propertyNames, mode);
            break;
        }
        prototype->methodTable(vm)->getOwnPropertyNames(prototype, exec, propertyNames, mode);
        RETURN_IF_EXCEPTION(scope, void());
        nextProto = prototype->getPrototype(vm, exec);
        RETURN_IF_EXCEPTION(scope, void());
        if (nextProto.isNull())
            break;
        prototype = asObject(nextProto);
    }
}

void JSObject::getOwnPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    if (!mode.includeJSObjectProperties()) {
        // We still have to get non-indexed properties from any subclasses of JSObject that have them.
        object->methodTable(exec->vm())->getOwnNonIndexPropertyNames(object, exec, propertyNames, mode);
        return;
    }

    if (propertyNames.includeStringProperties()) {
        // Add numeric properties first. That appears to be the accepted convention.
        // FIXME: Filling PropertyNameArray with an identifier for every integer
        // is incredibly inefficient for large arrays. We need a different approach,
        // which almost certainly means a different structure for PropertyNameArray.
        switch (object->indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
            break;
            
        case ALL_INT32_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES: {
            Butterfly* butterfly = object->butterfly();
            unsigned usedLength = butterfly->publicLength();
            for (unsigned i = 0; i < usedLength; ++i) {
                if (!butterfly->contiguous()[i])
                    continue;
                propertyNames.add(i);
            }
            break;
        }
            
        case ALL_DOUBLE_INDEXING_TYPES: {
            Butterfly* butterfly = object->butterfly();
            unsigned usedLength = butterfly->publicLength();
            for (unsigned i = 0; i < usedLength; ++i) {
                double value = butterfly->contiguousDouble()[i];
                if (value != value)
                    continue;
                propertyNames.add(i);
            }
            break;
        }
            
        case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
            ArrayStorage* storage = object->m_butterfly.get()->arrayStorage();
            
            unsigned usedVectorLength = std::min(storage->length(), storage->vectorLength());
            for (unsigned i = 0; i < usedVectorLength; ++i) {
                if (storage->m_vector[i])
                    propertyNames.add(i);
            }
            
            if (SparseArrayValueMap* map = storage->m_sparseMap.get()) {
                Vector<unsigned, 0, UnsafeVectorOverflow> keys;
                keys.reserveInitialCapacity(map->size());
                
                SparseArrayValueMap::const_iterator end = map->end();
                for (SparseArrayValueMap::const_iterator it = map->begin(); it != end; ++it) {
                    if (mode.includeDontEnumProperties() || !(it->value.attributes & DontEnum))
                        keys.uncheckedAppend(static_cast<unsigned>(it->key));
                }
                
                std::sort(keys.begin(), keys.end());
                for (unsigned i = 0; i < keys.size(); ++i)
                    propertyNames.add(keys[i]);
            }
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    object->methodTable(exec->vm())->getOwnNonIndexPropertyNames(object, exec, propertyNames, mode);
}

void JSObject::getOwnNonIndexPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    VM& vm = exec->vm();
    if (!object->staticPropertiesReified())
        getClassPropertyNames(exec, object->classInfo(vm), propertyNames, mode);

    if (!mode.includeJSObjectProperties())
        return;
    
    object->structure(vm)->getPropertyNamesFromStructure(vm, propertyNames, mode);
}

double JSObject::toNumber(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue primitive = toPrimitive(exec, PreferNumber);
    RETURN_IF_EXCEPTION(scope, 0.0); // should be picked up soon in Nodes.cpp
    return primitive.toNumber(exec);
}

JSString* JSObject::toString(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue primitive = toPrimitive(exec, PreferString);
    RETURN_IF_EXCEPTION(scope, jsEmptyString(exec));
    return primitive.toString(exec);
}

JSValue JSObject::toThis(JSCell* cell, ExecState*, ECMAMode)
{
    return jsCast<JSObject*>(cell);
}

void JSObject::seal(VM& vm)
{
    if (isSealed(vm))
        return;
    enterDictionaryIndexingMode(vm);
    setStructure(vm, Structure::sealTransition(vm, structure(vm)));
}

void JSObject::freeze(VM& vm)
{
    if (isFrozen(vm))
        return;
    enterDictionaryIndexingMode(vm);
    setStructure(vm, Structure::freezeTransition(vm, structure(vm)));
}

bool JSObject::preventExtensions(JSObject* object, ExecState* exec)
{
    if (!object->isStructureExtensible()) {
        // We've already set the internal [[PreventExtensions]] field to false.
        // We don't call the methodTable isExtensible here because it's not defined
        // that way in the specification. We are just doing an optimization here.
        return true;
    }

    VM& vm = exec->vm();
    object->enterDictionaryIndexingMode(vm);
    object->setStructure(vm, Structure::preventExtensionsTransition(vm, object->structure(vm)));
    return true;
}

bool JSObject::isExtensible(JSObject* obj, ExecState*)
{
    return obj->isExtensibleImpl();
}

bool JSObject::isExtensible(ExecState* exec)
{ 
    VM& vm = exec->vm();
    return methodTable(vm)->isExtensible(this, exec);
}

void JSObject::reifyAllStaticProperties(ExecState* exec)
{
    ASSERT(!staticPropertiesReified());
    VM& vm = exec->vm();

    // If this object's ClassInfo has no static properties, then nothing to reify!
    // We can safely set the flag to avoid the expensive check again in the future.
    if (!TypeInfo::hasStaticPropertyTable(inlineTypeFlags())) {
        structure(vm)->setStaticPropertiesReified(true);
        return;
    }

    if (!structure(vm)->isDictionary())
        setStructure(vm, Structure::toCacheableDictionaryTransition(vm, structure(vm)));

    for (const ClassInfo* info = classInfo(vm); info; info = info->parentClass) {
        const HashTable* hashTable = info->staticPropHashTable;
        if (!hashTable)
            continue;

        for (auto& value : *hashTable) {
            unsigned attributes;
            auto key = Identifier::fromString(&vm, value.m_key);
            PropertyOffset offset = getDirectOffset(vm, key, attributes);
            if (!isValidOffset(offset))
                reifyStaticProperty(vm, key, value, *this);
        }
    }

    structure(vm)->setStaticPropertiesReified(true);
}

NEVER_INLINE void JSObject::fillGetterPropertySlot(PropertySlot& slot, JSValue getterSetter, unsigned attributes, PropertyOffset offset)
{
    if (structure()->isUncacheableDictionary()) {
        slot.setGetterSlot(this, attributes, jsCast<GetterSetter*>(getterSetter));
        return;
    }

    // This access is cacheable because Structure requires an attributeChangedTransition
    // if this property stops being an accessor.
    slot.setCacheableGetterSlot(this, attributes, jsCast<GetterSetter*>(getterSetter), offset);
}

bool JSObject::putIndexedDescriptor(ExecState* exec, SparseArrayEntry* entryInMap, const PropertyDescriptor& descriptor, PropertyDescriptor& oldDescriptor)
{
    VM& vm = exec->vm();
    auto map = m_butterfly.get()->arrayStorage()->m_sparseMap.get();

    if (descriptor.isDataDescriptor()) {
        if (descriptor.value())
            entryInMap->set(vm, map, descriptor.value());
        else if (oldDescriptor.isAccessorDescriptor())
            entryInMap->set(vm, map, jsUndefined());
        entryInMap->attributes = descriptor.attributesOverridingCurrent(oldDescriptor) & ~Accessor;
        return true;
    }

    if (descriptor.isAccessorDescriptor()) {
        JSObject* getter = 0;
        if (descriptor.getterPresent())
            getter = descriptor.getterObject();
        else if (oldDescriptor.isAccessorDescriptor())
            getter = oldDescriptor.getterObject();
        JSObject* setter = 0;
        if (descriptor.setterPresent())
            setter = descriptor.setterObject();
        else if (oldDescriptor.isAccessorDescriptor())
            setter = oldDescriptor.setterObject();

        GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());
        if (getter)
            accessor->setGetter(vm, exec->lexicalGlobalObject(), getter);
        if (setter)
            accessor->setSetter(vm, exec->lexicalGlobalObject(), setter);

        entryInMap->set(vm, map, accessor);
        entryInMap->attributes = descriptor.attributesOverridingCurrent(oldDescriptor) & ~ReadOnly;
        return true;
    }

    ASSERT(descriptor.isGenericDescriptor());
    entryInMap->attributes = descriptor.attributesOverridingCurrent(oldDescriptor);
    return true;
}

// Defined in ES5.1 8.12.9
bool JSObject::defineOwnIndexedProperty(ExecState* exec, unsigned index, const PropertyDescriptor& descriptor, bool throwException)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    ASSERT(index <= MAX_ARRAY_INDEX);

    if (!inSparseIndexingMode()) {
        // Fast case: we're putting a regular property to a regular array
        // FIXME: this will pessimistically assume that if attributes are missing then they'll default to false
        // however if the property currently exists missing attributes will override from their current 'true'
        // state (i.e. defineOwnProperty could be used to set a value without needing to entering 'SparseMode').
        if (!descriptor.attributes() && descriptor.value()) {
            ASSERT(!descriptor.isAccessorDescriptor());
            return putDirectIndex(exec, index, descriptor.value(), 0, throwException ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
        }
        
        ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm);
    }

    if (descriptor.attributes() & (ReadOnly | Accessor))
        notifyPresenceOfIndexedAccessors(vm);

    SparseArrayValueMap* map = m_butterfly.get()->arrayStorage()->m_sparseMap.get();
    RELEASE_ASSERT(map);
    
    // 1. Let current be the result of calling the [[GetOwnProperty]] internal method of O with property name P.
    SparseArrayValueMap::AddResult result = map->add(this, index);
    SparseArrayEntry* entryInMap = &result.iterator->value;

    // 2. Let extensible be the value of the [[Extensible]] internal property of O.
    // 3. If current is undefined and extensible is false, then Reject.
    // 4. If current is undefined and extensible is true, then
    if (result.isNewEntry) {
        if (!isStructureExtensible()) {
            map->remove(result.iterator);
            return typeError(exec, scope, throwException, ASCIILiteral(NonExtensibleObjectPropertyDefineError));
        }

        // 4.a. If IsGenericDescriptor(Desc) or IsDataDescriptor(Desc) is true, then create an own data property
        // named P of object O whose [[Value]], [[Writable]], [[Enumerable]] and [[Configurable]] attribute values
        // are described by Desc. If the value of an attribute field of Desc is absent, the attribute of the newly
        // created property is set to its default value.
        // 4.b. Else, Desc must be an accessor Property Descriptor so, create an own accessor property named P of
        // object O whose [[Get]], [[Set]], [[Enumerable]] and [[Configurable]] attribute values are described by
        // Desc. If the value of an attribute field of Desc is absent, the attribute of the newly created property
        // is set to its default value.
        // 4.c. Return true.

        PropertyDescriptor defaults;
        entryInMap->setWithoutWriteBarrier(jsUndefined());
        entryInMap->attributes = DontDelete | DontEnum | ReadOnly;
        entryInMap->get(defaults);

        putIndexedDescriptor(exec, entryInMap, descriptor, defaults);
        Butterfly* butterfly = m_butterfly.get();
        if (index >= butterfly->arrayStorage()->length())
            butterfly->arrayStorage()->setLength(index + 1);
        return true;
    }

    // 5. Return true, if every field in Desc is absent.
    // 6. Return true, if every field in Desc also occurs in current and the value of every field in Desc is the same value as the corresponding field in current when compared using the SameValue algorithm (9.12).
    PropertyDescriptor current;
    entryInMap->get(current);
    if (descriptor.isEmpty() || descriptor.equalTo(exec, current))
        return true;

    // 7. If the [[Configurable]] field of current is false then
    if (!current.configurable()) {
        // 7.a. Reject, if the [[Configurable]] field of Desc is true.
        if (descriptor.configurablePresent() && descriptor.configurable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeConfigurabilityError));
        // 7.b. Reject, if the [[Enumerable]] field of Desc is present and the [[Enumerable]] fields of current and Desc are the Boolean negation of each other.
        if (descriptor.enumerablePresent() && current.enumerable() != descriptor.enumerable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeEnumerabilityError));
    }

    // 8. If IsGenericDescriptor(Desc) is true, then no further validation is required.
    if (!descriptor.isGenericDescriptor()) {
        // 9. Else, if IsDataDescriptor(current) and IsDataDescriptor(Desc) have different results, then
        if (current.isDataDescriptor() != descriptor.isDataDescriptor()) {
            // 9.a. Reject, if the [[Configurable]] field of current is false.
            if (!current.configurable())
                return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeAccessMechanismError));
            // 9.b. If IsDataDescriptor(current) is true, then convert the property named P of object O from a
            // data property to an accessor property. Preserve the existing values of the converted property's
            // [[Configurable]] and [[Enumerable]] attributes and set the rest of the property's attributes to
            // their default values.
            // 9.c. Else, convert the property named P of object O from an accessor property to a data property.
            // Preserve the existing values of the converted property's [[Configurable]] and [[Enumerable]]
            // attributes and set the rest of the property's attributes to their default values.
        } else if (current.isDataDescriptor() && descriptor.isDataDescriptor()) {
            // 10. Else, if IsDataDescriptor(current) and IsDataDescriptor(Desc) are both true, then
            // 10.a. If the [[Configurable]] field of current is false, then
            if (!current.configurable() && !current.writable()) {
                // 10.a.i. Reject, if the [[Writable]] field of current is false and the [[Writable]] field of Desc is true.
                if (descriptor.writable())
                    return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeWritabilityError));
                // 10.a.ii. If the [[Writable]] field of current is false, then
                // 10.a.ii.1. Reject, if the [[Value]] field of Desc is present and SameValue(Desc.[[Value]], current.[[Value]]) is false.
                if (descriptor.value() && !sameValue(exec, descriptor.value(), current.value()))
                    return typeError(exec, scope, throwException, ASCIILiteral(ReadonlyPropertyChangeError));
            }
            // 10.b. else, the [[Configurable]] field of current is true, so any change is acceptable.
        } else {
            ASSERT(current.isAccessorDescriptor() && current.getterPresent() && current.setterPresent());
            // 11. Else, IsAccessorDescriptor(current) and IsAccessorDescriptor(Desc) are both true so, if the [[Configurable]] field of current is false, then
            if (!current.configurable()) {
                // 11.i. Reject, if the [[Set]] field of Desc is present and SameValue(Desc.[[Set]], current.[[Set]]) is false.
                if (descriptor.setterPresent() && descriptor.setter() != current.setter())
                    return typeError(exec, scope, throwException, ASCIILiteral("Attempting to change the setter of an unconfigurable property."));
                // 11.ii. Reject, if the [[Get]] field of Desc is present and SameValue(Desc.[[Get]], current.[[Get]]) is false.
                if (descriptor.getterPresent() && descriptor.getter() != current.getter())
                    return typeError(exec, scope, throwException, ASCIILiteral("Attempting to change the getter of an unconfigurable property."));
            }
        }
    }

    // 12. For each attribute field of Desc that is present, set the correspondingly named attribute of the property named P of object O to the value of the field.
    putIndexedDescriptor(exec, entryInMap, descriptor, current);
    // 13. Return true.
    return true;
}

SparseArrayValueMap* JSObject::allocateSparseIndexMap(VM& vm)
{
    SparseArrayValueMap* result = SparseArrayValueMap::create(vm);
    arrayStorage()->m_sparseMap.set(vm, this, result);
    return result;
}

void JSObject::deallocateSparseIndexMap()
{
    if (ArrayStorage* arrayStorage = arrayStorageOrNull())
        arrayStorage->m_sparseMap.clear();
}

bool JSObject::attemptToInterceptPutByIndexOnHoleForPrototype(ExecState* exec, JSValue thisValue, unsigned i, JSValue value, bool shouldThrow, bool& putResult)
{
    for (JSObject* current = this; ;) {
        // This has the same behavior with respect to prototypes as JSObject::put(). It only
        // allows a prototype to intercept a put if (a) the prototype declares the property
        // we're after rather than intercepting it via an override of JSObject::put(), and
        // (b) that property is declared as ReadOnly or Accessor.
        
        ArrayStorage* storage = current->arrayStorageOrNull();
        if (storage && storage->m_sparseMap) {
            SparseArrayValueMap::iterator iter = storage->m_sparseMap->find(i);
            if (iter != storage->m_sparseMap->notFound() && (iter->value.attributes & (Accessor | ReadOnly))) {
                putResult = iter->value.put(exec, thisValue, storage->m_sparseMap.get(), value, shouldThrow);
                return true;
            }
        }

        if (current->type() == ProxyObjectType) {
            ProxyObject* proxy = jsCast<ProxyObject*>(current);
            putResult = proxy->putByIndexCommon(exec, thisValue, i, value, shouldThrow);
            return true;
        }
        
        JSValue prototypeValue = current->getPrototypeDirect();
        if (prototypeValue.isNull())
            return false;
        
        current = asObject(prototypeValue);
    }
}

bool JSObject::attemptToInterceptPutByIndexOnHole(ExecState* exec, unsigned i, JSValue value, bool shouldThrow, bool& putResult)
{
    JSValue prototypeValue = getPrototypeDirect();
    if (prototypeValue.isNull())
        return false;
    
    return asObject(prototypeValue)->attemptToInterceptPutByIndexOnHoleForPrototype(exec, this, i, value, shouldThrow, putResult);
}

template<IndexingType indexingShape>
bool JSObject::putByIndexBeyondVectorLengthWithoutAttributes(ExecState* exec, unsigned i, JSValue value)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    ASSERT((indexingType() & IndexingShapeMask) == indexingShape);
    ASSERT(!indexingShouldBeSparse());

    Butterfly* butterfly = m_butterfly.get();
    
    // For us to get here, the index is either greater than the public length, or greater than
    // or equal to the vector length.
    ASSERT(i >= butterfly->vectorLength());
    
    if (i > MAX_STORAGE_VECTOR_INDEX
        || (i >= MIN_SPARSE_ARRAY_INDEX && !isDenseEnoughForVector(i, countElements<indexingShape>(butterfly)))
        || indexIsSufficientlyBeyondLengthForSparseMap(i, butterfly->vectorLength())) {
        ASSERT(i <= MAX_ARRAY_INDEX);
        ensureArrayStorageSlow(vm);
        SparseArrayValueMap* map = allocateSparseIndexMap(vm);
        bool result = map->putEntry(exec, this, i, value, false);
        ASSERT(i >= arrayStorage()->length());
        arrayStorage()->setLength(i + 1);
        return result;
    }

    if (!ensureLength(vm, i + 1)) {
        throwOutOfMemoryError(exec, scope);
        return false;
    }
    butterfly = m_butterfly.get();

    RELEASE_ASSERT(i < butterfly->vectorLength());
    switch (indexingShape) {
    case Int32Shape:
        ASSERT(value.isInt32());
        butterfly->contiguousInt32()[i].setWithoutWriteBarrier(value);
        return true;
        
    case DoubleShape: {
        ASSERT(value.isNumber());
        double valueAsDouble = value.asNumber();
        ASSERT(valueAsDouble == valueAsDouble);
        butterfly->contiguousDouble()[i] = valueAsDouble;
        return true;
    }
        
    case ContiguousShape:
        butterfly->contiguous()[i].set(vm, this, value);
        return true;
        
    default:
        CRASH();
        return false;
    }
}

// Explicit instantiations needed by JSArray.cpp.
template bool JSObject::putByIndexBeyondVectorLengthWithoutAttributes<Int32Shape>(ExecState*, unsigned, JSValue);
template bool JSObject::putByIndexBeyondVectorLengthWithoutAttributes<DoubleShape>(ExecState*, unsigned, JSValue);
template bool JSObject::putByIndexBeyondVectorLengthWithoutAttributes<ContiguousShape>(ExecState*, unsigned, JSValue);

bool JSObject::putByIndexBeyondVectorLengthWithArrayStorage(ExecState* exec, unsigned i, JSValue value, bool shouldThrow, ArrayStorage* storage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // i should be a valid array index that is outside of the current vector.
    ASSERT(i <= MAX_ARRAY_INDEX);
    ASSERT(i >= storage->vectorLength());
    
    SparseArrayValueMap* map = storage->m_sparseMap.get();
    
    // First, handle cases where we don't currently have a sparse map.
    if (LIKELY(!map)) {
        // If the array is not extensible, we should have entered dictionary mode, and created the sparse map.
        ASSERT(isStructureExtensible());
    
        // Update m_length if necessary.
        if (i >= storage->length())
            storage->setLength(i + 1);

        // Check that it is sensible to still be using a vector, and then try to grow the vector.
        if (LIKELY(!indexIsSufficientlyBeyondLengthForSparseMap(i, storage->vectorLength()) 
            && isDenseEnoughForVector(i, storage->m_numValuesInVector)
            && increaseVectorLength(vm, i + 1))) {
            // success! - reread m_storage since it has likely been reallocated, and store to the vector.
            storage = arrayStorage();
            storage->m_vector[i].set(vm, this, value);
            ++storage->m_numValuesInVector;
            return true;
        }
        // We don't want to, or can't use a vector to hold this property - allocate a sparse map & add the value.
        map = allocateSparseIndexMap(vm);
        return map->putEntry(exec, this, i, value, shouldThrow);
    }

    // Update m_length if necessary.
    unsigned length = storage->length();
    if (i >= length) {
        // Prohibit growing the array if length is not writable.
        if (map->lengthIsReadOnly() || !isStructureExtensible())
            return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));
        length = i + 1;
        storage->setLength(length);
    }

    // We are currently using a map - check whether we still want to be doing so.
    // We will continue  to use a sparse map if SparseMode is set, a vector would be too sparse, or if allocation fails.
    unsigned numValuesInArray = storage->m_numValuesInVector + map->size();
    if (map->sparseMode() || !isDenseEnoughForVector(length, numValuesInArray) || !increaseVectorLength(vm, length))
        return map->putEntry(exec, this, i, value, shouldThrow);

    // Reread m_storage after increaseVectorLength, update m_numValuesInVector.
    storage = arrayStorage();
    storage->m_numValuesInVector = numValuesInArray;

    // Copy all values from the map into the vector, and delete the map.
    WriteBarrier<Unknown>* vector = storage->m_vector;
    SparseArrayValueMap::const_iterator end = map->end();
    for (SparseArrayValueMap::const_iterator it = map->begin(); it != end; ++it)
        vector[it->key].set(vm, this, it->value.getNonSparseMode());
    deallocateSparseIndexMap();

    // Store the new property into the vector.
    WriteBarrier<Unknown>& valueSlot = vector[i];
    if (!valueSlot)
        ++storage->m_numValuesInVector;
    valueSlot.set(vm, this, value);
    return true;
}

bool JSObject::putByIndexBeyondVectorLength(ExecState* exec, unsigned i, JSValue value, bool shouldThrow)
{
    VM& vm = exec->vm();

    // i should be a valid array index that is outside of the current vector.
    ASSERT(i <= MAX_ARRAY_INDEX);
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES: {
        if (indexingShouldBeSparse()) {
            return putByIndexBeyondVectorLengthWithArrayStorage(
                exec, i, value, shouldThrow,
                ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
        }
        if (indexIsSufficientlyBeyondLengthForSparseMap(i, 0) || i >= MIN_SPARSE_ARRAY_INDEX) {
            return putByIndexBeyondVectorLengthWithArrayStorage(
                exec, i, value, shouldThrow, createArrayStorage(vm, 0, 0));
        }
        if (structure(vm)->needsSlowPutIndexing()) {
            // Convert the indexing type to the SlowPutArrayStorage and retry.
            createArrayStorage(vm, i + 1, getNewVectorLength(0, 0, 0, i + 1));
            return putByIndex(this, exec, i, value, shouldThrow);
        }
        
        createInitialForValueAndSet(vm, i, value);
        return true;
    }
        
    case ALL_UNDECIDED_INDEXING_TYPES: {
        CRASH();
        break;
    }
        
    case ALL_INT32_INDEXING_TYPES:
        return putByIndexBeyondVectorLengthWithoutAttributes<Int32Shape>(exec, i, value);
        
    case ALL_DOUBLE_INDEXING_TYPES:
        return putByIndexBeyondVectorLengthWithoutAttributes<DoubleShape>(exec, i, value);
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        return putByIndexBeyondVectorLengthWithoutAttributes<ContiguousShape>(exec, i, value);
        
    case NonArrayWithSlowPutArrayStorage:
    case ArrayWithSlowPutArrayStorage: {
        // No own property present in the vector, but there might be in the sparse map!
        SparseArrayValueMap* map = arrayStorage()->m_sparseMap.get();
        bool putResult = false;
        if (!(map && map->contains(i)) && attemptToInterceptPutByIndexOnHole(exec, i, value, shouldThrow, putResult))
            return putResult;
        FALLTHROUGH;
    }

    case NonArrayWithArrayStorage:
    case ArrayWithArrayStorage:
        return putByIndexBeyondVectorLengthWithArrayStorage(exec, i, value, shouldThrow, arrayStorage());
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
    return false;
}

bool JSObject::putDirectIndexBeyondVectorLengthWithArrayStorage(ExecState* exec, unsigned i, JSValue value, unsigned attributes, PutDirectIndexMode mode, ArrayStorage* storage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    // i should be a valid array index that is outside of the current vector.
    ASSERT(hasAnyArrayStorage(indexingType()));
    ASSERT(arrayStorage() == storage);
    ASSERT(i >= storage->vectorLength() || attributes);
    ASSERT(i <= MAX_ARRAY_INDEX);

    SparseArrayValueMap* map = storage->m_sparseMap.get();

    // First, handle cases where we don't currently have a sparse map.
    if (LIKELY(!map)) {
        // If the array is not extensible, we should have entered dictionary mode, and created the spare map.
        ASSERT(isStructureExtensible());
    
        // Update m_length if necessary.
        if (i >= storage->length())
            storage->setLength(i + 1);

        // Check that it is sensible to still be using a vector, and then try to grow the vector.
        if (LIKELY(
                !attributes
                && (isDenseEnoughForVector(i, storage->m_numValuesInVector))
                && !indexIsSufficientlyBeyondLengthForSparseMap(i, storage->vectorLength()))
                && increaseVectorLength(vm, i + 1)) {
            // success! - reread m_storage since it has likely been reallocated, and store to the vector.
            storage = arrayStorage();
            storage->m_vector[i].set(vm, this, value);
            ++storage->m_numValuesInVector;
            return true;
        }
        // We don't want to, or can't use a vector to hold this property - allocate a sparse map & add the value.
        map = allocateSparseIndexMap(vm);
        return map->putDirect(exec, this, i, value, attributes, mode);
    }

    // Update m_length if necessary.
    unsigned length = storage->length();
    if (i >= length) {
        if (mode != PutDirectIndexLikePutDirect) {
            // Prohibit growing the array if length is not writable.
            if (map->lengthIsReadOnly())
                return typeError(exec, scope, mode == PutDirectIndexShouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));
            if (!isStructureExtensible())
                return typeError(exec, scope, mode == PutDirectIndexShouldThrow, ASCIILiteral(NonExtensibleObjectPropertyDefineError));
        }
        length = i + 1;
        storage->setLength(length);
    }

    // We are currently using a map - check whether we still want to be doing so.
    // We will continue  to use a sparse map if SparseMode is set, a vector would be too sparse, or if allocation fails.
    unsigned numValuesInArray = storage->m_numValuesInVector + map->size();
    if (map->sparseMode() || attributes || !isDenseEnoughForVector(length, numValuesInArray) || !increaseVectorLength(vm, length))
        return map->putDirect(exec, this, i, value, attributes, mode);

    // Reread m_storage after increaseVectorLength, update m_numValuesInVector.
    storage = arrayStorage();
    storage->m_numValuesInVector = numValuesInArray;

    // Copy all values from the map into the vector, and delete the map.
    WriteBarrier<Unknown>* vector = storage->m_vector;
    SparseArrayValueMap::const_iterator end = map->end();
    for (SparseArrayValueMap::const_iterator it = map->begin(); it != end; ++it)
        vector[it->key].set(vm, this, it->value.getNonSparseMode());
    deallocateSparseIndexMap();

    // Store the new property into the vector.
    WriteBarrier<Unknown>& valueSlot = vector[i];
    if (!valueSlot)
        ++storage->m_numValuesInVector;
    valueSlot.set(vm, this, value);
    return true;
}

bool JSObject::putDirectIndexBeyondVectorLength(ExecState* exec, unsigned i, JSValue value, unsigned attributes, PutDirectIndexMode mode)
{
    VM& vm = exec->vm();

    // i should be a valid array index that is outside of the current vector.
    ASSERT(i <= MAX_ARRAY_INDEX);
    
    if (attributes & (ReadOnly | Accessor))
        notifyPresenceOfIndexedAccessors(vm);
    
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES: {
        if (indexingShouldBeSparse() || attributes) {
            return putDirectIndexBeyondVectorLengthWithArrayStorage(
                exec, i, value, attributes, mode,
                ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
        }
        if (i >= MIN_SPARSE_ARRAY_INDEX) {
            return putDirectIndexBeyondVectorLengthWithArrayStorage(
                exec, i, value, attributes, mode, createArrayStorage(vm, 0, 0));
        }
        if (structure(vm)->needsSlowPutIndexing()) {
            ArrayStorage* storage = createArrayStorage(vm, i + 1, getNewVectorLength(0, 0, 0, i + 1));
            storage->m_vector[i].set(vm, this, value);
            storage->m_numValuesInVector++;
            return true;
        }
        
        createInitialForValueAndSet(vm, i, value);
        return true;
    }
        
    case ALL_UNDECIDED_INDEXING_TYPES: {
        convertUndecidedForValue(exec->vm(), value);
        // Reloop.
        return putDirectIndex(exec, i, value, attributes, mode);
    }
        
    case ALL_INT32_INDEXING_TYPES: {
        if (attributes) {
            if (i < m_butterfly.get()->vectorLength())
                return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
            return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, convertInt32ToArrayStorage(vm));
        }
        if (!value.isInt32()) {
            convertInt32ForValue(vm, value);
            return putDirectIndexBeyondVectorLength(exec, i, value, attributes, mode);
        }
        putByIndexBeyondVectorLengthWithoutAttributes<Int32Shape>(exec, i, value);
        return true;
    }
        
    case ALL_DOUBLE_INDEXING_TYPES: {
        if (attributes) {
            if (i < m_butterfly.get()->vectorLength())
                return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
            return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, convertDoubleToArrayStorage(vm));
        }
        if (!value.isNumber()) {
            convertDoubleToContiguous(vm);
            return putDirectIndexBeyondVectorLength(exec, i, value, attributes, mode);
        }
        double valueAsDouble = value.asNumber();
        if (valueAsDouble != valueAsDouble) {
            convertDoubleToContiguous(vm);
            return putDirectIndexBeyondVectorLength(exec, i, value, attributes, mode);
        }
        putByIndexBeyondVectorLengthWithoutAttributes<DoubleShape>(exec, i, value);
        return true;
    }
        
    case ALL_CONTIGUOUS_INDEXING_TYPES: {
        if (attributes) {
            if (i < m_butterfly.get()->vectorLength())
                return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
            return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, convertContiguousToArrayStorage(vm));
        }
        putByIndexBeyondVectorLengthWithoutAttributes<ContiguousShape>(exec, i, value);
        return true;
    }

    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        if (attributes) {
            if (i < m_butterfly.get()->vectorLength())
                return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, ensureArrayStorageExistsAndEnterDictionaryIndexingMode(vm));
        }
        return putDirectIndexBeyondVectorLengthWithArrayStorage(exec, i, value, attributes, mode, arrayStorage());
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
}

bool JSObject::putDirectNativeIntrinsicGetter(VM& vm, JSGlobalObject* globalObject, Identifier name, NativeFunction nativeFunction, Intrinsic intrinsic, unsigned attributes)
{
    GetterSetter* accessor = GetterSetter::create(vm, globalObject);
    JSFunction* function = JSFunction::create(vm, globalObject, 0, makeString("get ", name.string()), nativeFunction, intrinsic);
    accessor->setGetter(vm, globalObject, function);
    return putDirectNonIndexAccessor(vm, name, accessor, attributes);
}

bool JSObject::putDirectNativeFunction(VM& vm, JSGlobalObject* globalObject, const PropertyName& propertyName, unsigned functionLength, NativeFunction nativeFunction, Intrinsic intrinsic, unsigned attributes)
{
    StringImpl* name = propertyName.publicName();
    if (!name)
        name = vm.propertyNames->anonymous.impl();
    ASSERT(name);

    JSFunction* function = JSFunction::create(vm, globalObject, functionLength, name, nativeFunction, intrinsic);
    return putDirect(vm, propertyName, function, attributes);
}

bool JSObject::putDirectNativeFunction(VM& vm, JSGlobalObject* globalObject, const PropertyName& propertyName, unsigned functionLength, NativeFunction nativeFunction, Intrinsic intrinsic, const DOMJIT::Signature* signature, unsigned attributes)
{
    StringImpl* name = propertyName.publicName();
    if (!name)
        name = vm.propertyNames->anonymous.impl();
    ASSERT(name);

    JSFunction* function = JSFunction::create(vm, globalObject, functionLength, name, nativeFunction, intrinsic, callHostFunctionAsConstructor, signature);
    return putDirect(vm, propertyName, function, attributes);
}

JSFunction* JSObject::putDirectBuiltinFunction(VM& vm, JSGlobalObject* globalObject, const PropertyName& propertyName, FunctionExecutable* functionExecutable, unsigned attributes)
{
    StringImpl* name = propertyName.publicName();
    if (!name)
        name = vm.propertyNames->anonymous.impl();
    ASSERT(name);
    JSFunction* function = JSFunction::createBuiltinFunction(vm, static_cast<FunctionExecutable*>(functionExecutable), globalObject);
    putDirect(vm, propertyName, function, attributes);
    return function;
}

JSFunction* JSObject::putDirectBuiltinFunctionWithoutTransition(VM& vm, JSGlobalObject* globalObject, const PropertyName& propertyName, FunctionExecutable* functionExecutable, unsigned attributes)
{
    JSFunction* function = JSFunction::createBuiltinFunction(vm, static_cast<FunctionExecutable*>(functionExecutable), globalObject);
    putDirectWithoutTransition(vm, propertyName, function, attributes);
    return function;
}

void JSObject::putDirectNativeFunctionWithoutTransition(VM& vm, JSGlobalObject* globalObject, const PropertyName& propertyName, unsigned functionLength, NativeFunction nativeFunction, Intrinsic intrinsic, unsigned attributes)
{
    StringImpl* name = propertyName.publicName();
    if (!name)
        name = vm.propertyNames->anonymous.impl();
    ASSERT(name);
    JSFunction* function = JSFunction::create(vm, globalObject, functionLength, name, nativeFunction, intrinsic);
    putDirectWithoutTransition(vm, propertyName, function, attributes);
}

// NOTE: This method is for ArrayStorage vectors.
ALWAYS_INLINE unsigned JSObject::getNewVectorLength(unsigned indexBias, unsigned currentVectorLength, unsigned currentLength, unsigned desiredLength)
{
    ASSERT(desiredLength <= MAX_STORAGE_VECTOR_LENGTH);

    unsigned increasedLength;
    unsigned maxInitLength = std::min(currentLength, 100000U);

    if (desiredLength < maxInitLength)
        increasedLength = maxInitLength;
    else if (!currentVectorLength)
        increasedLength = std::max(desiredLength, lastArraySize);
    else {
        increasedLength = timesThreePlusOneDividedByTwo(desiredLength);
    }

    ASSERT(increasedLength >= desiredLength);

    lastArraySize = std::min(increasedLength, FIRST_ARRAY_STORAGE_VECTOR_GROW);

    return ArrayStorage::optimalVectorLength(
        indexBias, structure()->outOfLineCapacity(),
        std::min(increasedLength, MAX_STORAGE_VECTOR_LENGTH));
}

ALWAYS_INLINE unsigned JSObject::getNewVectorLength(unsigned desiredLength)
{
    unsigned indexBias = 0;
    unsigned vectorLength = 0;
    unsigned length = 0;
    
    if (hasIndexedProperties(indexingType())) {
        if (ArrayStorage* storage = arrayStorageOrNull())
            indexBias = storage->m_indexBias;
        vectorLength = m_butterfly.get()->vectorLength();
        length = m_butterfly.get()->publicLength();
    }

    return getNewVectorLength(indexBias, vectorLength, length, desiredLength);
}

template<IndexingType indexingShape>
unsigned JSObject::countElements(Butterfly* butterfly)
{
    unsigned numValues = 0;
    for (unsigned i = butterfly->publicLength(); i--;) {
        switch (indexingShape) {
        case Int32Shape:
        case ContiguousShape:
            if (butterfly->contiguous()[i])
                numValues++;
            break;
            
        case DoubleShape: {
            double value = butterfly->contiguousDouble()[i];
            if (value == value)
                numValues++;
            break;
        }
            
        default:
            CRASH();
        }
    }
    return numValues;
}

unsigned JSObject::countElements()
{
    switch (indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
    case ALL_UNDECIDED_INDEXING_TYPES:
        return 0;
        
    case ALL_INT32_INDEXING_TYPES:
        return countElements<Int32Shape>(butterfly());
        
    case ALL_DOUBLE_INDEXING_TYPES:
        return countElements<DoubleShape>(butterfly());
        
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        return countElements<ContiguousShape>(butterfly());
        
    default:
        CRASH();
        return 0;
    }
}

bool JSObject::increaseVectorLength(VM& vm, unsigned newLength)
{
    ArrayStorage* storage = arrayStorage();
    
    unsigned vectorLength = storage->vectorLength();
    unsigned availableVectorLength = storage->availableVectorLength(structure(vm), vectorLength); 
    if (availableVectorLength >= newLength) {
        // The cell was already big enough for the desired length!
        for (unsigned i = vectorLength; i < availableVectorLength; ++i)
            storage->m_vector[i].clear();
        storage->setVectorLength(availableVectorLength);
        return true;
    }
    
    // This function leaves the array in an internally inconsistent state, because it does not move any values from sparse value map
    // to the vector. Callers have to account for that, because they can do it more efficiently.
    if (newLength > MAX_STORAGE_VECTOR_LENGTH)
        return false;

    if (newLength >= MIN_SPARSE_ARRAY_INDEX
        && !isDenseEnoughForVector(newLength, storage->m_numValuesInVector))
        return false;

    unsigned indexBias = storage->m_indexBias;
    ASSERT(newLength > vectorLength);
    unsigned newVectorLength = getNewVectorLength(newLength);

    // Fast case - there is no precapacity. In these cases a realloc makes sense.
    Structure* structure = this->structure(vm);
    if (LIKELY(!indexBias)) {
        DeferGC deferGC(vm.heap);
        Butterfly* newButterfly = storage->butterfly()->growArrayRight(
            vm, this, structure, structure->outOfLineCapacity(), true,
            ArrayStorage::sizeFor(vectorLength), ArrayStorage::sizeFor(newVectorLength));
        if (!newButterfly)
            return false;
        for (unsigned i = vectorLength; i < newVectorLength; ++i)
            newButterfly->arrayStorage()->m_vector[i].clear();
        newButterfly->arrayStorage()->setVectorLength(newVectorLength);
        setButterfly(vm, newButterfly);
        return true;
    }
    
    // Remove some, but not all of the precapacity. Atomic decay, & capped to not overflow array length.
    DeferGC deferGC(vm.heap);
    unsigned newIndexBias = std::min(indexBias >> 1, MAX_STORAGE_VECTOR_LENGTH - newVectorLength);
    Butterfly* newButterfly = storage->butterfly()->resizeArray(
        vm, this,
        structure->outOfLineCapacity(), true, ArrayStorage::sizeFor(vectorLength),
        newIndexBias, true, ArrayStorage::sizeFor(newVectorLength));
    if (!newButterfly)
        return false;
    for (unsigned i = vectorLength; i < newVectorLength; ++i)
        newButterfly->arrayStorage()->m_vector[i].clear();
    newButterfly->arrayStorage()->setVectorLength(newVectorLength);
    newButterfly->arrayStorage()->m_indexBias = newIndexBias;
    setButterfly(vm, newButterfly);
    return true;
}

bool JSObject::ensureLengthSlow(VM& vm, unsigned length)
{
    Butterfly* butterfly = m_butterfly.get();
    
    ASSERT(length < MAX_ARRAY_INDEX);
    ASSERT(hasContiguous(indexingType()) || hasInt32(indexingType()) || hasDouble(indexingType()) || hasUndecided(indexingType()));
    ASSERT(length > butterfly->vectorLength());
    
    unsigned oldVectorLength = butterfly->vectorLength();
    unsigned newVectorLength;
    
    Structure* structure = this->structure(vm);
    unsigned propertyCapacity = structure->outOfLineCapacity();
    
    unsigned availableOldLength =
        Butterfly::availableContiguousVectorLength(propertyCapacity, oldVectorLength);
    Butterfly* newButterfly = nullptr;
    if (availableOldLength >= length) {
        // This is the case where someone else selected a vector length that caused internal
        // fragmentation. If we did our jobs right, this would never happen. But I bet we will mess
        // this up, so this defense should stay.
        newVectorLength = availableOldLength;
    } else {
        newVectorLength = Butterfly::optimalContiguousVectorLength(
            propertyCapacity, std::min(length << 1, MAX_STORAGE_VECTOR_LENGTH));
        butterfly = butterfly->growArrayRight(
            vm, this, structure, propertyCapacity, true,
            oldVectorLength * sizeof(EncodedJSValue),
            newVectorLength * sizeof(EncodedJSValue));
        if (!butterfly)
            return false;
        newButterfly = butterfly;
    }

    if (hasDouble(indexingType())) {
        for (unsigned i = oldVectorLength; i < newVectorLength; ++i)
            butterfly->indexingPayload<double>()[i] = PNaN;
    } else {
        for (unsigned i = oldVectorLength; i < newVectorLength; ++i)
            butterfly->indexingPayload<WriteBarrier<Unknown>>()[i].clear();
    }

    if (newButterfly) {
        butterfly->setVectorLength(newVectorLength);
        WTF::storeStoreFence();
        m_butterfly.set(vm, this, newButterfly);
    } else {
        WTF::storeStoreFence();
        butterfly->setVectorLength(newVectorLength);
    }

    return true;
}

void JSObject::reallocateAndShrinkButterfly(VM& vm, unsigned length)
{
    ASSERT(length < MAX_ARRAY_INDEX);
    ASSERT(length < MAX_STORAGE_VECTOR_LENGTH);
    ASSERT(hasContiguous(indexingType()) || hasInt32(indexingType()) || hasDouble(indexingType()) || hasUndecided(indexingType()));
    ASSERT(m_butterfly.get()->vectorLength() > length);
    ASSERT(!m_butterfly.get()->indexingHeader()->preCapacity(structure()));

    DeferGC deferGC(vm.heap);
    Butterfly* newButterfly = m_butterfly.get()->resizeArray(vm, this, structure(), 0, ArrayStorage::sizeFor(length));
    newButterfly->setVectorLength(length);
    newButterfly->setPublicLength(length);
    WTF::storeStoreFence();
    m_butterfly.set(vm, this, newButterfly);
}

Butterfly* JSObject::allocateMoreOutOfLineStorage(VM& vm, size_t oldSize, size_t newSize)
{
    ASSERT(newSize > oldSize);

    // It's important that this function not rely on structure(), for the property
    // capacity, since we might have already mutated the structure in-place.

    return Butterfly::createOrGrowPropertyStorage(m_butterfly.get(), vm, this, structure(vm), oldSize, newSize);
}

static JSCustomGetterSetterFunction* getCustomGetterSetterFunctionForGetterSetter(ExecState* exec, PropertyName propertyName, CustomGetterSetter* getterSetter, JSCustomGetterSetterFunction::Type type)
{
    auto key = std::make_pair(getterSetter, (int)type);
    JSCustomGetterSetterFunction* customGetterSetterFunction = exec->vm().customGetterSetterFunctionMap.get(key);
    if (!customGetterSetterFunction) {
        customGetterSetterFunction = JSCustomGetterSetterFunction::create(exec->vm(), exec->lexicalGlobalObject(), getterSetter, type, propertyName.publicName());
        exec->vm().customGetterSetterFunctionMap.set(key, customGetterSetterFunction);
    }
    return customGetterSetterFunction;
}

bool JSObject::getOwnPropertyDescriptor(ExecState* exec, PropertyName propertyName, PropertyDescriptor& descriptor)
{
    VM& vm = exec->vm();
    JSC::PropertySlot slot(this, PropertySlot::InternalMethodType::GetOwnProperty);
    if (!methodTable(vm)->getOwnPropertySlot(this, exec, propertyName, slot))
        return false;

    // DebuggerScope::getOwnPropertySlot() (and possibly others) may return attributes from the prototype chain
    // but getOwnPropertyDescriptor() should only work for 'own' properties so we exit early if we detect that
    // the property is not an own property.
    if (slot.slotBase() != this && slot.slotBase()) {
        JSProxy* jsProxy = jsDynamicCast<JSProxy*>(vm, this);
        if (!jsProxy || jsProxy->target() != slot.slotBase()) {
            // Try ProxyObject.
            ProxyObject* proxyObject = jsDynamicCast<ProxyObject*>(vm, this);
            if (!proxyObject || proxyObject->target() != slot.slotBase())
                return false;
        }
    }

    if (slot.isAccessor())
        descriptor.setAccessorDescriptor(slot.getterSetter(), slot.attributes());
    else if (slot.attributes() & CustomAccessor) {
        descriptor.setCustomDescriptor(slot.attributes());

        JSObject* thisObject = this;
        if (auto* proxy = jsDynamicCast<JSProxy*>(vm, this))
            thisObject = proxy->target();

        CustomGetterSetter* getterSetter;
        if (slot.isCustomAccessor())
            getterSetter = slot.customGetterSetter();
        else {
            JSValue maybeGetterSetter = thisObject->getDirect(exec->vm(), propertyName);
            if (!maybeGetterSetter) {
                thisObject->reifyAllStaticProperties(exec);
                maybeGetterSetter = thisObject->getDirect(exec->vm(), propertyName);
            }

            ASSERT(maybeGetterSetter);
            getterSetter = jsDynamicCast<CustomGetterSetter*>(vm, maybeGetterSetter);
        }
        ASSERT(getterSetter);
        if (!getterSetter)
            return false;

        if (getterSetter->getter())
            descriptor.setGetter(getCustomGetterSetterFunctionForGetterSetter(exec, propertyName, getterSetter, JSCustomGetterSetterFunction::Type::Getter));
        if (getterSetter->setter())
            descriptor.setSetter(getCustomGetterSetterFunctionForGetterSetter(exec, propertyName, getterSetter, JSCustomGetterSetterFunction::Type::Setter));
    } else
        descriptor.setDescriptor(slot.getValue(exec, propertyName), slot.attributes());
    return true;
}

static bool putDescriptor(ExecState* exec, JSObject* target, PropertyName propertyName, const PropertyDescriptor& descriptor, unsigned attributes, const PropertyDescriptor& oldDescriptor)
{
    VM& vm = exec->vm();
    if (descriptor.isGenericDescriptor() || descriptor.isDataDescriptor()) {
        if (descriptor.isGenericDescriptor() && oldDescriptor.isAccessorDescriptor()) {
            GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());
            if (oldDescriptor.getterPresent())
                accessor->setGetter(vm, exec->lexicalGlobalObject(), oldDescriptor.getterObject());
            if (oldDescriptor.setterPresent())
                accessor->setSetter(vm, exec->lexicalGlobalObject(), oldDescriptor.setterObject());
            target->putDirectAccessor(exec, propertyName, accessor, attributes | Accessor);
            return true;
        }
        JSValue newValue = jsUndefined();
        if (descriptor.value())
            newValue = descriptor.value();
        else if (oldDescriptor.value())
            newValue = oldDescriptor.value();
        target->putDirect(vm, propertyName, newValue, attributes & ~Accessor);
        if (attributes & ReadOnly)
            target->structure(vm)->setContainsReadOnlyProperties();
        return true;
    }
    attributes &= ~ReadOnly;
    GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());

    if (descriptor.getterPresent())
        accessor->setGetter(vm, exec->lexicalGlobalObject(), descriptor.getterObject());
    else if (oldDescriptor.getterPresent())
        accessor->setGetter(vm, exec->lexicalGlobalObject(), oldDescriptor.getterObject());
    if (descriptor.setterPresent())
        accessor->setSetter(vm, exec->lexicalGlobalObject(), descriptor.setterObject());
    else if (oldDescriptor.setterPresent())
        accessor->setSetter(vm, exec->lexicalGlobalObject(), oldDescriptor.setterObject());

    target->putDirectAccessor(exec, propertyName, accessor, attributes | Accessor);
    return true;
}

bool JSObject::putDirectMayBeIndex(ExecState* exec, PropertyName propertyName, JSValue value)
{
    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return putDirectIndex(exec, index.value(), value);
    return putDirect(exec->vm(), propertyName, value);
}

// 9.1.6.3 of the spec
// http://www.ecma-international.org/ecma-262/6.0/index.html#sec-validateandapplypropertydescriptor
bool validateAndApplyPropertyDescriptor(ExecState* exec, JSObject* object, PropertyName propertyName, bool isExtensible,
    const PropertyDescriptor& descriptor, bool isCurrentDefined, const PropertyDescriptor& current, bool throwException)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // If we have a new property we can just put it on normally
    // Step 2.
    if (!isCurrentDefined) {
        // unless extensions are prevented!
        // Step 2.a
        if (!isExtensible)
            return typeError(exec, scope, throwException, ASCIILiteral(NonExtensibleObjectPropertyDefineError));
        if (!object)
            return true;
        // Step 2.c/d
        PropertyDescriptor oldDescriptor;
        oldDescriptor.setValue(jsUndefined());
        // FIXME: spec says to always return true here.
        return putDescriptor(exec, object, propertyName, descriptor, descriptor.attributes(), oldDescriptor);
    }
    // Step 3.
    if (descriptor.isEmpty())
        return true;
    // Step 4.
    if (current.equalTo(exec, descriptor))
        return true;

    // Step 5.
    // Filter out invalid changes
    if (!current.configurable()) {
        if (descriptor.configurable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeConfigurabilityError));
        if (descriptor.enumerablePresent() && descriptor.enumerable() != current.enumerable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeEnumerabilityError));
    }
    
    // Step 6.
    // A generic descriptor is simply changing the attributes of an existing property
    if (descriptor.isGenericDescriptor()) {
        if (!current.attributesEqual(descriptor) && object) {
            object->methodTable(vm)->deleteProperty(object, exec, propertyName);
            return putDescriptor(exec, object, propertyName, descriptor, descriptor.attributesOverridingCurrent(current), current);
        }
        return true;
    }
    
    // Step 7.
    // Changing between a normal property or an accessor property
    if (descriptor.isDataDescriptor() != current.isDataDescriptor()) {
        if (!current.configurable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeAccessMechanismError));

        if (!object)
            return true;

        object->methodTable(vm)->deleteProperty(object, exec, propertyName);
        return putDescriptor(exec, object, propertyName, descriptor, descriptor.attributesOverridingCurrent(current), current);
    }

    // Step 8.
    // Changing the value and attributes of an existing property
    if (descriptor.isDataDescriptor()) {
        if (!current.configurable()) {
            if (!current.writable() && descriptor.writable())
                return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeWritabilityError));
            if (!current.writable()) {
                if (descriptor.value() && !sameValue(exec, current.value(), descriptor.value()))
                    return typeError(exec, scope, throwException, ASCIILiteral(ReadonlyPropertyChangeError));
            }
        }
        if (current.attributesEqual(descriptor) && !descriptor.value())
            return true;
        if (!object)
            return true;
        object->methodTable(vm)->deleteProperty(object, exec, propertyName);
        return putDescriptor(exec, object, propertyName, descriptor, descriptor.attributesOverridingCurrent(current), current);
    }

    // Step 9.
    // Changing the accessor functions of an existing accessor property
    ASSERT(descriptor.isAccessorDescriptor());
    if (!current.configurable()) {
        if (descriptor.setterPresent() && !(current.setterPresent() && JSValue::strictEqual(exec, current.setter(), descriptor.setter())))
            return typeError(exec, scope, throwException, ASCIILiteral("Attempting to change the setter of an unconfigurable property."));
        if (descriptor.getterPresent() && !(current.getterPresent() && JSValue::strictEqual(exec, current.getter(), descriptor.getter())))
            return typeError(exec, scope, throwException, ASCIILiteral("Attempting to change the getter of an unconfigurable property."));
        if (current.attributes() & CustomAccessor)
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeAccessMechanismError));
    }

    // Step 10/11.
    if (!object)
        return true;
    JSValue accessor = object->getDirect(vm, propertyName);
    if (!accessor)
        return false;
    GetterSetter* getterSetter;
    bool getterSetterChanged = false;
    if (accessor.isCustomGetterSetter()) {
        getterSetter = GetterSetter::create(vm, exec->lexicalGlobalObject());
        auto* customGetterSetter = jsCast<CustomGetterSetter*>(accessor);
        if (customGetterSetter->setter())
            getterSetter->setSetter(vm, exec->lexicalGlobalObject(), getCustomGetterSetterFunctionForGetterSetter(exec, propertyName, customGetterSetter, JSCustomGetterSetterFunction::Type::Setter));
        if (customGetterSetter->getter())
            getterSetter->setGetter(vm, exec->lexicalGlobalObject(), getCustomGetterSetterFunctionForGetterSetter(exec, propertyName, customGetterSetter, JSCustomGetterSetterFunction::Type::Getter));
    } else {
        ASSERT(accessor.isGetterSetter());
        getterSetter = asGetterSetter(accessor);
    }
    if (descriptor.setterPresent()) {
        getterSetter = getterSetter->withSetter(vm, exec->lexicalGlobalObject(), descriptor.setterObject());
        getterSetterChanged = true;
    }
    if (descriptor.getterPresent()) {
        getterSetter = getterSetter->withGetter(vm, exec->lexicalGlobalObject(), descriptor.getterObject());
        getterSetterChanged = true;
    }
    if (current.attributesEqual(descriptor) && !getterSetterChanged)
        return true;
    object->methodTable(vm)->deleteProperty(object, exec, propertyName);
    unsigned attrs = descriptor.attributesOverridingCurrent(current);
    object->putDirectAccessor(exec, propertyName, getterSetter, attrs | Accessor);
    return true;
}

bool JSObject::defineOwnNonIndexProperty(ExecState* exec, PropertyName propertyName, const PropertyDescriptor& descriptor, bool throwException)
{
    VM& vm  = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    // Track on the globaldata that we're in define property.
    // Currently DefineOwnProperty uses delete to remove properties when they are being replaced
    // (particularly when changing attributes), however delete won't allow non-configurable (i.e.
    // DontDelete) properties to be deleted. For now, we can use this flag to make this work.
    VM::DeletePropertyModeScope scope(vm, VM::DeletePropertyMode::IgnoreConfigurable);
    PropertyDescriptor current;
    bool isCurrentDefined = getOwnPropertyDescriptor(exec, propertyName, current);
    bool isExtensible = this->isExtensible(exec);
    RETURN_IF_EXCEPTION(throwScope, false);
    return validateAndApplyPropertyDescriptor(exec, this, propertyName, isExtensible, descriptor, isCurrentDefined, current, throwException);
}

bool JSObject::defineOwnProperty(JSObject* object, ExecState* exec, PropertyName propertyName, const PropertyDescriptor& descriptor, bool throwException)
{
    // If it's an array index, then use the indexed property storage.
    if (std::optional<uint32_t> index = parseIndex(propertyName)) {
        // c. Let succeeded be the result of calling the default [[DefineOwnProperty]] internal method (8.12.9) on A passing P, Desc, and false as arguments.
        // d. Reject if succeeded is false.
        // e. If index >= oldLen
        // e.i. Set oldLenDesc.[[Value]] to index + 1.
        // e.ii. Call the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", oldLenDesc, and false as arguments. This call will always return true.
        // f. Return true.
        return object->defineOwnIndexedProperty(exec, index.value(), descriptor, throwException);
    }
    
    return object->defineOwnNonIndexProperty(exec, propertyName, descriptor, throwException);
}

void JSObject::convertToDictionary(VM& vm)
{
    DeferredStructureTransitionWatchpointFire deferredWatchpointFire;
    setStructure(
        vm, Structure::toCacheableDictionaryTransition(vm, structure(vm), &deferredWatchpointFire));
}

void JSObject::shiftButterflyAfterFlattening(const GCSafeConcurrentJSLocker&, VM& vm, Structure* structure, size_t outOfLineCapacityAfter)
{
    // This could interleave visitChildren because some old structure could have been a non
    // dictionary structure. We have to be crazy careful. But, we are guaranteed to be holding
    // the structure's lock right now, and that helps a bit.
    
    Butterfly* oldButterfly = this->butterfly();
    size_t preCapacity;
    size_t indexingPayloadSizeInBytes;
    bool hasIndexingHeader = this->hasIndexingHeader();
    if (UNLIKELY(hasIndexingHeader)) {
        preCapacity = oldButterfly->indexingHeader()->preCapacity(structure);
        indexingPayloadSizeInBytes = oldButterfly->indexingHeader()->indexingPayloadSizeInBytes(structure);
    } else {
        preCapacity = 0;
        indexingPayloadSizeInBytes = 0;
    }

    Butterfly* newButterfly = Butterfly::createUninitialized(vm, this, preCapacity, outOfLineCapacityAfter, hasIndexingHeader, indexingPayloadSizeInBytes);

    // No need to copy the precapacity.
    void* currentBase = oldButterfly->base(0, outOfLineCapacityAfter);
    void* newBase = newButterfly->base(0, outOfLineCapacityAfter);

    memcpy(newBase, currentBase, Butterfly::totalSize(0, outOfLineCapacityAfter, hasIndexingHeader, indexingPayloadSizeInBytes));
    
    setButterfly(vm, newButterfly);
}

uint32_t JSObject::getEnumerableLength(ExecState* exec, JSObject* object)
{
    VM& vm = exec->vm();
    Structure* structure = object->structure(vm);
    if (structure->holesMustForwardToPrototype(vm))
        return 0;
    switch (object->indexingType()) {
    case ALL_BLANK_INDEXING_TYPES:
    case ALL_UNDECIDED_INDEXING_TYPES:
        return 0;
        
    case ALL_INT32_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES: {
        Butterfly* butterfly = object->butterfly();
        unsigned usedLength = butterfly->publicLength();
        for (unsigned i = 0; i < usedLength; ++i) {
            if (!butterfly->contiguous()[i])
                return 0;
        }
        return usedLength;
    }
        
    case ALL_DOUBLE_INDEXING_TYPES: {
        Butterfly* butterfly = object->butterfly();
        unsigned usedLength = butterfly->publicLength();
        for (unsigned i = 0; i < usedLength; ++i) {
            double value = butterfly->contiguousDouble()[i];
            if (value != value)
                return 0;
        }
        return usedLength;
    }
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = object->m_butterfly.get()->arrayStorage();
        if (storage->m_sparseMap.get())
            return 0;
        
        unsigned usedVectorLength = std::min(storage->length(), storage->vectorLength());
        for (unsigned i = 0; i < usedVectorLength; ++i) {
            if (!storage->m_vector[i])
                return 0;
        }
        return usedVectorLength;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

void JSObject::getStructurePropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    VM& vm = exec->vm();
    object->structure(vm)->getPropertyNamesFromStructure(vm, propertyNames, mode);
}

void JSObject::getGenericPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, propertyNames, EnumerationMode(mode, JSObjectPropertiesMode::Exclude));
    RETURN_IF_EXCEPTION(scope, void());

    JSValue nextProto = object->getPrototype(vm, exec);
    RETURN_IF_EXCEPTION(scope, void());
    if (nextProto.isNull())
        return;

    JSObject* prototype = asObject(nextProto);
    while (true) {
        if (prototype->structure(vm)->typeInfo().overridesGetPropertyNames()) {
            prototype->methodTable(vm)->getPropertyNames(prototype, exec, propertyNames, mode);
            break;
        }
        prototype->methodTable(vm)->getOwnPropertyNames(prototype, exec, propertyNames, mode);
        RETURN_IF_EXCEPTION(scope, void());
        nextProto = prototype->getPrototype(vm, exec);
        RETURN_IF_EXCEPTION(scope, void());
        if (nextProto.isNull())
            break;
        prototype = asObject(nextProto);
    }
}

// Implements GetMethod(O, P) in section 7.3.9 of the spec.
// http://www.ecma-international.org/ecma-262/6.0/index.html#sec-getmethod
JSValue JSObject::getMethod(ExecState* exec, CallData& callData, CallType& callType, const Identifier& ident, const String& errorMessage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue method = get(exec, ident);
    RETURN_IF_EXCEPTION(scope, JSValue());

    if (!method.isCell()) {
        if (method.isUndefinedOrNull())
            return jsUndefined();

        throwVMTypeError(exec, scope, errorMessage);
        return jsUndefined();
    }

    callType = method.asCell()->methodTable()->getCallData(method.asCell(), callData);
    if (callType == CallType::None) {
        throwVMTypeError(exec, scope, errorMessage);
        return jsUndefined();
    }

    return method;
}

} // namespace JSC
