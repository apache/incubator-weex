/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "GCLogging.h"

#include "ClassInfo.h"
#include "Heap.h"
#include "HeapIterationScope.h"
#include "JSCell.h"
#include "JSCellInlines.h"
#include <wtf/PrintStream.h>

namespace JSC {

const char* GCLogging::levelAsString(Level level)
{
    switch (level) {
    case None:
        return "None";
    case Basic:
        return "Basic";
    case Verbose:
        return "Verbose";
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return "";
    }
}

} // namespace JSC

namespace WTF {

void printInternal(PrintStream& out, JSC::GCLogging::Level level)
{
    switch (level) {
    case JSC::GCLogging::Level::None:
        out.print("None");
        return;
    case JSC::GCLogging::Level::Basic:
        out.print("Basic");
        return;
    case JSC::GCLogging::Level::Verbose:
        out.print("Verbose");
        return;
    default:
        out.print("Level=", level - JSC::GCLogging::Level::None);
        return;
    }
}

} // namespace WTF

