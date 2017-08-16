/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGCommon.h"

#include "DFGNode.h"
#include "JSCInlines.h"
#include <wtf/PrintStream.h>

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

static StaticLock crashLock;

void startCrashing()
{
    crashLock.lock();
}

bool isCrashing()
{
    return crashLock.isLocked();
}

bool stringLessThan(StringImpl& a, StringImpl& b)
{
    unsigned minLength = std::min(a.length(), b.length());
    for (unsigned i = 0; i < minLength; ++i) {
        if (a[i] == b[i])
            continue;
        return a[i] < b[i];
    }
    return a.length() < b.length();
}

} } // namespace JSC::DFG

namespace WTF {

using namespace JSC::DFG;

void printInternal(PrintStream& out, OptimizationFixpointState state)
{
    switch (state) {
    case BeforeFixpoint:
        out.print("BeforeFixpoint");
        return;
    case FixpointNotConverged:
        out.print("FixpointNotConverged");
        return;
    case FixpointConverged:
        out.print("FixpointConverged");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, GraphForm form)
{
    switch (form) {
    case LoadStore:
        out.print("LoadStore");
        return;
    case ThreadedCPS:
        out.print("ThreadedCPS");
        return;
    case SSA:
        out.print("SSA");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, UnificationState state)
{
    switch (state) {
    case LocallyUnified:
        out.print("LocallyUnified");
        return;
    case GloballyUnified:
        out.print("GloballyUnified");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, RefCountState state)
{
    switch (state) {
    case EverythingIsLive:
        out.print("EverythingIsLive");
        return;
    case ExactRefCount:
        out.print("ExactRefCount");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, ProofStatus status)
{
    switch (status) {
    case IsProved:
        out.print("IsProved");
        return;
    case NeedsCheck:
        out.print("NeedsCheck");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

namespace WTF {

using namespace JSC::DFG;

void printInternal(PrintStream& out, CapabilityLevel capabilityLevel)
{
    switch (capabilityLevel) {
    case CannotCompile:
        out.print("CannotCompile");
        return;
    case CanCompile:
        out.print("CanCompile");
        return;
    case CanCompileAndInline:
        out.print("CanCompileAndInline");
        return;
    case CapabilityLevelNotSet:
        out.print("CapabilityLevelNotSet");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

