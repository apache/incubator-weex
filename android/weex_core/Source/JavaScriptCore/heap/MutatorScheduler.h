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

