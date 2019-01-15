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

// We track collector phase in order to allow either the collector thread or the mutator thread to
// jump in and do work. The collector and mutator trade the conn
// (https://en.wikipedia.org/wiki/Conn_(nautical)) with each other based on who is available to do work,
// and they use the phase to help each other know what to do once they take the conn.
//
// The collector relinquishes the conn whenever it stopTheMutator's and the mutator is running. Then the
// collector thread goes to sleep.
//
// The mutator relinquishes the conn whenever it releaseAccess's. That wakes up the collector thread.
enum class CollectorPhase : uint8_t {
    // We use this phase when the collector is not running at all. After this state is Begin.
    NotRunning,
    
    // This is everything from when the collector begins to when it first yields to the mutator for
    // marking. After this is Fixpoint.
    Begin,
    
    // This means that we should try to do some progress with the world stopped. This usually means
    // doing an iteration of MarkingConstraintSet::executeConvergence, but it could also mean marking
    // with the world stopped. After this is either Concurrent or End.
    Fixpoint,
    
    // In this state the collector is relying on the parallel helpers and incremental mutator work to
    // make progress. After this is Reloop, once marking stalls.
    Concurrent,
        
    // We did some concurrent marking and now we ran out of work. This phase prepares the GC for another
    // Fixpoint. After this is Fixpoint.
    Reloop,
    
    // The collector is trying to finish up. After this state is NotRunning.
    End
};

bool worldShouldBeSuspended(CollectorPhase phase);

} // namespace JSC

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::CollectorPhase);

} // namespace WTF

