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
#include "TrackedReferences.h"

#include "JSCInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

TrackedReferences::TrackedReferences()
{
}

TrackedReferences::~TrackedReferences()
{
}

void TrackedReferences::add(JSCell* cell)
{
    if (cell)
        m_references.add(cell);
}

void TrackedReferences::add(JSValue value)
{
    if (value.isCell())
        add(value.asCell());
}

void TrackedReferences::check(JSCell* cell) const
{
    if (!cell)
        return;
    
    if (m_references.contains(cell))
        return;
    
    dataLog("Found untracked reference: ", JSValue(cell), "\n");
    dataLog("All tracked references: ", *this, "\n");
    RELEASE_ASSERT_NOT_REACHED();
}

void TrackedReferences::check(JSValue value) const
{
    if (value.isCell())
        check(value.asCell());
}

void TrackedReferences::dump(PrintStream& out) const
{
    CommaPrinter comma;
    for (JSCell* cell : m_references)
        out.print(comma, RawPointer(cell));
}

} // namespace JSC

