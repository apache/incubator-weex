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

namespace JSC {

// Are you tired of waiting for all of WebKit to build because you changed the implementation of a
// function in HeapInlines.h?  Does it bother you that you're waiting on rebuilding the JS DOM
// bindings even though your change is in a function called from only 2 .cpp files?  Then HeapUtil.h
// is for you!  Everything in this class should be a static method that takes a Heap& if needed.
// This is a friend of Heap, so you can access all of Heap's privates.
//
// This ends up being an issue because Heap exposes a lot of methods that ought to be inline for
// performance or that must be inline because they are templates.  This class ought to contain
// methods that are used for the implementation of the collector, or for unusual clients that need
// to reach deep into the collector for some reason.  Don't put things in here that would cause you
// to have to include it from more than a handful of places, since that would defeat the purpose.
// This class isn't here to look pretty.  It's to let us hack the GC more easily!

class HeapUtil {
public:
    // This function must be run after stopAllocation() is called and 
    // before liveness data is cleared to be accurate.
    template<typename Func>
    static void findGCObjectPointersForMarking(
        Heap& heap, HeapVersion markingVersion, TinyBloomFilter filter, void* passedPointer,
        const Func& func)
    {
        const HashSet<MarkedBlock*>& set = heap.objectSpace().blocks().set();
        
        ASSERT(heap.objectSpace().isMarking());
        static const bool isMarking = true;
        
        char* pointer = static_cast<char*>(passedPointer);
        
        // It could point to a large allocation.
        if (heap.objectSpace().largeAllocationsForThisCollectionSize()) {
            if (heap.objectSpace().largeAllocationsForThisCollectionBegin()[0]->aboveLowerBound(pointer)
                && heap.objectSpace().largeAllocationsForThisCollectionEnd()[-1]->belowUpperBound(pointer)) {
                LargeAllocation** result = approximateBinarySearch<LargeAllocation*>(
                    heap.objectSpace().largeAllocationsForThisCollectionBegin(),
                    heap.objectSpace().largeAllocationsForThisCollectionSize(),
                    LargeAllocation::fromCell(pointer),
                    [] (LargeAllocation** ptr) -> LargeAllocation* { return *ptr; });
                if (result) {
                    if (result > heap.objectSpace().largeAllocationsForThisCollectionBegin()
                        && result[-1]->contains(pointer))
                        func(result[-1]->cell());
                    if (result[0]->contains(pointer))
                        func(result[0]->cell());
                    if (result + 1 < heap.objectSpace().largeAllocationsForThisCollectionEnd()
                        && result[1]->contains(pointer))
                        func(result[1]->cell());
                }
            }
        }
    
        MarkedBlock* candidate = MarkedBlock::blockFor(pointer);
        // It's possible for a butterfly pointer to point past the end of a butterfly. Check this now.
        if (pointer <= bitwise_cast<char*>(candidate) + sizeof(IndexingHeader)) {
            // We may be interested in the last cell of the previous MarkedBlock.
            char* previousPointer = pointer - sizeof(IndexingHeader) - 1;
            MarkedBlock* previousCandidate = MarkedBlock::blockFor(previousPointer);
            if (!filter.ruleOut(bitwise_cast<Bits>(previousCandidate))
                && set.contains(previousCandidate)
                && previousCandidate->handle().cellKind() == HeapCell::Auxiliary) {
                previousPointer = static_cast<char*>(previousCandidate->handle().cellAlign(previousPointer));
                if (previousCandidate->handle().isLiveCell(markingVersion, isMarking, previousPointer))
                    func(previousPointer);
            }
        }
    
        if (filter.ruleOut(bitwise_cast<Bits>(candidate))) {
            ASSERT(!candidate || !set.contains(candidate));
            return;
        }
    
        if (!set.contains(candidate))
            return;
        
        auto tryPointer = [&] (void* pointer) {
            if (candidate->handle().isLiveCell(markingVersion, isMarking, pointer))
                func(pointer);
        };
    
        if (candidate->handle().cellKind() == HeapCell::JSCell) {
            if (!MarkedBlock::isAtomAligned(pointer))
                return;
        
            tryPointer(pointer);
            return;
        }
    
        // A butterfly could point into the middle of an object.
        char* alignedPointer = static_cast<char*>(candidate->handle().cellAlign(pointer));
        tryPointer(alignedPointer);
    
        // Also, a butterfly could point at the end of an object plus sizeof(IndexingHeader). In that
        // case, this is pointing to the object to the right of the one we should be marking.
        if (candidate->atomNumber(alignedPointer) > MarkedBlock::firstAtom()
            && pointer <= alignedPointer + sizeof(IndexingHeader))
            tryPointer(alignedPointer - candidate->cellSize());
    }
    
    static bool isPointerGCObjectJSCell(
        Heap& heap, TinyBloomFilter filter, const void* pointer)
    {
        // It could point to a large allocation.
        const Vector<LargeAllocation*>& largeAllocations = heap.objectSpace().largeAllocations();
        if (!largeAllocations.isEmpty()) {
            if (largeAllocations[0]->aboveLowerBound(pointer)
                && largeAllocations.last()->belowUpperBound(pointer)) {
                LargeAllocation*const* result = approximateBinarySearch<LargeAllocation*const>(
                    largeAllocations.begin(), largeAllocations.size(),
                    LargeAllocation::fromCell(pointer),
                    [] (LargeAllocation*const* ptr) -> LargeAllocation* { return *ptr; });
                if (result) {
                    if (result > largeAllocations.begin()
                        && result[-1]->cell() == pointer
                        && result[-1]->attributes().cellKind == HeapCell::JSCell)
                        return true;
                    if (result[0]->cell() == pointer
                        && result[0]->attributes().cellKind == HeapCell::JSCell)
                        return true;
                    if (result + 1 < largeAllocations.end()
                        && result[1]->cell() == pointer
                        && result[1]->attributes().cellKind == HeapCell::JSCell)
                        return true;
                }
            }
        }
    
        const HashSet<MarkedBlock*>& set = heap.objectSpace().blocks().set();
        
        MarkedBlock* candidate = MarkedBlock::blockFor(pointer);
        if (filter.ruleOut(bitwise_cast<Bits>(candidate))) {
            ASSERT(!candidate || !set.contains(candidate));
            return false;
        }
        
        if (!MarkedBlock::isAtomAligned(pointer))
            return false;
        
        if (!set.contains(candidate))
            return false;
        
        if (candidate->handle().cellKind() != HeapCell::JSCell)
            return false;
        
        if (!candidate->handle().isLiveCell(pointer))
            return false;
        
        return true;
    }
    
    static bool isValueGCObject(
        Heap& heap, TinyBloomFilter filter, JSValue value)
    {
        if (!value.isCell())
            return false;
        return isPointerGCObjectJSCell(heap, filter, static_cast<void*>(value.asCell()));
    }
};

} // namespace JSC

