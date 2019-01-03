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

#include <wtf/FastMalloc.h>
#include <wtf/MonotonicTime.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class MutatorScheduler {
    WTF_MAKE_NONCOPYABLE(MutatorScheduler);
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum State {
        Normal, // Not collecting.
        Stopped,
        Resumed
    };
    
    MutatorScheduler();
    virtual ~MutatorScheduler();
    
    virtual State state() const = 0;
    
    virtual void beginCollection() = 0;
    
    virtual void didStop();
    virtual void willResume();
    
    // At the top of an iteration, the GC will may call didReachTermination.
    virtual void didReachTermination();
    
    // If it called didReachTermination, it will then later call didExecuteConstraints.
    virtual void didExecuteConstraints();
    
    // After doing that, it will do synchronous draining. When this stalls - either due to timeout or
    // just 'cause, it will call this.
    virtual void synchronousDrainingDidStall();
    
    virtual MonotonicTime timeToStop() = 0; // Call while resumed, to ask when to stop.
    virtual MonotonicTime timeToResume() = 0; // Call while stopped, to ask when to resume.
    
    virtual void log();
    
    bool shouldStop(); // Call while resumed, to ask if we should stop now.
    bool shouldResume(); // Call while stopped, to ask if we should resume now.
    
    virtual void endCollection() = 0;
};

} // namespace JSC

